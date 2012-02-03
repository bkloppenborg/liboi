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
	:CRoutine_Reduce_Sum(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("chi.cl");
	mChiSourceID = mSource.size() - 1;

	mChiTemp = NULL;
	mChiOutput = NULL;
	mChiKernelID = -1;
}

CRoutine_Chi::~CRoutine_Chi()
{
	if(mChiTemp) clReleaseMemObject(mChiTemp);
	if(mChiOutput) clReleaseMemObject(mChiOutput);
}

void CRoutine_Chi::Chi(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = 0;
	size_t global = (size_t) n;
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

/// Helper function, calls the chi and then square routines, stores output in the internal mChiTemp buffer.
float CRoutine_Chi::Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, CRoutine_Square * rSquare, bool compute_sum)
{
	float sum = 0;
	Chi(data, data_err, model_data, n);
	rSquare->Square(mChiTemp, mChiTemp, n);

#ifdef DEBUG_VERBOSE
	// Copy back the data, model, and errors:
	Chi2_CPU(data, data_err, model_data, n);
	ComputeSum_CPU(mChi2Output, n);
#endif // DEBUG_VERBOSE

	// Now fire up the parallel sum kernel and return the output.
	if(compute_sum)
		sum = ComputeSum(true, mChiOutput, mChiTemp, tmp_buff1, tmp_buff2);

	return sum;
}

float CRoutine_Chi::Chi2_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = 0;
	cl_float * cpu_data = new cl_float[n];
	err |= clEnqueueReadBuffer(mQueue, data, CL_TRUE, 0, n * sizeof(cl_float), cpu_data, 0, NULL, NULL);
	cl_float * cpu_data_err = new cl_float[n];
	err |= clEnqueueReadBuffer(mQueue, data_err, CL_TRUE, 0, n * sizeof(cl_float), cpu_data_err, 0, NULL, NULL);
	cl_float * cpu_model_data = new cl_float[n];
	err |= clEnqueueReadBuffer(mQueue, model_data, CL_TRUE, 0, n * sizeof(cl_float), cpu_model_data, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy chi buffers back to the CPU.", err);

	// we do this verbose
	float sum = 0;
	float tmp = 0;
	for(int i = 0; i < n; i++)
	{
		tmp = (cpu_data[i] - cpu_model_data[i]) / cpu_data_err[i];
		printf("%i %f %f %e %e \n", i, cpu_data[i], cpu_model_data[i], cpu_data[i] - cpu_model_data[i], cpu_data_err[i]);
		sum += tmp * tmp;
	}

	printf("Chi2: %f\n", sum);

	delete[] cpu_data;
	delete[] cpu_data_err;
	delete[] cpu_model_data;

	return sum;
}

/// Computes the chi values and returns them in the array, output, which has size n
/// Note, the user is responsible for allocating and deallocating output!
void CRoutine_Chi::GetChi(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output)
{
	// Compute the chi
	Chi(data, data_err, model_data, n);
	clFinish(mQueue);

	// Copy data to the CPU.  Note, we use an intermedate array in case cl_float != float
	int err = 0;
	cl_float * tmp = new cl_float[n];
	err = clEnqueueReadBuffer(mQueue, mChiTemp, CL_TRUE, 0, n * sizeof(cl_float), tmp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to copy buffer back to CPU.  CRoutine_Chi::GetChi().", err);

	for(int i = 0; i < n; i++)
		output[i] = float(tmp[i]);
}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_Chi::Init(int num_elements)
{
	int err = CL_SUCCESS;

	// First initialize the base-class constructor:
	CRoutine_Reduce_Sum::Init(num_elements, true);

	// Now allocate some memory
	if(mChiTemp == NULL)
		mChiTemp = clCreateBuffer(mContext, CL_MEM_READ_WRITE, num_elements * sizeof(cl_float), NULL, &err);

	if(mChiOutput == NULL)
		mChiOutput = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	// Read the kernel, compile it
	string source = ReadSource(mSource[mChiSourceID]);
    BuildKernel(source, "chi", mSource[mChiSourceID]);
    mChiKernelID = mKernels.size() - 1;
}
