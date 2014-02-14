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

void CRoutine_ImageToBuffer::CopyImage(cl_mem gl_input, cl_mem cl_output,
		unsigned int image_width, unsigned int image_height, unsigned int image_depth)
{
	int status = CL_SUCCESS;

	// Wait for the OpenGL queue to finish, lock resources.
	glFinish();
	status = clEnqueueAcquireGLObjects(mQueue, 1, &gl_input, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueAcquireGLObjects failed.");
	status = clFinish(mQueue);
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	// Now that we have exclusive access to the OpenGL memory object, copy it to the OpenCL buffer
	size_t global[2] = {image_width, image_height};

	// Enqueue the kernel.
//	status |= clSetKernelArg(mKernels[0],  0, sizeof(cl_uint), &image_width);
//	status |= clSetKernelArg(mKernels[0],  1, sizeof(cl_uint), &image_height);
	status  = clSetKernelArg(mKernels[0],  0, sizeof(cl_mem), (void *) &gl_input);
	status |= clSetKernelArg(mKernels[0],  1, sizeof(cl_mem), (void *) &cl_output);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

	cout << "Image buffer: " << gl_input << endl;
	cout << "Kernel claims: ";	// text written by printf in kernel.

    status |= clEnqueueNDRangeKernel(mQueue, mKernels[0], 2, NULL, global, NULL, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	// Wait for the queue to finish, then release the OpenGL buffer
	status = clFinish(mQueue);
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	// All done.  Unlock resources
	status = clEnqueueReleaseGLObjects(mQueue, 1, &gl_input, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueReleaseGLObjects failed.");
}

} /* namespace liboi */
