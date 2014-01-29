/*
 * CRoutine_Square.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to compute the square of the elements in a OpenCL cl_mem buffer.
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

#include "CRoutine_Square.h"

namespace liboi
{

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

void CRoutine_Square::Square(cl_mem input, cl_mem output, unsigned int buffer_size, unsigned int data_size)
{
	int status = 0;
	size_t global = (size_t) buffer_size;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");

	// Set the arguments to our compute kernel
	status  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &input);
	status |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &output);
	status |= clSetKernelArg(mKernels[0], 2, sizeof(int), &data_size);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

	// Execute the kernel over the entire range of the data set
	status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
}

} /* namespace liboi */
