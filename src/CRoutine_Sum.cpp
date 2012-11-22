/*
 * CRoutine_Sum.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Routine to compute the sum of an OpenCL cl_mem buffer
 *  
 *  Note:
 *      This file contains the following function bodies from the NVIDIA 
 *      OpenCL SDK parallel reduction sample code in unmodified form:
 *          nextPow2, isPow2, getNumBlocksAndThread.
 *      The following function bodies are mostly from the NVIDIA OpenCL
 *      SDK parallel reduction sample code, with some modifications to
 *      work within the LIBOI framework:
 *          BuildKernels, ComputeSum
 */
 
 /*
 * Portions of this file are 
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 */
 
/* 
 * Remaining material 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * If you use this software as part of a scientific publication, please cite as:
 *
 * Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library"
 * (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
 *
 * This file is part of the OpenCL Interferometry Library (LIBOI).
 * 
 * LIBOI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
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

		clSetKernelArg(reductionKernel, 0, sizeof(cl_mem), (void *) &buff1);
		clSetKernelArg(reductionKernel, 1, sizeof(cl_mem), (void *) &buff2);
		clSetKernelArg(reductionKernel, 2, sizeof(cl_int), &mNElements);
		clSetKernelArg(reductionKernel, 3, sizeof(cl_float) * numThreads, NULL);
		err = clEnqueueNDRangeKernel(mQueue,reductionKernel, 1, 0, globalWorkSize, localWorkSize, 0, NULL, NULL);
		COpenCL::CheckOCLError("Unable to enqueue final parallel reduction kernel.", err);

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

	return float(gpu_result);
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
