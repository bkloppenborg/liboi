/*
 * CRoutine_FTtoT3.h
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

#ifndef CROUTINE_FTTOT3_H_
#define CROUTINE_FTTOT3_H_

#include "CRoutine.h"

using namespace std;

class CRoutine_FTtoT3: public CRoutine
{
public:
	CRoutine_FTtoT3(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_FTtoT3();

	unsigned int CalculateOffset(unsigned int n_vis, unsigned int n_v2);

	void Init(void);
	void FTtoT3(cl_mem ft_input, cl_mem t3_uv_ref, cl_mem t3_uv_sign, cl_mem output, int n_vis, int n_v2, int n_t3);
	void FTtoT3_CPU(cl_mem ft_input, cl_mem t3_uv_ref, cl_mem t3_uv_sign, valarray<cl_float> & cpu_output, int n_vis, int n_v2, int n_t3, int n_uv);
	bool FTtoT3_Test(cl_mem ft_input, cl_mem t3_uv_ref, cl_mem t3_uv_sign, cl_mem output, int n_vis, int n_v2, int n_t3, int n_uv);
};

#endif /* CROUTINE_FTTOT3_H_ */
