/*
 * CRoutine_Chi2.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_Chi2.h"

CRoutine_Chi2::CRoutine_Chi2(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_Reduce(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("chi2.cl");
	mChi2SourceID = mSource.size() - 1;

	mChi2Temp = NULL;
	mChi2Output = NULL;
}

CRoutine_Chi2::~CRoutine_Chi2()
{
	if(mChi2Temp) clReleaseMemObject(mChi2Temp);
	if(mChi2Output) clReleaseMemObject(mChi2Output);
}

float CRoutine_Chi2::Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = 0;
	size_t global = (size_t) n;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mChi2KernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for chi2 kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mChi2KernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 2, sizeof(cl_mem), &model_data);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 3, sizeof(cl_mem), &mChi2Temp);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 4, sizeof(int), &n);
	COpenCL::CheckOCLError("Failed to set chi2 kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mChi2KernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue chi2 kernel", err);

	// Now fire up the parallel sum kernel and return the output.
	return ComputeSum(true, mChi2Output, mChi2Temp, NULL, NULL);
}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_Chi2::Init(int num_elements)
{
	// First initialize the base-class constructor:
	CRoutine_Reduce::Init(num_elements, true);

	int err = CL_SUCCESS;

	// Now allocate some memory
	if(mChi2Temp == NULL)
		mChi2Temp = clCreateBuffer(mContext, CL_MEM_READ_WRITE, num_elements * sizeof(cl_float), NULL, &err);

	if(mChi2Output == NULL)
		mChi2Output = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	// Read the kernel, compile it
	string source = ReadSource(mSource[mChi2SourceID]);
    BuildKernel(source, "chi2");
    mChi2KernelID = mKernels.size() - 1;
}
