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

#ifndef CROUTINE_LOGLIKE_H_
#define CROUTINE_LOGLIKE_H_

#include "CRoutine_Chi.h"

namespace liboi
{

class CRoutine_Zero;

class CRoutine_LogLike: public CRoutine_Chi
{
protected:
	cl_mem mLogLikeOutput;

	int mLogLikeSourceID;
	int mLogLikeKernelID;

public:
	CRoutine_LogLike(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero);
	virtual ~CRoutine_LogLike();

	void LogLike(cl_mem chi_output, cl_mem data_err, cl_mem output, unsigned int n);
	void LogLike(cl_mem data, cl_mem data_err, cl_mem model_data,
			LibOIEnums::Chi2Types complex_chi_method,
			unsigned int n_vis, unsigned int n_v2, unsigned int n_t3);

	float LogLike(cl_mem data, cl_mem data_err, cl_mem model_data,
			LibOIEnums::Chi2Types complex_chi_method,
			unsigned int n_vis, unsigned int n_v2, unsigned int n_t3, bool compute_sum);

	static void LogLike(valarray<cl_float> & chi_output, valarray<cl_float> & data_err, valarray<cl_float> & output, unsigned int n);

	void Init(int num_elements);
};

} /* namespace liboi */

#endif /* CROUTINE_LOGLIKE_H_ */
