/*
 * CLibOI.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "CLibOI.h"
#include <cstdio>

CLibOI::CLibOI()
{
	// init datamembers
	mOCL = new COpenCL();
	mCLImage = NULL;
	mGLImage = NULL;
	mFluxBuffer = NULL;
	mImageType = OpenCLBuffer;	// By default we assume the image is stored in an OpenCL buffer.
	mImageScale = 1;
	mMaxData = 0;
	mMaxUV = 0;

	// Temporary buffers:
	mFluxBuffer = NULL;
	mFTBuffer = NULL;
	mSimDataBuffer = NULL;
}

CLibOI::~CLibOI()
{
	FreeOpenCLMem();
	delete mOCL;
}

/// Copies gl_image, an OpenGL image in of the internal format GL_R, to a floating point image buffer, cl_image.
void CLibOI::CopyImageToBuffer(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int layer)
{
	mrCopyImage->CopyImage(gl_image, cl_buffer, width, height, layer);
}

/// Computes the chi2 between the current simulated data, and the observed data set specified in data
float CLibOI::DataToChi2(COILibData * data)
{
	return mrChi2->Chi2(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, data->GetNumData());
}

void CLibOI::FreeOpenCLMem()
{
	// First free datamembers:
	if(mrTotalFlux) delete mrTotalFlux;
	if(mrCopyImage) delete mrCopyImage;
	if(mrNormalize) delete mrNormalize;

	// Now free OpenCL buffers:
	if(mFluxBuffer) clReleaseMemObject(mFluxBuffer);
	if(mFTBuffer) clReleaseMemObject(mFTBuffer);
	if(mSimDataBuffer) clReleaseMemObject(mSimDataBuffer);
}

/// Computes the Fourier transform of the image, then generates Vis2 and T3's.
void CLibOI::FTToData(COILibData * data)
{
	// First compute the Fourier transform
	mrFT->FT(data->GetLoc_DataUVPoints(), data->GetNumUV(), mCLImage, mImageWidth, mImageHeight, mFTBuffer);

	// Now create the V2 and T3's
	mrV2->FTtoV2(mFTBuffer, data->GetNumV2(), mSimDataBuffer);
	mrT3->FTtoT3(mFTBuffer, data->GetLoc_DataT3Phi(), data->GetLoc_DataBSRef(),
			data->GetLoc_DataT3Sign(), data->GetNumT3(), data->GetNumV2(), mSimDataBuffer);
}

/// Uses the current active image to compute the chi2 with respect to the specified data.
/// This is a convenience function that calls FTToData and DataToChi2.
float CLibOI::ImageToChi2(COILibData * data)
{
	// Simple, call the other functions
	Normalize();
	FTToData(data);
	return DataToChi2(data);
}

float CLibOI::ImageToChi2(int data_num)
{
	COILibData * data = mDataList[data_num];
	return ImageToChi2(data);
}

void CLibOI::Init(cl_device_type device_type, int image_width, int image_height, int image_depth, float scale)
{
	// First register the width, height, and depth of the images we will be using.
	RegisterImageInfo(image_width, image_height, image_depth, scale);

	// Now initalize the OpenCL context.  The programmer should call InitMemory and InitRoutines
	// after loading any data into memory.
	mOCL->Init(device_type);
}

/// Initializes memory used for storing various things on the OpenCL context.
void CLibOI::InitMemory()
{
	int err = CL_SUCCESS;

	// Determine the maximum data size, cache it locally.
	mMaxData = mDataList.MaxNumData();
	mMaxUV = mDataList.MaxUVPoints();

	// Allocate some memory on the OpenCL device
	mFluxBuffer = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);
	mFTBuffer = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * mMaxUV, NULL, &err);
	mSimDataBuffer = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * mMaxData, NULL, &err);
	COpenCL::CheckOCLError("Could not initialize liboi required memory objects.", err);
}

void CLibOI::InitRoutines()
{
	// Init all routines.  For now pre-allocate all buffers.
	mrTotalFlux = new CRoutine_Reduce(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
	mrTotalFlux->SetSourcePath(mKernelSourcePath);
	mrTotalFlux->Init(mImageWidth * mImageHeight, true);

	mrCopyImage = new CRoutine_ImageToBuffer(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
	mrCopyImage->SetSourcePath(mKernelSourcePath);
	mrCopyImage->Init();

	mrNormalize = new CRoutine_Normalize(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
	mrNormalize->SetSourcePath(mKernelSourcePath);
	mrNormalize->Init();

	// TODO: Permit the Fourier Transform routine to be switched from DFT to something else, like NFFT
	mrFT = new CRoutine_DFT(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
	mrFT->SetSourcePath(mKernelSourcePath);
	mrFT->Init(mImageScale);

	// Initialize the FTtoV2 and FTtoT3 routines
	mrV2 = new CRoutine_FTtoV2(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
	mrV2->SetSourcePath(mKernelSourcePath);
	mrV2->Init(mImageScale);

	mrT3 = new CRoutine_FTtoT3(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
	mrT3->SetSourcePath(mKernelSourcePath);
	mrT3->Init();

	mrChi2 = new CRoutine_Chi2(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
	mrChi2->SetSourcePath(mKernelSourcePath);
	mrChi2->Init(mMaxData);
}

/// Reads in an OIFITS file and stores it into OpenCL memory
void CLibOI::LoadData(string filename)
{
	mDataList.ReadFile(filename);
	mDataList[mDataList.size() - 1]->CopyToOpenCLDevice(mOCL->GetContext(), mOCL->GetQueue());
}

/// Normalizes a floating point buffer by dividing by the sum of the buffer
void CLibOI::Normalize()
{
	// Temporary variables
#ifdef DEBUG
	float tmp1, tmp2;
#endif // DEBUG

	// First compute and store the total flux:
#ifdef DEBUG
	tmp1 = TotalFlux(true);
#else // DEBUG
	TotalFlux(false);
#endif // DEBUG

	// Now normalize the image
	mrNormalize->Normalize(mCLImage, mImageWidth, mImageHeight, mFluxBuffer);

#ifdef DEBUG
	// If we are debugging, do another call to ensure the buffer was indeed normalized.
	// Note we call the mrTotalFlux routine directly because TotalFlux copies the image over from the OpenGL buffer.
	tmp2 = mrTotalFlux->ComputeSum(true, mFluxBuffer, mCLImage, NULL, NULL);
	printf("Pre/post normalization image sums: %f %f\n", tmp1, tmp2);

#endif //DEBUG
}

/// Computes the total flux for the current image.
float CLibOI::TotalFlux(bool return_value)
{
	int err = CL_SUCCESS;

	if(mImageType == OpenGLBuffer)
	{
		// Wait for the OpenGL queue to finish.
		glFinish();
		err |= clEnqueueAcquireGLObjects(mOCL->GetQueue(), 1, &mGLImage, 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not acquire OpenGL Memory object.", err);

		// TODO: Implement depth channel for 3D images
		CopyImageToBuffer(mGLImage, mCLImage, mImageWidth, mImageHeight, 0);
		COpenCL::CheckOCLError("Could not copy OpenGL image to the OpenCL Memory buffer", err);
	}

	float flux = mrTotalFlux->ComputeSum(return_value, mFluxBuffer, mCLImage, NULL, NULL);

	if(mImageType == OpenGLBuffer)
	{
		clFinish(mOCL->GetQueue());
		err |= clEnqueueReleaseGLObjects (mOCL->GetQueue(), 1, &mGLImage, 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not Release OpenGL Memory object.", err);
	}

	return flux;
}

/// Tells OpenCL about the size of the image.
/// The image must have a depth of at least one.
void   CLibOI::RegisterImageInfo(int width, int height, int depth, float scale)
{
	// TODO: Check on the size of the image.

	mImageWidth = width;
	mImageHeight = height;
	mImageDepth = depth;
	mImageScale = scale;
}

/// Registers image as the current image object against which liboi operations will be undertaken.
void   CLibOI::RegisterImage_CLMEM(cl_mem image)
{
	mCLImage = image;
}

/// Creates an OpenCL memory object from the renderbuffer
/// Registers it as the currentt image object against which liboi operations will be undertaken.
void CLibOI::RegisterImage_GLFB(GLuint framebuffer)
{
	this->mImageType = OpenGLBuffer;
	int err = CL_SUCCESS;
	mGLImage = clCreateFromGLBuffer(mOCL->GetContext(), CL_MEM_READ_ONLY, framebuffer, &err);
	COpenCL::CheckOCLError("Could not create OpenCL image object from framebuffer", err);

	mCLImage = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, mImageWidth * mImageHeight * sizeof(cl_float), NULL, &err);
	COpenCL::CheckOCLError("Could not create temporary OpenCL image buffer", err);
}

/// Creates an OpenCL memory object from a texturebuffer
/// Registers it as the currentt image object against which liboi operations will be undertaken.
void CLibOI::RegisterImage_GLTB(GLuint texturebuffer)
{
	// TODO: Permit loading of 3D textures for spectral imaging.
	this->mImageType = OpenGLBuffer;
	int err = CL_SUCCESS;

	// TODO: note that the clCreateFromGLTexture2D was depreciated in the OpenCL 1.2 specifications.
	mGLImage = clCreateFromGLTexture2D(mOCL->GetContext(), CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, texturebuffer, &err);
	COpenCL::CheckOCLError("Could not create OpenCL image object from GLTexture", err);

//#ifdef DEBUG
//
//	cl_image_format image_format;
//	size_t param_value_size;
//
//	clGetImageInfo(mGLImage, CL_IMAGE_FORMAT, param_value_size, (void*) &image_format, NULL);
//	//printf("Image format: %f", image_format);
//
//#endif //DEBUG
	mCLImage = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, mImageWidth * mImageHeight * sizeof(cl_float), NULL, &err);
	COpenCL::CheckOCLError("Could not create temporary OpenCL image buffer", err);
}

void CLibOI::SetKernelSourcePath(string path_to_kernels)
{
	mKernelSourcePath = path_to_kernels;
}
