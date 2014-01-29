/*
 * CRoutine_Sum_test.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_Sum.h"
#include "CRoutine_Sum_AMD.h"
#include "CRoutine_Sum_NVidia.h"
#include "CRoutine_Zero.h"

using namespace liboi;

extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

/// Checks that the summation algorithm is working on the CPU side
TEST(CRoutine_Sum, CPU_Sum)
{
	// Check a few sums:

	// Here we test it simply using \sum_{i=1}{100}i = \frac{n(n+1)}{2}
	double sum;

	// 100 elements sums to 5050
	valarray<double> a(100);
	for(int i = 0; i < a.size(); i++)
		a[i] = i+1;
	sum = CRoutine_Sum::Sum(a);
    EXPECT_EQ(5050, sum);

    // 10000 elements sums to 50005000
	valarray<double> b(10000);
	for(int i = 0; i < b.size(); i++)
		b[i] = i+1;
	sum = CRoutine_Sum::Sum(b);
    EXPECT_EQ(50005000, sum);
}

/// Checks that the sum on the OpenCL device and CPU match
TEST(CRoutine_Sum_NVidia, CL_Sum_CPU_CHECK)
{
	unsigned int test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_Zero r_zero(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_zero.SetSourcePath(LIBOI_KERNEL_PATH);
	r_zero.Init();
	CRoutine_Sum_NVidia r_sum(cl.GetDevice(), cl.GetContext(), cl.GetQueue(), &r_zero);
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
	float cl_sum = r_sum.Sum(input_buffer);

	// Free buffers
	clReleaseMemObject(input_buffer);
	clReleaseMemObject(final_buffer);

	EXPECT_EQ(cpu_sum, cl_sum);
}

/// Checks that the sum on the OpenCL device and CPU match
TEST(CRoutine_Sum_AMD, CL_Sum_CPU_CHECK)
{
	unsigned int test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_Zero r_zero(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_zero.SetSourcePath(LIBOI_KERNEL_PATH);
	r_zero.Init();
	CRoutine_Sum_AMD r_sum(cl.GetDevice(), cl.GetContext(), cl.GetQueue(), &r_zero);
	r_sum.SetSourcePath(LIBOI_KERNEL_PATH);
	r_sum.Init(test_size);

	valarray<cl_float> data(test_size);
	for(int i = 0; i < data.size(); i++)
		data[i] = i;

	// Create buffers
	int err = CL_SUCCESS;
	cl_mem input_buffer = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, &err);
	cl_mem final_buffer = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, &err);
	// Fill the input buffer (it doesn't matter what is in the output buffer)
    err = clEnqueueWriteBuffer(cl.GetQueue(), input_buffer, CL_TRUE, 0, sizeof(cl_float) * test_size, &data[0], 0, NULL, NULL);

	cl_float cpu_sum = CRoutine_Sum::Sum(data);
	float cl_sum = r_sum.Sum(input_buffer);

	// Free buffers
	clReleaseMemObject(input_buffer);
	clReleaseMemObject(final_buffer);

	EXPECT_EQ(cpu_sum, cl_sum);
}
