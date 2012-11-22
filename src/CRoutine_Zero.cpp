/*
 * CRoutine_Zero.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to zero out an OpenCL cl_mem buffer.
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

