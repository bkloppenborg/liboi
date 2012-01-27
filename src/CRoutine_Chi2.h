/*
 * CRoutine_Chi2.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_CHI2_H_
#define CROUTINE_CHI2_H_

#include "CRoutine_Reduce_Sum.h"

class CRoutine_Chi2: public CRoutine_Reduce_Sum
{
	int mChi2SourceID;
	int mChi2KernelID;

	cl_mem mChi2Temp;
	cl_mem mChi2Output;

public:
	CRoutine_Chi2(cl_device_id device, cl_context context, cl_command_queue queue);
	~CRoutine_Chi2();

	float Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n);
	float Chi2_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n);

	void Init(int num_elements);
};

#endif /* CROUTINE_CHI2_H_ */
