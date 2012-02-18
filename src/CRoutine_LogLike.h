/*
 * CRoutine_LogLike.h
 *
 *  Created on: Jan 27, 2012
 *      Author: bkloppenborg
 *
 *  Computes the log likelihood based upon a chi2 estimate.
 */

#ifndef CROUTINE_LOGLIKE_H_
#define CROUTINE_LOGLIKE_H_

#include "CRoutine_Sum.h"

class CRoutine_LogLike: public CRoutine_Sum
{
protected:
	cl_mem mTempLogLike;
	cl_mem mOutput;

	int mLogLikeSourceID;
	int mLogLikeKernelID;

public:
	CRoutine_LogLike(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_LogLike();

	float LogLike(cl_mem data, cl_mem data_err, cl_mem model_data, int n);
	float LogLike_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output);
	bool LogLike_Test(cl_mem data, cl_mem data_err, cl_mem model_data, int n);

	void Init(int num_elements);
};

#endif /* CROUTINE_LOGLIKE_H_ */
