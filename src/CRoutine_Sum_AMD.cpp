/*
 * CRoutine_Sum_AMD.cpp
 *
 *  Created on: Jan 28, 2014
 *      Author: bkloppenborg
 */

 /*
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

#include "CRoutine_Sum_AMD.h"
#include "CRoutine_Zero.h"
#include "CRoutine_Sum_NVidia.h"

namespace liboi {

CRoutine_Sum_AMD::CRoutine_Sum_AMD(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	: 	GROUP_SIZE(256), VECTOR_SIZE(4), MULTIPLY(2),
	  	CRoutine_Sum(device, context, queue, rZero)
{

	// Specify the source location, set temporary buffers to null
	mSource.push_back("reduce_sum_float_amd.cl");

    globalThreads = new size_t[1];
    localThreads = new size_t[1];

	mrZero = rZero;

	// Set temporary buffer sizes and memory addresses to zero:
	mBufferSize = 0;
	mInputBuffer = NULL;
	mOutputBuffer = NULL;

    numBlocks = 0;
    groupSize = GROUP_SIZE;


	// Init the kernel and device info structs.
	kernelInfo.localMemoryUsed = 0;
	kernelInfo.kernelWorkGroupSize = 0;
	kernelInfo.compileWorkGroupSize[0] = 0;
	kernelInfo.compileWorkGroupSize[1] = 0;
	kernelInfo.compileWorkGroupSize[2] = 0;

	deviceInfo.maxWorkItemDims = 0;
	deviceInfo.maxWorkItemSizes = NULL;
	deviceInfo.maxWorkGroupSize = 0;
	deviceInfo.localMemSize = 0;
}

CRoutine_Sum_AMD::~CRoutine_Sum_AMD()
{
	if(mInputBuffer) clReleaseMemObject(mInputBuffer);
	if(mOutputBuffer) clReleaseMemObject(mOutputBuffer);

	delete globalThreads;
	delete localThreads;
}


/// Computes the sum of the input_buffer. Stores the result in the final_buffer and returns the result
/// if return_value is true. Returns 0 otherwise.
float CRoutine_Sum_AMD::Sum(cl_mem input_buffer)
{
	int status = CL_SUCCESS;
	cl_float output = 0;
    cl_event sumCompleteEvent;
	cl_event outputMappedEvent;
	cl_event outputUnmapEvent;

	// First zero out the temporary sum buffer.
	mrZero->Zero(mInputBuffer, mBufferSize);

	int err = CL_SUCCESS;
	// Copy the input buffer into mTempBuffer1
	// The work was all completed on the GPU.  Copy the summed value to the final buffer:
	err = clEnqueueCopyBuffer(mQueue, input_buffer, mInputBuffer, 0, 0, mInputSize * sizeof(cl_float), 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueCopyBuffer failed.");
	clFinish(mQueue);

	// Set appropriate arguments to the kernel the input array
	status  = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), (void *) &mInputBuffer);
	status |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), (void *) &mOutputBuffer);
	status |= clSetKernelArg(mKernels[0], 2, groupSize * sizeof(cl_float), NULL);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

	// Enqueue the kernel:
	status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, 0, globalThreads, localThreads, 0, NULL, &sumCompleteEvent);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	status = clFlush(mQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&sumCompleteEvent);
	CHECK_OPENCL_ERROR(status, "waitForEventAndRelease failed.");

	// Map the output buffer:
	cl_float * output_map = (cl_float*)clEnqueueMapBuffer(mQueue, mOutputBuffer, CL_FALSE, CL_MAP_READ, 0,
			numBlocks * sizeof(cl_float), 0, NULL, &outputMappedEvent, &status);
	CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer failed.");

	status = clFlush(mQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&outputMappedEvent);
	CHECK_ERROR(status, OPENCL_SUCCESS, "waitForEventAndRelease failed.");

	// Now that the buffer is mapped, sum it.
	for(int i = 0; i < numBlocks; i++)
	{
		output += output_map[i];
	}

	// Release the mapped buffer:
	status = clEnqueueUnmapMemObject(mQueue, mOutputBuffer, (void*) output_map, 0, NULL, &outputUnmapEvent);
	CHECK_OPENCL_ERROR(status, "clEnqueueUnmapMemObject failed.");

	status = clFlush(mQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.");

	status = waitForEventAndRelease(&outputUnmapEvent);
	CHECK_ERROR(status, OPENCL_SUCCESS, "waitForEventAndRelease failed.");

	return output;
}

/// Run queries to get additional details about the kernel.
void CRoutine_Sum_AMD::setKernelInfo()
{
	cl_int status = CL_SUCCESS;

	status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE,
			sizeof(size_t), &kernelInfo.kernelWorkGroupSize, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo(CL_KERNEL_WORK_GROUP_SIZE) failed.");

	status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_LOCAL_MEM_SIZE,
			sizeof(cl_ulong), &kernelInfo.localMemoryUsed, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo(CL_KERNEL_LOCAL_MEM_SIZE) failed.");

	status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
			sizeof(size_t) * 3, kernelInfo.compileWorkGroupSize, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo(CL_KERNEL_COMPILE_WORK_GROUP_SIZE) failed.");
}

/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Sum_AMD::Init(int n)
{
	int status = CL_SUCCESS;
	mInputSize = n;
	mBufferSize = n;

	// Push the buffer up to the next greatest power of two. This ensures that
	// the global work size is evenly divisible by the local work size in
	// the call to clEnqueueNDKernel.
	if(!isPow2(mBufferSize))
		mBufferSize = nextPow2(mBufferSize);

	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "reduce_sum_float_amd", mSource[0]);

    // Determine and set work group size, set the number of blocks apropriately.
	setWorkGroupSize();
    numBlocks = mBufferSize / ((cl_float)groupSize * MULTIPLY);

    if(mInputBuffer == NULL)
	{
    	mInputBuffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mBufferSize * sizeof(cl_float), NULL, &status);
    	CHECK_OPENCL_ERROR(status, "clCreateBuffer(mInputBuffer) failed.");
    	// fill the buffer with zeros
    	mrZero->Zero(mInputBuffer, mBufferSize);
	}

	if(mOutputBuffer == NULL)
	{
		mOutputBuffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mBufferSize * sizeof(cl_float), NULL, &status);
    	CHECK_OPENCL_ERROR(status, "clCreateBuffer(mOutputBuffer) failed.");
    	// fill the buffer with zeros
    	mrZero->Zero(mOutputBuffer, mBufferSize);
	}
}

void CRoutine_Sum_AMD::setWorkGroupSize()
{
    cl_int status = 0;

    setKernelInfo();
    setDeviceInfo();

    /**
     * If groupSize exceeds the maximum supported on kernel
     * fall back
     */
    if(groupSize > kernelInfo.kernelWorkGroupSize)
    {
// Enable to be more verbose.
//        if(!sampleArgs->quiet)
//        {
//            std::cout << "Out of Resources!" << std::endl;
//            std::cout << "Group Size specified : " << groupSize << std::endl;
//            std::cout << "Max Group Size supported on the kernel : "
//                      << kernelInfo.kernelWorkGroupSize << std::endl;
//            std::cout << "Falling back to " << kernelInfo.kernelWorkGroupSize << std::endl;
//        }
        groupSize = kernelInfo.kernelWorkGroupSize;
    }

    if(groupSize > deviceInfo.maxWorkItemSizes[0] ||
            groupSize > deviceInfo.maxWorkGroupSize)
    {
        std::cout << "Unsupported: Device does not support"
                  << "requested number of work items.";
    }

    if(kernelInfo.localMemoryUsed > deviceInfo.localMemSize)
    {
        std::cout << "Unsupported: Insufficient local memory on device." << std::endl;
    }

    globalThreads[0] = mBufferSize / MULTIPLY;
    localThreads[0] = groupSize;
}

void CRoutine_Sum_AMD::setDeviceInfo()
{
    cl_int status = 0;

    status = clGetDeviceInfo(mDeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),
    		&deviceInfo.maxWorkGroupSize, NULL);
	CHECK_OPENCL_ERROR(status, "clGetDeviceInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE) failed.");

    status = clGetDeviceInfo(mDeviceID, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong),
    		&deviceInfo.localMemSize, NULL);
	CHECK_OPENCL_ERROR(status, "clGetDeviceInfo(CL_DEVICE_LOCAL_MEM_SIZE) failed.");

    //Get max work item dimensions
    status = clGetDeviceInfo(mDeviceID, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint),
    		&deviceInfo.maxWorkItemDims, NULL);
	CHECK_OPENCL_ERROR(status, "clGetDeviceInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS) failed.");

    //Get max work item sizes
	if(deviceInfo.maxWorkItemSizes != NULL)
		delete deviceInfo.maxWorkItemSizes;

    deviceInfo.maxWorkItemSizes = new size_t[deviceInfo.maxWorkItemDims];

    status = clGetDeviceInfo(mDeviceID, CL_DEVICE_MAX_WORK_ITEM_SIZES, deviceInfo.maxWorkItemDims * sizeof(size_t),
    		deviceInfo.maxWorkItemSizes, NULL);
	CHECK_OPENCL_ERROR(status, "clGetDeviceInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES) failed.");
}


} /* namespace liboi */
