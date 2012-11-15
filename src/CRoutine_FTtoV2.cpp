/*
 * CRoutine_FTtoV2.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to convert Fourier transformed data into squared visibilities.
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

#include "CRoutine_FTtoV2.h"
#include <cstdio>

CRoutine_FTtoV2::CRoutine_FTtoV2(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("ft_to_vis2.cl");
}

CRoutine_FTtoV2::~CRoutine_FTtoV2()
{
	// TODO Auto-generated destructor stub
}

/// Calculates the number of floats before the V2 data segment following the definition in COILibData.h
unsigned int CRoutine_FTtoV2::CalculateOffset(unsigned int n_vis)
{
	return 2*n_vis;
}

void CRoutine_FTtoV2::Init()
{
	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "ft_to_vis2", mSource[0]);
}

void CRoutine_FTtoV2::FTtoV2(cl_mem ft_input, cl_mem v2_uv_ref, cl_mem output, unsigned int n_vis, unsigned int n_v2)
{
	if(n_v2 == 0)
		return;

	// By the storage definition (see COILibData), there are 2*n_vis elements in the data buffer before the V2
	// data starts.
	int offset = CalculateOffset(n_vis);

    int err = 0;
    size_t global = (size_t) n_v2;
    size_t local;

    // Get the maximum work-group size for executing the kernel on the device
    err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
    COpenCL::CheckOCLError("Failed to determine maximum group size for ft_to_vis2 kernel.", err);

    // Set the kernel arguments
    err  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &ft_input);
    err  = clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &v2_uv_ref);
    err  = clSetKernelArg(mKernels[0], 2, sizeof(unsigned int), &offset);
    err  = clSetKernelArg(mKernels[0], 3, sizeof(unsigned int), &n_v2);
    err |= clSetKernelArg(mKernels[0], 4, sizeof(cl_mem), &output);
    COpenCL::CheckOCLError("Failed to set ft_to_vis2 kernel arguments.", err);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
    COpenCL::CheckOCLError("Failed to enqueue the ft_to_vis2 kernel.", err);

//#ifdef DEBUG_VERBOSE
//    FTtoV2_CPU(ft_loc, n_v2_points, output);
//#endif //DEBUG_VERBOSE
}

/// Computes the V2 using the input data on the CPU, compares the values and writes out to the console.
void CRoutine_FTtoV2::FTtoV2_CPU(cl_mem ft_input, cl_mem v2_uv_ref, cl_float * cpu_output, unsigned int n_vis, unsigned int n_v2, unsigned int n_uv)
{
	if(n_v2 == 0)
		return;

	int err = CL_SUCCESS;
	cl_float2 cpu_ft[n_uv];
	cl_uint cpu_uv_ref[n_v2];

	err  = clEnqueueReadBuffer(mQueue, ft_input, CL_TRUE, 0, n_uv * sizeof(cl_float2), cpu_ft, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, v2_uv_ref, CL_TRUE, 0, sizeof(cl_uint) * n_v2, cpu_uv_ref, 0, NULL, NULL);
    COpenCL::CheckOCLError("Failed to copy values back to the CPU, Routine_FTtoV2::FTtoV2_CPU().", err);

    unsigned int uv_index = 0;
	for(int i = 0; i < n_v2; i++)
	{
		uv_index = cpu_uv_ref[i];
		cpu_output[i] = cpu_ft[uv_index].s0 * cpu_ft[uv_index].s0 + cpu_ft[uv_index].s1 * cpu_ft[uv_index].s1;
	}
}

bool CRoutine_FTtoV2::FTtoV2_Test(cl_mem ft_input, cl_mem v2_uv_ref, cl_mem output, unsigned int n_vis, unsigned int n_v2, unsigned int n_uv)
{
	cl_float cpu_output[n_v2];
	FTtoV2(ft_input, v2_uv_ref, output, n_vis, n_v2);
	FTtoV2_CPU(ft_input, v2_uv_ref, cpu_output, n_vis, n_v2, n_uv);

	int offset = CalculateOffset(n_vis);

	printf("Checking FT -> V2 Routine:\n");
	bool v2_pass = Verify(cpu_output, output, n_v2, sizeof(cl_float) * offset);
	PassFail(v2_pass);
	return v2_pass;
}
