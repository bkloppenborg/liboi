/*
 * CRoutine_FTtoT3_test.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: bkloppen
 */

#include "gtest/gtest.h"
#include <algorithm>

#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_FTtoT3.h"
#include "CPointSource.h"

using namespace std;
using namespace liboi;

extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

/// Checks that the CPU routine functions correctly.
TEST(CRoutine_FTtoT3, CPU_PointSource)
{
	unsigned int test_size = 10000;

	unsigned int n_uv = 3 * test_size ;

	// Generate a point source model, get input data from it.
	CPointSource pnt(128, 128, 0.025);
	valarray<cl_float2> uv_points = pnt.GenerateUVSpiral_CL(n_uv);
	// GenerateUVSpiral doesn't make closed triangles, so we replace every third
	// UV point with the closed arm of a triplet.
	for(int i = 2; i < test_size; i += 3)
	{
		uv_points[i].s[0] = -1*(uv_points[i-2].s[0] + uv_points[i-1].s[0]);
		uv_points[i].s[1] = -1*(uv_points[i-2].s[1] + uv_points[i-1].s[1]);
	}

	valarray<cl_float2> ft_input = pnt.GetVis_CL(uv_points);

	// Create linear indexing for the T3 points
	valarray<cl_uint4> uv_ref(test_size);
	valarray<cl_short4> uv_sign(test_size);
	for(int i = 0; i < test_size; i++)
	{
		// Create the UV references
		uv_ref[i].s[0] = 3*i;
		uv_ref[i].s[1] = 3*i+1;
		uv_ref[i].s[2] = 3*i+2;
		uv_ref[i].s[3] = 0;

		// Create the T3 signs
		uv_sign[i].s[0] = 1;
		uv_sign[i].s[1] = 1;
		uv_sign[i].s[2] = 1;	// The conjugation above ensures the triangle is closed.
		uv_sign[i].s[3] = 0;
	}

	// Allocate place to store the output
	// Output data is stored as two cl_floats
	valarray<cl_float> output;

	// Run the CPU routine, get results from the model.
	CRoutine_FTtoT3::FTtoT3(ft_input, uv_ref, uv_sign, output);
	valarray<cl_float2> model_out = pnt.GetT3_CL(uv_points, uv_ref);

	// Now check the results:
	// Odd indexing is to conform with the format specified in COILibData.h for the output data.
	//  [t3_amp_0, ..., t3_amp_n, t3_phi_0, ..., t3_phi_n]
	for(unsigned int i = 0; i < test_size; i++)
	{
		EXPECT_NEAR(output[i], model_out[i].s[0], MAX_REL_ERROR * model_out[i].s[0]);
		EXPECT_NEAR(output[test_size + i], model_out[i].s[1], MAX_REL_ERROR * model_out[i].s[1]);
	}
}

/// Checks that the OpenCL routine functions correctly.
TEST(CRoutine_FTtoT3, CL_PointSource)
{
	unsigned int test_size = 10000;

	unsigned int n_uv = 3 * test_size ;

	// Generate a point source model, get input data from it.
	CPointSource pnt(128, 128, 0.025);
	valarray<cl_float2> uv_points = pnt.GenerateUVSpiral_CL(n_uv);
	// GenerateUVSpiral doesn't make closed triangles, so we replace every third
	// UV point with the closed arm of a triplet.
	for(int i = 2; i < test_size; i += 3)
	{
		uv_points[i].s[0] = -1*(uv_points[i-2].s[0] + uv_points[i-1].s[0]);
		uv_points[i].s[1] = -1*(uv_points[i-2].s[1] + uv_points[i-1].s[1]);
	}

	valarray<cl_float2> ft_input = pnt.GetVis_CL(uv_points);

	// Create linear indexing for the T3 points
	valarray<cl_uint4> uv_ref(test_size);
	valarray<cl_short4> uv_sign(test_size);
	for(int i = 0; i < test_size; i++)
	{
		// Create the UV references
		uv_ref[i].s[0] = 3*i;
		uv_ref[i].s[1] = 3*i+1;
		uv_ref[i].s[2] = 3*i+2;
		uv_ref[i].s[3] = 0;

		// Create the T3 signs
		uv_sign[i].s[0] = 1;
		uv_sign[i].s[1] = 1;
		uv_sign[i].s[2] = 1;	// The conjugation above ensures the triangle is closed.
		uv_sign[i].s[3] = 0;
	}

	// Init the OpenCL device and necessary routines:
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_FTtoT3 r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init();

	// Allocate memory on the OpenCL device and copy things over.
	int err = CL_SUCCESS;
	// FT input
	cl_mem ft_input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * n_uv, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), ft_input_cl, CL_FALSE, 0, sizeof(cl_float2) * n_uv, &ft_input[0], 0, NULL, NULL);
	// UV references
	cl_mem uv_ref_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_uint4) * test_size, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), uv_ref_cl, CL_FALSE, 0, sizeof(cl_uint4) * test_size, &uv_ref[0], 0, NULL, NULL);
	// UV signs
	cl_mem uv_sign_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_short4) * test_size, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), uv_sign_cl, CL_FALSE, 0, sizeof(cl_short4) * test_size, &uv_sign[0], 0, NULL, NULL);
    // Output buffer (remember, we write out to a cl_float, not cl_float 2 so we need twice the storage)
	cl_mem output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * 2 * test_size, NULL, NULL);

	// Wait for memory transfers to finish.
	clFinish(cl.GetQueue());

	// Run the CPU routine, get results from the model.
	r.FTtoT3(ft_input_cl, uv_ref_cl, uv_sign_cl, output_cl, 0, 0, test_size);
	valarray<cl_float2> model_out = pnt.GetT3_CL(uv_points, uv_ref);

	// Copy back the results
	valarray<cl_float> output(2 * test_size);
	err = clEnqueueReadBuffer(cl.GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float) * 2 * test_size, &output[0], 0, NULL, NULL);

	// Free OpenCL memory
	clReleaseMemObject(ft_input_cl);
	clReleaseMemObject(uv_ref_cl);
	clReleaseMemObject(uv_sign_cl);
	clReleaseMemObject(output_cl);

	// Now check the results:
	// Odd indexing is to conform with the format specified in COILibData.h for the output data.
	//  [t3_amp_0, ..., t3_amp_n, t3_phi_0, ..., t3_phi_n]
	for(unsigned int i = 0; i < test_size; i++)
	{
		EXPECT_NEAR(output[i], model_out[i].s[0], MAX_REL_ERROR * model_out[i].s[0]) << "Amplitude calculation error.";
		EXPECT_NEAR(output[test_size + i], model_out[i].s[1], MAX_REL_ERROR * model_out[i].s[1]) << "Phase calculation error.";
	}
}
