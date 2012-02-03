/*
 * CRoutine_Square.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: bkloppenborg
 */

#include "CRoutine_Square.h"

CRoutine_Square::CRoutine_Square(cl_device_id device, cl_context context, cl_command_queue queue)
	: CRoutine(device, context, queue)
{
	mSource.push_back("square.cl");

}

CRoutine_Square::~CRoutine_Square()
{

}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_Square::Init()
{
	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "square", mSource[0]);
}

void CRoutine_Square::Square(cl_mem input, cl_mem output, int n)
{
	int err = 0;
	size_t global = (size_t) n;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for square kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &output);
	COpenCL::CheckOCLError("Failed to set chi2 kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue square kernel.", err);
}
