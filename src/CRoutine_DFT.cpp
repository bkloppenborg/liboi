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
void CRoutine_DFT::FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem output)
{
	// NOTE: Below we use the clGetKernelWorkGroupInfo to determine the local execution size of the
	// kernel.  On present-generation GPUs, the maximum work items per work group is 1024, so we
	// allocate 4 * sizeof(cl_float) * local = 4 kB < 32 (or 48) kB of memory. If future OpenCL
	// implementations support more local threads, this may become an issue.

	int status = CL_SUCCESS;
    size_t global = (size_t) n_uv_points;
    size_t local = 256;                     // init to some value, not important anymore.

    // Get the maximum work-group size for executing the kernel on the device
    status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");
	// Round up the work size to be an integer multiple of the local work size.
	global = next_multiple(global, local);

	// Set the kernel arguments and enqueue the kernel
	status = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &uv_points);
	status |= clSetKernelArg(mKernels[0], 1, sizeof(int), &n_uv_points);
	status |= clSetKernelArg(mKernels[0], 2, sizeof(cl_mem), &image);
	status |= clSetKernelArg(mKernels[0], 3, sizeof(int), &image_width);
	status |= clSetKernelArg(mKernels[0], 4, sizeof(int), &image_height);
	status |= clSetKernelArg(mKernels[0], 5, sizeof(cl_mem), &output);
	status |= clSetKernelArg(mKernels[0], 6, local * sizeof(cl_float), NULL);
	status |= clSetKernelArg(mKernels[0], 7, local * sizeof(cl_uint2), NULL);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    // Execute the kernel over the entire range of the data set
	status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
}

/// Compute the Fourier transform of the image for a specific UV point
void CRoutine_DFT::FT(cl_float2 uv_point,
		valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
		cl_float2 & cpu_output)
{
	double x_center = double(image_width) / 2;
	double y_center = double(image_height) / 2;

	double x_temp = 0;
	complex<double> exp_x_vals;
	double y_temp = 0;
	complex<double> exp_y_vals;
	complex<double> dft_output = complex<float>(0,0);

	double arg_u =  2.0 * PI * RPMAS * image_scale * uv_point.s[0];	// note, positive due to U definition in interferometry.
	double arg_v = -2.0 * PI * RPMAS * image_scale * uv_point.s[1];

	for(unsigned int y = 0; y < image_height; y++)
	{
		y_temp = arg_v * (y - y_center);
		exp_y_vals = complex<double>(cos(y_temp), sin(y_temp));

		for(unsigned int x = 0; x < image_width; x++)
		{
			x_temp = arg_u * (x - x_center);
			exp_x_vals = complex<double>(cos(x_temp), sin(x_temp));

			dft_output += double(image[x + image_width * y]) * exp_x_vals * exp_y_vals;
		}
	}

	// assign the output
	cpu_output.s[0] = real(dft_output);
	cpu_output.s[1] = imag(dft_output);
}

/// CPU implementation of the Fourier transform
void CRoutine_DFT::FT(valarray<cl_float2> & uv_points, unsigned int n_uv_points,
		valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
		valarray<cl_float2> & cpu_output)
{
	// compute the Fourier transform of the image for each Uv point
	for(unsigned int i = 0; i < uv_points.size(); i++)
	{
		FT(uv_points[i], image, image_width, image_height, image_scale, cpu_output[i]);
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
