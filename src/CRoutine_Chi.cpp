/*
 * CRoutine_Chi.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_Chi.h"
#include <cstdio>

#include "CRoutine_Square.h"

using namespace std;

CRoutine_Chi::CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_Sum(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("chi.cl");
	mChiSourceID = mSource.size() - 1;

	mChiTemp = NULL;
	mChiOutput = NULL;
	mChiKernelID = -1;
	mCPUChiTemp = NULL;
}

CRoutine_Chi::~CRoutine_Chi()
{
	if(mChiTemp) clReleaseMemObject(mChiTemp);
	if(mChiOutput) clReleaseMemObject(mChiOutput);

	delete[] mCPUChiTemp;
}

/// Computes chi = (data - model)/(data_err) and stores it in the internal mChiTemp buffer.
void CRoutine_Chi::Chi(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = CL_SUCCESS;
   	// The loglikelihood kernel executes on the entire output buffer
   	// because the reduce_sum_float kernel uses the entire buffer as input.
   	// Therefore we zero out the elements not directly involved in this computation.
	size_t global = (size_t) mNElements;
	size_t local = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mChiKernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for chi kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mChiKernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mChiKernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mChiKernelID], 2, sizeof(cl_mem), &model_data);
	err |= clSetKernelArg(mKernels[mChiKernelID], 3, sizeof(cl_mem), &mChiTemp);
	err |= clSetKernelArg(mKernels[mChiKernelID], 4, sizeof(int), &n);
	COpenCL::CheckOCLError("Failed to set chi kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mChiKernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue chi kernel.", err);
}

void CRoutine_Chi::Chi_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = CL_SUCCESS;
	cl_float cpu_data[mNElements];
	cl_float cpu_data_err[mNElements];
	cl_float cpu_model_data[mNElements];
	cl_float tmp = 0;

	err  = clEnqueueReadBuffer(mQueue, data, CL_TRUE, 0, mNElements * sizeof(cl_float), &cpu_data, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, data_err, CL_TRUE, 0, mNElements * sizeof(cl_float), &cpu_data_err, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(mQueue, model_data, CL_TRUE, 0, mNElements * sizeof(cl_float), &cpu_model_data, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy chi buffers back to the CPU.", err);

	// Now compute the chi, store it.
	for(int i = 0; i < mNElements; i++)
	{
		tmp = 0;

		if(i < n)
			tmp = (cpu_data[i] - cpu_model_data[i]) / cpu_data_err[i];

		mCPUChiTemp[i] = tmp;
	}
}

/// Compares the OpenCL and CPU-only chi outputs given the same input data
bool CRoutine_Chi::Chi_Test(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	// execute the chi kernel and cpu versions
	Chi(data, data_err, model_data, n);
	Chi_CPU(data, data_err, model_data, n);

	printf("Checking individual Chi values:\n");
	bool test_result = Verify(mCPUChiTemp, mChiTemp, mNElements, 0);
	PassFail(test_result);

	return test_result;
}

/// Helper function, calls the chi and then square routines, stores output in the internal mChiTemp buffer.
float CRoutine_Chi::Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, CRoutine_Square * rSquare, bool compute_sum, bool return_value)
{
	float sum = 0;
	Chi(data, data_err, model_data, n);
	rSquare->Square(mChiTemp, mChiTemp, mNElements, n);

	// Now fire up the parallel sum kernel and return the output.  Wrap this in a try/catch block.
	try
	{
		if(compute_sum)
			sum = ComputeSum(mChiTemp, mChiOutput, return_value);
	}
	catch (...)
	{
		printf("Warning, exception in CRoutine_Chi2.  Writing out buffers:\n");
		Chi(data, data_err, model_data, n);
		rSquare->Square(mChiTemp, mChiTemp, mNElements, n);
		DumpFloatBuffer(mChiTemp, mNElements);
		throw;
	}

	return sum;
}

float CRoutine_Chi::Chi2_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n, CRoutine_Square * rSquare, bool compute_sum)
{
	float sum = 0;
	float temp = 0;

	// Now square
	for(int i = 0; i < mNElements; i++)
	{
		temp = 0;

		if(i < n)
		{
			temp = mCPUChiTemp[i];
			temp *= temp;
			sum += temp;
		}

		mCPUChiTemp[i] = temp;
	}

	if(compute_sum)
		return sum;

	return 0;
}

/// Verifies the chi2 and chi2_cpu values match.  Also compares the intermediate chi2 array elements.
bool CRoutine_Chi::Chi2_Test(cl_mem data, cl_mem data_err, cl_mem model_data, int n, CRoutine_Square * rSquare, bool compute_sum)
{
	bool chi2_match = true;
	bool sum_match = true;
	float cpu_sum = 0;
	// Run the OpenCL function first without computing the sum.
	Chi2(data, data_err, model_data, n, rSquare, false, false);
	cpu_sum = Chi2_CPU(data, data_err, model_data, n, rSquare, true);

	// Compare the CL and CPU chi2 elements:
	printf("Checking individual Chi2 values:\n");
	chi2_match = Verify(mCPUChiTemp, mChiTemp, mNElements, 0);
	PassFail(chi2_match);

	printf("Checking summed Chi2 values:\n");
	float cl_sum = ComputeSum(mChiTemp, mChiOutput, true);
	bool sum_pass = bool(fabs((cpu_sum - cl_sum)/cpu_sum) < MAX_REL_ERROR);
	printf("  CPU Value:  %0.4f\n", cpu_sum);
	printf("  CL  Value:  %0.4f\n", cl_sum);
	printf("  Difference: %0.4f\n", cpu_sum - cl_sum);
	PassFail(sum_pass);

	return sum_match && chi2_match;
}

/// Computes the chi values and returns them in the array, output, which has size n
/// Note, the user is responsible for allocating and deallocating output!
void CRoutine_Chi::GetChi(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output)
{
	// Compute the chi
	Chi(data, data_err, model_data, n);
	clFinish(mQueue);

	// Copy data to the CPU.  Note, we use an intermediate array in case cl_float != float
	int err = 0;
	err = clEnqueueReadBuffer(mQueue, mChiTemp, CL_TRUE, 0, n * sizeof(cl_float), mCPUChiTemp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy buffer back to CPU.  CRoutine_Chi::GetChi().", err);

	for(int i = 0; i < n; i++)
		output[i] = float(mCPUChiTemp[i]);
}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_Chi::Init(int n)
{
	int err = CL_SUCCESS;

	// First initialize the base-class constructor:
	CRoutine_Sum::Init(n);

	// Now allocate some memory
	if(mChiTemp == NULL)
		mChiTemp = clCreateBuffer(mContext, CL_MEM_READ_WRITE, n * sizeof(cl_float), NULL, &err);

	if(mChiOutput == NULL)
		mChiOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	if(mCPUChiTemp == NULL)
		mCPUChiTemp = new cl_float[n];

	// Read the kernel, compile it
	string source = ReadSource(mSource[mChiSourceID]);
    BuildKernel(source, "chi", mSource[mChiSourceID]);
    mChiKernelID = mKernels.size() - 1;
}
