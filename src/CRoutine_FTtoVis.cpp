/*
 * CRoutine_FTtoV2.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to convert Fourier transformed data into visibilities.
 *      Note: as there are no computations involved, this routine just
 *      moves values around in memory to conform to the data format in
 *      COILibData.h.
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

#include "CRoutine_FTtoVis.h"
#include <cstdio>

namespace liboi
{

CRoutine_FTtoVis::CRoutine_FTtoVis(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("ft_to_vis.cl");
}

CRoutine_FTtoVis::~CRoutine_FTtoVis()
{
	// TODO Auto-generated destructor stub
}

/// Calculates the number of floats before the vis data segment following the definition in COILibData.h
unsigned int CRoutine_FTtoVis::CalculateOffset()
{
	// there are no data before Vis data in the format specification
	return 0;
}

void CRoutine_FTtoVis::Init()
{
	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "ft_to_vis", mSource[0]);
}

void CRoutine_FTtoVis::FTtoVis(cl_mem ft_input, cl_mem vis_uv_ref, cl_mem output, unsigned int n_vis)
{
	if(n_vis == 0)
		return;

	// By the storage definition (see COILibData), there are 2*n_vis elements in the data buffer before the V2
	// data starts.
	int offset = CalculateOffset();

    int status = CL_SUCCESS;
    size_t global = (size_t) n_vis;
    size_t local;

    // Get the maximum work-group size for executing the kernel on the device
    status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");

    // Set the kernel arguments
	status  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &ft_input);
	status |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &vis_uv_ref);
	status |= clSetKernelArg(mKernels[0], 2, sizeof(unsigned int), &offset);
	status |= clSetKernelArg(mKernels[0], 3, sizeof(unsigned int), &n_vis);
	status |= clSetKernelArg(mKernels[0], 4, sizeof(cl_mem), &output);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    // Execute the kernel over the entire range of the data set
	status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
}

/// Computes the V2 using the input data on the CPU, compares the values and writes out to the console.
void CRoutine_FTtoVis::FTtoVis_CPU(cl_mem ft_input, cl_mem vis_uv_ref, valarray<cl_float> & cpu_output, unsigned int n_vis, unsigned int n_uv)
{
//	if(n_v2 == 0)
//		return;
//
//	int err = CL_SUCCESS;
//	cl_float2 cpu_ft[n_uv];
//	cl_uint cpu_uv_ref[n_v2];
//
//	err  = clEnqueueReadBuffer(mQueue, ft_input, CL_TRUE, 0, sizeof(cl_float2) * n_uv, cpu_ft, 0, NULL, NULL);
//	err |= clEnqueueReadBuffer(mQueue, v2_uv_ref, CL_TRUE, 0, sizeof(cl_uint) * n_v2, cpu_uv_ref, 0, NULL, NULL);
//    COpenCL::CheckOCLError("Failed to copy values back to the CPU, Routine_FTtoV2::FTtoV2_CPU().", err);
//
//    unsigned int uv_index = 0;
//	for(int i = 0; i < n_v2; i++)
//	{
//		uv_index = cpu_uv_ref[i];
//		cpu_output[i] = cpu_ft[uv_index].s0 * cpu_ft[uv_index].s0 + cpu_ft[uv_index].s1 * cpu_ft[uv_index].s1;
//	}
}

bool CRoutine_FTtoVis::FTtoVis_Test(cl_mem ft_input, cl_mem v2_uv_ref, cl_mem output, unsigned int n_vis, unsigned int n_uv)
{
//	valarray<cl_float> cpu_output(n_v2);
//	FTtoV2(ft_input, v2_uv_ref, output, n_vis, n_v2);
//	FTtoV2_CPU(ft_input, v2_uv_ref, cpu_output, n_vis, n_v2, n_uv);
//
//	int offset = CalculateOffset(n_vis);
//
//	printf("Checking FT -> V2 Routine:\n");
//	bool v2_pass = Verify(cpu_output, output, n_v2, sizeof(cl_float) * offset);
//	PassFail(v2_pass);
//	return v2_pass;
}

} /* namespace liboi */
