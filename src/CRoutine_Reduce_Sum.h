/*
 * CRoutine_Reduce_Sum.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_REDUCE_SUM_H_
#define CROUTINE_REDUCE_SUM_H_

#include "CRoutine_Reduce.h"

class CRoutine_Reduce_Sum: public CRoutine_Reduce
{
protected:
	cl_mem mTempBuffer;

public:
	CRoutine_Reduce_Sum(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Reduce_Sum();

public:
	cl_kernel getReductionKernel(int whichKernel, int blockSize, int isPowOf2);

	float ComputeSum(cl_mem input_buffer, cl_mem final_buffer, bool copy_back);
	float ComputeSum_CPU(cl_mem input_buffer);
	bool  ComputeSum_Test(cl_mem input_buffer, cl_mem final_buffer, bool copy_back);
	void Init(int n);


};

#endif /* CROUTINE_REDUCE_SUM_H_ */
