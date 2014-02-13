/*
 * CRoutine_DFT_test.cpp
 *
 *  Created on: Nov 23, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_DFT.h"
#include "CPointSource.h"
#include "CUniformDisk.h"

using namespace liboi;
extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

/// Checks that the CPU algorithm can replicate a point source DFT
TEST(CRoutine_DFT, CPU_PointSource)
{
	// Create a (normalized) image with a point source at the center:
	unsigned int image_width = 128;
	unsigned int image_height = 128;
	unsigned int image_size = image_width * image_height;
	float image_scale = 0.025; // mas/pixel
	unsigned int n_uv = 10;

	// Create the model
	CPointSource model(image_width, image_height, image_scale);

	// Get UV points, the image, and init an output buffer:
	valarray<cl_float2> uv_points = model.GenerateUVSpiral_CL(n_uv);
	valarray<cl_float> image = model.GetImage_CL();
	valarray<cl_float2> cpu_output(n_uv);

	// Init an CRoutine_DFT object (we aren't using the OpenCL functionality, but we still init it)
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_DFT r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init(image_scale);

	r.FT(uv_points, n_uv, image, image_width, image_height, image_scale, cpu_output);

	// Now run the checks
	cl_float2 theory_val;
	for(int i = 0; i < n_uv; i++)
	{
		theory_val = model.GetVis_CL(uv_points[i]);
		EXPECT_FLOAT_EQ(theory_val.s0, cpu_output[i].s0);	// real
		EXPECT_FLOAT_EQ(theory_val.s1, cpu_output[i].s1);	// imaginary
	}
}

// Checks that the DFT routine replicates the DFT for a point source.
// Computations performed on the CPU
TEST(CRoutine_DFT, UniformDisk_CPU)
{
	// Create a (normalized) image with a point source at the center:
	unsigned int image_width = 128;
	unsigned int image_height = 128;
	unsigned int image_size = image_width * image_height;
	float image_scale = 0.025; // mas/pixel
	unsigned int n_uv = 10;

	// Create the model
	CUniformDisk model(image_width, image_height, image_scale);

	// Get UV points, the image, and init an output buffer:
	valarray<cl_float2> uv_points = model.GenerateUVSpiral_CL(n_uv);
	valarray<cl_float> image = model.GetImage_CL();
	valarray<cl_float2> cpu_output(n_uv);

	// Init an CRoutine_DFT object (we aren't using the OpenCL functionality, but we still init it)
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_DFT r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init(image_scale);

	r.FT(uv_points, n_uv, image, image_width, image_height, image_scale, cpu_output);

	// Now run the checks
	cl_float2 theory_val;
	for(int i = 0; i < n_uv; i++)
	{
		theory_val = model.GetVis_CL(uv_points[i]);
		EXPECT_FLOAT_EQ(theory_val.s0, cpu_output[i].s0);
		EXPECT_FLOAT_EQ(theory_val.s1, cpu_output[i].s1);
	}
}

/// Checks that the DFT routine replicates the DFT for a point source.
/// Computations performed on an OpenCL device
//TEST(CRoutine_DFT, CL_PointSource)
//{
//	// First
//
//
//	unsigned int test_size = 10000;
//
//	// Init the OpenCL device and necessary routines:
//	COpenCL cl(OPENCL_DEVICE_TYPE);
//	CRoutine_Normalize r_norm(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
//	r_norm.SetSourcePath(LIBOI_KERNEL_PATH);
//	r_norm.Init();
//
//	valarray<cl_float> cpu_val(test_size);
//	valarray<cl_float> cl_val(test_size);
//	for(int i = 0; i < cpu_val.size(); i++)
//		cpu_val[i] = i;
//
//	// Calculate the divisor
//	cl_float divisor = CRoutine_Sum::Sum(cpu_val);
//
//	// Create buffers
//	int err = CL_SUCCESS;
//	cl_mem input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
//	cl_mem divisor_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float), NULL, NULL);
//
//	// Fill the input buffer
//    err = clEnqueueWriteBuffer(cl.GetQueue(), input_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &cpu_val[0], 0, NULL, NULL);
//    err = clEnqueueWriteBuffer(cl.GetQueue(), divisor_cl, CL_TRUE, 0, sizeof(cl_float), &divisor, 0, NULL, NULL);
//
//    // Normalize on the OpenCL device, do the same on the CPU:
//	r_norm.Normalize(input_cl, test_size, divisor_cl);
//	CRoutine_Normalize::Normalize(cpu_val, cpu_val.size());
//
//	// Read back the results.
//	err = clEnqueueReadBuffer(cl.GetQueue(), input_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &cl_val[0], 0, NULL, NULL);
//
//	// Free buffers
//	clReleaseMemObject(input_cl);
//	clReleaseMemObject(divisor_cl);
//
//	// Check the results.
//	for(int i = 0; i < test_size; i++)
//		EXPECT_NEAR(float(cpu_val[i]), float(cl_val[i]), MAX_REL_ERROR) << " at index " << i;
//}
