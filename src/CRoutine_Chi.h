/*
 * CRoutine_Chi.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
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

#ifndef CROUTINE_CHI_H_
#define CROUTINE_CHI_H_

#include "CRoutine_Sum.h"

class CRoutine_Square;
class CRoutine_Zero;

class CRoutine_Chi: public CRoutine_Sum
{
	int mChiSourceID;
	int mChiKernelID;

	cl_mem mChiTemp;
	cl_mem mChiOutput;

	cl_float * mCPUChiTemp;

	// External routines, deleted elsewhere.
	CRoutine_Square * mrSquare;

public:
	CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero, CRoutine_Square * rSquare);
	~CRoutine_Chi();

	void Chi(cl_mem data, cl_mem data_err, cl_mem model_data, int n);
	void Chi_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n);
	bool Chi_Test(cl_mem data, cl_mem data_err, cl_mem model_data, int n);

	float Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, bool compute_sum, bool return_value);
	float Chi2_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n, bool compute_sum);
	bool Chi2_Test(cl_mem data, cl_mem data_err, cl_mem model_data, int n, bool compute_sum);

	void GetChi(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output);

	void Init(int num_elements);
};

#endif /* CROUTINE_CHI_H_ */
