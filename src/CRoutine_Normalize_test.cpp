/*
 * CRoutine_Normalize_test.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.h"
#include "CRoutine_Normalize.h"

extern string LIBOI_KERNEL_PATH;

/// Checks that the CPU normalization routine is working correctly.
TEST(CRoutine_Normalize, CPU_Normalize)
{
	unsigned int test_size = 10000;

	valarray<float> cpu_val(test_size);
	for(int i = 0; i < cpu_val.size(); i++)
		cpu_val[i] = i;

	CRoutine_Normalize::Normalize(cpu_val, cpu_val.size());
	float post_norm_sum = CRoutine_Sum::Sum(cpu_val);

	EXPECT_NEAR(post_norm_sum, 1.0, MAX_REL_ERROR);
}

/// Verifies that the CPU and OpenCL normalization routines are working similarly.
TEST(CRoutine_Normalize, CL_Normalize)
{
	unsigned int test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_Normalize r_norm(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_norm.SetSourcePath(LIBOI_KERNEL_PATH);
	r_norm.Init();

	valarray<cl_float> cpu_val(test_size);
	valarray<cl_float> cl_val(test_size);
	for(int i = 0; i < cpu_val.size(); i++)
		cpu_val[i] = i;

	// Calculate the divisor
	cl_float divisor = CRoutine_Sum::Sum(cpu_val);

	// Create buffers
	int err = CL_SUCCESS;
	cl_mem input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	cl_mem divisor_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float), NULL, NULL);

	// Fill the input buffer
    err = clEnqueueWriteBuffer(cl.GetQueue(), input_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &cpu_val[0], 0, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), divisor_cl, CL_TRUE, 0, sizeof(cl_float), &divisor, 0, NULL, NULL);

    // Normalize on the OpenCL device, do the same on the CPU:
	r_norm.Normalize(input_cl, test_size, divisor_cl);
	CRoutine_Normalize::Normalize(cpu_val, cpu_val.size());

	// Read back the results.
	err = clEnqueueReadBuffer(cl.GetQueue(), input_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &cl_val[0], 0, NULL, NULL);

	// Free buffers
	clReleaseMemObject(input_cl);
	clReleaseMemObject(divisor_cl);

	// Check the results.
	for(int i = 0; i < test_size; i++)
		EXPECT_NEAR(float(cpu_val[i]), float(cl_val[i]), MAX_REL_ERROR) << " at index " << i;
}
