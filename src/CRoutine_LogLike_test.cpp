/*
 * CRoutine_LogLike_test.cpp
 *
 *  Created on: Dec 6, 2012
 *      Author: bkloppen
 */

#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_LogLike.h"
#include "CRoutine_Zero.h"

using namespace std;
using namespace liboi;

extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

///
TEST(LogLikeTest, CPU_LogLike_ZERO)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> chi_output(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> output(test_size);

	// Initalize the buffer to yield zero.
	for(size_t i = 0; i < test_size; i++)
	{
		chi_output = 0;
		data_err = 1;
	}

	// Run the loglike test.
	CRoutine_LogLike::LogLike(chi_output, data_err, output, test_size);

	// Compare results. Because data = model every chi element should be of unit magnitude
	for(size_t i = 0; i < test_size; i++)
	{
		// Check the real and imaginary chi values
		EXPECT_NEAR(fabs(output[i]), 0, MAX_REL_ERROR);
	}
}

///
TEST(LogLikeTest, CL_LogLike_ZERO)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> chi_output(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> output(test_size);

	// Initalize the buffer to yield zero.
	for(size_t i = 0; i < test_size; i++)
	{
		chi_output = 0;
		data_err = 1;
	}

	// Init OpenCL and the routine
	COpenCL cl(OPENCL_DEVICE_TYPE);
	CRoutine_Zero zero(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	zero.SetSourcePath(LIBOI_KERNEL_PATH);
	zero.Init();
	CRoutine_LogLike r(cl.GetDevice(), cl.GetContext(), cl.GetQueue(), &zero);
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init(test_size);

	// Make OpenCL buffers for the data, data_err, model, and output.
	cl_mem chi_output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	cl_mem data_err_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
	cl_mem output_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);

	// Fill the input buffers
	int err = CL_SUCCESS;
	err = clEnqueueWriteBuffer(cl.GetQueue(), chi_output_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &chi_output[0], 0, NULL, NULL);
	err = clEnqueueWriteBuffer(cl.GetQueue(), data_err_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &data_err[0], 0, NULL, NULL);
	CHECK_ERROR(err, CL_SUCCESS, "EnqueueWriteBuffer Failed");

	// Run the loglike test.
	r.LogLike(chi_output_cl, data_err_cl, output_cl, test_size);

	// Copy back the result
	err = clEnqueueReadBuffer(cl.GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &output[0], 0, NULL, NULL);
	CHECK_ERROR(err, CL_SUCCESS, "clEnqueueReadBuffer Failed");

	// Free OpenCL memory:
	if(chi_output_cl) clReleaseMemObject(chi_output_cl);
	if(data_err_cl) clReleaseMemObject(data_err_cl);
	if(output_cl) clReleaseMemObject(output_cl);

	// Compare results. Because data = model every chi element should be of unit magnitude
	for(size_t i = 0; i < test_size; i++)
	{
		// Check the real and imaginary chi values
		EXPECT_NEAR(fabs(output[i]), 0, MAX_REL_ERROR);
	}
}



