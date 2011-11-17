/*
 * CLibOI.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "CLibOI.h"

CLibOI::CLibOI()
{
	// init datamembers
	mOCL = COpenCL();
}

CLibOI::~CLibOI()
{
	FreeOpenCLMem();
}

void CLibOI::FreeOpenCLMem()
{
	// First free datamembers:
	if(mImage_flux) delete mImage_flux;

	// Now free OpenCL buffers:
	if(mFluxBuffer) clReleaseMemObject(mFluxBuffer);
}

void CLibOI::Init(cl_device_type device_type, int image_width, int image_height, int image_depth)
{
	// First register the width, height, and depth of the images we will be using.
	RegisterImageSize(image_width, image_height, image_depth);

	// Now initalize the OpenCL context and all required routines.
	mOCL.Init(device_type);
	InitMemory();
	InitRoutines();
}

/// Initializes memory used for storing various things on the OpenCL context.
void CLibOI::InitMemory()
{
	int err = CL_SUCCESS;
	// Allocate some memory on the OpenCL device
	mFluxBuffer = clCreateBuffer(mOCL.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	COpenCL::CheckOCLError("Could not initialize liboi required memory objects.", err);
}

void CLibOI::InitRoutines()
{
	// Init all routines.  For now pre-allocate all buffers.
	mImage_flux = new CRoutine_Reduce(mOCL.GetDevice(), mOCL.GetContext(), mOCL.GetQueue());
	mImage_flux->SetSourcePath(mKernelSourcePath);
	mImage_flux->Init(mImageWidth * mImageHeight, true);
}

/// Computes the total flux for the current image.
float CLibOI::TotalFlux(bool return_value)
{
	return mImage_flux->ComputeSum(return_value, mFluxBuffer, mImage, NULL, NULL);
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
	mImage = image;
}

/// Creates an OpenCL memory object from the renderbuffer
/// Registers it as the currentt image object against which liboi operations will be undertaken.
cl_mem CLibOI::RegisterImage_GLRB(GLuint renderbuffer)
{
	int err = CL_SUCCESS;
	mImage = clCreateFromGLRenderbuffer(mOCL.GetContext(), CL_MEM_READ_WRITE, renderbuffer, &err);
	COpenCL::CheckOCLError("Could not create OpenCL image object from renderbuffer", err);

	return mImage;
}

/// Creates an OpenCL memory object from a texturebuffer
/// Registers it as the currentt image object against which liboi operations will be undertaken.
cl_mem CLibOI::RegisterImage_GLTB(GLuint texturebuffer)
{
	// TODO: Permit loading of 3D textures for spectral imaging.

	int err = CL_SUCCESS;
	// TODO: note that the clCreateFromGLTexture2D was depreciated in the OpenCL 1.2 specifications.
	mImage = clCreateFromGLTexture2D(mOCL.GetContext(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texturebuffer, &err);
	COpenCL::CheckOCLError("Could not create OpenCL image object from GLTexture", err);

	return mImage;
}

void CLibOI::SetKernelSoucePath(string path_to_kernels)
{
	mKernelSourcePath = path_to_kernels;
}
