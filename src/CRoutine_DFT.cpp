/*
 * CRoutine_DFT.cpp
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine that implements a discrete Fourier transform.
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


#include "CRoutine_DFT.h"
#include <complex>

using namespace std;

namespace liboi
{

CRoutine_DFT::CRoutine_DFT(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_FT(device, context, queue)
{
	mImageScale = 0;
	// Specify the source location for the kernel.
	mSource.push_back("ft_dft2d.cl");
}

CRoutine_DFT::~CRoutine_DFT()
{
	// TODO Auto-generated destructor stub
}

/// Computes the discrete Fourier transform of a (real) image for the specified (cl_float2) UV points and stores
/// the result in output.
void CRoutine_DFT::FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output)
{
	// NOTE: Below we use the clGetKernelWorkGroupInfo to determine the local execution size of the
	// kernel.  On present-generation GPUs, the maximum work items per work group is 1024, so we
	// allocate 4 * sizeof(cl_float) * local = 4 kB < 32 (or 48) kB of memory. If future OpenCL
	// implementations support more local threads, this may become an issue.

    int status = 0;
    size_t global = (size_t) n_uv_points;
    size_t local = 256;                     // init to some value, not important anymore.

    // Get the maximum work-group size for executing the kernel on the device
    status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");

	// Set the kernel arguments and enqueue the kernel
	status = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &uv_points);
	status |= clSetKernelArg(mKernels[0], 1, sizeof(int), &n_uv_points);
	status |= clSetKernelArg(mKernels[0], 2, sizeof(cl_mem), &image);
	status |= clSetKernelArg(mKernels[0], 3, sizeof(int), &image_width);
	status |= clSetKernelArg(mKernels[0], 4, sizeof(int), &image_height);
	status |= clSetKernelArg(mKernels[0], 5, sizeof(cl_mem), &output);
	status |= clSetKernelArg(mKernels[0], 6, local * sizeof(cl_float), NULL);
	status |= clSetKernelArg(mKernels[0], 7, local * sizeof(cl_float), NULL);
	status |= clSetKernelArg(mKernels[0], 8, local * sizeof(cl_float2), NULL);
	status |= clSetKernelArg(mKernels[0], 9, sizeof(cl_mem), &image_flux);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    // Execute the kernel over the entire range of the data set
	status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
}

/// CPU implementation of the discrete Fourier transform algorithm.
void CRoutine_DFT::FT(valarray<cl_float2> & uv_points, unsigned int n_uv_points,
		valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
		valarray<cl_float2> & cpu_output)
{
	int uu = 0;
	int ii = 0;
	int jj = 0;

	// First create the DFT tables:
	int dft_size = n_uv_points * image_width;
	valarray<complex<float>> DFT_tablex(dft_size);
	valarray<complex<float>> DFT_tabley(dft_size);
	float tmp = 0;

	// First pre-compute the DFT exponent values
	for (uu = 0; uu < n_uv_points; uu++)
	{
		for (ii = 0; ii < image_width; ii++)
		{
			tmp = 2.0 * PI * RPMAS * image_scale * uv_points[uu].s0 * (float) ii;
			DFT_tablex[image_width * uu + ii] = complex<float>(cos(tmp), sin(tmp));
			tmp = -2.0 * PI * RPMAS * image_scale * uv_points[uu].s1 * (float) ii;
			DFT_tabley[image_width * uu + ii] = complex<float>(cos(tmp), sin(tmp));
		}
	}

	// Now generate the FT values.
	complex<float> ctmp;
	for(uu=0; uu < n_uv_points; uu++)
	{
		// Reset the value.
		ctmp = complex<float>(0.0, 0.0);
		for(ii=0; ii < image_width; ii++)
		{
			for(jj=0; jj < image_width; jj++)
			{
				ctmp += image[ ii + image_width * jj ]
					* DFT_tablex[ image_width * uu + ii]
					* DFT_tabley[ image_width * uu + jj];
			}
		}

		// Write the value out to the buffer.
		cpu_output[uu].s0 = real(ctmp);
		cpu_output[uu].s1 = imag(ctmp);
	}
}

void CRoutine_DFT::Init(float image_scale)
{
	mImageScale = image_scale;

	// Compile the image scale into the kernel.
	double RPMAS = (M_PI / 180.0) / 3600000.0; // Number of radians per milliarcsecond
	string source = ReadSource(mSource[0]);
	float arg = 2.0 * M_PI * RPMAS * mImageScale;
    stringstream tmp;

    // Insert macro definition for ARG (arg to 10 decimals in SI notation)
    tmp.str("");
    tmp << "#define ARG " << setprecision(10) << arg << "\n";
    tmp << source;

    BuildKernel(tmp.str(), "dft_2d", mSource[0]);
}

} /* namespace liboi */
