/*
 * CRoutine_FTtoV2_test.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: bkloppenborg
 */


#include "gtest/gtest.h"
#include <algorithm>

#include "liboi_tests.h"
#include "COpenCL.h"
#include "CRoutine_FTtoV2.h"
#include "CModel.h"

using namespace std;

extern string LIBOI_KERNEL_PATH;

/// Checks that the CPU routine functions correctly.
TEST(CRoutine_FTtoV2, CPU_Linear_UVRef)
{
	unsigned int test_size = 10000;

	// Allocate storage and generate synthetic complex data.
	valarray<complex<float>> ft_input_complex(test_size);
	valarray<cl_float2> ft_input_cl = CModel::GenerateUVSpiral_CL(test_size);
	valarray<cl_uint> uv_ref(test_size);
	valarray<cl_float> cpu_output(test_size);

	// Copy these numbers int to the complex array
	for(int i = 0; i < test_size; i++)
	{
		// Assign the UV references linearly.
		uv_ref[i] = i;
		// Copy the pseudo FT values.
		ft_input_complex[i] = complex<float>(ft_input_cl[i].s0, ft_input_cl[i].s1);
	}

	// Run the routine
	CRoutine_FTtoV2::FTtoV2(ft_input_cl, uv_ref, cpu_output, test_size);

	// Check the results:
	for(int i = 0; i < test_size; i++)
	{
		float complex_norm = norm(ft_input_complex[i]);
		EXPECT_NEAR(complex_norm, float(cpu_output[i]), MAX_REL_ERROR * complex_norm);
	}
}

/// Checks that the CPU routine functions correctly.
TEST(CRoutine_FTtoV2, CPU_Reordered_UVRef)
{
	unsigned int test_size = 10000;

	// Allocate storage and generate synthetic complex data.
	valarray<complex<float>> ft_input_complex(test_size);
	valarray<cl_float2> ft_input_cl = CModel::GenerateUVSpiral_CL(test_size);
	valarray<cl_uint> uv_ref(test_size);
	valarray<cl_float> cpu_output(test_size);

	// Copy these numbers int to the complex array
	for(unsigned int i = 0; i < test_size; i++)
	{
		// Assign the UV references linearly, but with every 10th element changed
		if(i % 10 == 0)
		{
			uv_ref[i] = min(i+3, test_size);
		}
		else
			uv_ref[i] = i;

		// Copy the pseudo FT values.
		ft_input_complex[i] = complex<float>(ft_input_cl[i].s0, ft_input_cl[i].s1);
	}

	// Run the routine
	CRoutine_FTtoV2::FTtoV2(ft_input_cl, uv_ref, cpu_output, test_size);

	// Check the results:
	for(int i = 0; i < test_size; i++)
	{
		unsigned int id = uv_ref[i];
		float complex_norm = norm(ft_input_complex[id]);
		EXPECT_NEAR(complex_norm, float(cpu_output[i]), MAX_REL_ERROR * complex_norm);
	}
}

/// Verifies the CPU and OpenCL routines produce the same result
/// for linear UV references.
TEST(CRoutine_FTtoV2, OpenCL_Linear_UVRef)
{
	unsigned int test_size = 10000;

	// Allocate storage and generate synthetic complex data.
	valarray<cl_float2> ft_input = CModel::GenerateUVSpiral_CL(test_size);
	valarray<cl_uint> uv_ref(test_size);
	valarray<cl_float> cpu_output(test_size);
	valarray<cl_float> cl_output(test_size);

	// Assign the UV references linearly:
	for(int i = 0; i < test_size; i++)
		uv_ref[i] = i;

	// Init the OpenCL device and necessary routines:
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_FTtoV2 r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init();

	// Creat buffers on the OpenCL device, copy data over.
	int err = CL_SUCCESS;
	cl_mem ft_input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * test_size, NULL, NULL);
	cl_mem uv_ref_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * test_size, NULL, NULL);
	cl_mem tmp_output = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);

    err = clEnqueueWriteBuffer(cl.GetQueue(), ft_input_cl, CL_TRUE, 0, sizeof(cl_float2) * test_size, &ft_input[0], 0, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), uv_ref_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &uv_ref[0], 0, NULL, NULL);

	// Run the routines
	CRoutine_FTtoV2::FTtoV2(ft_input, uv_ref, cpu_output, test_size);
	r.FTtoV2(ft_input_cl, uv_ref_cl, tmp_output, 0, test_size);

	// Copy back the results from the OpenCL device
	err = clEnqueueReadBuffer(cl.GetQueue(), tmp_output, CL_TRUE, 0, sizeof(cl_float) * test_size, &cl_output[0], 0, NULL, NULL);

	// Free OpenCL memory:
	clReleaseMemObject(ft_input_cl);
	clReleaseMemObject(uv_ref_cl);
	clReleaseMemObject(tmp_output);

	// Compare the results:
	for(int i = 0; i < test_size; i++)
	{
		EXPECT_NEAR(cl_output[i], cpu_output[i], MAX_REL_ERROR * cpu_output[i]);
	}
}

/// Verifies the CPU and OpenCL routines produce the same result
/// for slightly reordered UV references.
TEST(CRoutine_FTtoV2, OpenCL_Reordered_UVRef)
{
	unsigned int test_size = 10000;

	// Allocate storage and generate synthetic complex data.
	valarray<cl_float2> ft_input = CModel::GenerateUVSpiral_CL(test_size);
	valarray<cl_uint> uv_ref(test_size);
	valarray<cl_float> cpu_output(test_size);
	valarray<cl_float> cl_output(test_size);

	// Assign the UV references linearly:
	for(unsigned int i = 0; i < test_size; i++)
	{
		// Assign the UV references linearly, but with every 10th element changed
		if(i % 10 == 0)
		{
			uv_ref[i] = min(i+3, test_size);
		}
		else
			uv_ref[i] = i;
	}

	// Init the OpenCL device and necessary routines:
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_FTtoV2 r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init();

	// Creat buffers on the OpenCL device, copy data over.
	int err = CL_SUCCESS;
	cl_mem ft_input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * test_size, NULL, NULL);
	cl_mem uv_ref_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * test_size, NULL, NULL);
	cl_mem tmp_output = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * test_size, NULL, NULL);

    err = clEnqueueWriteBuffer(cl.GetQueue(), ft_input_cl, CL_TRUE, 0, sizeof(cl_float2) * test_size, &ft_input[0], 0, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), uv_ref_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &uv_ref[0], 0, NULL, NULL);

	// Run the routines
	CRoutine_FTtoV2::FTtoV2(ft_input, uv_ref, cpu_output, test_size);
	r.FTtoV2(ft_input_cl, uv_ref_cl, tmp_output, 0, test_size);

	// Copy back the results from the OpenCL device
	err = clEnqueueReadBuffer(cl.GetQueue(), tmp_output, CL_TRUE, 0, sizeof(cl_float) * test_size, &cl_output[0], 0, NULL, NULL);

	// Free OpenCL memory:
	clReleaseMemObject(ft_input_cl);
	clReleaseMemObject(uv_ref_cl);
	clReleaseMemObject(tmp_output);

	// Compare the results:
	for(int i = 0; i < test_size; i++)
	{
		EXPECT_NEAR(cl_output[i], cpu_output[i], MAX_REL_ERROR * cpu_output[i]);
	}
}

/// Verifies the CPU and OpenCL routines produce the same result
/// when the OpenCL memory zero point is slightly offset
TEST(CRoutine_FTtoV2, OpenCL_OffsetMemory)
{
	unsigned int test_size = 10000;
	unsigned int n_vis = test_size/6;
	unsigned int offset = CRoutine_FTtoV2::CalculateOffset(n_vis);

	// Allocate storage and generate synthetic complex data.
	valarray<cl_float2> ft_input = CModel::GenerateUVSpiral_CL(test_size);
	valarray<cl_uint> uv_ref(test_size);
	valarray<cl_float> cpu_output(test_size);
	valarray<cl_float> cl_output(test_size);

	// Assign the UV references linearly:
	for(unsigned int i = 0; i < test_size; i++)
	{
		// Assign the UV references linearly, but with every 10th element changed
		if(i % 10 == 0)
		{
			uv_ref[i] = min(i+3, test_size);
		}
		else
			uv_ref[i] = i;
	}

	// Init the OpenCL device and necessary routines:
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_FTtoV2 r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init();

	// Creat buffers on the OpenCL device
	// The output from the kernel will start at index "offset", so the output buffer needs to be large
	// enough to account for this shift.
	int err = CL_SUCCESS;
	cl_mem ft_input_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * test_size, NULL, NULL);
	cl_mem uv_ref_cl = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float2) * test_size, NULL, NULL);
	cl_mem tmp_output = clCreateBuffer(cl.GetContext(), CL_MEM_READ_WRITE, sizeof(cl_float) * (test_size + offset), NULL, NULL);

	// Copy the ft input values to the OpenCL device, remember to offset them.
    err = clEnqueueWriteBuffer(cl.GetQueue(), ft_input_cl, CL_TRUE, 0, sizeof(cl_float2) * test_size, &ft_input[0], 0, NULL, NULL);
    err = clEnqueueWriteBuffer(cl.GetQueue(), uv_ref_cl, CL_TRUE, 0, sizeof(cl_float) * test_size, &uv_ref[0], 0, NULL, NULL);

	// Run the routines
	CRoutine_FTtoV2::FTtoV2(ft_input, uv_ref, cpu_output, test_size);
	r.FTtoV2(ft_input_cl, uv_ref_cl, tmp_output, n_vis, test_size);

	// Copy back the results from the OpenCL device.  Notice we start copying at "offset" so the
	// indicies of the CPU and OpenCL computations are the same in the comparision function below.
	err = clEnqueueReadBuffer(cl.GetQueue(), tmp_output, CL_TRUE, sizeof(cl_float) * offset, sizeof(cl_float) * test_size, &cl_output[0], 0, NULL, NULL);

	// Free OpenCL memory
	clReleaseMemObject(ft_input_cl);
	clReleaseMemObject(uv_ref_cl);
	clReleaseMemObject(tmp_output);

	// Compare the results:
	for(int i = 0; i < test_size; i++)
	{
		EXPECT_NEAR(cl_output[i], cpu_output[i], MAX_REL_ERROR * cpu_output[i]);
	}
}
