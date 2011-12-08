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

void CRoutine_FTtoV2::CRoutine_FTtoV2::FTtoV2(cl_mem ft_loc, int n_v2_points, cl_mem output)
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

#ifdef DEBUG
	// Copy back the input/output buffers.
	cl_float * tmp = new cl_float[n_v2_points];
	err = clEnqueueReadBuffer(mQueue, output, CL_TRUE, 0, n_v2_points * sizeof(cl_float), tmp, 0, NULL, NULL);

	printf("V2 Buffer elements:\n");
	for(int i = 0; i < n_v2_points; i++)
	{
		printf("%f ", tmp[i]);
	}

	// End the line, free memory.
	printf("\n");
	delete tmp;

#endif //DEBUG

}
