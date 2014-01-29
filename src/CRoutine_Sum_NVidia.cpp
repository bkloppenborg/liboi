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
#include "CRoutine_Sum_NVidia.h"
#include "CRoutine_Zero.h"

using namespace std;

namespace liboi
{

CRoutine_Sum_NVidia::CRoutine_Sum_NVidia(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	: CRoutine_Sum(device, context, queue, rZero)
{
	// Specify the source location, set temporary buffers to null
	mSource.push_back("reduce_sum_float_nvidia.cl");
	mTempBuffer1 = NULL;
	mTempBuffer2 = NULL;
	mBufferSize = 0;
	mInputSize = 0;
	mFinalS = 0;
	mReductionPasses = 0;

	// External routines, do not delete/deallocate here.
	mrZero = rZero;
}

CRoutine_Sum_NVidia::~CRoutine_Sum_NVidia()
{
	if(mTempBuffer1) clReleaseMemObject(mTempBuffer1);
	if(mTempBuffer2) clReleaseMemObject(mTempBuffer2);
}

void CRoutine_Sum_NVidia::getNumBlocksAndThreads(int whichKernel, int n, int maxBlocks, int maxThreads, int &blocks, int &threads)
{

	threads = (n < maxThreads*2) ? nextPow2((n + 1)/ 2) : maxThreads;
	blocks = (n + (threads * 2 - 1)) / (threads * 2);

	if (whichKernel == 6)
		blocks = min(maxBlocks, blocks);
}


void CRoutine_Sum_NVidia::BuildKernels()
{
	int whichKernel = 6;
	int numBlocks = 0;
	int numThreads = 0;
#ifdef __APPLE__
	int maxThreads = 64;
#else
	int maxThreads = 128;
#endif
	int maxBlocks = 64;
	int cpuFinalThreshold = 1;

	getNumBlocksAndThreads(whichKernel, mBufferSize, maxBlocks, maxThreads, numBlocks, numThreads);
	BuildReductionKernel(whichKernel, numThreads, isPow2(mBufferSize) );
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

cl_kernel CRoutine_Sum_NVidia::BuildReductionKernel(int whichKernel, int blockSize, int isPowOf2)
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

float CRoutine_Sum_NVidia::Sum(cl_mem input_buffer)
{
	// First zero out the temporary sum buffer.
	mrZero->Zero(mTempBuffer1, mBufferSize);

	int err = CL_SUCCESS;
	// Copy the input buffer into mTempBuffer1
	// The work was all completed on the GPU.  Copy the summed value to the final buffer:
	err = clEnqueueCopyBuffer(mQueue, input_buffer, mTempBuffer1, 0, 0, mInputSize * sizeof(cl_float), 0, NULL, NULL);
	COpenCL::CheckOCLError("Unable to copy summed value into final buffer. CRoutine_Sum::ComputeSum", err);
	clFinish(mQueue);

	// Init locals:
	cl_float gpu_result = 0;
	int numThreads = mThreads[0];

	int threads = 0;
	int blocks = 0;
	cl_mem buff1 = mTempBuffer1;
	cl_mem buff2 = mTempBuffer2;
    size_t globalWorkSize[1];
    size_t localWorkSize[1];
    cl_kernel reductionKernel;

	for(int kernel_id = 0; kernel_id < mReductionPasses; kernel_id++)
	{
		threads = mThreads[kernel_id];
		blocks = mBlocks[kernel_id];

		globalWorkSize[0] = blocks * threads;
		localWorkSize[0] = threads;
		reductionKernel = mKernels[kernel_id];

		clSetKernelArg(reductionKernel, 0, sizeof(cl_mem), (void *) &buff1);
		clSetKernelArg(reductionKernel, 1, sizeof(cl_mem), (void *) &buff2);
		clSetKernelArg(reductionKernel, 2, sizeof(cl_int), &mBufferSize);
		clSetKernelArg(reductionKernel, 3, sizeof(cl_float) * numThreads, NULL);
		err = clEnqueueNDRangeKernel(mQueue, reductionKernel, 1, 0, globalWorkSize, localWorkSize, 0, NULL, NULL);
		COpenCL::CheckOCLError("Unable to enqueue parallel reduction kernel. CRoutine_Sum_NVidia::ComputeSum", err);

		buff1 = buff2;
	}

	clFinish(mQueue);

	// If a few elements remain, we will need to compute their sum on the CPU:
    if (mFinalS > 1)
    {
    	cl_float h_odata[mFinalS];
        // copy result from device to host
        err = clEnqueueReadBuffer(mQueue, mTempBuffer2, CL_TRUE, 0, mFinalS * sizeof(cl_float), h_odata, 0, NULL, NULL);
		COpenCL::CheckOCLError("Unable to copy temporary sum buffer to host. CRoutine_Sum::ComputeSum", err);

        for(int i=0; i < mFinalS; i++)
        {
            gpu_result += h_odata[i];
        }

    }
    else
    {
    	// The work was all completed on the GPU.  Copy the summed value to the CPU:
		err = clEnqueueReadBuffer(mQueue, mTempBuffer2, CL_TRUE, 0, sizeof(cl_float), &gpu_result, 0, NULL, NULL);
		COpenCL::CheckOCLError("Unable to copy summed value to host. CRoutine_Sum::ComputeSum", err);
    }

	return float(gpu_result);
}


/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Sum_NVidia::Init(int n)
{
	int err = CL_SUCCESS;

	mInputSize = n;
	mBufferSize = n;

	// The NVidia SDK kernel on which this routine is based is designed only for power-of-two
	// sized buffers. Because of this, we'll create internal buffers that round up to the
	// next highest power of two.
	if(!isPow2(mBufferSize))
		mBufferSize = nextPow2(mBufferSize);

	// TODO: Workaround for issue 32 in which kernel fails to compute sums for N = [33 - 64]
	// https://github.com/bkloppenborg/liboi/issues/32
	if(mBufferSize < 128)
		mBufferSize = 128;

	BuildKernels();

	if(mTempBuffer1 == NULL)
	{
		mTempBuffer1 = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mBufferSize * sizeof(cl_float), NULL, &err);
		COpenCL::CheckOCLError("Could not create parallel sum temporary buffer.", err);
	}

	if(mTempBuffer2 == NULL)
	{
		mTempBuffer2 = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mBufferSize * sizeof(cl_float), NULL, &err);
		COpenCL::CheckOCLError("Could not create parallel sum temporary buffer.", err);
	}
}

} /* namespace liboi */
