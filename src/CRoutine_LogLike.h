/*
 * CRoutine_LogLike.h
 *
 *  Created on: Jan 27, 2012
 *      Author: bkloppenborg
 *
 *  Computes the log likelihood based upon a chi2 estimate.
 */
 
  /* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * The authors request, but do not require, that you acknowledge the
 * use of this software in any publications.  See 
 * https://github.com/bkloppenborg/liboi/wiki
 * for example citations
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

#ifndef CROUTINE_LOGLIKE_H_
#define CROUTINE_LOGLIKE_H_

#include "CRoutine_Sum.h"

class CRoutine_Zero;

class CRoutine_LogLike: public CRoutine_Sum
{
protected:
	cl_mem mTempLogLike;
	cl_mem mOutput;

	int mLogLikeSourceID;
	int mLogLikeKernelID;

public:
	CRoutine_LogLike(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero);
	virtual ~CRoutine_LogLike();

	float LogLike(cl_mem data, cl_mem data_err, cl_mem model_data, int n, bool compute_sum, bool return_value);
	float LogLike_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output);
	bool LogLike_Test(cl_mem data, cl_mem data_err, cl_mem model_data, int n);

	void Init(int num_elements);
};

#endif /* CROUTINE_LOGLIKE_H_ */
