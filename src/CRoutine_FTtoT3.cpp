/*
 * CRoutine_FTtoT3.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to convert a Fourier transformed data into bispectra.
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

#include "CRoutine_FTtoT3.h"
#include <cstdio>
#include <complex>

CRoutine_FTtoT3::CRoutine_FTtoT3(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("ft_to_t3.cl");
}

CRoutine_FTtoT3::~CRoutine_FTtoT3()
{
	// TODO Auto-generated destructor stub
}

void CRoutine_FTtoT3::Init(void)
{
	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "ft_to_t3", mSource[0]);
}

void CRoutine_FTtoT3::FTtoT3(cl_mem ft_input, cl_mem t3_uv_ref, cl_mem t3_uv_sign, cl_mem output, int n_vis, int n_v2, int n_t3)
{
	if(n_t3 == 0)
		return;

	int err = 0;
	size_t global = (size_t) n_t3;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine local size for ft_to_t3 kernel.", err);

	unsigned int offset = COILibData::CalculateOffset(n_vis, n_v2);

	// Set kernel arguments:
	err  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &ft_input);
	err |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &t3_uv_ref);
	err |= clSetKernelArg(mKernels[0], 2, sizeof(cl_mem), &t3_uv_sign);
	err |= clSetKernelArg(mKernels[0], 3, sizeof(unsigned int), &offset);
	err |= clSetKernelArg(mKernels[0], 4, sizeof(int), &n_t3);
	err |= clSetKernelArg(mKernels[0], 5, sizeof(cl_mem), &output);      // Output is stored on the GPU.
	COpenCL::CheckOCLError("Failed to set ft_to_t3 kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to set ft_to_t3 kernel arguments.", err);
}

// Calculates the T3 from the Fourier transform input.
// Stores the output separated according to the COILibData.h specifications, namely T3Amp and T3Phi are separated.
void CRoutine_FTtoT3::FTtoT3(valarray<cl_float2> & ft_input, valarray<cl_uint4> & uv_ref, valarray<cl_short4> & signs, valarray<cl_float> & output)
{
	// How many T3s do we have?
	unsigned int n_t3 = min(uv_ref.size(), signs.size());
	output.resize(2*n_t3);

	// Locals
	cl_float2 t_ab;
	cl_float2 t_bc;
	cl_float2 t_ca;
	cl_uint4 uvpoint;
	cl_short4 sign;
	complex<float> V_ab;
	complex<float> V_bc;
	complex<float> V_ca;
	complex<float> T3;
	for(int i = 0; i < n_t3; i++)
	{
		// Look up the UV points
		uvpoint = uv_ref[i];
	    t_ab = ft_input[uvpoint.s0];
	    t_bc = ft_input[uvpoint.s1];
	    t_ca = ft_input[uvpoint.s2];

	    // Look up the signs, conjugate when necessary.
	    sign = signs[i];
	    t_ab.s1 *= sign.s0;
	    t_bc.s1 *= sign.s1;
	    t_ca.s1 *= sign.s2;

	    // Form complex numbers, carry out the multiplication.
		V_ab = complex<float>(t_ab.s0, t_ab.s1);
		V_bc = complex<float>(t_bc.s0, t_bc.s1);
		V_ca = complex<float>(t_ca.s0, t_ca.s1);
		T3 = V_ab * V_bc * V_ca;

		// Assign values to the output (following the specification in COILibData.h)
		output[i] = real(T3);
		output[n_t3 + i] = imag(T3);
	}
}
