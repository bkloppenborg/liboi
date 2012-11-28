/*
 * CRoutine_Zero_test.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.h"
#include "CRoutine_Zero.h"

extern string LIBOI_KERNEL_PATH;

TEST(CRoutine_Zero, CL_ZeroBuffer)
{
	unsigned int test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_Zero r_zero(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_zero.SetSourcePath(LIBOI_KERNEL_PATH);
	r_zero.Init();

	valarray<cl_float> data(test_size);
	for(int i = 0; i < data.size(); i++)
		data[i] = i;

	// Create buffers
	int err = CL_SUCCESS;
	cl_mem input_buffer = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	// Fill the input buffer
    err = clEnqueueWriteBuffer(cl.GetQueue(), input_buffer, CL_TRUE, 0, sizeof(cl_float) * test_size, &data[0], 0, NULL, NULL);

	r_zero.Zero(input_buffer, test_size);

	// Read back the results.
	err = clEnqueueReadBuffer(cl.GetQueue(), input_buffer, CL_TRUE, 0, sizeof(cl_float) * test_size, &data[0], 0, NULL, NULL);

	// Free buffers
	clReleaseMemObject(input_buffer);

	// Check the results.
	for(int i = 0; i < test_size; i++)
		EXPECT_EQ(0, data[i]);
}




