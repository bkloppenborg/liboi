/*
 * CRoutine_Chi.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Implementation of chi-computing routine.
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

#include "CRoutine_Chi.h"
#include <cstdio>

#include "CRoutine_Square.h"
#include "COILibData.h"

#ifndef PI
#include <cmath>
#define PI M_PI
#endif

#define TWO_PI (2 * PI)

using namespace std;

CRoutine_Chi::CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero, CRoutine_Square * rSquare)
	:CRoutine_Sum(device, context, queue, rZero)
{
	// Specify the source location for the kernel.
	mSource.push_back("chi.cl");
	mChiSourceID = mSource.size() - 1;

	mrSquare = rSquare;

	mChiTemp = NULL;
	mChiOutput = NULL;
	mChiKernelID = -1;
}

CRoutine_Chi::~CRoutine_Chi()
{
	// Note, the routines are deleted elsewhere, leave them alone.

	if(mChiTemp) clReleaseMemObject(mChiTemp);
	if(mChiOutput) clReleaseMemObject(mChiOutput);
}

///// Computes chi = (data - model)/(data_err) and stores it in the internal mChiTemp buffer.
//void CRoutine_Chi::Chi(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
//{
//	int err = CL_SUCCESS;
//   	// The loglikelihood kernel executes on the entire output buffer
//   	// because the reduce_sum_float kernel uses the entire buffer as input.
//   	// Therefore we zero out the elements not directly involved in this computation.
//	size_t global = (size_t) mNElements;
//	size_t local = 0;
//
//	// Get the maximum work-group size for executing the kernel on the device
//	err = clGetKernelWorkGroupInfo(mKernels[mChiKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
//	COpenCL::CheckOCLError("Failed to determine workgroup size for chi kernel.", err);
//
//	// Set the arguments to our compute kernel
//	err  = clSetKernelArg(mKernels[mChiKernelID], 0, sizeof(cl_mem), &data);
//	err |= clSetKernelArg(mKernels[mChiKernelID], 1, sizeof(cl_mem), &data_err);
//	err |= clSetKernelArg(mKernels[mChiKernelID], 2, sizeof(cl_mem), &model_data);
//	err |= clSetKernelArg(mKernels[mChiKernelID], 3, sizeof(cl_mem), &mChiTemp);
//	err |= clSetKernelArg(mKernels[mChiKernelID], 4, sizeof(int), &n);
//	COpenCL::CheckOCLError("Failed to set chi kernel arguments.", err);
//
//	// Execute the kernel over the entire range of the data set
//	err = clEnqueueNDRangeKernel(mQueue, mKernels[mChiKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
//	COpenCL::CheckOCLError("Failed to enqueue chi kernel.", err);
//}

// Computes the chi for vis, V2, and T3 data following the specified chi approximation method.
void CRoutine_Chi::Chi(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model, valarray<cl_float> & output,
		unsigned int n_vis, unsigned int n_v2, unsigned int n_t3,
		LibOIEnums::Chi2Types chi_method)
{
	// Asser the data is all of the same size.
	unsigned int n_data = data.size();
	assert(n_data == data_err.size());
	assert(n_data == model.size());

	// Resize the output buffer.
	output.resize(n_data);

	// Calculate offsets and run the chi routines
	// # Vis
	unsigned int vis_offset = COILibData::CalculateOffset_Vis();
	if(chi_method == LibOIEnums::CONVEX)
		Chi_complex_convex(data, data_err, model, vis_offset, n_vis, output);
	else
		Chi_complex_nonconvex(data, data_err, model, vis_offset, n_vis, output);

	// # V2
	unsigned int v2_offset = COILibData::CalculateOffset_V2(n_vis);
	Chi(data, data_err, model, v2_offset, n_v2, output);

	// # T3
	unsigned int t3_offset = COILibData::CalculateOffset_T3(n_vis, n_v2);
	if(chi_method == LibOIEnums::CONVEX)
		Chi_complex_convex(data, data_err, model, t3_offset, n_t3, output);
	else
		Chi_complex_nonconvex(data, data_err, model, t3_offset, n_t3, output);
}

/// Straight (traditional) chi computation under the convex approximation
void CRoutine_Chi::Chi(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
		unsigned int start_index, unsigned int n,
		valarray<cl_float> & output)
{
	// Verify the input sizes are ok.
	unsigned int n_data = data.size();
	assert(start_index + n <= n_data);
	assert(n_data == data_err.size());
	assert(n_data == model.size());

	// Verify the output sizes are ok.
	unsigned int n_output = output.size();
	assert(start_index + n <= n_output);

	// Everything is ok, compute the chi elements
	for(int i = start_index; i < n; i++)
		output[i] = (data[i] - model[i]) / data_err[i];
}

/// Computes the chi for complex quantities under the convex approximation
void CRoutine_Chi::Chi_complex_convex(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
		unsigned int start_index, unsigned int n,
		valarray<cl_float> & output)
{
	// Verify the input sizes are ok.
	unsigned int n_data = data.size();
	assert(start_index + n <= n_data);
	assert(n_data == data_err.size());
	assert(n_data == model.size());

	// Verify the output sizes are ok.
	unsigned int n_output = output.size();
	assert(start_index + n < n_output);

	// Compute the chi elements
	// Under the convex approximation the complex quantities are rotated to zero phase and compared
	// in cartesian coordinates.
	// The data, data err, and model are already stored as complex numbers so simply extract them from the buffers
	for(int i = start_index; i < n; i++)
	{
		// Form the complex quantities:
		complex<float> c_data(data[i], data[n+i]);
		complex<float> c_model(model[i], model[n+i]);
		complex<float> c_phasor(cos(data[n+i]), -sin(data[n+i]));

		// Rotate the model and the data to the +x axis by multiplying by the phasor
		c_data = c_data * c_phasor;
		c_model = c_model * c_phasor;

		// Now compute the chi elements and store the result
		output[i] =  (abs(c_data) - abs(c_model)) / data_err[i];
		// Notice, the phase error is divided by the total "swing" of the phase error
		output[n+i] =  (arg(c_data) - arg(c_model)) / (abs(c_data) * data_err[n+i]);
	}
}

/// Computes the chi for complex quantities under the non-convex approximation
void CRoutine_Chi::Chi_complex_nonconvex(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
		unsigned int start_index, unsigned int n,
		valarray<cl_float> & output)
{
	// Verify the input sizes are ok.
	unsigned int n_data = data.size();
	assert(start_index + n <= n_data);
	assert(n_data == data_err.size());
	assert(n_data == model.size());

	// Verify the output sizes are ok.
	unsigned int n_output = output.size();
	assert(start_index + n < n_output);

	// Compute the chi elements
	// Under the non-convex approximation the complex quantities are compared in polar coordinates
	// without rotation to the +x axis
	cl_float2 c_data;
	cl_float2 c_model;
	cl_float2 c_error;
	for(int i = start_index; i < n; i++)
	{
		// Form the complex quantities:
		c_data.s0 = data[i];
		c_data.s1 = data[n+i];
		c_model.s0 = model[i];
		c_model.s1 = model[n+i];
		c_error.s0 = data_err[i];
		c_error.s1 = data_err[n+i];

		// Now compute the chi elements and store the result
		output[i] =  (c_data.s0 - c_model.s0) / c_error.s0;
		// For the error in phase we need the remainder of the subtraction operation.
		// We explicitly upcast these to doubles to make the compiler happy, then downcast back to a cl_float
		output[n+i] = cl_float( fmod(double(c_data.s1) - double(c_model.s1), TWO_PI) / double(c_error.s1) );
	}
}

//void CRoutine_Chi::Chi(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model, valarray<cl_float> & output)
//{
//	unsigned int n = data.size();
//	assert(n == data_err.size() && n== model.size());
//	output.resize(n);
//
//	// Now compute the chi, store it.
//	for(int i = 0; i < n; i++)
//	{
//		tmp = 0;
//
//		if(i < n)
//			tmp = (cpu_data[i] - cpu_model_data[i]) / cpu_data_err[i];
//
//		output[i] = tmp;
//	}
//}


/// Helper function, calls the chi and then square routines, stores output in the internal mChiTemp buffer.
//float CRoutine_Chi::Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, bool compute_sum, bool return_value)
//{
//	float sum = 0;
//	Chi(data, data_err, model_data, n);
//	mrSquare->Square(mChiTemp, mChiTemp, n, n);
//
//	// Now fire up the parallel sum kernel and return the output.  Wrap this in a try/catch block.
//	try
//	{
//		if(compute_sum)
//			sum = ComputeSum(mChiTemp, mChiOutput, return_value);
//	}
//	catch (...)
//	{
//		printf("Warning, exception in CRoutine_Chi2.  Writing out buffers:\n");
//		Chi(data, data_err, model_data, n);
//		mrSquare->Square(mChiTemp, mChiTemp, mNElements, n);
//		DumpFloatBuffer(mChiTemp, n);
//		throw;
//	}
//
//	return sum;
//}

//float CRoutine_Chi::Chi2(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model, valarray<cl_float> & output)
//{
//	// First compute the chi elements
//	valarray<cl_float> output;
//	CRoutine_Chi::Chi(data, data_err, model, output);
//
//	// Now square the elements
//	valarray<cl_float> squared_output;
//	CRoutine_Square::Square(chi_output, squared_output, chi_output.size(), chi_output.size());
//
//	if(compute_sum)
//		return CRoutine_Sum::Sum(squared_output);
//
//	return 0;
//}

/// Computes the chi values and returns them in the array, output, which has size n
/// Note, the user is responsible for allocating and deallocating output!
//void CRoutine_Chi::GetChi(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output)
//{
//	// Compute the chi
//	Chi(data, data_err, model_data, n);
//	clFinish(mQueue);
//
//	// Copy data to the CPU.  Note, we use an intermediate array in case cl_float != float
//	int err = 0;
//	err = clEnqueueReadBuffer(mQueue, mChiTemp, CL_TRUE, 0, n * sizeof(cl_float), &mCPUChiTemp[0], 0, NULL, NULL);
//	COpenCL::CheckOCLError("Failed to copy buffer back to CPU.  CRoutine_Chi::GetChi().", err);
//
//	for(int i = 0; i < n; i++)
//		output[i] = float(mCPUChiTemp[i]);
//}

/// Computes the chi2 values and returns them in the array, output, which has size n
/// Note, the user is responsible for allocating and deallocating output!
//void CRoutine_Chi::GetChi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output)
//{
//	// Compute the chi
//	Chi2(data, data_err, model_data, n, false, false);
//	clFinish(mQueue);
//
//	// Copy data to the CPU.  Note, we use an intermediate array in case cl_float != float
//	int err = 0;
//	err = clEnqueueReadBuffer(mQueue, mChiTemp, CL_TRUE, 0, n * sizeof(cl_float), &mCPUChiTemp[0], 0, NULL, NULL);
//	COpenCL::CheckOCLError("Failed to copy buffer back to CPU.  CRoutine_Chi::GetChi().", err);
//
//	for(int i = 0; i < n; i++)
//		output[i] = float(mCPUChiTemp[i]);
//}


/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
//void CRoutine_Chi::Init(int n)
//{
//	int err = CL_SUCCESS;
//
//	// First initialize the base-class constructor:
//	CRoutine_Sum::Init(n);
//
//	// Now allocate some memory
//	if(mChiTemp == NULL)
//		mChiTemp = clCreateBuffer(mContext, CL_MEM_READ_WRITE, n * sizeof(cl_float), NULL, &err);
//
//	if(mChiOutput == NULL)
//		mChiOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);
//
//	// Read the kernel, compile it
//	string source = ReadSource(mSource[mChiSourceID]);
//    BuildKernel(source, "chi", mSource[mChiSourceID]);
//    mChiKernelID = mKernels.size() - 1;
//}
