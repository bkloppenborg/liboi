/*
 * CRoutine_Sum_test.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.h"
#include "CRoutine_Sum.h"
#include "CRoutine_Zero.h"

extern string LIBOI_KERNEL_PATH;

/// Checks the Kahan summation
TEST(CRoutine_Sum, Sum_Kahan)
{
	valarray<double> d(100);
	for(int i = 0; i < d.size(); i++)
		d[i] = i+1;

	double sum = CRoutine_Sum::Sum(d);

    EXPECT_EQ(5050, sum);
}

/// Checks that the sum on the OpenCL device and CPU match
TEST(CRoutine_Sum, CL_CPU_Match)
{
	unsigned int test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_Zero r_zero(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_zero.SetSourcePath(LIBOI_KERNEL_PATH);
	r_zero.Init();
	CRoutine_Sum r_sum(cl.GetDevice(), cl.GetContext(), cl.GetQueue(), &r_zero);
	r_sum.SetSourcePath(LIBOI_KERNEL_PATH);
	r_sum.Init(test_size);

	valarray<cl_float> data(test_size);
	for(int i = 0; i < data.size(); i++)
		data[i] = i;

	// Create buffers
	int err = CL_SUCCESS;
	cl_mem input_buffer = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	cl_mem final_buffer = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	// Fill the input buffer (it doesn't matter what is in the output buffer)
    err= clEnqueueWriteBuffer(cl.GetQueue(), input_buffer, CL_TRUE, 0, sizeof(cl_float) * test_size, &data[0], 0, NULL, NULL);

	cl_float cpu_sum = CRoutine_Sum::Sum(data);
	float cl_sum = r_sum.ComputeSum(input_buffer, final_buffer, true);

	// Free buffers
	clReleaseMemObject(input_buffer);
	clReleaseMemObject(final_buffer);

	EXPECT_EQ(cpu_sum, cl_sum);
}
