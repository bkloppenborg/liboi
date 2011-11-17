/*
 * CRoutine_Normalize.cpp
 *
 *  Created on: Nov 17, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_Normalize.h"

CRoutine_Normalize::CRoutine_Normalize()
{
	// Specify the source location for the kernel.
	mSource.push_back("normalize.cl");
}

CRoutine_Normalize::~CRoutine_Normalize()
{
	// TODO Auto-generated destructor stub
}

// Read in the kernel source and build program object.
void CRoutine_Normalize::Init()
{
	string source = ReadSource(mSource[0]);
	BuildKernel(source);
}

void CRoutine_Normalize::Normalize(cl_mem image, cl_mem divisor, int width, int height, int depth)
{
	// Create the image size memory object
	cl_int3 tmp;
	tmp.x = width;
	tmp.y = height;
	tmp.z = depth;

	// TODO: We need to redo this for 3D data sets and for non-square images.
	// TODO: Figure out how to determine these sizes dynamically.
	size_t * global = new size_t[2];
	global[0] = global[1] = width;
	size_t * local = new size_t[2];
	local[0] = local[1] = 16;

	// Enqueue the kernel.
	int err = CL_SUCCESS;
    err |= clSetKernelArg(mKernels[0],  0, sizeof(cl_mem), &image);
    err |= clSetKernelArg(mKernels[0],  1, sizeof(cl_mem), &divisor);
    err |= clSetKernelArg(mKernels[0],  2, sizeof(cl_int3), &tmp);
	COpenCL::CheckOCLError("Failed to set normalization kernel arguments.", err);


    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(mQueue, mKernels[0], 2, NULL, global, local, 0, NULL, NULL);
    COpenCL::CheckOCLError("Failed to enqueue normalization kernel.", err);

    // Free memory
    delete global;
    delete local;
}
