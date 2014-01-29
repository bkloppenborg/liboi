/*
 * CRoutine_Sum_Nvidia.h
 *
 *  Created on: Nov 14, 2011
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

#ifndef CROUTINE_SUM_NVIDIA_H_
#define CROUTINE_SUM_NVIDIA_H_

#include "CRoutine_Sum.h"

namespace liboi
{

class CRoutine_Zero;

class CRoutine_Sum_NVidia: public CRoutine_Sum
{
protected:
	unsigned int mInputSize;
	unsigned int mBufferSize;
	cl_mem mTempBuffer1;
	cl_mem mTempBuffer2;
	vector<int> mBlocks;
	vector<int> mThreads;
	int mFinalS;
	int mReductionPasses;

public:
	CRoutine_Sum_NVidia(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero);
	virtual ~CRoutine_Sum_NVidia();

public:
	cl_kernel BuildReductionKernel(int whichKernel, int blockSize, int isPowOf2);

	void BuildKernels();

	float ComputeSum(cl_mem input_buffer, cl_mem final_buffer, bool return_value);
	static void getNumBlocksAndThreads(int whichKernel, int n, int maxBlocks, int maxThreads, int &blocks, int &threads);

	void Init(int n);
};

} /* namespace liboi */

#endif /* CROUTINE_REDUCE_SUM_H_ */
