/*
 * CLibOI.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *  
 *  Description:
 *      Main source file for LIBOI.
 */

/* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * If you use this software as part of a scientific publication, please cite as:
 *
 * Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library"
 * (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
 *
 * This file is part of the OpenCL Interferometry Library (LIBOI).
 * 
 * LIBOI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "liboi.hpp"
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "COILibDataList.h"
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
#include "CRoutine_Zero.h"

namespace liboi
{

CLibOI::CLibOI(cl_device_type type)
{
	// init datamembers
	mOCL = new COpenCL(type);
	mDataList = new COILibDataList();

	mImage_cl = NULL;
	mImage_gl = NULL;
	mImage_host = NULL;
	mImageHeight = 1;
	mImageWidth = 1;
	mImageDepth = 1;
	mFluxBuffer = NULL;
	mImageType = LibOIEnums::OPENCL_BUFFER;	// By default we assume the image is stored in an OpenCL buffer.
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
	mrZeroBuffer = NULL;
}

CLibOI::~CLibOI()
{
	// First free datamembers:
	delete mDataList;
	delete mrTotalFlux;
	delete mrCopyImage;
	delete mrNormalize;
	delete mrFT;
	delete mrV2;
	delete mrT3;
	delete mrChi;
	delete mrLogLike;
	delete mrSquare;
	delete mrZeroBuffer;

	// Now free OpenCL buffers:
	if(mFluxBuffer) clReleaseMemObject(mFluxBuffer);
	if(mFTBuffer) clReleaseMemObject(mFTBuffer);
	if(mSimDataBuffer) clReleaseMemObject(mSimDataBuffer);
	if(mImage_gl) clReleaseMemObject(mImage_gl);
	if(mImage_cl) clReleaseMemObject(mImage_cl);

	delete mOCL;
}

/// Copies the specified layer from the registered image buffer over to an OpenCL memory buffer.
/// If the image is already in an OpenCL buffer, this function need not be called.
void CLibOI::CopyImageToBuffer(int layer)
{
	int err = CL_SUCCESS;

	// Decide where we need to copy from

	if(mImageType == LibOIEnums::OPENGL_FRAMEBUFFER || mImageType == LibOIEnums::OPENGL_TEXTUREBUFFER)
	{
		// Wait for the OpenGL queue to finish, lock resources.
		glFinish();
		err |= clEnqueueAcquireGLObjects(mOCL->GetQueue(), 1, &mImage_gl, 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not acquire OpenGL Memory object.", err);

		// TODO: Implement depth channel for 3D images
		CopyImageToBuffer(mImage_gl, mImage_cl, mImageWidth, mImageHeight, layer);
		COpenCL::CheckOCLError("Could not copy OpenGL image to the OpenCL Memory buffer", err);
		clFinish(mOCL->GetQueue());

		// All done.  Unlock resources
		err |= clEnqueueReleaseGLObjects (mOCL->GetQueue(), 1, &mImage_gl, 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not Release OpenGL Memory object.", err);
	}
	else if(mImageType == LibOIEnums::HOST_MEMORY)
	{
		CopyImageToBuffer(mImage_host, mImage_cl, mImageWidth, mImageHeight, layer);
	}
//	else if(mImageType == LibOIEnums::OPENCL_BUFFER)
//	{
//		// do nothing, it's already in device memory!
//	}
	else
	{
		// We don't know about this image type.  Automatically fail.
		assert(false);
	}
}

/// Copies gl_image, an OpenGL image in of the internal format GL_R, to a floating point image buffer, cl_image.
void CLibOI::CopyImageToBuffer(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int layer)
{
	mrCopyImage->CopyImage(gl_image, cl_buffer, width, height, layer);
}

/// Copies host memory to a cl_mem buffer
void CLibOI::CopyImageToBuffer(float * host_mem, cl_mem cl_buffer, int width, int height, int layer)
{
	int err = CL_SUCCESS;
	int size = width *  height;
	int offset = size * layer;

	cl_float * tmp = new cl_float[size];
	for(int i = 0; i < size; i++)
		tmp[i] = host_mem[i];

	// Enqueue a blocking write
    err  = clEnqueueWriteBuffer(mOCL->GetQueue(), mImage_cl, CL_TRUE, offset, sizeof(float) * size, tmp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy host image to OpenCL device.", err);

	delete[] tmp;
}

/// Computes the chi2 between the current simulated data, and the observed data set specified in data
float CLibOI::DataToChi2(COILibDataPtr data)
{
	unsigned int n_vis = data->GetNumVis();
	unsigned int n_v2 = data->GetNumV2();
	unsigned int n_t3 = data->GetNumT3();

	return mrChi->Chi2(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, LibOIEnums::NON_CONVEX, n_vis, n_v2, n_t3, true);
}

float CLibOI::DataToLogLike(COILibDataPtr data)
{
	unsigned int n_vis = data->GetNumVis();
	unsigned int n_v2 = data->GetNumV2();
	unsigned int n_t3 = data->GetNumT3();

	return mrLogLike->LogLike(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, LibOIEnums::NON_CONVEX, n_vis, n_v2, n_t3, true);
}

/// Copies the current image in mCLImage to the floating point buffer, image, iff the sizes match exactly.
void CLibOI::ExportImage(float * image, unsigned int width, unsigned int height, unsigned int depth)
{
	if(width != mImageWidth || height != mImageHeight || depth != mImageDepth)
		return;

	int err = CL_SUCCESS;
	int num_elements = mImageWidth * mImageHeight * mImageDepth;
	cl_float tmp[num_elements];
	err |= clEnqueueReadBuffer(mOCL->GetQueue(), mImage_cl, CL_TRUE, 0, num_elements * sizeof(cl_float), tmp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy buffer back to CPU, CLibOI::ExportImage() ", err);

	// Copy to the output buffer, converting as we go.
	for(unsigned int i = 0; i < num_elements; i++)
		image[i] = tmp[i];

}

/// Computes the Fourier transform of the image, then generates Vis2 and T3's.
/// This routine assumes the image has been normalized using Normalize() (and that the total flux is stored in mFluxBuffer)
void CLibOI::FTToData(COILibDataPtr data)
{
	// First compute the Fourier transform
	mrFT->FT(data->GetLoc_DataUVPoints(), data->GetNumUV(), mImage_cl, mImageWidth, mImageHeight, mFluxBuffer, mFTBuffer);

	// Now create the V2 and T3's
	int n_vis = data->GetNumVis();
	int n_v2 = data->GetNumV2();
	int n_t3 = data->GetNumT3();

	mrV2->FTtoV2(mFTBuffer, data->GetLoc_V2_UVRef(), mSimDataBuffer, n_vis, n_v2);

	mrT3->FTtoT3(mFTBuffer, data->GetLoc_T3_UVRef(),
			data->GetLoc_T3_sign(), mSimDataBuffer, n_vis, n_v2, n_t3);
}

OIDataList CLibOI::GetData(unsigned int data_num)
{
	return mDataList->GetData(data_num);
}

double CLibOI::GetDataAveJD(int data_num)
{
	return mDataList->at(data_num)->GetAveJD();
}

int CLibOI::GetNData()
{
	return mDataList->GetNData();
}

int CLibOI::GetNDataAllocated()
{
	return mDataList->GetNDataAllocated();
}

int CLibOI::GetNDataAllocated(int data_num)
{
	return mDataList->GetNDataAllocated(data_num);
}

int CLibOI::GetNDataSets()
{
	return mDataList->size();
}

/// Returns the number of T3 data points in the specified data set.  If the data set does not exist, returns 0.
int CLibOI::GetNT3(int data_num)
{
	if(data_num < mDataList->size())
		return mDataList->at(data_num)->GetNumT3();

	return 0;
}

/// Returns the number of V2 data points in the specified data set.  If the data set does not exist, returns 0.
int CLibOI::GetNV2(int data_num)
{
	if(data_num < mDataList->size())
		return mDataList->at(data_num)->GetNumV2();

	return 0;
}

/// Copies up to buffer_size elements from mSimDataBuffer to output_buffer
/// Note, the data is exported as a floating point array with the first
/// N(V2) elements being visibilities and 2*N(T3) elements being T3's
//void CLibOI::GetSimulatedData(unsigned int data_set, float * output_buffer, unsigned int buffer_size)
//{
//	if(data_set > mDataList.size())
//		return;
//
//	unsigned int num_elements = min(int(buffer_size), int(mDataList[data_set]->GetNumData()) );
//	unsigned int num_v2 = mDataList[data_set]->GetNumV2();
//	unsigned int num_t3 = mDataList[data_set]->GetNumT3();
//
//	// Pull over the T3 data
//	vector<CT3DataPtr> t3_data;
//	mDataList[data_set]->GetT3(t3_data);
//
//	// Pull the data down from the OpenCL device in it's native format, convert to float afterward
//	int err = CL_SUCCESS;
//	cl_float tmp[num_elements];
//	err |= clEnqueueReadBuffer(mOCL->GetQueue(), mSimDataBuffer, CL_TRUE, 0, num_elements * sizeof(cl_float), tmp, 0, NULL, NULL);
//	COpenCL::CheckOCLError("Could not copy buffer back to CPU, CLibOI::ExportImage() ", err);
//
//	// First copy over the V2:
//	for(int i = 0; i < num_v2; i++)
//		output_buffer[i] = float(tmp[i]);
//
//	// Now do the T3's
//	complex<float> t3_model_tmp;
//	complex<float> t3_phase_tmp;
//	complex<float> t3_out;
//	float data_phi = 0;
//	for(int i = 0; i < num_t3; i++)
//	{
//		// Compute the complex model t3:
//		t3_model_tmp = complex<float>(float(tmp[num_v2 + 2*i]), float(tmp[num_v2 + 2*i + 1]));
//
//		// Compute the phasor (undoes rotation in COILibData::InitData)
//		data_phi = t3_data[i]->t3_phi * PI / 180;
//		t3_phase_tmp = complex<float>(cos(data_phi), -sin(data_phi));
//
//		// Rotate the model back to the
//		t3_out = t3_model_tmp / t3_phase_tmp;
//
//		output_buffer[num_v2 + 2*i] = abs(t3_out);
//		output_buffer[num_v2 + 2*i + 1] = arg(t3_out);
//	}
//
//	// Zero out the remainder of the buffer:
//	for(int i = num_elements; i < buffer_size; i++)
//		output_buffer[i] = 0;
//}

/// Returns the T3 data in a structured vector, does nothing if data_set is out of range.
//void CLibOI::GetT3(unsigned int data_set, vector<CT3DataPtr> & t3)
//{
//	if(data_set < mDataList.size())
//		mDataList[data_set]->GetT3(t3);
//}
//
///// Returns the V2 data in a structured vector, does nothing if data_set is out of range.
//void CLibOI::GetV2(unsigned int data_set, vector<CV2DataPtr> & v2)
//{
//	if(data_set < mDataList.size())
//		mDataList[data_set]->GetV2(v2);
//}

/// Uses the current active image to compute the chi (i.e. non-squared version) with respect to the
/// specified data and returns the chi elements in the floating point array, output.
/// This is a convenience function that calls FTToData, DataToChi
void CLibOI::ImageToChi(COILibDataPtr data, float * output, unsigned int & n)
{
	// Simple, call the other functions
	Normalize();
	FTToData(data);

	unsigned int n_vis = data->GetNumVis();
	unsigned int n_v2 = data->GetNumV2();
	unsigned int n_t3 = data->GetNumT3();

	return mrChi->Chi(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, LibOIEnums::NON_CONVEX, n_vis, n_v2, n_t3, output, n);
}

/// Same as ImageToChi above.
/// Returns false if the data number does not exist, true otherwise.
bool CLibOI::ImageToChi(int data_num, float * output, unsigned int & n)
{
	if(data_num > mDataList->size() - 1)
		return false;

	COILibDataPtr data = mDataList->at(data_num);
	ImageToChi(data, output, n);
	return true;
}

/// Uses the current active image to compute the chi2 with respect to the specified data.
/// This is a convenience function that calls FTToData and DataToChi2.
float CLibOI::ImageToChi2(COILibDataPtr data)
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
	if(data_num > mDataList->size() - 1)
		return -1;

	COILibDataPtr data = mDataList->at(data_num);
	return ImageToChi2(data);
}

/// Uses the current active image to compute the chi2 with respect to the
/// specified data and returns the chi elements in the floating point array, output.
/// This is a convenience function that calls FTToData, DataToChi
void CLibOI::ImageToChi2(COILibDataPtr data, float * output, unsigned int & n)
{
	// Simple, call the other functions
	Normalize();
	FTToData(data);

	unsigned int n_vis = data->GetNumVis();
	unsigned int n_v2 = data->GetNumV2();
	unsigned int n_t3 = data->GetNumT3();

	return mrChi->Chi(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, LibOIEnums::NON_CONVEX, n_vis, n_v2, n_t3, output, n);
}

/// Same as ImageToChi above.
/// Returns false if the data number does not exist, true otherwise.
bool CLibOI::ImageToChi2(int data_num, float * output, unsigned int & n)
{
	if(data_num > mDataList->size() - 1)
		return false;

	COILibDataPtr data = mDataList->at(data_num);
	ImageToChi2(data, output, n);
	return true;
}

/// Uses the currently loaded image and specified data set to
/// compute simulated data.
void CLibOI::ImageToData(int data_num)
{
	if(data_num > mDataList->size() - 1)
		return;

	COILibDataPtr data = mDataList->at(data_num);
	ImageToData(data);
}

/// Uses the currently loaded image and specified data set to
/// compute simulated data.
void CLibOI::ImageToData(COILibDataPtr data)
{
	Normalize();
	FTToData(data);
}

float CLibOI::ImageToLogLike(COILibDataPtr data)
{
	// Simple, call the other functions
	Normalize();
	FTToData(data);
	float llike = DataToLogLike(data);
	return llike;
}
float CLibOI::ImageToLogLike(int data_num)
{
	if(data_num > mDataList->size() - 1)
		return -1;

	COILibDataPtr data = mDataList->at(data_num);
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

	// Create a location to store the image if it comes from host or OpenGL memory locations
	if(mImageType == LibOIEnums::HOST_MEMORY || mImageType == LibOIEnums::OPENGL_FRAMEBUFFER || mImageType == LibOIEnums::OPENGL_TEXTUREBUFFER)
	{
		mImage_cl = clCreateBuffer(mOCL->GetContext(), CL_MEM_READ_WRITE, mImageWidth * mImageHeight * sizeof(cl_float), NULL, &err);
		COpenCL::CheckOCLError("Could not create temporary OpenCL image buffer", err);
	}

	// Determine the maximum data size, cache it locally.
	mMaxData = mDataList->MaxNumData();
	mMaxUV = mDataList->MaxUVPoints();

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
	if(mrZeroBuffer == NULL)
	{
		mrZeroBuffer = new CRoutine_Zero(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
		mrZeroBuffer->SetSourcePath(mKernelSourcePath);
		mrZeroBuffer->Init();
	}

	if(mrSquare == NULL)
	{
		mrSquare = new CRoutine_Square(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue());
		mrSquare->SetSourcePath(mKernelSourcePath);
		mrSquare->Init();
	}

	if(mrTotalFlux == NULL)
	{
		mrTotalFlux = new CRoutine_Sum(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue(), mrZeroBuffer);
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
			mrChi = new CRoutine_Chi(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue(), mrZeroBuffer, mrSquare);
			mrChi->SetSourcePath(mKernelSourcePath);
			mrChi->Init(mMaxData);
		}

		if(mrLogLike == NULL)
		{
			mrLogLike = new CRoutine_LogLike(mOCL->GetDevice(), mOCL->GetContext(), mOCL->GetQueue(), mrZeroBuffer);
			mrLogLike->SetSourcePath(mKernelSourcePath);
			mrLogLike->Init(mMaxData);
		}
	}
}

/// Reads in an OIFITS file and stores it into OpenCL memory
/// Note, this routine will not load data any routine that uses data is initialized.
int CLibOI::LoadData(string filename)
{
	if(!mDataRoutinesInitialized)
	{
		mDataList->LoadData(filename, mOCL->GetContext(), mOCL->GetQueue());
		return mDataList->size() - 1;
	}

	return -1;
}

int CLibOI::LoadData(const OIDataList & data)
{
	if(!mDataRoutinesInitialized)
	{
		mDataList->LoadData(data, mOCL->GetContext(), mOCL->GetQueue());
		return mDataList->size() - 1;
	}

	return -1;
}

/// Normalizes a floating point buffer by dividing by the sum of the buffer
void CLibOI::Normalize()
{
	TotalFlux(false);

	// Now normalize the image
	mrNormalize->Normalize(mImage_cl, mImageWidth, mImageHeight, mFluxBuffer);
}

void CLibOI::PrintDeviceInfo()
{
	if(mOCL != NULL)
		mOCL->PrintDeviceInfo(mOCL->GetDevice());
}

/// Computes the total flux for the current image/layer
/// If the image is 2D, use zero for the layer.
float CLibOI::TotalFlux(bool return_value)
{
	float flux = mrTotalFlux->ComputeSum(mImage_cl, mFluxBuffer, true);
	return flux;
}

/// Removes the specified data set from memory.
void CLibOI::RemoveData(int data_num)
{
	mDataList->RemoveData(data_num);
}

/// Runs the verification functions of each kernel.  Assumes all initialization has been complete
/// and at least one data set has been loaded.
void CLibOI::RunVerification(int data_num)
{
//	if(data_num > mDataList->size() - 1)
//		return;
//
//	// Get the data and some information about the data.
//	COILibDataPtr data = mDataList->at(data_num);
//	int n_vis = data->GetNumVis();
//	int n_v2 = data->GetNumV2();
//	int n_t3 = data->GetNumT3();
//	int n_uv = data->GetNumUV();
//
//	mrTotalFlux->ComputeSum_Test(mImage_cl, mFluxBuffer);
//	mrNormalize->Normalize_Test(mImage_cl, mImageWidth, mImageHeight, mFluxBuffer);
//	mrFT->FT_Test(data->GetLoc_DataUVPoints(), n_uv, mImage_cl, mImageWidth,
//			mImageHeight, mFluxBuffer, mFTBuffer);
//
//	mrV2->FTtoV2_Test(mFTBuffer, data->GetLoc_V2_UVRef(), mSimDataBuffer, n_vis, n_v2, n_uv);
//
//	mrT3->FTtoT3_Test(mFTBuffer, data->GetLoc_T3_UVRef(),
//			data->GetLoc_T3_sign(), mSimDataBuffer, n_vis, n_v2, n_t3, n_uv);
//
//	// Now run the chi, chi2, and loglike kernels:
//	int n = data->GetNumData();
//	mrChi->Chi_Test(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, n);
//	mrChi->Chi2_Test(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, n, true);
//	mrLogLike->LogLike_Test(data->GetLoc_Data(), data->GetLoc_DataErr(), mSimDataBuffer, n);
}

/// Saves the current image in the OpenCL memory buffer to the specified FITS file
/// If the OpenCL memory has not been initialzed, this function immediately returns
void   CLibOI::SaveImage(string filename)
{
	if(mImage_cl == NULL)
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
void   CLibOI::SetImageInfo(unsigned int width, unsigned int height, unsigned int depth, float scale)
{
	// Assert that the image and scale are greater than zero in size.
	assert(width > 0);
	assert(height > 0);
	assert(depth > 0);
	assert(scale > 0);

	mImageWidth = width;
	mImageHeight = height;
	mImageDepth = depth;
	mImageScale = scale;
}

/// Tells LibOI that the image source is located in host memory at the address specified by host_memory.
/// All subsequent CopyImageToBuffer commands will read from this location.
void CLibOI::SetImageSource(float * host_memory)
{
	mImageType = LibOIEnums::ImageTypes::HOST_MEMORY;
	mImage_host = host_memory;
}

/// Tells LibOI that the image source is already in device memory.
/// All subsequent CopyImageToBuffer commands will read from this location.
void CLibOI::SetImageSource(cl_mem cl_device_memory)
{
	mImageType = LibOIEnums::ImageTypes::OPENCL_BUFFER;
	mImage_cl = cl_device_memory;
}

/// Tells LibOI that the image source is located in OpenGL device memory at the location
/// specified.  You must also indicate whether the OpenGL location is a
///  OPENGL_FRAMEBUFFER | OPENGL_TEXTUREBUFFER
/// All subsequent CopyImageToBuffer commands will read from this location.
void CLibOI::SetImageSource(GLuint gl_device_memory, LibOIEnums::ImageTypes type)
{
	mImageType = type;

	int err = CL_SUCCESS;

	switch(type)
	{
	case LibOIEnums::OPENGL_FRAMEBUFFER:
		mImage_gl = clCreateFromGLBuffer(mOCL->GetContext(), CL_MEM_READ_ONLY, gl_device_memory, &err);
		COpenCL::CheckOCLError("Could not create OpenCL image object from framebuffer", err);

		break;

	case LibOIEnums::OPENGL_TEXTUREBUFFER:
		// TODO: note that the clCreateFromGLTexture2D was depreciated in the OpenCL 1.2 specifications.
		mImage_gl = clCreateFromGLTexture2D(mOCL->GetContext(), CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, gl_device_memory, &err);
		COpenCL::CheckOCLError("Could not create OpenCL image object from GLTexture", err);

		break;

	case LibOIEnums::OPENGL_RENDERBUFFER:
		// TODO: note that the clCreateFromGLTexture2D was depreciated in the OpenCL 1.2 specifications.
		mImage_gl = clCreateFromGLRenderbuffer(mOCL->GetContext(), CL_MEM_READ_ONLY, gl_device_memory, &err);
		COpenCL::CheckOCLError("Could not create OpenCL image object from GLTexture", err);

		break;

	default:
		// We don't know what type of image this is!
		assert(false);
		break;
	}
}


void CLibOI::SetKernelSourcePath(string path_to_kernels)
{
	mKernelSourcePath = path_to_kernels;
}

/// Replaces the data set loaded into old_data_id with new_data
void CLibOI::ReplaceData(unsigned int old_data_id, const OIDataList & new_data)
{
	mDataList->ReplaceData(old_data_id, new_data, mOCL->GetContext(), mOCL->GetQueue());
}

} /* namespace liboi */
