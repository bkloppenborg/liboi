/*
 * CRoutine_Zero.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: bkloppenborg
 */

#include "CRoutine_Zero.h"

CRoutine_Zero::CRoutine_Zero(cl_device_id device, cl_context context, cl_command_queue queue)
	: CRoutine(device, context, queue)
{
	mSource.push_back("zero_buffer.cl");

}

CRoutine_Zero::~CRoutine_Zero()
{

}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_Zero::Init()
{
	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "zero_buffer", mSource[0]);
}

void CRoutine_Zero::Zero(cl_mem input, int buffer_size)
{
	int err = 0;
	size_t global = (size_t) buffer_size;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for square kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(mKernels[0], 1, sizeof(int), &buffer_size);
	COpenCL::CheckOCLError("Failed to set square kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue square kernel.", err);
}
