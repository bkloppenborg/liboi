/*
 * CRoutine_Sum_AMD.h
 *
 *  Created on: Jan 28, 2014
 *      Author: bkloppenborg
 *
 *
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

#ifndef CROUTINE_SUM_AMD_H_
#define CROUTINE_SUM_AMD_H_

#include "CRoutine_Sum.h"

namespace liboi {

class CRoutine_Sum_AMD: public liboi::CRoutine_Sum
{
protected:
	const unsigned int GROUP_SIZE;
	const unsigned int VECTOR_SIZE;
	const unsigned int MULTIPLY;  // Require because of extra addition before loading to local memory

protected:

	cl_mem mInputBuffer;				/// Local buffer of size n*group_size to store a copy of the input data
	cl_mem mOutputBuffer;				/// Local buffer of size n*group_size to store the output from the reduction process.
	unsigned int mBufferSize;

    size_t globalThreads[1] = {0};        /**< Global NDRange for the kernel */
    size_t localThreads[1] = {0};         /**< Local WorkGroup for kernel */

	struct
	{
		cl_ulong localMemoryUsed;           /**< localMemoryUsed amount of local memory used by kernel */
		size_t kernelWorkGroupSize;         /**< kernelWorkGroupSize Supported WorkGroup size as per OpenCL Runtime*/
		size_t compileWorkGroupSize[3];     /**< compileWorkGroupSize WorkGroup size as mentioned in kernel source */
	} kernelInfo;

	struct
	{
        cl_uint maxWorkItemDims;            /**< maxWorkItemDims maxWorkItemDimensions VendorId of device*/
        size_t * maxWorkItemSizes;          /**< maxWorkItemSizes maxWorkItemSizes of device*/
        size_t maxWorkGroupSize;            /**< maxWorkGroupSize max WorkGroup Size of device*/
        cl_ulong localMemSize;              /**< localMemSize localMem Size of device*/
	} deviceInfo;

    int numBlocks;                  /**< Number of groups */
    size_t groupSize;               /**< Work-group size */


public:
	CRoutine_Sum_AMD(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero);
	virtual ~CRoutine_Sum_AMD();

	float Sum(cl_mem input_buffer);

	void Init(int n);

	void setKernelInfo();
	void setDeviceInfo();
	void setWorkGroupSize();

};

} /* namespace liboi */
#endif /* CROUTINE_SUM_AMD_H_ */
