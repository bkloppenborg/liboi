/*
 * CRoutine_DFT.cpp
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_DFT.h"

CRoutine_DFT::CRoutine_DFT(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_FT(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("ft_dft.cl");
}

CRoutine_DFT::~CRoutine_DFT()
{
	// TODO Auto-generated destructor stub
}

CRoutine_DFT::Init(float image_scale)
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

    BuildKernel(tmp.str(), "ft_dft");
}

/// Computes the discrete Fourier transform of a (real) image for the specified (cl_float2) UV points and stores
/// the result in output.
CRoutine_DFT::FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem output)
{
#ifdef DEBUG
	printf("Computing the DFT using %s\n", mSource[0]);
#endif //DEBUG

    int err = 0;
    global = (size_t) n_uv_points;
    size_t local;                     // local domain size for our calculation

   // Get the maximum work-group size for executing the kernel on the device
    err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine the kernel workgroup size for ft_dft2d kernel.", err);

#ifdef DEBUG
    // Output some information about kernel sizes:
    if(gpu_enable_debug || gpu_enable_verbose)
        printf("ft_dft2d Kernel Sizes: Global: %i Local %i \n", (int)global, (int)local);
#endif //DEBUG

	// Set the kernel arguments and enqueue the kernel
	err = clSetKernelArg(mKernel[0], 0, sizeof(cl_mem), uv_points);
	err = clSetKernelArg(mKernel[0], 1, sizeof(int), &n_uv_points);
	err = clSetKernelArg(mKernel[0], 2, sizeof(cl_mem), image);
	err = clSetKernelArg(mKernel[0], 3, sizeof(int), &image_width);
	err = clSetKernelArg(mKernel[0], 4, sizeof(cl_mem), output);
	err = clSetKernelArg(mKernel[0], 6, local * sizeof(float), NULL);
	err = clSetKernelArg(mKernel[0], 7, local * sizeof(float), NULL);
	COpenCL::CheckOCLError("Failed to set ft_dft2d kernel arguments.", err);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(*pQueue, *pKernel_visi, 1, NULL, &global, &local, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue ft_dft2d kernel.", err);
}
