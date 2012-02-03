/*
 * CRoutine_LogLike.cpp
 *
 *  Created on: Jan 27, 2012
 *      Author: bkloppenborg
 */

#include "CRoutine_LogLike.h"

CRoutine_LogLike::CRoutine_LogLike(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_Reduce_Sum(device, context, queue)
{
	mSource.push_back("loglike.cl");
	mLogLikeSourceID = mSource.size() - 1;

	mTemp = NULL;
	mOutput = NULL;
}

CRoutine_LogLike::~CRoutine_LogLike()
{

}

float CRoutine_LogLike::LogLike(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = 0;
	size_t global = (size_t) n;
	size_t local = 0;
	float sum = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mLogLikeKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for loglike kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mLogLikeKernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 2, sizeof(cl_mem), &model_data);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 3, sizeof(cl_mem), &mTemp);
	err |= clSetKernelArg(mKernels[mLogLikeKernelID], 4, sizeof(int), &n);
	COpenCL::CheckOCLError("Failed to set loglike kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mLogLikeKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue loglike kernel.", err);

#ifdef DEBUG_VERBOSE
	// Copy back the data, model, and errors:
	LogLike_CPU(data, data_err, model_data, n);
	ComputeSum_CPU(mOutput, n);
#endif // DEBUG_VERBOSE

	// Now fire up the parallel sum kernel and return the output.
	sum = ComputeSum(true, mOutput, mTemp, tmp_buff1, tmp_buff2);

	// Todo: Add in model priors.

	return -1*n * log(2 * PI) + sum;
}

float CRoutine_LogLike::LogLike_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = CL_SUCCESS;
	cl_float * cpu_data = new cl_float[n];
	err |= clEnqueueReadBuffer(mQueue, data, CL_TRUE, 0, n * sizeof(cl_float), cpu_data, 0, NULL, NULL);
	cl_float * cpu_data_err = new cl_float[n];
	err |= clEnqueueReadBuffer(mQueue, data_err, CL_TRUE, 0, n * sizeof(cl_float), cpu_data_err, 0, NULL, NULL);
	cl_float * cpu_model_data = new cl_float[n];
	err |= clEnqueueReadBuffer(mQueue, model_data, CL_TRUE, 0, n * sizeof(cl_float), cpu_model_data, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy values back to the CPU, CRoutine_LogLike::LogLike_CPU().", err);


	// we do this verbose
	float sum = 0;
	float tmp = 0;
	for(int i = 0; i < n; i++)
	{
		tmp = (cpu_data[i] - cpu_model_data[i]) / cpu_data_err[i];
		tmp *= -tmp;
		tmp -= -2 * log(cpu_data_err[i]);
//		printf("%i %f %f %e %e \n", i, cpu_data[i], cpu_model_data[i], cpu_data[i] - cpu_model_data[i], cpu_data_err[i]);
		sum += tmp;
	}

	printf("LogLike: %f\n", sum);

	delete[] cpu_data;
	delete[] cpu_data_err;
	delete[] cpu_model_data;

	// Todo: Add in model priors.

	return sum;
}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_LogLike::Init(int num_elements)
{
	int err = CL_SUCCESS;

	// First initialize the base-class constructor:
	CRoutine_Reduce_Sum::Init(num_elements, true);

	// Now allocate some memory
	if(mTemp == NULL)
		mTemp = clCreateBuffer(mContext, CL_MEM_READ_WRITE, num_elements * sizeof(cl_float), NULL, &err);

	if(mOutput == NULL)
		mOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	// Read the kernel, compile it
	string source = ReadSource(mSource[mLogLikeSourceID]);
    BuildKernel(source, "loglike", mSource[mLogLikeSourceID]);
    mLogLikeKernelID = mKernels.size() - 1;
}
