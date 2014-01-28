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
#include "CRoutine_Zero.h"
#include "COILibData.h"

#ifndef PI
#include <cmath>
#define PI M_PI
#endif

using namespace std;

namespace liboi
{

#define TWO_PI (2 * PI)

CRoutine_Chi::CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	:CRoutine_Sum_NVidia(device, context, queue, rZero)
{
	// Specify the source location for the kernel.
	mSource.push_back("chi.cl");
	mChiSourceID = mSource.size() - 1;

	mSource.push_back("chi_complex_convex.cl");
	mChiConvexSourceID = mSource.size() - 1;

	mSource.push_back("chi_complex_nonconvex.cl");
	mChiNonConvexSourceID = mSource.size() - 1;

	mrSquare = NULL;

	// Set the temporary buffers and compiled kernel IDs to something we can verify is invalid.
	mChiOutput = NULL;
	mChiSquaredOutput = NULL;
	mChiKernelID = -1;
	mChiConvexKernelID = -1;
	mChiNonConvexKernelID = -1;
}

CRoutine_Chi::CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero, CRoutine_Square * rSquare)
	:CRoutine_Sum_NVidia(device, context, queue, rZero)
{
	// Specify the source location for the kernel.
	mSource.push_back("chi.cl");
	mChiSourceID = mSource.size() - 1;

	mSource.push_back("chi_complex_convex.cl");
	mChiConvexSourceID = mSource.size() - 1;

	mSource.push_back("chi_complex_nonconvex.cl");
	mChiNonConvexSourceID = mSource.size() - 1;

	mrSquare = rSquare;

	// Set the temporary buffers and compiled kernel IDs to something we can verify is invalid.
	mChiOutput = NULL;
	mChiSquaredOutput = NULL;
	mChiKernelID = -1;
	mChiConvexKernelID = -1;
	mChiNonConvexKernelID = -1;
}

CRoutine_Chi::~CRoutine_Chi()
{
	// Note, the routines are deleted elsewhere, leave them alone.
	if(mChiOutput) clReleaseMemObject(mChiOutput);
	if(mChiSquaredOutput) clReleaseMemObject(mChiSquaredOutput);
}

/// Computes the chi on the entire data buffer. Results are stored on the OpenCL
/// device for later use in mChiOutput.
void CRoutine_Chi::Chi(cl_mem data, cl_mem data_err, cl_mem model_data,
		LibOIEnums::Chi2Types complex_chi_method,
		unsigned int n_vis, unsigned int n_v2, unsigned int n_t3)
{
	// Zero out the result buffer:
	mrZero->Zero(mChiOutput, mChiBufferSize);

	unsigned int vis_offset = COILibData::CalculateOffset_Vis();
	unsigned int v2_offset = COILibData::CalculateOffset_V2(n_vis);
	unsigned int t3_offset = COILibData::CalculateOffset_T3(n_vis, n_v2);
	unsigned int n_data = COILibData::TotalBufferSize(n_vis, n_v2, n_t3);

	// V2 is always calculated using the standard chi routine.
	Chi(data, data_err, model_data, mChiOutput, v2_offset, n_v2);

	// Vis and T3 have different chi formulae
	if(complex_chi_method == LibOIEnums::CONVEX)
	{
		ChiComplexConvex(data, data_err, model_data, mChiOutput, vis_offset, n_vis);
		ChiComplexConvex(data, data_err, model_data, mChiOutput, t3_offset, n_t3);
	}
	else	// LibOIEnums::NON_CONVEX is the default method
	{
		ChiComplexNonConvex(data, data_err, model_data, mChiOutput, vis_offset, n_vis);
		ChiComplexNonConvex(data, data_err, model_data, mChiOutput, t3_offset, n_t3);
	}
}

/// Computes the chi on the entire data buffer and returns the result as an array of floats.
void CRoutine_Chi::Chi(cl_mem data, cl_mem data_err, cl_mem model_data,
		LibOIEnums::Chi2Types complex_chi_method,
		unsigned int n_vis, unsigned int n_v2, unsigned int n_t3,
		float * output, unsigned int & output_size)
{
	// Compute the chi
	Chi(data, data_err, model_data, complex_chi_method, n_vis, n_v2, n_t3);

	// Computations complete, copy back the chi values:
	output_size = min(mChiBufferSize, output_size);
	int err = CL_SUCCESS;
	err = clEnqueueReadBuffer(mQueue, mChiOutput, CL_TRUE, 0, sizeof(cl_float) * output_size, output, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy back chi elements.", err);
}

/// Traditional chi computation under the convex approximation in cartesian coordinates
void CRoutine_Chi::Chi(cl_mem data, cl_mem data_err, cl_mem model, cl_mem output, unsigned int start, unsigned int n)
{
	if(n == 0)
		return;

	int err = CL_SUCCESS;
	size_t global = (size_t) n;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mChiKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for chi kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mChiKernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mChiKernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mChiKernelID], 2, sizeof(cl_mem), &model);
	err |= clSetKernelArg(mKernels[mChiKernelID], 3, sizeof(cl_mem), &output);
	err |= clSetKernelArg(mKernels[mChiKernelID], 4, sizeof(unsigned int), &start);
	err |= clSetKernelArg(mKernels[mChiKernelID], 5, sizeof(unsigned int), &n);
	COpenCL::CheckOCLError("Failed to set chi kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mChiKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue chi kernel.", err);
}

/// Traditional chi implementation for polar coordinantes in the convex assumption.
/// Complex data and model vectors are converted to cartesian by rotating by the data phase. Then the
/// convex elliptical approximation is applied in computing the chi values.
void CRoutine_Chi::ChiComplexConvex(cl_mem data, cl_mem data_err, cl_mem model, cl_mem output, unsigned int start, unsigned int n)
{
	if(n == 0)
		return;

	int err = CL_SUCCESS;
	size_t global = (size_t) n;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mChiConvexKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for chi_complex_convex kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mChiConvexKernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mChiConvexKernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mChiConvexKernelID], 2, sizeof(cl_mem), &model);
	err |= clSetKernelArg(mKernels[mChiConvexKernelID], 3, sizeof(cl_mem), &output);
	err |= clSetKernelArg(mKernels[mChiConvexKernelID], 4, sizeof(unsigned int), &start);
	err |= clSetKernelArg(mKernels[mChiConvexKernelID], 5, sizeof(unsigned int), &n);
	COpenCL::CheckOCLError("Failed to set chi_complex_convex kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mChiConvexKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue chi_complex_convex kernel.", err);
}

/// Chi implementation for polar coordinantes under the non-convex assumption
/// Chi values are computed between the complex data and model vectors in polar coordinates.
/// The phase error is moduo TWO_PI to ensure the minimum difference is reported.
void CRoutine_Chi::ChiComplexNonConvex(cl_mem data, cl_mem data_err, cl_mem model, cl_mem output, unsigned int start, unsigned int n)
{
	if(n == 0)
		return;

	int err = CL_SUCCESS;
	size_t global = (size_t) n;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mChiNonConvexKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for chi_complex_nonconvex kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mChiNonConvexKernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mChiNonConvexKernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mChiNonConvexKernelID], 2, sizeof(cl_mem), &model);
	err |= clSetKernelArg(mKernels[mChiNonConvexKernelID], 3, sizeof(cl_mem), &output);
	err |= clSetKernelArg(mKernels[mChiNonConvexKernelID], 4, sizeof(unsigned int), &start);
	err |= clSetKernelArg(mKernels[mChiNonConvexKernelID], 5, sizeof(unsigned int), &n);
	COpenCL::CheckOCLError("Failed to set chi_complex_nonconvex kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mChiNonConvexKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue chi_complex_nonconvex kernel.", err);
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

/// Computes the Chi squared.
float CRoutine_Chi::Chi2(cl_mem data, cl_mem data_err, cl_mem model_data,
		LibOIEnums::Chi2Types complex_chi_method,
		unsigned int n_vis, unsigned int n_v2, unsigned int n_t3, bool compute_sum)
{
	if(mrSquare == NULL)
		throw "Square routine is not allocated. This is a programming error (wrong constructor called).";

	// Clean out the buffer
	mrZero->Zero(mChiSquaredOutput, mChiBufferSize);

	// Calculate the chi, then square it.
	Chi(data, data_err, model_data, complex_chi_method, n_vis, n_v2, n_t3);
	unsigned int n_data = COILibData::TotalBufferSize(n_vis, n_v2, n_t3);
	mrSquare->Square(mChiOutput, mChiSquaredOutput, n_data, n_data);

	// If we are to compute the sum, do so. Store the result in the ChiSquared output buffer.
	if(compute_sum)
		return ComputeSum(mChiSquaredOutput, mChiSquaredOutput, true);

	return 0;
}

void CRoutine_Chi::Chi2(cl_mem data, cl_mem data_err, cl_mem model_data,
		LibOIEnums::Chi2Types complex_chi_method,
		unsigned int n_vis, unsigned int n_v2, unsigned int n_t3,
		float * output, unsigned int & output_size)
{
	// Compute the chi
	Chi2(data, data_err, model_data, complex_chi_method, n_vis, n_v2, n_t3, false);

	// Computations complete, copy back the chi values:
	output_size = min(mChiBufferSize, output_size);
	int err = CL_SUCCESS;
	err = clEnqueueReadBuffer(mQueue, mChiOutput, CL_TRUE, 0, sizeof(cl_float) * output_size, output, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy back chi elements.", err);
}

// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_Chi::Init(unsigned int n)
{
	int err = CL_SUCCESS;
	mChiBufferSize = n;

	// First initialize the base-class constructor:
	CRoutine_Sum_NVidia::Init(mChiBufferSize);

	// Output buffer
	if(mChiOutput) clReleaseMemObject(mChiOutput);
	mChiOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float) * mChiBufferSize, NULL, &err);

	if(mChiSquaredOutput) clReleaseMemObject(mChiSquaredOutput);
	mChiSquaredOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float) * mChiBufferSize, NULL, &err);

	// Read the kernels, compile them
	string source = ReadSource(mSource[mChiSourceID]);
    BuildKernel(source, "chi", mSource[mChiSourceID]);
    mChiKernelID = mKernels.size() - 1;

	// Read the kernel, compile it
	source = ReadSource(mSource[mChiConvexSourceID]);
    BuildKernel(source, "chi_complex_convex", mSource[mChiConvexSourceID]);
    mChiConvexKernelID = mKernels.size() - 1;

	// The non-convex kernel needs TWO_PI to be defined
    stringstream tmp;
    tmp << "#define TWO_PI " << TWO_PI << endl;
    tmp << ReadSource(mSource[mChiNonConvexSourceID]);
    source = ReadSource(mSource[mChiNonConvexSourceID]);
    BuildKernel(tmp.str(), "chi_complex_nonconvex", mSource[mChiNonConvexSourceID]);
    mChiNonConvexKernelID = mKernels.size() - 1;
}

} /* namespace liboi */
