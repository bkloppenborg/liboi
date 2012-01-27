/*
 * CRoutine_FTtoV2.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_FTtoV2.h"
#include <cstdio>

CRoutine_FTtoV2::CRoutine_FTtoV2(cl_device_id device, cl_context context, cl_command_queue queue)
	:COpenCLRoutine(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("ft_to_vis2.cl");
}

CRoutine_FTtoV2::~CRoutine_FTtoV2()
{
	// TODO Auto-generated destructor stub
}

void CRoutine_FTtoV2::Init(float image_scale)
{
	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "ft_to_vis2");
}

void CRoutine_FTtoV2::FTtoV2(cl_mem ft_loc, int n_v2_points, cl_mem output)
{
    int err = 0;
    size_t global = (size_t) n_v2_points;
    size_t local;

    // Get the maximum work-group size for executing the kernel on the device
    err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
    COpenCL::CheckOCLError("Failed to determine maximum group size for ft_to_vis2 kernel.", err);

    // Set the kernel arguments
    err  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &ft_loc);
    err |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &output);
    COpenCL::CheckOCLError("Failed to set ft_to_vis2 kernel arguments.", err);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
    COpenCL::CheckOCLError("Failed to enqueue the ft_to_vis2 kernel.", err);

#ifdef DEBUG_VERBOSE
    FTtoV2_CPU(ft_loc, n_v2_points, output);
#endif //DEBUG_VERBOSE
}

/// Computes the V2 using the input data on the CPU, compares the values and writes out to the console.
void CRoutine_FTtoV2::FTtoV2_CPU(cl_mem ft_loc, int n_v2_points, cl_mem output)
{
	int err = 0;
	// Pull back values from the OpenCL devices:
	cl_float2 * cpu_dft = new cl_float2[n_v2_points];
	err = clEnqueueReadBuffer(mQueue, ft_loc, CL_TRUE, 0, n_v2_points * sizeof(cl_float2), cpu_dft, 0, NULL, NULL);
	cl_float * cl_output = new cl_float[n_v2_points];
	err = clEnqueueReadBuffer(mQueue, output, CL_TRUE, 0, n_v2_points * sizeof(cl_float), cl_output, 0, NULL, NULL);

	cl_float vis2 = 0;
	cl_float real = 0;
	cl_float imag = 0;
	printf("Vis2 Buffer elements: (CPU, OpenCL, Diff)\n");
	for(int i = 0; i < n_v2_points; i++)
	{
		real = cpu_dft[i].s0;
		imag = cpu_dft[i].s1;
		vis2 = real * real + imag * imag;
		printf("\t%i (%f %f %e)\n", i, vis2, cl_output[i], vis2 - cl_output[i]);
	}

	// Free memory
	delete[] cpu_dft;
	delete[] cl_output;
}
