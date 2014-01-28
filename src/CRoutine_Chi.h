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

#include "CRoutine_Sum_NVidia.h"
#include "liboi.hpp"

namespace liboi
{

class CRoutine_Square;
class CRoutine_Zero;

class CRoutine_Chi: public CRoutine_Sum_NVidia
{
protected:
	int mChiSourceID;
	int mChiConvexSourceID;
	int mChiNonConvexSourceID;

	int mChiKernelID;
	int mChiConvexKernelID;
	int mChiNonConvexKernelID;

	unsigned int mChiBufferSize;
	cl_mem mChiOutput;	// All OpenCL calculations store their result here if the convenience functions are used.
	cl_mem mChiSquaredOutput;	// Chi2 values are stored here.

	// External routines, deleted elsewhere.
	CRoutine_Square * mrSquare;

public:
	CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero);
	CRoutine_Chi(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero, CRoutine_Square * rSquare);
	virtual ~CRoutine_Chi();

	void Chi(cl_mem data, cl_mem data_err, cl_mem model, cl_mem output, unsigned int start, unsigned int n);
	void ChiComplexConvex(cl_mem data, cl_mem data_err, cl_mem model, cl_mem output, unsigned int start, unsigned int n);
	void ChiComplexNonConvex(cl_mem data, cl_mem data_err, cl_mem model, cl_mem output, unsigned int start, unsigned int n);

	void Chi(cl_mem data, cl_mem data_err, cl_mem model_data,
			LibOIEnums::Chi2Types complex_chi_method,
			unsigned int n_vis, unsigned int n_v2, unsigned int n_t3);

	void Chi(cl_mem data, cl_mem data_err, cl_mem model_data,
			LibOIEnums::Chi2Types complex_chi_method,
			unsigned int n_vis, unsigned int n_v2, unsigned int n_t3,
			float * output, unsigned int & output_size);

	static void Chi(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
			unsigned int start_index, unsigned int n,
			valarray<cl_float> & output);

	static void Chi_complex_convex(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
			unsigned int start_index, unsigned int n,
			valarray<cl_float> & output);

	static void Chi_complex_nonconvex(valarray<cl_float> & data, valarray<cl_float> & data_err, valarray<cl_float> & model,
			unsigned int start_index, unsigned int n,
			valarray<cl_float> & output);

	float Chi2(cl_mem data, cl_mem data_err, cl_mem model_data,
			LibOIEnums::Chi2Types complex_chi_method,
			unsigned int n_vis, unsigned int n_v2, unsigned int n_t3, bool compute_sum);

	void Chi2(cl_mem data, cl_mem data_err, cl_mem model_data,
			LibOIEnums::Chi2Types complex_chi_method,
			unsigned int n_vis, unsigned int n_v2, unsigned int n_t3,
			float * output, unsigned int & output_size);

	void Init(unsigned int num_elements);
};

} /* namespace liboi */

#endif /* CROUTINE_CHI_H_ */
