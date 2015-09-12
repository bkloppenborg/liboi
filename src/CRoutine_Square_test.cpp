/*
 * CRoutine_Square_test.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_Square.h"

using namespace liboi;

extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

TEST(CRoutine_Square, CPU_Square)
{
	unsigned int test_size = 8;
	valarray<cl_float> input(test_size);
	valarray<cl_float> output(test_size);

	input[0] = 2;
	input[1] = 4;
	input[2] = 8;
	input[3] = 16;
	input[4] = 32;
	input[5] = 64;
	input[6] = 128;
	input[7] = 256;

    CRoutine_Square::Square(input, output, input.size(), input.size());

    EXPECT_EQ(output[0], 4);
    EXPECT_EQ(output[1], 16);
    EXPECT_EQ(output[2], 64);
    EXPECT_EQ(output[3], 256);
    EXPECT_EQ(output[4], 1024);
    EXPECT_EQ(output[5], 4096);
    EXPECT_EQ(output[6], 16384);
    EXPECT_EQ(output[7], 65536);
}

TEST(CRoutine_Square, CL_Square)
{
	size_t test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_Square r_square(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_square.SetSourcePath(LIBOI_KERNEL_PATH);
	r_square.Init();

	valarray<cl_float> input(test_size);
	valarray<cl_float> cpu_val(test_size);
	valarray<cl_float> cl_val(test_size);
	for(size_t i = 0; i < test_size; i++)
		input[i] = i;

	// Create buffers
	int err = CL_SUCCESS;
	cl_mem input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	cl_mem output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	CHECK_ERROR(err, CL_SUCCESS, "clCreateBuffer Failed");

	// Fill the input buffer
    err = clEnqueueWriteBuffer(cl.GetQueue(), input_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &input[0], 0, NULL, NULL);
	CHECK_ERROR(err, CL_SUCCESS, "clEnqueueWriteBuffer Failed");

    // Normalize on the OpenCL device, do the same on the CPU:
    r_square.Square(input_cl, output_cl, test_size, test_size);
    r_square.Square(input, cpu_val, input.size(), input.size());

	// Read back the results.
	err = clEnqueueReadBuffer(cl.GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &cl_val[0], 0, NULL, NULL);
	CHECK_ERROR(err, CL_SUCCESS, "clEnqueueReadBuffer Failed");

	// Free OpencL buffers
	clReleaseMemObject(input_cl);
	clReleaseMemObject(output_cl);

	// Check the results.
	for(size_t i = 0; i < test_size; i++)
		EXPECT_NEAR(float(cpu_val[i]), float(cl_val[i]), MAX_REL_ERROR) << " at index " << i;
}
