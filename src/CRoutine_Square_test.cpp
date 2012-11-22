/*
 * CRoutine_Square_test.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.h"
#include "CRoutine_Square.h"

extern string LIBOI_KERNEL_PATH;

TEST(CRoutine_Square, SquareBuffer)
{
	unsigned int test_size = 10000;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_Square r_square(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r_square.SetSourcePath(LIBOI_KERNEL_PATH);
	r_square.Init();

	valarray<cl_float> input(test_size);
	valarray<cl_float> cpu_val(test_size);
	valarray<cl_float> cl_val(test_size);
	for(int i = 0; i < cpu_val.size(); i++)
		input[i] = i;

	// Create buffers
	int err = CL_SUCCESS;
	cl_mem input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	cl_mem output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);

	// Fill the input buffer
    err = clEnqueueWriteBuffer(cl.GetQueue(), input_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &input[0], 0, NULL, NULL);

    // Normalize on the OpenCL device, do the same on the CPU:
    r_square.Square(input_cl, output_cl, test_size, test_size);
    r_square.Square(input, cpu_val, input.size(), input.size());

	// Read back the results.
	err = clEnqueueReadBuffer(cl.GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &cl_val[0], 0, NULL, NULL);

	// Free OpencL buffers
	clReleaseMemObject(input_cl);
	clReleaseMemObject(output_cl);

	// Check the results.
	for(int i = 0; i < test_size; i++)
		EXPECT_NEAR(float(cpu_val[i]), float(cl_val[i]), MAX_REL_ERROR) << " at index " << i;
}
