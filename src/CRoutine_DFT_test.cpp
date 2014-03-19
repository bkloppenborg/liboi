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
	float max_vis_error = 1E-5;
	float max_percent_error = 0;
	float one_degree = 1.0 / 360 * PI;
	for(int i = 0; i < n_uv; i++)
	{
		theory_val = model.GetVis_CL(uv_points[i]);
		max_percent_error = abs(max_vis_error * theory_val.s[0]);

		EXPECT_NEAR(theory_val.s[0], cpu_output[i].s[0], max_percent_error);
		EXPECT_NEAR(theory_val.s[1], cpu_output[i].s[1], one_degree);	// imaginary
	}
}

// Checks that the DFT routine replicates the DFT for a point source.
// Computations performed on the CPU
TEST(CRoutine_DFT, CPU_UniformDisk)
{
	// Create a (normalized) image with a point source at the center:
	unsigned int image_width = 1024;
	unsigned int image_height = 1024;
	unsigned int image_size = image_width * image_height;
	float image_scale = 0.025; // mas/pixel
	unsigned int n_uv = 10;
	float radius = float(image_width) / 2 * image_scale;

	// Create the model
	CUniformDisk model(image_width, image_height, image_scale, radius, 0, 0);
	valarray<double> image_temp = model.GetImage();
	model.WriteImage(image_temp, image_width, image_height, image_scale, "!model_cpu_uniform_disk.fits");

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
	float max_vis_error = 0.03;
	float three_percent_error = 0;
	float one_degree = 1.0 / 360 * PI;
	for(int i = 0; i < n_uv; i++)
	{
		theory_val = model.GetVis_CL(uv_points[i]);
		three_percent_error = abs(max_vis_error * theory_val.s[0]);

		EXPECT_NEAR(theory_val.s[0], cpu_output[i].s[0], three_percent_error);
		EXPECT_NEAR(theory_val.s[1], cpu_output[i].s[1], one_degree); // imaginary
	}
}

/// Checks that the OpenCL algorithm can replicate a point source DFT
TEST(CRoutine_DFT, CL_PointSource)
{
	int status = CL_SUCCESS;
	// Create a (normalized) image with a point source at the center:
	unsigned int image_width = 128;
	unsigned int image_height = 128;
	unsigned int image_size = image_width * image_height;
	float image_scale = 0.025; // mas/pixel
	unsigned int n_uv_points = 10;

	// Create the model
	CPointSource model(image_width, image_height, image_scale);

	// Get UV points, the image, and init an output buffer:
	valarray<cl_float2> uv_points = model.GenerateUVSpiral_CL(n_uv_points);
	valarray<cl_float> image = model.GetImage_CL();
	valarray<cl_float2> output(n_uv_points);

	// Init an CRoutine_DFT object (we aren't using the OpenCL functionality, but we still init it)
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_DFT r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init(image_scale);

	// Create the OpenCL memory locations
	cl_mem uv_points_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * n_uv_points, NULL, &status);
	cl_mem image_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * image_size, NULL, &status);
	cl_mem output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * n_uv_points, NULL, &status);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.");

	// Copy the data to the buffer:
	status |= clEnqueueWriteBuffer(cl.GetQueue(), uv_points_cl, CL_TRUE, 0, sizeof(cl_float2) * uv_points.size(), &uv_points[0], 0, NULL, NULL);
	status |= clEnqueueWriteBuffer(cl.GetQueue(), image_cl, CL_TRUE, 0, sizeof(cl_float) * image.size(), &image[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.");

	// Run the DFT:
	r.FT(uv_points_cl, n_uv_points, image_cl, image_width, image_height, output_cl);

	// Copy back the results
	status = clEnqueueReadBuffer(cl.GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float2) * n_uv_points, &output[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");

	// Now run the checks
	cl_float2 theory_val;
	float max_vis_error = 0.03;
	float three_percent_error = 0;
	// Permit up to 1% error between the DFT and theoretical phases.
	float one_degree = 1.0 / 360 * PI;
	for(unsigned int i = 0; i < n_uv_points; i++)
	{
		theory_val = model.GetVis_CL(uv_points[i]);

		// Permit up to 3% error between the DFT and theoretical visibility amplitudes
		three_percent_error = abs(max_vis_error * theory_val.s[0]);

		EXPECT_NEAR(theory_val.s[0], output[i].s[0], three_percent_error);	// real
		EXPECT_NEAR(theory_val.s[1], output[i].s[1], one_degree);	// imaginary
	}
}


/// Checks that the OpenCL algorithm can replicate a point source DFT
TEST(CRoutine_DFT, CL_UniformDisk)
{
	int status = CL_SUCCESS;
	// Create a (normalized) image with a point source at the center:
	unsigned int image_width = 1024;
	unsigned int image_height = 1024;
	unsigned int image_size = image_width * image_height;
	float image_scale = 0.025; // mas/pixel
	unsigned int n_uv_points = 10;
	float radius = float(image_width) / 2 * image_scale;

	// Create the model
	CUniformDisk model(image_width, image_height, image_scale, radius, 0, 0);
	valarray<double> image_temp = model.GetImage();
	model.WriteImage(image_temp, image_width, image_height, image_scale, "!model_cl_uniform_disk.fits");

	// Get UV points, the image, and init an output buffer:
	valarray<cl_float2> uv_points = model.GenerateUVSpiral_CL(n_uv_points);
	valarray<cl_float> image = model.GetImage_CL();
	valarray<cl_float2> output(n_uv_points);

	// Init an CRoutine_DFT object (we aren't using the OpenCL functionality, but we still init it)
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_DFT r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init(image_scale);

	// Create the OpenCL memory locations
	cl_mem uv_points_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * n_uv_points, NULL, &status);
	cl_mem image_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * image_size, NULL, &status);
	cl_mem output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * n_uv_points, NULL, &status);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.");

	// Copy the data to the buffer:
	status |= clEnqueueWriteBuffer(cl.GetQueue(), uv_points_cl, CL_TRUE, 0, sizeof(cl_float2) * uv_points.size(), &uv_points[0], 0, NULL, NULL);
	status |= clEnqueueWriteBuffer(cl.GetQueue(), image_cl, CL_TRUE, 0, sizeof(cl_float) * image.size(), &image[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.");

	// Run the DFT:
	r.FT(uv_points_cl, n_uv_points, image_cl, image_width, image_height, output_cl);

	// Copy back the results
	status = clEnqueueReadBuffer(cl.GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float2) * n_uv_points, &output[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");

	// Now run the checks
	cl_float2 theory_val;
	float max_vis_error = 0.03;
	float three_percent_error = 0;
	// Permit up to 1% error between the DFT and theoretical phases.
	float one_degree = 1.0 / 360 * PI;
	for(unsigned int i = 0; i < n_uv_points; i++)
	{
		theory_val = model.GetVis_CL(uv_points[i]);

		// Permit up to 3% error between the DFT and theoretical visibility amplitudes
		three_percent_error = abs(max_vis_error * theory_val.s[0]);

		EXPECT_NEAR(theory_val.s[0], output[i].s[0], three_percent_error);	// real
		EXPECT_NEAR(theory_val.s[1], output[i].s[1], one_degree);	// imaginary
	}
}
