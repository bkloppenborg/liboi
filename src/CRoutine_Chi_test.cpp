/*
 * CRoutine_Chi_test.cpp
 *
 *  Created on: Nov 27, 2012
 *      Author: bkloppen
 */

#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_Chi.h"
#include "CRoutine_Zero.h"
#include "CRoutine_Square.h"
#include "CModel.h"

#include <cmath>

using namespace std;
using namespace liboi;

extern string LIBOI_KERNEL_PATH;
extern cl_device_type OPENCL_DEVICE_TYPE;

class ChiTest : public testing::Test
{
protected:
	COpenCL * cl;
	CRoutine_Zero * zero;
	CRoutine_Square * square;
	CRoutine_Chi * r;

	cl_mem data_cl;
	cl_mem data_err_cl;
	cl_mem model_cl;
	cl_mem output_cl;

	cl_float amp_err;
	cl_float phi_err;

	/// Creates a buffer where data[i] = model[i], thus the chi values should always evaluate to zero.
	void MakeChiZeroBuffers(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model, valarray<cl_float> & output, unsigned int test_size)
	{
		size_t n = 2*test_size;
		// Create buffers
		data.resize(n);
		data_err.resize(n);
		model.resize(n);
		output.resize(n);

		// Note, we generate one more value than requested because GenerateUVSpiral_CL
		// Creates the first element with a value near zero.
		valarray<cl_float2> temp = CModel::GenerateUVSpiral_CL(test_size + 1);

		// Set data = model to produce a zero chi result.
		// The temp[i + 1] indexing is due to GenerateUVSpiral_CL creating a value
		// near zero in the first element.
		for(size_t i = 0; i < test_size; i++)
		{
			data[i] = temp[i + 1].s[0];
			data[test_size + i] = temp[i + 1].s[1];

			model[i] = temp[i + 1].s[0];
			model[test_size + i] = temp[i + 1].s[1];

			// 1% error on amplitudes, 10% error on phases
			data_err[i] = amp_err * data[i];
			data_err[test_size + i] = phi_err * data[i];
		}
	}

	/// Creates a buffer in which model is one-sigma away from model, thus all chi elements should evaluate to 1.
	void MakeChiOneBuffers(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model, valarray<cl_float> & output, unsigned int test_size)
	{
		size_t n = 2*test_size;
		// Create buffers
		data.resize(n);
		data_err.resize(n);
		model.resize(n);
		output.resize(n);

		// Note, we generate one more value than requested because GenerateUVSpiral_CL
		// Creates the first element with a value near zero.
		valarray<cl_float2> t_data = CModel::GenerateUVSpiral_CL(test_size + 1);

		// Set data = model to produce a zero chi result.
		// The temp[i + 1] indexing is due to GenerateUVSpiral_CL creating a value
		// near zero in the first element.
		for(size_t i = 0; i < test_size; i++)
		{
			complex<float> c_data(t_data[i + 1].s[0], t_data[i + 1].s[1]);

			// Data are stored as amplitude and phase.
			data[i] = abs(c_data);
			data[test_size + i] = arg(c_data);

			// Avoid any cases where the data yield chi=0.
			if(fabs(data[i]) < amp_err)
				model[i] = amp_err;
			else
				model[i] = abs(c_data) * (1 + amp_err);

			if(fabs(data[test_size + i]) < phi_err)
				model[test_size + i] = copysign(phi_err, data[test_size + i]);
			else
				model[test_size + i] = arg(c_data) * (1 + phi_err);

			// Set the errors, avoid zero error
			data_err[i] = max(fabs(amp_err * data[i]), amp_err);
			data_err[test_size + i] = max(fabs(phi_err * data[test_size + i]), phi_err);
		}
	}

    void ReadCLResult(valarray<cl_float> & output)
    {
    	size_t test_size = output.size();
		int err = CL_SUCCESS;
		err = clEnqueueReadBuffer(cl->GetQueue(), output_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &output[0], 0, NULL, NULL);
		CHECK_ERROR(err, CL_SUCCESS, "clEnqueueReadBuffer Failed");
    }

	virtual void SetUp()
	{
		// Init OpenCL routines and memory to null/zero.
		cl = NULL;
		zero = NULL;
		square = NULL;
		r = NULL;
		data_cl = 0;
		data_err_cl = 0;
		model_cl = 0;
		output_cl = 0;

		// set the errors
		amp_err = 0.01;
		phi_err = 0.1;
	}

	void SetUpCL(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model)
	{
		unsigned int test_size = data.size();

		assert(data_err.size() == test_size);
		assert(model.size() == test_size);

		// Init OpenCL and the routine
		cl = new COpenCL(OPENCL_DEVICE_TYPE);
		zero = new CRoutine_Zero(cl->GetDevice(), cl->GetContext(), cl->GetQueue());
		zero->SetSourcePath(LIBOI_KERNEL_PATH);
		zero->Init();
		square = new CRoutine_Square(cl->GetDevice(), cl->GetContext(), cl->GetQueue());
		square->SetSourcePath(LIBOI_KERNEL_PATH);
		square->Init();
		r = new CRoutine_Chi(cl->GetDevice(), cl->GetContext(), cl->GetQueue(), zero, square);
		r->SetSourcePath(LIBOI_KERNEL_PATH);
		r->Init(test_size);

		// Make OpenCL buffers for the data, data_err, model, and output.
		data_cl = clCreateBuffer(cl->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
		data_err_cl = clCreateBuffer(cl->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
		model_cl = clCreateBuffer(cl->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);
		output_cl = clCreateBuffer(cl->GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);

		// Fill the input buffer
		int err = CL_SUCCESS;
		err = clEnqueueWriteBuffer(cl->GetQueue(), data_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &data[0], 0, NULL, NULL);
		err = clEnqueueWriteBuffer(cl->GetQueue(), data_err_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &data_err[0], 0, NULL, NULL);
		err = clEnqueueWriteBuffer(cl->GetQueue(), model_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &model[0], 0, NULL, NULL);
		CHECK_ERROR(err, CL_SUCCESS, "clEnqueueWriteBuffer Failed");
	}

	template <typename T> int sign(T val) {
	    return (T(0) < val) - (val < T(0));
	}

	void TearDown()
	{
		// Free memory objects if they have been allocated
		if(data_cl) clReleaseMemObject(data_cl);
		if(data_err_cl) clReleaseMemObject(data_err_cl);
		if(model_cl) clReleaseMemObject(model_cl);
		if(output_cl) clReleaseMemObject(output_cl);

		// Free routines if they have been allocated.
		delete r;
		delete square;
		delete zero;
		delete cl;
	}

};

/// Checks that the CPU chi algorithm evaluates to zero
/// when data and model are equal.
TEST_F(ChiTest, CPU_Chi_ZERO)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiZeroBuffers(data, data_err, model, output, test_size);

	// Run the chi algorithm
	CRoutine_Chi::Chi(data, data_err, model, 0, test_size, output);

	// Compare results. Because data = model every chi element should be zero
	for(size_t i = 0; i < test_size; i++)
		EXPECT_EQ(output[i], 0);
}

/// Tests that the CPU Chi algorithm evaluates to one when data and model differ
/// by one standard deviation.
TEST_F(ChiTest, CPU_Chi_ONE)
{
	size_t test_size = 10000;
	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	// Run the chi algorithm
	CRoutine_Chi::Chi(data, data_err, model, 0, test_size, output);

	// Compare results. Because data = model every chi element should be of unit magnitude
	for(size_t i = 0; i < test_size; i++)
		EXPECT_NEAR(fabs(output[i]), 1.0, MAX_REL_ERROR);
}

/// Tests the CPU implementation of the convex chi approximation
TEST_F(ChiTest, CPU_Chi_Convex_ZERO)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data;
	valarray<cl_float> data_err;
	valarray<cl_float> model;
	valarray<cl_float> output;
	MakeChiZeroBuffers(data, data_err, model, output, test_size);

	// Run the chi algorithm
	CRoutine_Chi::Chi_complex_convex(data, data_err, model, 0, test_size, output);

	// Compare results. Because data = model every chi element should be of unit magnitude
	for(size_t i = 0; i < test_size; i++)
	{
		// Check the real and imaginary chi values
		EXPECT_NEAR(fabs(output[i]), 0, MAX_REL_ERROR);
		EXPECT_NEAR(fabs(output[test_size + i]), 0, MAX_REL_ERROR);
	}
}

/// Tests the CPU implementation of the convex chi approximation
TEST_F(ChiTest, CPU_Chi_Convex_ONE)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data;
	valarray<cl_float> data_err;
	valarray<cl_float> model;
	valarray<cl_float> output;
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	// Run the chi algorithm
	CRoutine_Chi::Chi_complex_convex(data, data_err, model, 0, test_size, output);

	// Compare results. Because data = model every chi element should be of unit magnitude
	for(size_t i = 0; i < test_size; i++)
	{
		// Check the real and imaginary chi values
		EXPECT_LT(fabs(output[i]), 1 + amp_err);
		EXPECT_LT(fabs(output[test_size + i]), 1 + phi_err);
	}
}

/// Tests the CPU implementation of the convex chi approximation
TEST_F(ChiTest, CPU_Chi_NonConvex_ZERO)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data;
	valarray<cl_float> data_err;
	valarray<cl_float> model;
	valarray<cl_float> output;
	MakeChiZeroBuffers(data, data_err, model, output, test_size);

	// Run the chi algorithm
	CRoutine_Chi::Chi_complex_nonconvex(data, data_err, model, 0, test_size, output);

	// Compare results. Because data = model every chi element should be of unit magnitude
	for(size_t i = 0; i < test_size; i++)
	{
		// Check the real and imaginary chi values
		EXPECT_EQ(fabs(output[i]), 0) << "Amplitude error exceeded.";
		EXPECT_EQ(fabs(output[test_size + i]), 0) << "Phase error exceeded.";
	}
}

/// Tests the CPU implementation of the convex chi approximation
TEST_F(ChiTest, CPU_Chi_NonConvex_ONE)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data;
	valarray<cl_float> data_err;
	valarray<cl_float> model;
	valarray<cl_float> output;
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	// Run the chi algorithm
	CRoutine_Chi::Chi_complex_nonconvex(data, data_err, model, 0, test_size, output);

	// Compare results. Because data = model every chi element should be of unit magnitude
	for(size_t i = 0; i < test_size; i++)
	{
		// Check the real and imaginary chi values
		EXPECT_LT(fabs(output[i]), 1 + amp_err) << "Amplitude error exceeded.";
		EXPECT_LT(fabs(output[test_size + i]), 1 + phi_err) << "Phase error exceeded." << i;
	}
}

/// Checks that the CPU chi algorithm evaluates to zero
/// when data and model are equal.
TEST_F(ChiTest, CL_Chi_ZERO)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiZeroBuffers(data, data_err, model, output, test_size);

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
    r->Chi(data_cl, data_err_cl, model_cl, output_cl, 0, test_size);
    ReadCLResult(output);

	// Compare results. Because data = model every chi element should be zero
	for(size_t i = 0; i < test_size; i++)
		EXPECT_EQ(output[i], 0);
}

/// Checks that the CPU chi algorithm evaluates to zero
/// when data and model are equal.
TEST_F(ChiTest, CL_Chi_ONE)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
    r->Chi(data_cl, data_err_cl, model_cl, output_cl, 0, test_size);
    ReadCLResult(output);

	// Compare results. Because data = model every chi element should be zero
	for(size_t i = 0; i < test_size; i++)
		EXPECT_NEAR(fabs(output[i]), 1, MAX_REL_ERROR);
}

/// Checks that the OpenCL chi algorithm evaluates to zero
/// when data and model are equal.
TEST_F(ChiTest, CL_Chi_Convex_Zero)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiZeroBuffers(data, data_err, model, output, test_size);

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
    r->ChiComplexConvex(data_cl, data_err_cl, model_cl, output_cl, 0, test_size);
    ReadCLResult(output);

	// Compare results. Because data = model every chi element should be zero
	for(size_t i = 0; i < test_size; i++)
		EXPECT_EQ(fabs(output[i]), 0);
}

/// Checks that the OpenCL chi algorithm evaluates to one
/// when data and model are different by one sigma
TEST_F(ChiTest, CL_Chi_Convex_One)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
    r->ChiComplexConvex(data_cl, data_err_cl, model_cl, output_cl, 0, test_size);
    ReadCLResult(output);

	// Compare results. Because model = data + 1sigma all elements should be ~1
	for(size_t i = 0; i < test_size; i++)
		EXPECT_NEAR(fabs(output[i]), 1, MAX_REL_ERROR);
}

/// Checks that the OpenCL chi algorithm evaluates to zero
/// when data and model are equal.
TEST_F(ChiTest, CL_Chi_NonConvex_Zero)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiZeroBuffers(data, data_err, model, output, test_size);

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
    r->ChiComplexNonConvex(data_cl, data_err_cl, model_cl, output_cl, 0, test_size);
    ReadCLResult(output);

	// Compare results. Because data = model every chi element should be zero
	for(size_t i = 0; i < test_size; i++)
		EXPECT_EQ(fabs(output[i]), 0);
}

/// Checks that the OpenCL chi algorithm evaluates to one
/// when data and model are different by one sigma
TEST_F(ChiTest, CL_Chi_NonConvex_One)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
    r->ChiComplexNonConvex(data_cl, data_err_cl, model_cl, output_cl, 0, test_size);
    ReadCLResult(output);

	// Compare results. Because model = data + 1sigma all elements should be ~1
	for(size_t i = 0; i < test_size; i++)
		EXPECT_NEAR(fabs(output[i]), 1, MAX_REL_ERROR);
}

/// Checks that the chi2 functions are working for V2 data
TEST_F(ChiTest, CL_Chi2_V2)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	// Create a buffer where the chi values should evaluate to 1.
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	unsigned int n_vis = 0;
	unsigned int n_v2 = test_size;
	unsigned int n_t3 = 0;

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
	float should_be_one = r->Chi2(data_cl, data_err_cl, model_cl, LibOIEnums::NON_CONVEX, n_vis, n_v2, n_t3, true);
	should_be_one /= test_size;

	EXPECT_NEAR(should_be_one, 1, MAX_REL_ERROR);
}

/// Checks that the chi2 functions are working for T3 data
TEST_F(ChiTest, CL_Chi2_T3)
{
	size_t test_size = 10000;

	// Create buffers
	valarray<cl_float> data(test_size);
	valarray<cl_float> data_err(test_size);
	valarray<cl_float> model(test_size);
	valarray<cl_float> output(test_size);
	MakeChiOneBuffers(data, data_err, model, output, test_size);

	unsigned int n_vis = 0;
	unsigned int n_v2 = 0;
	unsigned int n_t3 = test_size;

	// Setup OpenCL and the Chi routine. Teardown is automatic.
	SetUpCL(data, data_err, model);
	float should_be_one = r->Chi2(data_cl, data_err_cl, model_cl, LibOIEnums::NON_CONVEX, n_vis, n_v2, n_t3, true);
	should_be_one /= test_size;

	EXPECT_NEAR(should_be_one, 1, MAX_REL_ERROR);
}

///// Checks that a mixture of V2 and T3 yield a chi2 < 1
//TEST_F(ChiTest, CL_Chi2_Mix)
//{
//	unsigned int test_size = 10000;
//
//	unsigned int n_values = 2*test_size;
//
//	// Create buffers
//	valarray<cl_float> data(test_size);
//	valarray<cl_float> data_err(test_size);
//	valarray<cl_float> model(test_size);
//	valarray<cl_float> output(test_size);
//	MakeChiOneBuffers(data, data_err, model, output, test_size);
//
//	unsigned int n_vis = 0;
//	unsigned int n_v2 = test_size/2;
//	unsigned int n_t3 = test_size/2;
//
//	// Setup OpenCL and the Chi routine. Teardown is automatic.
//	SetUpCL(data, data_err, model);
//	float should_be_one = r->Chi2(data_cl, data_err_cl, model_cl, LibOIEnums::NON_CONVEX, n_vis, n_v2, n_t3, true);
//	should_be_one /= n_values;
//
//	EXPECT_NEAR(should_be_one, 1, MAX_REL_ERROR);
//}
