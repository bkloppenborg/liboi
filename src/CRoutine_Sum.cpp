/*
 * CRoutine_Sum.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include <cstdio>
#include <sstream>
#include "CRoutine_Sum.h"
#include "CRoutine_Zero.h"

using namespace std;

CRoutine_Sum::CRoutine_Sum(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	: CRoutine(device, context, queue)
{
	// Specify the source location, set temporary buffers to null
	mSource.push_back("reduce_sum_float.cl");
	mTempSumBuffer = NULL;
	mNElements = 0;
	mFinalS = 0;
	mReductionPasses = 0;

	// External routines, do not delete/deallocate here.
	mrZero = rZero;
}

CRoutine_Sum::~CRoutine_Sum()
{
	if(mTempSumBuffer) clReleaseMemObject(mTempSumBuffer);
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

void CRoutine_Sum::BuildKernels()
{
	int whichKernel = 6;
	int numBlocks = 0;
	int numThreads = 0;
	int maxThreads = 128;
	int maxBlocks = 64;
	int cpuFinalThreshold = 1;

	getNumBlocksAndThreads(whichKernel, mNElements, maxBlocks, maxThreads, numBlocks, numThreads);
	BuildReductionKernel(whichKernel, numThreads, isPow2(mNElements) );
	mBlocks.push_back(numBlocks);
	mThreads.push_back(numThreads);
	mReductionPasses += 1;

	int s = numBlocks;
	int threads = 0, blocks = 0;
	int kernel = (whichKernel == 6) ? 5 : whichKernel;

	while(s > cpuFinalThreshold)
	{
		getNumBlocksAndThreads(kernel, s, maxBlocks, maxThreads, blocks, threads);
		BuildReductionKernel(kernel, threads, isPow2(s) );
		mBlocks.push_back(blocks);
		mThreads.push_back(threads);

		s = (s + (threads*2-1)) / (threads*2);
		mReductionPasses += 1;
	}

	mFinalS = s;
}

cl_kernel CRoutine_Sum::BuildReductionKernel(int whichKernel, int blockSize, int isPowOf2)
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
float CRoutine_Sum::ComputeSum(cl_mem input_buffer, cl_mem final_buffer, bool return_value)
{
	// First zero out the temporary sum buffer.
	mrZero->Zero(mTempSumBuffer, mNElements);

	int err = CL_SUCCESS;
	cl_float gpu_result = 0;
	int numThreads = mThreads[0];

	int threads = 0;
	int blocks = 0;
	cl_mem buff1 = input_buffer;
	cl_mem buff2 = mTempSumBuffer;
    size_t globalWorkSize[1];
    size_t localWorkSize[1];

	for(int kernel_id = 0; kernel_id < mReductionPasses; kernel_id++)
	{
		threads = mThreads[kernel_id];
		blocks = mBlocks[kernel_id];

		globalWorkSize[0] = blocks * threads;
		localWorkSize[0] = threads;
		cl_kernel reductionKernel = mKernels[kernel_id];

//		printf("\n Kernel: %x Blocks: %i Threads %i \n", reductionKernel, blocks, threads);
//		printf("\n Buff1:\n");
//		DumpFloatBuffer(buff1, mNElements);

#ifdef DEBUG_VERBOSE
		printf("Global: %i Local: %i\n", globalWorkSize[0], localWorkSize[0]);
#endif // DEBUG_VERBOSE

		clSetKernelArg(reductionKernel, 0, sizeof(cl_mem), (void *) &buff1);
		clSetKernelArg(reductionKernel, 1, sizeof(cl_mem), (void *) &buff2);
		clSetKernelArg(reductionKernel, 2, sizeof(cl_int), &mNElements);
		clSetKernelArg(reductionKernel, 3, sizeof(cl_float) * numThreads, NULL);
		err = clEnqueueNDRangeKernel(mQueue,reductionKernel, 1, 0, globalWorkSize, localWorkSize, 0, NULL, NULL);
		COpenCL::CheckOCLError("Unable to enqueue final parallel reduction kernel.", err);

//		printf("\n\n Buff2:\n\n");
//		DumpFloatBuffer(buff2, mNElements);

		//clFinish(mQueue);

		buff1 = buff2;
	}

	clFinish(mQueue);

	// If a few elements remain, we will need to compute their sum on the CPU:
    if (mFinalS > 1)
    {
    	cl_float h_odata[mFinalS];
        // copy result from device to host
        clEnqueueReadBuffer(mQueue, mTempSumBuffer, CL_TRUE, 0, mFinalS * sizeof(cl_float),
                            h_odata, 0, NULL, NULL);

        for(int i=0; i < mFinalS; i++)
        {
            gpu_result += h_odata[i];
        }

        // Copy this value back to the GPU
    	clEnqueueWriteBuffer(mQueue, final_buffer, CL_TRUE, 0, sizeof(cl_float), &gpu_result, 0, NULL, NULL);

        return_value = false;	// don't need to read the value anymore, it is already stored locally.
    }
    else
    {
    	// The work was all completed on the GPU.  Copy the summed value to the final buffer:
    	clEnqueueCopyBuffer(mQueue, mTempSumBuffer, final_buffer, 0, 0, sizeof(cl_float), 0, NULL, NULL);
    }

	clFinish(mQueue);

	if (return_value)
	{
		// copy final sum from device to host
		clEnqueueReadBuffer(mQueue, mTempSumBuffer, CL_TRUE, 0, sizeof(cl_float), &gpu_result, 0, NULL, NULL);
	}

	// Now copy the data over to the final GPU location.
	//clEnqueueWriteBuffer(mQueue, final_buffer, CL_TRUE, 0, sizeof(cl_float), &gpu_result, 0, NULL, NULL);

	return float(gpu_result);
}

/// Computes the sum of the OpenCL buffer, input_buffer, using Kahan summation to minimize precision losses.
float CRoutine_Sum::ComputeSum_CPU(cl_mem input_buffer)
{
	int err = CL_SUCCESS;
	cl_float tmp[mNElements];
	err |= clEnqueueReadBuffer(mQueue, input_buffer, CL_TRUE, 0, mNElements * sizeof(cl_float), tmp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy buffer back to CPU, CRoutine_Reduce_Sum::Compute_CPU() ", err);

	// Use Kahan summation to minimize lost precision.
	// http://en.wikipedia.org/wiki/Kahan_summation_algorithm
	float sum = tmp[0];
	float c = float(0.0);
	for (int i = 1; i < mNElements; i++)
	{
		float y = tmp[i] - c;
		float t = sum + y;
		c = (t - sum) - y;
		sum = t;
	}
	return sum;
}

/// Tests that the CPU and OpenCL versions of ComputeSum return the same value.
bool CRoutine_Sum::ComputeSum_Test(cl_mem input_buffer, cl_mem final_buffer)
{
	// First run the CPU version as the CL version modifies the buffers.
	float cpu_sum = ComputeSum_CPU(input_buffer);
	float cl_sum = ComputeSum(input_buffer, final_buffer, true);

	bool sum_pass = bool(fabs((cpu_sum - cl_sum)/cpu_sum) < MAX_REL_ERROR);
	printf("  CPU Value:  %0.4f\n", cpu_sum);
	printf("  CL  Value:  %0.4f\n", cl_sum);
	printf("  Difference: %0.4f\n", cpu_sum - cl_sum);
	printf("  Rel. Error: %0.4e\n", (cpu_sum - cl_sum) / cpu_sum);
	PassFail(sum_pass);

	return sum_pass;
}

/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Sum::Init(int n)
{
	int err = CL_SUCCESS;
	// Set the number of elements on which this kernel will operate.
	mNElements = n;
	BuildKernels();

	if(mTempSumBuffer == NULL)
	{
		mTempSumBuffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mNElements * sizeof(cl_float), NULL, &err);
		COpenCL::CheckOCLError("Could not create parallel sum temporary buffer.", err);
	}
}
