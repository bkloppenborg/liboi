/*
 * CLibOI.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "CLibOI.h"
#include <cstdio>

#include "CRoutine_Sum.h"
#include "CRoutine_Normalize.h"
#include "CRoutine_ImageToBuffer.h"
#include "CRoutine_FT.h"
#include "CRoutine_DFT.h"
#include "CRoutine_FTtoV2.h"
#include "CRoutine_FTtoT3.h"
#include "CRoutine_Chi.h"
#include "CRoutine_LogLike.h"
#include "CRoutine_Square.h"

CLibOI::CLibOI(cl_device_type type)
{
	// init datamembers
	mOCL = new COpenCL(type);

	mImageHeight = 1;
	mImageWidth = 1;
	mImageDepth = 1;
	mCLImage = NULL;
	mGLImage = NULL;
	mFluxBuffer = NULL;
	mImageType = LibOIEnums::OpenCLBuffer;	// By default we assume the image is stored in an OpenCL buffer.
	mImageScale = 1;
	mMaxData = 0;
	mMaxUV = 0;

	// Temporary buffers:
	mFluxBuffer = NULL;
	mFTBuffer = NULL;
	mSimDataBuffer = NULL;

	// Routines
	mDataRoutinesInitialized = false;
	mrTotalFlux = NULL;
	mrCopyImage = NULL;
	mrNormalize = NULL;
	mrFT = NULL;
	mrV2 = NULL;
	mrT3 = NULL;
	mrChi = NULL;
	mrLogLike = NULL;
	mrSquare = NULL;
}

CLibOI::~CLibOI()
{
	// First free datamembers:
	delete mrTotalFlux;
	delete mrCopyImage;
	delete mrNormalize;
	delete mrFT;
	delete mrV2;
	delete mrT3;
	delete mrChi;
	delete mrLogLike;
	delete mrSquare;

	// Now free OpenCL buffers:
	if(mFluxBuffer) clReleaseMemObject(mFluxBuffer);
	if(mFTBuffer) clReleaseMemObject(mFTBuffer);
	if(mSimDataBuffer) clReleaseMemObject(mSimDataBuffer);
	if(mGLImage) clReleaseMemObject(mGLImage);

	delete mOCL;
}

/// Copies the specified layer from the registered image buffer over to an OpenCL memory buffer.
/// If the image is already in an OpenCL buffer, this function need not be called.
void CLibOI::CopyImageToBuffer(int layer)
{
	int err = CL_SUCCESS;

	if(mImageType == LibOIEnums::OpenGLBuffer)
	{
		// Wait for the OpenGL queue to finish.
		glFinish();
		err |= clEnqueueAcquireGLObjects(mOCL->GetQueue(), 1, &mGLImage, 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not acquire OpenGL Memory object.", err);

		// TODO: Implement depth channel for 3D images
		CopyImageToBuffer(mGLImage, mCLImage, mImageWidth, mImageHeight, layer);
		COpenCL::CheckOCLError("Could not copy OpenGL image to the OpenCL Memory buffer", err);
		clFinish(mOCL->GetQueue());
		err |= clEnqueueReleaseGLObjects (mOCL->GetQueue(), 1, &mGLImage, 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not Release OpenGL Memory object.", err);
	}
}

/// Copies gl_image, an OpenGL image in of the internal format GL_R, to a floating point image buffer, cl_image.
void CLibOI::CopyImageToBuffer(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int layer)
{
	mrCopyImage->CopyImage(gl_image, cl_buffer, width, height, layer);
}

/// Computes the chi2 between the current simulated data, and the observed data set specified in data
float CLibOI::DataToChi2(COILibData * data)
{
	return mrChi->Chi2(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, data->GetNumData(), mrSquare, true, true);
}

float CLibOI::DataToLogLike(COILibData * data)
{
	return mrLogLike->LogLike(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, data->GetNumData(), true, true);
}

/// Copies the current image in mCLImage to the floating point buffer, image, iff the sizes match exactly.
void CLibOI::ExportImage(float * image, unsigned int width, unsigned int height, unsigned int depth)
{
	if(width != mImageWidth || height != mImageHeight || depth != mImageDepth)
		return;

	int err = CL_SUCCESS;
	int num_elements = mImageWidth * mImageHeight * mImageDepth;
	cl_float tmp[num_elements];
	err |= clEnqueueReadBuffer(mOCL->GetQueue(), mCLImage, CL_TRUE, 0, num_elements * sizeof(cl_float), tmp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy buffer back to CPU, CLibOI::ExportImage() ", err);

	// Copy to the output buffer, converting as we go.
	for(unsigned int i = 0; i < num_elements; i++)
		image[i] = tmp[i];

}

/// Computes the Fourier transform of the image, then generates Vis2 and T3's.
/// This routine assumes the image has been normalized using Normalize() (and that the total flux is stored in mFluxBuffer)
void CLibOI::FTToData(COILibData * data)
{
	// First compute the Fourier transform
	mrFT->FT(data->GetLoc_DataUVPoints(), data->GetNumUV(), mCLImage, mImageWidth, mImageHeight, mFluxBuffer, mFTBuffer);

	// Now create the V2 and T3's
	mrV2->FTtoV2(mFTBuffer, data->GetNumV2(), mSimDataBuffer);
	mrT3->FTtoT3(mFTBuffer, data->GetLoc_DataT3Phi(), data->GetLoc_DataBSRef(),
			data->GetLoc_DataT3Sign(), data->GetNumT3(), data->GetNumV2(), mSimDataBuffer);
}

/// Uses the current active image to compute the chi (i.e. non-squared version) with respect to the
/// specified data and returns the chi array in output.
/// This is a convenience function that calls FTToData, DataToChi
void CLibOI::ImageToChi(COILibData * data, float * output, int & n)
{
	// Simple, call the other functions
	Normalize();
	FTToData(data);

	n = data->GetNumData();
	mrChi->GetChi(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, n, output);
}

/// Same as ImageToChi above.
/// Returns false if the data number does not exist, true otherwise.
bool CLibOI::ImageToChi(int data_num, float * output, int & n)
{
	if(data_num > mDataList.size() - 1)
		return false;

	COILibData * data = mDataList[data_num];
	ImageToChi(data, output, n);
	return true;
}

/// Uses the current active image to compute the chi2 with respect to the specified data.
/// This is a convenience function that calls FTToData and DataToChi2.
float CLibOI::ImageToChi2(COILibData * data)
{
	// Simple, call the other functions
	Normalize();
	FTToData(data);
	float chi2 = DataToChi2(data);
	return chi2;
}

/// Same as ImageToChi2 above
float CLibOI::ImageToChi2(int data_num)
{
	if(data_num > mDataList.size() - 1)
		return -1;

	COILibData * data = mDataList[data_num];
	return ImageToChi2(data);
}

float CLibOI::ImageToLogLike(COILibData * data)
{
	// Simple, call the other functions
	Normalize();
	FTToData(data);
	float llike = DataToLogLike(data);
	return llike;
}
float CLibOI::ImageToLogLike(int data_num)
{
	if(data_num > mDataList.size() - 1)
		return -1;

	COILibData * data = mDataList[data_num];
	return ImageToLogLike(data);
}

void CLibOI::Init()
{
	InitMemory();
	InitRoutines();
}

/// Initializes memory used for storing various things on the OpenCL context.
void CLibOI::InitMemory()
{
	int err = CL_SUCCESS;

	if(mImageType == LibOIEnums::OpenGLBuffer && mCLImage == NULL)
	{
		mCLImage = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, mImageWidth * mImageHeight * sizeof(cl_float), NULL, &err);
		COpenCL::CheckOCLError("Could not create temporary OpenCL image buffer", err);
	}

	// Determine the maximum data size, cache it locally.
	mMaxData = mDataList.MaxNumData();
	mMaxUV = mDataList.MaxUVPoints();

	// Allocate some memory on the OpenCL device
	if(mFluxBuffer == NULL)
		mFluxBuffer = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	if(mMaxData > 0)
	{
		mFTBuffer = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * mMaxUV, NULL, &err);
		mSimDataBuffer = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * mMaxData, NULL, &err);
	}
	COpenCL::CheckOCLError("Could not initialize liboi required memory objects.", err);
}

void CLibOI::InitRoutines()
{
	// Init all routines.  For now pre-allocate all buffers.
	// Remember, Init can be called multiple times, so only init if not inited already.
	if(mrTotalFlux == NULL)
	{
		mrTotalFlux = new CRoutine_Sum(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
		mrTotalFlux->SetSourcePath(mKernelSourcePath);
		mrTotalFlux->Init(mImageWidth * mImageHeight);
	}

	if(mrCopyImage == NULL)
	{
		mrCopyImage = new CRoutine_ImageToBuffer(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
		mrCopyImage->SetSourcePath(mKernelSourcePath);
		mrCopyImage->Init();
	}

	if(mrNormalize == NULL)
	{
		mrNormalize = new CRoutine_Normalize(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
		mrNormalize->SetSourcePath(mKernelSourcePath);
		mrNormalize->Init();
	}

	if(mrSquare == NULL)
	{
		mrSquare = new CRoutine_Square(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
		mrSquare->SetSourcePath(mKernelSourcePath);
		mrSquare->Init();
	}

	// only initialize these routines if we have data:
	if(mMaxData > 0)
	{
		mDataRoutinesInitialized = true;
		if(mrFT == NULL)
		{
			// TODO: Permit the Fourier Transform routine to be switched from DFT to something else, like NFFT
			mrFT = new CRoutine_DFT(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
			mrFT->SetSourcePath(mKernelSourcePath);
			mrFT->Init(mImageScale);
		}

		if(mrV2 == NULL)
		{
			// Initialize the FTtoV2 and FTtoT3 routines
			mrV2 = new CRoutine_FTtoV2(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
			mrV2->SetSourcePath(mKernelSourcePath);
			mrV2->Init();
		}

		if(mrT3 == NULL)
		{
			mrT3 = new CRoutine_FTtoT3(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
			mrT3->SetSourcePath(mKernelSourcePath);
			mrT3->Init();
		}

		if(mrChi == NULL)
		{
			mrChi = new CRoutine_Chi(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
			mrChi->SetSourcePath(mKernelSourcePath);
			mrChi->Init(mMaxData);
		}

		if(mrLogLike == NULL)
		{
			mrLogLike = new CRoutine_LogLike(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
			mrLogLike->SetSourcePath(mKernelSourcePath);
			mrLogLike->Init(mMaxData);
		}
	}
}

/// Reads in an OIFITS file and stores it into OpenCL memory
/// Note, this routine will not load data any routine that uses data is initialized.
void CLibOI::LoadData(string filename)
{
	if(!mDataRoutinesInitialized)
	{
		mDataList.ReadFile(filename);
		mDataList[mDataList.size() - 1]->CopyToOpenCLDevice(mOCL->GetContext(), mOCL->GetQueue());
	}
}

/// Normalizes a floating point buffer by dividing by the sum of the buffer
void CLibOI::Normalize()
{
	// Temporary variables
#ifdef DEBUG_VERBOSE
	float tmp1, tmp2;
#endif // DEBUG

	// First compute and store the total flux:
#ifdef DEBUG_VERBOSE
	tmp1 = TotalFlux(true);
#else // DEBUG
	TotalFlux(false);
#endif // DEBUG

	// Now normalize the image
	mrNormalize->Normalize(mCLImage, mImageWidth, mImageHeight, mFluxBuffer);

#ifdef DEBUG_VERBOSE
	// If we are debugging, do another call to ensure the buffer was indeed normalized.
	// Note we call the mrTotalFlux routine directly because TotalFlux copies the image over from the OpenGL buffer.
	tmp2 = mrTotalFlux->ComputeSum(true, mFluxBuffer, mCLImage, NULL, NULL);
	printf("Pre/post normalization image sums: %f %f\n", tmp1, tmp2);

#endif //DEBUG
}

/// Computes the total flux for the current image/layer
/// If the image is 2D, use zero for the layer.
float CLibOI::TotalFlux(bool return_value)
{
	//CopyImageToBuffer(layer);

	float flux = mrTotalFlux->ComputeSum(mCLImage, mFluxBuffer, true);
	return flux;
}

/// Removes the specified data set from memory.
void CLibOI::RemoveData(int data_num)
{
	mDataList.RemoveData(data_num);
}

/// Runs the verification functions of each kernel.  Assumes all initialization has been complete
/// and at least one data set has been loaded.
void CLibOI::RunVerification(int data_num)
{
	if(data_num > mDataList.size() - 1)
		return;

	COILibData * data = mDataList[data_num];
	printf("Checking summed flux values:\n");
	mrTotalFlux->ComputeSum_Test(mCLImage, mFluxBuffer);
	mrNormalize->Normalize_Test(mCLImage, mImageWidth, mImageHeight, mFluxBuffer);
	mrFT->FT_Test(data->GetLoc_DataUVPoints(), data->GetNumUV(), mCLImage, mImageWidth,
			mImageHeight, mFluxBuffer, mFTBuffer);
	mrV2->FTtoV2_Test(mFTBuffer, data->GetNumV2(), mSimDataBuffer);
	mrT3->FTtoT3_Test(mFTBuffer, data->GetNumUV(), data->GetLoc_DataT3Phi(), data->GetLoc_DataBSRef(),
			data->GetLoc_DataT3Sign(), data->GetNumT3(), data->GetNumV2(), mSimDataBuffer);

	// Now run the chi, chi2, and loglike kernels:
	int n = data->GetNumData();
	mrChi->Chi_Test(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, n);
	mrChi->Chi2_Test(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, n, mrSquare, true);
	mrLogLike->LogLike_Test(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, n);
}

/// Saves the current image in the OpenCL memory buffer to the specified FITS file
/// If the OpenCL memory has not been initialzed, this function immediately returns
void   CLibOI::SaveImage(string filename)
{
	if(mCLImage == NULL)
		return;

	// TODO: Adapt for multi-spectral images
	Normalize();

	// Create storage space for the image, copy it.
	float image[mImageWidth * mImageHeight * mImageDepth];
	ExportImage(image, mImageWidth, mImageHeight, mImageDepth);

	// write out the FITS file:
	fitsfile *fptr;
	int error = 0;
	int* status = &error;
	long fpixel = 1, naxis = 2, nelements;
	long naxes[2];

	/*Initialise storage*/
	naxes[0] = (long) mImageWidth;
	naxes[1] = (long) mImageHeight;
	nelements = mImageWidth * mImageWidth;

	/*Create new file*/
	if (*status == 0)
		fits_create_file(&fptr, filename.c_str(), status);

	/*Create primary array image*/
	if (*status == 0)
		fits_create_img(fptr, FLOAT_IMG, naxis, naxes, status);
	/*Write a keywords (datafile, target, image pixelation) */
//	if (*status == 0)
//		fits_update_key(fptr, TSTRING, "DATAFILE", "FakeImage", "Data File Name", status);
//	if (*status == 0)
//		fits_update_key(fptr, TSTRING, "TARGET", "FakeImage", "Target Name", status);
//	if (*status == 0)
//		fits_update_key(fptr, TFLOAT, "SCALE", &scale, "Scale (mas/pixel)", status);


	/*Write image*/
	if (*status == 0)
		fits_write_img(fptr, TFLOAT, fpixel, nelements, &image[0], status);

	/*Close file*/
	if (*status == 0)
		fits_close_file(fptr, status);

	/*Report any errors*/
	fits_report_error(stderr, *status);
}

/// Tells OpenCL about the size of the image.
/// The image must have a depth of at least one.
void   CLibOI::SetImageInfo(int width, int height, int depth, float scale)
{
	// TODO: Check on the size of the image.

	mImageWidth = width;
	mImageHeight = height;
	mImageDepth = depth;
	mImageScale = scale;
}

/// Registers image as the current image object against which liboi operations will be undertaken.
void   CLibOI::SetImage_CLMEM(cl_mem image)
{
	mCLImage = image;
}

/// Creates an OpenCL memory object from the renderbuffer
/// Registers it as the currentt image object against which liboi operations will be undertaken.
void CLibOI::SetImage_GLFB(GLuint framebuffer)
{
	this->mImageType = LibOIEnums::OpenGLBuffer;
	int err = CL_SUCCESS;
	mGLImage = clCreateFromGLBuffer(mOCL->GetContext(), CL_MEM_READ_ONLY, framebuffer, &err);
	COpenCL::CheckOCLError("Could not create OpenCL image object from framebuffer", err);
}

/// Creates an OpenCL memory object from a texturebuffer
/// Registers it as the currentt image object against which liboi operations will be undertaken.
void CLibOI::SetImage_GLTB(GLuint texturebuffer)
{
	// TODO: Permit loading of 3D textures for spectral imaging.
	this->mImageType = LibOIEnums::OpenGLBuffer;
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
}

void CLibOI::SetKernelSourcePath(string path_to_kernels)
{
	mKernelSourcePath = path_to_kernels;
}
