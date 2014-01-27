/*
 * CRoutine_FTtoV2_test.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include <algorithm>

#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_FTtoV2.h"
#include "CPointSource.h"

using namespace std;

using namespace liboi;

extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

/// Checks that the CPU routine functions correctly.
TEST(CRoutine_FTtoV2, CPU_PointSource)
{
	unsigned int test_size = 10000;

	// Generate a point source model, get input data from it.
	CPointSource pnt(128, 128, 0.025);
	valarray<cl_float2> uv_points = pnt.GenerateUVSpiral_CL(test_size);
	valarray<cl_float2> ft_input = pnt.GetVis_CL(uv_points);

	// Create linear indexing
	valarray<cl_uint> uv_ref(test_size);
	for(int i = 0; i < test_size; i++)
		uv_ref[i] = i;

	// Allocate place to store the output
	valarray<cl_float> output(test_size);

	// Run the CPU routine, get results from the model.
	CRoutine_FTtoV2::FTtoV2(ft_input, uv_ref, output, test_size);
	valarray<cl_float> model_out = pnt.GetV2_CL(uv_points);

	// Now check the results:
	for(unsigned int i = 0; i < test_size; i++)
	{
		EXPECT_NEAR(output[i], model_out[i], MAX_REL_ERROR * model_out[i]);
	}
}

/// Checks that the OpenCL routine functions correctly.
TEST(CRoutine_FTtoV2, CL_PointSource)
{
	unsigned int test_size = 10000;

	// Generate a point source model, get input data from it.
	CPointSource pnt(128, 128, 0.025);
	valarray<cl_float2> uv_points = pnt.GenerateUVSpiral_CL(test_size);
	valarray<cl_float2> ft_input = pnt.GetVis_CL(uv_points);
	valarray<cl_float> model_out = pnt.GetV2_CL(uv_points);

	// Init the OpenCL device and necessary routines:
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_FTtoV2 r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init();

	// Create linear indexing
	valarray<cl_uint> uv_ref(test_size);
	for(int i = 0; i < test_size; i++)
		uv_ref[i] = i;

	// Allocate memory on the OpenCL device and copy things over.
	int err = CL_SUCCESS;
	// FT input
	cl_mem ft_input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * test_size, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), ft_input_cl, CL_FALSE, 0, sizeof(cl_float2) * test_size, &ft_input[0], 0, NULL, NULL);
	// UV references
	cl_mem uv_ref_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_uint) * test_size, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), uv_ref_cl, CL_FALSE, 0, sizeof(cl_uint) * test_size, &uv_ref[0], 0, NULL, NULL);
    // Output bufer
	cl_mem output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);

	// Wait for memory transfers to finish.
	clFinish(cl.GetQueue());

	// Run the OpenCL routine
	r.FTtoV2(ft_input_cl, uv_ref_cl, output_cl, 0, test_size);

	// Copy back the buffer
	valarray<cl_float> output(test_size);
	err = clEnqueueReadBuffer(cl.GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &output[0], 0, NULL, NULL);

	// Free OpenCL memory
	clReleaseMemObject(ft_input_cl);
	clReleaseMemObject(uv_ref_cl);
	clReleaseMemObject(output_cl);

	// Now check the results:
	for(unsigned int i = 0; i < test_size; i++)
	{
		EXPECT_NEAR(output[i], model_out[i], MAX_REL_ERROR * model_out[i]);
	}
}
