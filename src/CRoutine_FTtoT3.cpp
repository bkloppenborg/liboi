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

/// Calculates the number of floats before the V2 data segment following the definition in COILibData.h
unsigned int CRoutine_FTtoT3::CalculateOffset(unsigned int n_vis, unsigned int n_v2)
{
	return 2*n_vis + n_v2;
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

	unsigned int offset = CalculateOffset(n_vis, n_v2);

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

//#ifdef DEBUG_VERBOSE
//	clFinish(mQueue);
//	FTtoT3_CPU(ft_loc, data_phasor, data_bsref, data_sign, n_t3, n_v2, output);
//
//#endif //DEBUG_VERBOSE

}

void CRoutine_FTtoT3::FTtoT3_CPU(cl_mem ft_input, cl_mem t3_uv_ref, cl_mem t3_uv_sign, valarray<complex<float>> & cpu_output, int n_vis, int n_v2, int n_t3, int n_uv)
{
	if(n_t3 == 0)
		return;

	int err = CL_SUCCESS;
	// Pull all of the data from the OpenCL device:
	valarray<cl_float2> cpu_dft(n_uv);
	valarray<cl_long4> cpu_uv_ref(n_t3);
	valarray<cl_short4> cpu_uv_sign(n_t3);

	err  = clEnqueueReadBuffer(mQueue, ft_input, CL_TRUE, 0, n_uv * sizeof(cl_float2), &cpu_dft[0], 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, t3_uv_ref, CL_TRUE, 0, n_t3 * sizeof(cl_long4), &cpu_uv_ref[0], 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, t3_uv_sign, CL_TRUE, 0, n_t3 * sizeof(cl_short4), &cpu_uv_sign[0], 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy values back to CPU CRoutine_FTtoT3::FTtoT3_CPU().", err);


	// Compute the T3, output the difference between CPU and GPU versions:
	complex<float> V_ab;
	complex<float> V_bc;
	complex<float> V_ca;
	cl_long4 uvpoint;
	cl_short4 sign;
	for(int i = 0; i < n_t3; i++)
	{
		uvpoint = cpu_uv_ref[i];
	    sign = cpu_uv_sign[i];
		// Look up the visibility values, conjugating as necessary:
		V_ab = complex<float>(cpu_dft[uvpoint.s0].s0, cpu_dft[uvpoint.s0].s1 * sign.s0);
		V_bc = complex<float>(cpu_dft[uvpoint.s1].s0, cpu_dft[uvpoint.s1].s1 * sign.s1);
		V_ca = complex<float>(cpu_dft[uvpoint.s2].s0, cpu_dft[uvpoint.s2].s1 * sign.s2);

		cpu_output[i] = V_ab * V_bc * V_ca;
	}
}

bool CRoutine_FTtoT3::FTtoT3_Test(cl_mem ft_input, cl_mem t3_uv_ref, cl_mem t3_uv_sign, cl_mem output, int n_vis, int n_v2, int n_t3, int n_uv)
{
	valarray<complex<float>> cpu_output(n_t3);
	FTtoT3(ft_input, t3_uv_ref, t3_uv_sign, output, n_vis, n_v2, n_t3);
	FTtoT3_CPU(ft_input, t3_uv_ref, t3_uv_sign, cpu_output, n_vis, n_v2, n_t3, n_uv);

	unsigned int offset = CalculateOffset(n_vis, n_v2);

	printf("Checking FT -> T3 Routine:\n");
	bool t3_pass = Verify(cpu_output, output, n_t3, sizeof(cl_float) * offset);
	PassFail(t3_pass);

	return t3_pass;
}
