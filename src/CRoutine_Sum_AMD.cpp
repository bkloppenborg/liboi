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

namespace liboi {

CRoutine_Sum_AMD::CRoutine_Sum_AMD(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	: CRoutine_Sum(device, context, queue, rZero)
{
	// Specify the source location, set temporary buffers to null
	mSource.push_back("reduce_sum_float_amd.cl");
	mInputSize = 0;

	mrZero = rZero;
}

CRoutine_Sum_AMD::~CRoutine_Sum_AMD()
{
	if(output_buffer) clReleaseMemObject(output_buffer);
}


/// Computes the sum of the input_buffer. Stores the result in the final_buffer and returns the result
/// if return_value is true. Returns 0 otherwise.
float CRoutine_Sum_AMD::ComputeSum(cl_mem input_buffer, cl_mem final_buffer, bool return_value)
{
	// TODO: Copy data back to the final buffer
	// TODO: Determine where these are defined.
    size_t globalThreads[1];        /**< Global NDRange for the kernel */
    size_t localThreads[1];         /**< Local WorkGroup for kernel */
    cl_uint length;                 /**< length of the input array */
    int numBlocks;                  /**< Number of groups */
    size_t groupSize;               /**< Work-group size */
#define GROUP_SIZE 256
#define VECTOR_SIZE 4
#define MULTIPLY  2  // Require because of extra addition before loading to local memory

	int status = CL_SUCCESS;
	cl_float output = 0;
    cl_event sumCompleteEvent;
	cl_event outputMappedEvent;
	cl_event outputUnmapEvent;

	// Set appropriate arguments to the kernel the input array
	status = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), input_buffer);
	status |= clSetKernelArg(mKernels[0], 1, sizeof(cl_mem), output_buffer);
	status |= clSetKernelArg(mKernels[0], 2, groupSize * sizeof(cl_float), NULL);
	COpenCL::CheckOCLError("Unable to set kernel arguments. CRoutine_Sum_AMD::ComputeSum", status);

	// Enqueue the kernel:
	status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, globalThreads, localThreads, 0, NULL, &sumCompleteEvent);
	COpenCL::CheckOCLError("Unable to enqueue parallel sum kernel. CRoutine_Sum_AMD::ComputeSum", status);

	status = clFlush(mQueue);
	COpenCL::CheckOCLError("clFlush failed. CRoutine_Sum_AMD::ComputeSum", status);

	status = waitForEventAndRelease(&sumCompleteEvent);
	COpenCL::CheckOCLError("Wait for event 'sum_complete' failed. CRoutine_Sum_AMD::ComputeSum", status);

	// Map the output buffer:
	cl_float * output_map = (cl_float*)clEnqueueMapBuffer(mQueue, output_buffer, CL_FALSE, CL_MAP_READ, 0,
			numBlocks * sizeof(cl_float), 0, NULL, &outputMappedEvent, &status);
	COpenCL::CheckOCLError("Could not map output buffer. CRoutine_Sum_AMD::ComputeSum", status);

	status = clFlush(mQueue);
	COpenCL::CheckOCLError("clFlush failed. CRoutine_Sum_AMD::ComputeSum", status);

	status = waitForEventAndRelease(&outputMappedEvent);
	COpenCL::CheckOCLError("waitForEventAndRelease(&outputMappedEvent) failed. CRoutine_Sum_AMD::ComputeSum", status);

	// Now that the buffer is mapped, sum it.
	for(int i = 0; i < numBlocks * VECTOR_SIZE; i++)
	{
		output += output_map[i];
	}

	// Release the mapped buffer:
	status = clEnqueueUnmapMemObject(mQueue, output_buffer, (void*) output_map, 0, NULL, &outputUnmapEvent);
	COpenCL::CheckOCLError("clEnqueueUnmapMemObject(output_buffer) failed. CRoutine_Sum_AMD::ComputeSum", status);

	status = clFlush(mQueue);
	COpenCL::CheckOCLError("clFlush failed. CRoutine_Sum_AMD::ComputeSum", status);

	status = waitForEventAndRelease(&outputUnmapEvent);
	COpenCL::CheckOCLError("WaitForEventAndRelease(outUnMapEvt). CRoutine_Sum_AMD::ComputeSum", status);

	return output;
}

/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Sum_AMD::Init(int n)
{
	int status = CL_SUCCESS;

	mInputSize = n;
	mBufferSize = n;

	// Read the kernel, compile it
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "reduce_sum_float_amd", mSource[0]);

	if(output_buffer == NULL)
	{
		output_buffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, mBufferSize * sizeof(cl_float), NULL, &status);
		COpenCL::CheckOCLError("Could not create parallel sum temporary buffer.", status);
	}
}

} /* namespace liboi */
