/*
 * CRoutine_Sum.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_SUM_H_
#define CROUTINE_SUM_H_

#include "CRoutine.h"

class CRoutine_Sum: public CRoutine
{
protected:
	int num_elements;
	cl_mem mTempBuffer;
	vector<int> mBlocks;
	vector<int> mThreads;
	int mFinalS;
	int mReductionPasses;

public:
	CRoutine_Sum(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Sum();

public:
	cl_kernel BuildReductionKernel(int whichKernel, int blockSize, int isPowOf2);

	void BuildKernels();

	float ComputeSum(cl_mem input_buffer, cl_mem final_buffer);
	float ComputeSum_CPU(cl_mem input_buffer);
	bool  ComputeSum_Test(cl_mem input_buffer, cl_mem final_buffer);
	void Init(int n);


};

#endif /* CROUTINE_REDUCE_SUM_H_ */
