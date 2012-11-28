/*
 * CRoutine_Chi.h
 *
 *  Created on: Dec 5, 2011
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

#ifndef CROUTINE_CHI_H_
#define CROUTINE_CHI_H_

#include "CRoutine_Sum.h"
#include "liboi.hpp"

class CRoutine_Square;
class CRoutine_Zero;

class CRoutine_Chi: public CRoutine_Sum
{
	int mChiSourceID;
	int mChiKernelID;

	cl_mem mChiTemp;
	cl_mem mChiOutput;

	// External routines, deleted elsewhere.
	CRoutine_Square * mrSquare;

public:
	CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero, CRoutine_Square * rSquare);
	~CRoutine_Chi();

	// OpenCL routines
	void Chi(cl_mem data, cl_mem data_err, cl_mem model_data, unsigned int start, unsigned int n);
	void Chi(cl_mem data, cl_mem data_err, cl_mem model, cl_mem output, unsigned int start, unsigned int n);





	float Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, bool compute_sum, bool return_value);;

	void GetChi(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output);
	void GetChi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n, float * output);

	// CPU routines:
	static void Chi(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model, valarray<cl_float> & output,
			unsigned int n_vis, unsigned int n_v2, unsigned int n_t3,
			LibOIEnums::Chi2Types chi_method);
	static void Chi(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
			unsigned int start_index, unsigned int n,
			valarray<cl_float> & output);
	static void Chi_complex_convex(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
			unsigned int start_index, unsigned int n,
			valarray<cl_float> & output);
	static void Chi_complex_nonconvex(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
			unsigned int start_index, unsigned int n,
			valarray<cl_float> & output);

	void Init(int num_elements);
};

#endif /* CROUTINE_CHI_H_ */
