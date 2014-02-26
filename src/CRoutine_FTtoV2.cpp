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

#include "COILibData.h"

namespace liboi
{

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
	int offset = COILibData::CalculateOffset_V2(n_vis);

    int status = CL_SUCCESS;
    size_t global = (size_t) n_v2;
    size_t local;

    // Get the maximum work-group size for executing the kernel on the device
    status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");

    // Set the kernel arguments
	status  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &ft_input);
    status |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &v2_uv_ref);
    status |= clSetKernelArg(mKernels[0], 2, sizeof(unsigned int), &offset);
    status |= clSetKernelArg(mKernels[0], 3, sizeof(unsigned int), &n_v2);
    status |= clSetKernelArg(mKernels[0], 4, sizeof(cl_mem), &output);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    // Execute the kernel over the entire range of the data set
    status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
}

/// CPU-bound implementation.  Computes the norm of the visibilities.
void CRoutine_FTtoV2::FTtoV2(valarray<cl_float2> & ft_input, valarray<cl_uint> & uv_ref, valarray<cl_float> & cpu_output, unsigned int n_v2)
{
	// Reset and resize the output array:
	cpu_output.resize(n_v2);

	// Make a couple of assertations about the input data
	assert(cpu_output.size() <= ft_input.size());
	assert(cpu_output.size() <= uv_ref.size());

	// Square the visibility, save it to the output array:
	unsigned int uv_index = 0;
	for(int i = 0; i < n_v2; i++)
	{
		uv_index = uv_ref[i];
		cpu_output[i] = ft_input[uv_index].s[0] * ft_input[uv_index].s[0] + ft_input[uv_index].s[1] * ft_input[uv_index].s[1];
	}
}

} /* namespace liboi */
