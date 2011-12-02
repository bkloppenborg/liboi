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

void CLibOI::FreeOpenCLMem()
{
	// First free datamembers:
	if(mrTotalFlux) delete mrTotalFlux;
	if(mrCopyImage) delete mrCopyImage;
	if(mrNormalize) delete mrNormalize;

	// Now free OpenCL buffers:
	if(mFluxBuffer) clReleaseMemObject(mFluxBuffer);
}

void CLibOI::Init(cl_device_type device_type, int image_width, int image_height, int image_depth)
{
	// First register the width, height, and depth of the images we will be using.
	RegisterImageSize(image_width, image_height, image_depth);

	// Now initalize the OpenCL context and all required routines.
	mOCL->Init(device_type);
	InitMemory();
	InitRoutines();
}

/// Initializes memory used for storing various things on the OpenCL context.
void CLibOI::InitMemory()
{
	int err = CL_SUCCESS;
	// Allocate some memory on the OpenCL device
	mFluxBuffer = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

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
void   CLibOI::RegisterImageSize(int width, int height, int depth)
{
	// TODO: Check on the size of the image.

	mImageWidth = width;
	mImageHeight = height;
	mImageDepth = depth;
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

void CLibOI::SetKernelSoucePath(string path_to_kernels)
{
	mKernelSourcePath = path_to_kernels;
}
