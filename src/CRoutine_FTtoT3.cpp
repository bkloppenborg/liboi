/*
 * CRoutine_FTtoT3.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
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

void CRoutine_FTtoT3::FTtoT3(cl_mem ft_loc, cl_mem data_phasor, cl_mem data_bsref, cl_mem data_sign, int n_t3, int n_v2, cl_mem output)
{
	int err = 0;
	size_t global = (size_t) n_t3;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine local size for ft_to_t3 kernel.", err);

	// Set kernel arguments:
	err  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &ft_loc);
	err |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), &data_phasor);
	err |= clSetKernelArg(mKernels[0], 2, sizeof(cl_mem), &data_bsref);
	err |= clSetKernelArg(mKernels[0], 3, sizeof(cl_mem), &data_sign);
	err |= clSetKernelArg(mKernels[0], 4, sizeof(int), &n_v2);
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

void CRoutine_FTtoT3::FTtoT3_CPU(cl_mem ft_loc, cl_mem data_phasor, cl_mem data_bsref, cl_mem data_sign, int n_t3, int n_v2, cl_mem output)
{
	// Approximate the number of UV points.
	// TODO: Note, this could actually result in memory access errors if n_uv < the number specified below!
	int n_uv = 3 * n_t3 + n_v2;

	int err = 0;
	// Pull all of the data from the OpenCL device:
	cl_float2 * cpu_dft = new cl_float2[n_uv];
	err = clEnqueueReadBuffer(mQueue, ft_loc, CL_TRUE, 0, n_uv * sizeof(cl_float2), cpu_dft, 0, NULL, NULL);

	cl_float2 * cpu_phasor = new cl_float2[n_t3];
	err = clEnqueueReadBuffer(mQueue, data_phasor, CL_TRUE, 0, n_t3 * sizeof(cl_float), cpu_phasor, 0, NULL, NULL);

	cl_long4 * cpu_bsref = new cl_long4[n_t3];
	err = clEnqueueReadBuffer(mQueue, data_bsref, CL_TRUE, 0, n_t3 * sizeof(cl_long4), cpu_bsref, 0, NULL, NULL);

	cl_short4 * cpu_sign = new cl_short4[n_t3];
	err = clEnqueueReadBuffer(mQueue, data_sign, CL_TRUE, 0, n_t3 * sizeof(cl_short4), cpu_sign, 0, NULL, NULL);

	// Lastly pull in the output, don't forget to offset the read (output is a big cl_float of vis2 followed by t3's)
	cl_float2 * cl_output = new cl_float2[n_t3];
	err = clEnqueueReadBuffer(mQueue, output, CL_TRUE, n_v2 * sizeof(cl_float), n_t3 * sizeof(cl_float2), cl_output, 0, NULL, NULL);

	// Compute the T3, output the difference between CPU and GPU versions:
	printf("T3 Buffer elements: (CPU, OpenCL, Diff)\n");
	complex<float> V_ab;
	complex<float> V_bc;
	complex<float> V_ca;
	complex<float> phi;
	complex<float> T3;
	cl_long4 uvpoint;
	cl_short4 sign;
	for(int i = 0; i < n_t3; i++)
	{
		uvpoint = cpu_bsref[i];
	    sign = cpu_sign[i];
		// Look up the visibility values, conjugating as necessary:
		V_ab = complex<float>(cpu_dft[uvpoint.s0].s0, cpu_dft[uvpoint.s0].s1 * sign.s0);
		V_bc = complex<float>(cpu_dft[uvpoint.s1].s0, cpu_dft[uvpoint.s1].s1 * sign.s1);
		V_ca = complex<float>(cpu_dft[uvpoint.s2].s0, cpu_dft[uvpoint.s2].s1 * sign.s2);
		phi = complex<float>(cpu_phasor[i].s0, cpu_phasor[i].s1);

		T3 = V_ab * V_bc * V_ca * phi;

//		printf("\t%i Re: (%f %f %e) Im: (%f %f %e)\n", i,
//				T3.real(), cl_output[i].s0, T3.real() - cl_output[i].s0,
//				T3.imag(), cl_output[i].s1, T3.imag()- cl_output[i].s1);

		printf("\t%i Re: (%f %f %e) Im: (%f %f %e)\n", i,
				0.0, cl_output[i].s0, 0.0,
				0.0, cl_output[i].s1, 0.0);
	}

	// Free memory:
	delete[] cpu_dft;
	delete[] cpu_phasor;
	delete[] cpu_bsref;
	delete[] cpu_sign;
	delete[] cl_output;
}
