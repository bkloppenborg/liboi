/*
 * CRoutine_LogLike.cpp
 *
 *  Created on: Jan 27, 2012
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to compute the logarithm of the likelihood.
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

#include "CRoutine_LogLike.h"
#include "COILibData.h"

namespace liboi
{

CRoutine_LogLike::CRoutine_LogLike(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	:CRoutine_Chi(device, context, queue, rZero)
{
	mSource.push_back("loglike.cl");
	mLogLikeSourceID = mSource.size() - 1;
	mLogLikeKernelID = 0;

	mLogLikeOutput = NULL;
}

CRoutine_LogLike::~CRoutine_LogLike()
{
	if(mLogLikeOutput) clReleaseMemObject(mLogLikeOutput);
}

// Computes the log likelihood of the individual elements in the chi_output buffer
void CRoutine_LogLike::LogLike(cl_mem chi_output, cl_mem data_err, cl_mem output, unsigned int n)
{
	int err = CL_SUCCESS;
	// The loglikelihood kernel executes on the entire output buffer
	// because the reduce_sum_float kernel uses the entire buffer as input.
	// Therefore we zero out the elements not directly involved in this computation.
	size_t global = (size_t) mInputSize;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mLogLikeKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for loglike kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mLogLikeKernelID], 0, sizeof(cl_mem), &chi_output);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 2, sizeof(cl_mem), &output);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 3, sizeof(unsigned int), &n);
	COpenCL::CheckOCLError("Failed to set loglike kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mLogLikeKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue loglike kernel.", err);
}

/// Computes the log-likeihoods for the specified OpenCL buffers.
/// The result is stored in the (protected) buffer mLogLikeOutput
void CRoutine_LogLike::LogLike(cl_mem data, cl_mem data_err, cl_mem model_data,
		LibOIEnums::Chi2Types complex_chi_method,
		unsigned int n_vis, unsigned int n_v2, unsigned int n_t3)
{
	// First call the chi routine to compute the individual elements
	Chi(data, data_err, model_data, complex_chi_method, n_vis, n_v2, n_t3);

	// Now compute the loglike using the mChiOutput buffer:
	unsigned int n_data = COILibData::TotalBufferSize(n_vis, n_v2, n_t3);
	LogLike(mChiOutput, data_err, mLogLikeOutput, n_data);
}

/// Computes the log of the likelihoods for the specified OpenCL buffers then returns the sum if compute_sum is true.
float CRoutine_LogLike::LogLike(cl_mem data, cl_mem data_err, cl_mem model_data,
		LibOIEnums::Chi2Types complex_chi_method,
		unsigned int n_vis, unsigned int n_v2, unsigned int n_t3, bool compute_sum)
{
	float sum = 0;
	unsigned int n_data = COILibData::TotalBufferSize(n_vis, n_v2, n_t3);

	// Call the loglike kernel on the buffer.
	LogLike(data, data_err, model_data, complex_chi_method, n_vis, n_v2, n_t3);

	// Now compute the sum and return the value
	if(compute_sum)
		sum = ComputeSum(mLogLikeOutput, mLogLikeOutput, true);

	// Compute the logZ value, don't forget to include the -N/2 log(TWO_PI) prefix!
	return -0.5 * n_data * log(2 * PI) + sum;
}

/// \brief Computes the log likelihood per each element, returns the result in output.
///
/// Note, this does not include the -N/2 log(TWO_PI) prefix
void CRoutine_LogLike::LogLike(valarray<cl_float> & chi_output, valarray<cl_float> & data_err, valarray<cl_float> & output, unsigned int n)
{
	// Verify the buffer sizes are valid
	assert(n == chi_output.size());
	assert(n == data_err.size());

	// Resize the output buffer if the user didn't already do this.
	if(n != output.size())
		output.resize(n);

	cl_float log_two_pi = log(TWO_PI);

	// Compute the individual loglike values.
	for(int i = 0; i < n; i++)
	{
		output[i] = -2* log(data_err[i]) - chi_output[i] * chi_output[i] / 2;
	}
}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_LogLike::Init(int num_max_elements)
{
	int err = CL_SUCCESS;

	// First initialize the base-class constructor:
	CRoutine_Chi::Init(num_max_elements);

	// Read the kernel, compile it
	string source = ReadSource(mSource[mLogLikeSourceID]);
    BuildKernel(source, "loglike", mSource[mLogLikeSourceID]);
    mLogLikeKernelID = mKernels.size() - 1;

	if(mLogLikeOutput == NULL)
	{
		mLogLikeOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mInputSize * sizeof(cl_float), NULL, &err);
		COpenCL::CheckOCLError("Could not create loglike temporary buffer.", err);
	}
}

} /* namespace liboi */
