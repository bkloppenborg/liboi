/*
 * CRoutine_Sum.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include <cstdio>
#include <sstream>
#include "CRoutine_Sum.h"

using namespace std;

CRoutine_Sum::CRoutine_Sum(cl_device_id device, cl_context context, cl_command_queue queue)
	: CRoutine(device, context, queue)
{
	// Specify the source location, set temporary buffers to null
	mSource.push_back("reduce_sum_float.cl");
	mTempBuffer = NULL;
	num_elements = 0;
}

CRoutine_Sum::~CRoutine_Sum()
{
	if(mTempBuffer) clReleaseMemObject(mTempBuffer);
}

void CRoutine_Sum::BuildKernels()
{

}

unsigned int nextPow2( unsigned int x ) {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

bool isPow2(unsigned int x)
{
    return ((x&(x-1))==0);
}

void getNumBlocksAndThreads(int whichKernel, int n, int maxBlocks, int maxThreads, int &blocks, int &threads)
{
    if (whichKernel < 3)
    {
        threads = (n < maxThreads) ? nextPow2(n) : maxThreads;
        blocks = (n + threads - 1) / threads;
    }
    else
    {
        threads = (n < maxThreads*2) ? nextPow2((n + 1)/ 2) : maxThreads;
        blocks = (n + (threads * 2 - 1)) / (threads * 2);
    }


    if (whichKernel == 6)
        blocks = min(maxBlocks, blocks);
}

cl_kernel CRoutine_Sum::getReductionKernel(int whichKernel, int blockSize, int isPowOf2)
{

    stringstream tmp;
    tmp << "#define T float" << std::endl;
    tmp << "#define blockSize " << blockSize << std::endl;
    tmp << "#define nIsPow2 " << isPowOf2 << std::endl;
    tmp << ReadSource(mSource[0]);

    stringstream kernelName;
    kernelName << "reduce" << whichKernel;
    BuildKernel(tmp.str(), kernelName.str());

    return mKernels[mKernels.size() - 1];
}

// Performs an out-of-plate sum storing temporary values in output_buffer and partial_sum_buffer.
float CRoutine_Sum::ComputeSum(cl_mem input_buffer, cl_mem final_buffer, bool copy_back)
{
	int err = CL_SUCCESS;
	int whichKernel = 6;
	float gpu_result = 0;
	bool needReadBack = true;
	cl_kernel finalReductionKernel[10];
	int finalReductionIterations=0;
	int numBlocks = 0;
	int numThreads = 0;
	int maxThreads = 128;
	int maxBlocks = 64;
	int cpuFinalThreshold = 1;

	getNumBlocksAndThreads(whichKernel, num_elements, maxBlocks, maxThreads, numBlocks, numThreads);

	cl_kernel reductionKernel = getReductionKernel(whichKernel, numThreads, isPow2(num_elements) );
	clSetKernelArg(reductionKernel, 0, sizeof(cl_mem), (void *) &input_buffer);
	clSetKernelArg(reductionKernel, 1, sizeof(cl_mem), (void *) &mTempBuffer);
	clSetKernelArg(reductionKernel, 2, sizeof(cl_int), &num_elements);
	clSetKernelArg(reductionKernel, 3, sizeof(cl_float) * numThreads, NULL);


	int s = numBlocks;
	int threads = 0, blocks = 0;
	int kernel = (whichKernel == 6) ? 5 : whichKernel;

	while(s > cpuFinalThreshold)
	{
		getNumBlocksAndThreads(kernel, s, maxBlocks, maxThreads, blocks, threads);

		finalReductionKernel[finalReductionIterations] = getReductionKernel(kernel, threads, isPow2(s) );
		clSetKernelArg(finalReductionKernel[finalReductionIterations], 0, sizeof(cl_mem), (void *) &mTempBuffer);
		clSetKernelArg(finalReductionKernel[finalReductionIterations], 1, sizeof(cl_mem), (void *) &mTempBuffer);
		clSetKernelArg(finalReductionKernel[finalReductionIterations], 2, sizeof(cl_int), &num_elements);
		clSetKernelArg(finalReductionKernel[finalReductionIterations], 3, sizeof(cl_float) * numThreads, NULL);

		s = (s + (threads*2-1)) / (threads*2);

		finalReductionIterations++;
	}


	size_t globalWorkSize[1];
	size_t localWorkSize[1];

	gpu_result = 0;

	clFinish(mQueue);

	// execute the kernel
	globalWorkSize[0] = numBlocks * numThreads;
	localWorkSize[0] = numThreads;

	err = clEnqueueNDRangeKernel(mQueue,reductionKernel, 1, 0, globalWorkSize, localWorkSize, 0, NULL, NULL);
	COpenCL::CheckOCLError("Unable to enqueue parallel reduction kernel.", err);

	// sum partial block sums on GPU
	s = numBlocks;
	kernel = (whichKernel == 6) ? 5 : whichKernel;
	int it = 0;

	while(s > cpuFinalThreshold)
	{
		int threads = 0, blocks = 0;
		getNumBlocksAndThreads(kernel, s, maxBlocks, maxThreads, blocks, threads);

		globalWorkSize[0] = threads * blocks;
		localWorkSize[0] = threads;

		err = clEnqueueNDRangeKernel(mQueue, finalReductionKernel[it], 1, 0,
										  globalWorkSize, localWorkSize, 0, NULL, NULL);
		COpenCL::CheckOCLError("Unable to enqueue final parallel reduction kernel.", err);

		s = (s + (threads*2-1)) / (threads*2);

		it++;
	}

	// If a few elements remain, we will need to compute their sum on the CPU:
    if (s > 1)
    {
    	cl_float h_odata[s];
        // copy result from device to host
        clEnqueueReadBuffer(mQueue, mTempBuffer, CL_TRUE, 0, s * sizeof(cl_float),
                            h_odata, 0, NULL, NULL);

        for(int i=0; i < s; i++)
        {
            gpu_result += h_odata[i];
        }

        // Copy this value back to the GPU
    	clEnqueueWriteBuffer(mQueue, final_buffer, CL_TRUE, 0, sizeof(cl_float), &gpu_result, 0, NULL, NULL);

        needReadBack = false;
    }
    else
    {
    	// The work was all completed on the GPU.  Copy the summed value to the final buffer:
    	clEnqueueCopyBuffer(mQueue, mTempBuffer, final_buffer, 0, 0, sizeof(cl_float), 0, NULL, NULL);
    }

	clFinish(mQueue);

	if (needReadBack)
	{
		// copy final sum from device to host
		clEnqueueReadBuffer(mQueue, mTempBuffer, CL_TRUE, 0, sizeof(cl_float), &gpu_result, 0, NULL, NULL);
	}


	// Now copy the data over to the final GPU location.
	clEnqueueWriteBuffer(mQueue, final_buffer, CL_TRUE, 0, sizeof(cl_float), &gpu_result, 0, NULL, NULL);

	// Release the kernels
	clReleaseKernel(reductionKernel);

	for(int it=0; it<finalReductionIterations; ++it)
	{
		clReleaseKernel(finalReductionKernel[it]);
	}

	return gpu_result;
}

/// Computes the sum of the OpenCL buffer, input_buffer, using Kahan summation to minimize precision losses.
float CRoutine_Sum::ComputeSum_CPU(cl_mem input_buffer)
{
	int err = CL_SUCCESS;
	cl_float tmp[num_elements];
	err |= clEnqueueReadBuffer(mQueue, input_buffer, CL_TRUE, 0, num_elements * sizeof(cl_float), tmp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy buffer back to CPU, CRoutine_Reduce_Sum::Compute_CPU() ", err);

	// Use Kahan summation to minimize lost precision.
	// http://en.wikipedia.org/wiki/Kahan_summation_algorithm
	float sum = tmp[0];
	float c = float(0.0);
	for (int i = 1; i < num_elements; i++)
	{
		float y = tmp[i] - c;
		float t = sum + y;
		c = (t - sum) - y;
		sum = t;
	}
	return sum;
}

/// Tests that the CPU and OpenCL versions of ComputeSum return the same value.
bool CRoutine_Sum::ComputeSum_Test(cl_mem input_buffer, cl_mem final_buffer, bool copy_back)
{
	// First run the CPU version as the CL version modifies the buffers.
	float cpu_sum = ComputeSum_CPU(input_buffer);
	float cl_sum = ComputeSum(input_buffer, final_buffer, true);

	bool sum_pass = bool(fabs(cpu_sum - cl_sum)/cpu_sum < MAX_REL_ERROR);
	printf("  CPU Value:  %0.4f\n", cpu_sum);
	printf("  CL  Value:  %0.4f\n", cl_sum);
	printf("  Difference: %0.4f\n", cpu_sum - cl_sum);
	PassFail(sum_pass);
}

/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Sum::Init(int n)
{
	int err = CL_SUCCESS;
	// Set the number of elements on which this kernel will operate.
	this->num_elements = n;
	BuildKernels();

	mTempBuffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, num_elements * sizeof(cl_float), NULL, &err);
	COpenCL::CheckOCLError("Could not create parallel sum temporary buffer.", err);
}
