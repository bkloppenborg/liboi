/*
 * CRoutine_Zero_test.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_Zero.h"

using namespace liboi;

extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

TEST(CRoutine_Zero, CL_ZeroBuffer)
{
	size_t test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_Zero r_zero(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_zero.SetSourcePath(LIBOI_KERNEL_PATH);
	r_zero.Init();

	valarray<cl_float> data(test_size);
	for(size_t i = 0; i < data.size(); i++)
		data[i] = i;

	// Create buffers
	int status = CL_SUCCESS;
	cl_mem input_buffer = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.");
	// Fill the input buffer
	status = clEnqueueWriteBuffer(cl.GetQueue(), input_buffer, CL_TRUE, 0, sizeof(cl_float) * test_size, &data[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.");

	r_zero.Zero(input_buffer, test_size);

	// Read back the results.
	status = clEnqueueReadBuffer(cl.GetQueue(), input_buffer, CL_TRUE, 0, sizeof(cl_float) * test_size, &data[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");

	// Free buffers
	clReleaseMemObject(input_buffer);

	// Check the results.
	for(size_t i = 0; i < test_size; i++)
		EXPECT_EQ(0, data[i]);
}
