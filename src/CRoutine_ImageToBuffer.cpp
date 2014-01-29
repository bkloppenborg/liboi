/*
 * CRoutine_ImageToBuffer.cpp
 *
 *  Created on: Nov 18, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to convert an OpenGL image buffer into an OpenCL cl_mem buffer.
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

#include "CRoutine_ImageToBuffer.h"
#include <cstdio>

namespace liboi
{

CRoutine_ImageToBuffer::CRoutine_ImageToBuffer(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine(device, context, queue)
{
	mSource.push_back("image2buf_GL_R.cl");

}

CRoutine_ImageToBuffer::~CRoutine_ImageToBuffer()
{
	// TODO Auto-generated destructor stub
}

// Read in the kernel source and build program object.
void CRoutine_ImageToBuffer::Init()
{
	string source = ReadSource(mSource[0]);
	BuildKernel(source, "image2buf_GL_R", mSource[0]);
}

void CRoutine_ImageToBuffer::CopyImage(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int depth)
{
	// TODO: We need to redo this for 3D data sets and for non-square images.
	// TODO: Figure out how to determine these sizes dynamically.
	size_t global[2];
//	size_t local[2];
	global[0] = global[1] = width;
//	local[0] = local[1] = 16;

	// Enqueue the kernel.
	int status = CL_SUCCESS;
	status |= clSetKernelArg(mKernels[0],  0, sizeof(cl_mem), &gl_image);
	status |= clSetKernelArg(mKernels[0],  1, sizeof(cl_mem), &cl_buffer);
	status |= clSetKernelArg(mKernels[0],  2, sizeof(int), &width);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    status |= clEnqueueNDRangeKernel(mQueue, mKernels[0], 2, NULL, global, NULL, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
}

} /* namespace liboi */
