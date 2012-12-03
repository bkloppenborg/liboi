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

namespace liboi
{

CRoutine_LogLike::CRoutine_LogLike(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	:CRoutine_Sum(device, context, queue, rZero)
{
	mSource.push_back("loglike.cl");
	mLogLikeSourceID = mSource.size() - 1;
	mLogLikeKernelID = 0;

	mTempLogLike = NULL;
	mOutput = NULL;
}

CRoutine_LogLike::~CRoutine_LogLike()
{
	if(mTempLogLike) clReleaseMemObject(mTempLogLike);
	if(mOutput) clReleaseMemObject(mOutput);
}

/// Computes the loglikelihood, returns it as a floating point number.
float CRoutine_LogLike::LogLike(cl_mem data, cl_mem data_err, cl_mem model_data, int n, bool compute_sum, bool return_value)
{
	float sum = 0;
	int err = CL_SUCCESS;
	// The loglikelihood kernel executes on the entire output buffer
	// because the reduce_sum_float kernel uses the entire buffer as input.
	// Therefore we zero out the elements not directly involved in this computation.
	size_t global = (size_t) mNElements;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mLogLikeKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for loglike kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mLogLikeKernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 2, sizeof(cl_mem), &model_data);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 3, sizeof(cl_mem), &mTempLogLike);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 4, sizeof(int), &n);
	COpenCL::CheckOCLError("Failed to set loglike kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mLogLikeKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue loglike kernel.", err);

	// Now fire up the parallel sum kernel and return the output.  Wrap this in a try/catch block.
	try
	{
		if(compute_sum)
			sum = ComputeSum(mTempLogLike, mOutput, return_value);
	}
	catch (...)
	{
		printf("Warning, exception in CRoutine_LogLike.  Writing out buffers:\n");
		LogLike(data, data_err, model_data, n, false, false);
		DumpFloatBuffer(mTempLogLike, mNElements);
		throw;
	}

	return -1*n * log(2 * PI) + sum;
}

float CRoutine_LogLike::LogLike_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n, valarray<float> & output)
{
	int err = CL_SUCCESS;
	cl_float cpu_data[n];
	cl_float cpu_data_err[n];
	cl_float cpu_model_data[n];
	err |= clEnqueueReadBuffer(mQueue, data, CL_TRUE, 0, n * sizeof(cl_float), cpu_data, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, data_err, CL_TRUE, 0, n * sizeof(cl_float), cpu_data_err, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, model_data, CL_TRUE, 0, n * sizeof(cl_float), cpu_model_data, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy values back to the CPU, CRoutine_LogLike::LogLike_CPU().", err);

	// we do this verbose
	float sum = 0;
	float temp = 0;
	for(int i = 0; i < n; i++)
	{
		temp = 0;

		if(i < n)
		{
			temp = (cpu_data[i] - cpu_model_data[i]) / cpu_data_err[i];
		    temp = -2 * log(cpu_data_err[i]) - temp * temp;
		}

		output[i] = temp;
		sum += output[i];
	}

	// TODO: Add in model priors
	return -1*n * log(2 * PI) + sum;
}

bool CRoutine_LogLike::LogLike_Test(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	valarray<float> cpu_output(n);
	LogLike(data, data_err, model_data, n, false, false);
	float cpu_sum = LogLike_CPU(data, data_err, model_data, n, cpu_output);

	// Compare the CL and CPU chi2 elements:
	printf("Checking individual loglike values:\n");
	bool loglike_match = Verify(cpu_output, mTempLogLike, n, 0);
	PassFail(loglike_match);

	printf("Checking summed loglike values:\n");
	float cl_sum = ComputeSum(mTempLogLike, mOutput, true);
	bool sum_pass = bool(fabs((cpu_sum - cl_sum)/cpu_sum) < MAX_REL_ERROR);
	printf("  CPU Value:  %0.4f\n", cpu_sum);
	printf("  CL  Value:  %0.4f\n", cl_sum);
	printf("  Difference: %0.4f\n", cpu_sum - cl_sum);
	PassFail(sum_pass);

	return sum_pass;
}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_LogLike::Init(int num_max_elements)
{
	int err = CL_SUCCESS;

	// First initialize the base-class constructor:
	CRoutine_Sum::Init(num_max_elements);

	// Read the kernel, compile it
	string source = ReadSource(mSource[mLogLikeSourceID]);
    BuildKernel(source, "loglike", mSource[mLogLikeSourceID]);
    mLogLikeKernelID = mKernels.size() - 1;

	if(mOutput == NULL)
		mOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	if(mTempLogLike == NULL)
		mTempLogLike = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mNElements * sizeof(cl_float), NULL, &err);

	COpenCL::CheckOCLError("Could not create loglike temporary buffer.", err);
}

} /* namespace liboi */
