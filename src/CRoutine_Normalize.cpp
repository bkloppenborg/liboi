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
 * The authors request, but do not require, that you acknowledge the
 * use of this software in any publications.  See 
 * https://github.com/bkloppenborg/liboi/wiki
 * for example citations
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

/// Normalizes the specified image.
void CRoutine_Normalize::Normalize(cl_mem image, int image_width, int image_height, cl_mem divisor)
{
	// Create the image size memory object
	cl_int2 tmp;
	tmp.x = image_width;
	tmp.y = image_height;

	// TODO: We need to rewrite this for 3D data sets and for non-square images.
	// TODO: Figure out how to determine these sizes dynamically.
	size_t * global = new size_t[2];
	global[0] = global[1] = image_width;
	size_t * local = new size_t[2];
	local[0] = local[1] = 16;

	// Enqueue the kernel.
	int err = CL_SUCCESS;
    err |= clSetKernelArg(mKernels[0],  0, sizeof(cl_mem), &image);
    err |= clSetKernelArg(mKernels[0],  1, sizeof(cl_mem), &divisor);
    err |= clSetKernelArg(mKernels[0],  2, sizeof(cl_int2), &tmp);
	COpenCL::CheckOCLError("Failed to set normalization kernel arguments.", err);

    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(mQueue, mKernels[0], 2, NULL, global, local, 0, NULL, NULL);
    COpenCL::CheckOCLError("Failed to enqueue normalization kernel.", err);

    delete[] local;
    delete[] global;
}

void CRoutine_Normalize::Normalize_CPU(cl_mem image, int image_width, int image_height, cl_mem divisor, cl_float * output)
{
	int n_pixels = image_width * image_height;
	cl_float cpu_divisor;

	// Copy the data back to the CPU
	int err = CL_SUCCESS;
	err |= clEnqueueReadBuffer(mQueue, image, CL_TRUE, 0, n_pixels * sizeof(cl_float), output, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, divisor, CL_TRUE, 0, sizeof(cl_float), &cpu_divisor, 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy buffer back to CPU, CRoutine_Normalize::Normalize_CPU() ", err);

	for(int i = 0; i < n_pixels; i++)
		output[i] /= cpu_divisor;
}

/// Compares the individual elements of the normalized image, CPU vs. GPU.
bool CRoutine_Normalize::Normalize_Test(cl_mem image, int image_width, int image_height, cl_mem divisor)
{
	int n_pixels = image_width * image_height;
	cl_float cpu_output[n_pixels];

	Normalize_CPU(image, image_width, image_height, divisor, cpu_output);
	Normalize(image, image_width, image_height, divisor);

	printf("Checking Normalization Routine:\n");
	bool norm_pass = Verify(cpu_output, image, n_pixels, 0);
	PassFail(norm_pass);
	return norm_pass;
}
