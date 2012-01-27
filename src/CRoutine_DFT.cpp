/*
 * CRoutine_DFT.cpp
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_DFT.h"
#include <complex>

using namespace std;

CRoutine_DFT::CRoutine_DFT(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_FT(device, context, queue)
{
	mImageScale = 0;
	// Specify the source location for the kernel.
	mSource.push_back("ft_dft2d.cl");
}

CRoutine_DFT::~CRoutine_DFT()
{
	// TODO Auto-generated destructor stub
}

void CRoutine_DFT::Init(float image_scale)
{
	mImageScale = image_scale;

	// Compile the image scale into the kernel.
	double RPMAS = (M_PI / 180.0) / 3600000.0; // Number of radians per milliarcsecond
	string source = ReadSource(mSource[0]);
	float arg = 2.0 * M_PI * RPMAS * mImageScale;
    stringstream tmp;

    // Insert macro definition for ARG (arg to 10 decimals in SI notation)
    tmp.str("");
    tmp << "#define ARG " << setprecision(10) << arg << "\n";
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
	// Compare the CPU and OpenCL computed DFT values.
	FT_CPU(uv_points, n_uv_points, image, image_width, image_height, image_flux, output);
#endif //DEBUG_VERBOSE
}

/// Computes the DFT on the CPU, then compares CPU and OpenCL output.
/// Note: This routine copies data back from the OpenCL device and should only be used for debugging purposes.
void CRoutine_DFT::FT_CPU(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output)
{
    int err = 0;
	double RPMAS = (M_PI / 180.0) / 3600000.0; // Number of radians per milliarcsecond

	// First pull down the UV points and image.
	cl_float2 * cpu_uvpts = new cl_float2[n_uv_points];
	err = clEnqueueReadBuffer(mQueue, uv_points, CL_TRUE, 0, n_uv_points * sizeof(cl_float2), cpu_uvpts, 0, NULL, NULL);
	cl_float * cpu_image = new cl_float[image_width * image_height];
	err = clEnqueueReadBuffer(mQueue, image, CL_TRUE, 0, image_width * image_height * sizeof(cl_float), cpu_image, 0, NULL, NULL);
	cl_float flux = 0;
	err = clEnqueueReadBuffer(mQueue, image_flux, CL_TRUE, 0, sizeof(cl_float), &flux, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy back DFT data, CRoutine_DFT.cpp", err);

	int uu = 0;
	int ii = 0;
	int jj = 0;

	// First create the DFT tables:
	int dft_size = n_uv_points * image_width;
	complex<float> * DFT_tablex = new complex<float>[dft_size];
	complex<float> * DFT_tabley = new complex<float>[dft_size];
	float tmp = 0;

	for (uu = 0; uu < n_uv_points; uu++)
	{
		for (ii = 0; ii < image_width; ii++)
		{
			tmp = 2.0 * PI * RPMAS * mImageScale * cpu_uvpts[uu].s0 * (float) ii;
			DFT_tablex[image_width * uu + ii] = complex<float>(cos(tmp), sin(tmp));
			tmp = -2.0 * PI * RPMAS * mImageScale * cpu_uvpts[uu].s1 * (float) ii;
			DFT_tabley[image_width * uu + ii] = complex<float>(cos(tmp), sin(tmp));
		}
	}

	// Now generate the FT values.
	complex<float> * visi = new complex<float>[n_uv_points];

	for(uu=0; uu < n_uv_points; uu++)
	{
		visi[uu] = complex<float>(0.0, 0.0);
		for(ii=0; ii < image_width; ii++)
		{
			for(jj=0; jj < image_width; jj++)
			{
				visi[uu] += cpu_image[ ii + image_width * jj ] * DFT_tablex[ image_width * uu + ii] * DFT_tabley[ image_width * uu + jj];
			}
		}
			if (flux > 0)
			{
				visi[uu] /= flux;
			}
	}

	// Lastly copy back the OpenCL values and compare them to what was computed above.

	// Copy back the input/output buffers.
	cl_float2 * cl_visi = new cl_float2[n_uv_points];
	err = clEnqueueReadBuffer(mQueue, output, CL_TRUE, 0, n_uv_points * sizeof(cl_float2), cl_visi, 0, NULL, NULL);

	printf("DFT Buffer elements: (CPU, OpenCL, Diff)\n");
	float real_cl = 0;
	float real_cpu = 0;
	float imag_cl = 0;;
	float imag_cpu = 0;
	for(int i = 0; i < n_uv_points; i++)
	{
		real_cl = cl_visi[i].s0;
		real_cpu = visi[i].real();
		imag_cl = cl_visi[i].s1;
		imag_cpu = visi[i].imag();
		printf("\t %i Re: (%f, %f, %e) Im: (%f, %f, %e)\n ", i, real_cpu, real_cl, real_cpu - real_cl, imag_cpu, imag_cl, imag_cpu - imag_cl);
	}

	// Free memory:
	delete[] cpu_image;
	delete[] cpu_uvpts;
	delete[] DFT_tablex;
	delete[] DFT_tabley;
	delete[] cl_visi;
}
