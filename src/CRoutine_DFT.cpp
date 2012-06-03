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


#include "CRoutine_DFT.h"
#include <complex>

using namespace std;

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
#ifdef DEBUG_VERBOSE
	printf("Computing the FT using DFT method, %s\n", mSource[0].c_str());
#endif //DEBUG

    int err = 0;
    size_t global = (size_t) n_uv_points;
    size_t local = 128;                     // local domain size for our calculation

    // Get the maximum work-group size for executing the kernel on the device
    //err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	//COpenCL::CheckOCLError("Failed to determine the kernel workgroup size for ft_dft2d kernel.", err);

#ifdef DEBUG_VERBOSE
    // Output some information about kernel sizes:
	printf("ft_dft2d Kernel Sizes: Global: %i Local %i \n", (int)global, (int)local);
#endif //DEBUG

	// Set the kernel arguments and enqueue the kernel
	err = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &uv_points);
	err |= clSetKernelArg(mKernels[0], 1, sizeof(int), &n_uv_points);
	err |= clSetKernelArg(mKernels[0], 2, sizeof(cl_mem), &image);
	err |= clSetKernelArg(mKernels[0], 3, sizeof(int), &image_width);
	err |= clSetKernelArg(mKernels[0], 4, sizeof(cl_mem), &output);
	err |= clSetKernelArg(mKernels[0], 5, local * sizeof(float), NULL);
	err |= clSetKernelArg(mKernels[0], 6, local * sizeof(float), NULL);
	err |= clSetKernelArg(mKernels[0], 7, sizeof(cl_mem), &image_flux);
	COpenCL::CheckOCLError("Failed to set ft_dft2d kernel arguments.", err);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue ft_dft2d kernel.", err);

#ifdef DEBUG_VERBOSE
	// Compare the CPU and OpenCL computed DFT values.
	FT_CPU(uv_points, n_uv_points, image, image_width, image_height, image_flux, output);
#endif //DEBUG_VERBOSE
}

/// Computes the DFT on the CPU, then compares CPU and OpenCL output.
/// Note: This routine copies data back from the OpenCL device and should only be used for debugging purposes.
void CRoutine_DFT::FT_CPU(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, complex<float> * visi)
{
    int err = 0;
	double RPMAS = (M_PI / 180.0) / 3600000.0; // Number of radians per milliarcsecond

	// First pull down the UV points and image.
	cl_float2 cpu_uvpts[n_uv_points];
	cl_float cpu_image[image_width * image_height];
	cl_float flux = 0;

	err = clEnqueueReadBuffer(mQueue, uv_points, CL_TRUE, 0, n_uv_points * sizeof(cl_float2), cpu_uvpts, 0, NULL, NULL);
	err = clEnqueueReadBuffer(mQueue, image, CL_TRUE, 0, image_width * image_height * sizeof(cl_float), cpu_image, 0, NULL, NULL);
	err = clEnqueueReadBuffer(mQueue, image_flux, CL_TRUE, 0, sizeof(cl_float), &flux, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy back DFT data, CRoutine_DFT.cpp", err);

	int uu = 0;
	int ii = 0;
	int jj = 0;

	// First create the DFT tables:
	int dft_size = n_uv_points * image_width;
	complex<float> * DFT_tablex = new complex<float>[dft_size];
	complex<float> * DFT_tabley = new complex<float>[dft_size];
	float tmp = 0;

	for (uu = 0; uu < n_uv_points; uu++)
	{
		for (ii = 0; ii < image_width; ii++)
		{
			tmp = 2.0 * PI * RPMAS * mImageScale * cpu_uvpts[uu].s0 * (float) ii;
			DFT_tablex[image_width * uu + ii] = complex<float>(cos(tmp), sin(tmp));
			tmp = -2.0 * PI * RPMAS * mImageScale * cpu_uvpts[uu].s1 * (float) ii;
			DFT_tabley[image_width * uu + ii] = complex<float>(cos(tmp), sin(tmp));
		}
	}

	// Now generate the FT values.
	for(uu=0; uu < n_uv_points; uu++)
	{
		visi[uu] = complex<float>(0.0, 0.0);
		for(ii=0; ii < image_width; ii++)
		{
			for(jj=0; jj < image_width; jj++)
			{
				visi[uu] += cpu_image[ ii + image_width * jj ] * DFT_tablex[ image_width * uu + ii]
															   * DFT_tabley[ image_width * uu + jj];
			}
		}
	}

	delete[] DFT_tablex;
	delete[] DFT_tabley;
}

bool CRoutine_DFT::FT_Test(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output)
{
	// Run the OpenCL DFT routine:
	complex<float> * cpu_output = new complex<float>[n_uv_points];
	FT(uv_points, n_uv_points, image, image_width, image_height, image_flux, output);
	FT_CPU(uv_points, n_uv_points, image, image_width, image_height, image_flux, cpu_output);

	printf("Checking FT (DFT) Routine:\n");
	bool norm_pass = Verify(cpu_output, output, n_uv_points, 0);
	PassFail(norm_pass);

	delete[] cpu_output;

	return norm_pass;
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
