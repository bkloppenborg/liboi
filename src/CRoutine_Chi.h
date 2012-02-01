/*
 * CRoutine_Chi.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_CHI_H_
#define CROUTINE_CHI_H_

#include "CRoutine_Reduce_Sum.h"

class CRoutine_Square;

class CRoutine_Chi: public CRoutine_Reduce_Sum
{
	int mChiSourceID;
	int mChiKernelID;

	cl_mem mChiTemp;
	cl_mem mChiOutput;

public:
	CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue);
	~CRoutine_Chi();

	void Chi(cl_mem data, cl_mem data_err, cl_mem model_data, int n);
	float Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, CRoutine_Square * rSquare, bool compute_sum);
	float Chi2_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n);

	void Init(int num_elements);
};

#endif /* CROUTINE_CHI_H_ */
