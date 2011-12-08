/*
 * CRoutine_DFT.cpp
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_DFT.h"
#include <sstream>
#include <cstdio>

using namespace std;

CRoutine_DFT::CRoutine_DFT(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_FT(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("ft_dft2d.cl");
}

CRoutine_DFT::~CRoutine_DFT()
{
	// TODO Auto-generated destructor stub
}

void CRoutine_DFT::Init(float image_scale)
{
	// Allocate a few locals
	double RPMAS = (M_PI / 180.0) / 3600000.0; // Number of radians per milliarcsecond
	string source = ReadSource(mSource[0]);
	float arg = 2.0 * M_PI * RPMAS * image_scale;
    stringstream tmp;

    // Insert macro definition for ARG
    tmp.str("");
    tmp << "#define ARG " << arg << "\n";
    tmp << source;

    BuildKernel(tmp.str(), "dft_2d");
}

/// Computes the discrete Fourier transform of a (real) image for the specified (cl_float2) UV points and stores
/// the result in output.
void CRoutine_DFT::FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output)
{
#ifdef DEBUG
	printf("Computing the FT using DFT method, %s\n", mSource[0].c_str());
#endif //DEBUG

    int err = 0;
    size_t global = (size_t) n_uv_points;
    size_t local = 128;                     // local domain size for our calculation

    // Get the maximum work-group size for executing the kernel on the device
//    err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
//	COpenCL::CheckOCLError("Failed to determine the kernel workgroup size for ft_dft2d kernel.", err);

#ifdef DEBUG
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
	// Copy back the input/output buffers.
	cl_float * tmp = new cl_float[n_uv_points];
	err = clEnqueueReadBuffer(mQueue, output, CL_TRUE, 0, n_uv_points * sizeof(cl_float), tmp, 0, NULL, NULL);

	printf("DFT Buffer elements:\n");
	for(int i = 0; i < n_uv_points/2; i++)
	{
		printf("%f ", tmp[2*i]);
		printf("%f ", tmp[2*i+1]);
	}

	// End the line, free memory.
	printf("\n");
	delete tmp;

#endif //DEBUG_VERBOSE
}
