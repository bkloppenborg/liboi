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

public:
	CRoutine_Reduce_Sum(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Reduce_Sum();

public:

	float ComputeSum(bool copy_back, cl_mem final_buffer, cl_mem input_buffer, cl_mem output_buffer, cl_mem partial_sum_buffer);
	float Compute_CPU(cl_mem input_buffer, int n);
	void Init(int n);


};

#endif /* CROUTINE_REDUCE_SUM_H_ */
