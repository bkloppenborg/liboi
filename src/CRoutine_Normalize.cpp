/*
 * CRoutine_Normalize.cpp
 *
 *  Created on: Nov 17, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to normalize a 2D image.
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

#include "CRoutine_Normalize.h"

CRoutine_Normalize::CRoutine_Normalize(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("normalize_float.cl");
}

CRoutine_Normalize::~CRoutine_Normalize()
{
	// TODO Auto-generated destructor stub
}

// Read in the kernel source and build program object.
void CRoutine_Normalize::Init()
{
	string source = ReadSource(mSource[0]);
	BuildKernel(source, "normalize_float", mSource[0]);
}

/// Calls a kernel to normalize an OpenCL buffer
void CRoutine_Normalize::Normalize(cl_mem buffer, unsigned int buffer_size, cl_mem divisor)
{
	int err = CL_SUCCESS;
	size_t global = size_t(buffer_size);
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for normalization kernel.", err);

	// Enqueue the kernel.
    err |= clSetKernelArg(mKernels[0],  0, sizeof(cl_mem), &buffer);
    err |= clSetKernelArg(mKernels[0],  1, sizeof(cl_mem), &divisor);
    err |= clSetKernelArg(mKernels[0],  2, sizeof(unsigned int), &buffer_size);
	COpenCL::CheckOCLError("Failed to set normalization kernel arguments.", err);

    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
    COpenCL::CheckOCLError("Failed to enqueue normalization kernel.", err);
}

/// Normalizes the specified image.
void CRoutine_Normalize::Normalize(cl_mem image, unsigned int image_width, unsigned int image_height, cl_mem divisor)
{
	// Calculate the size of the buffer and then normalize it
	Normalize(image, image_width * image_height, divisor);
}

