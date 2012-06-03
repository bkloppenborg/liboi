/*
 * CRoutine_FTtoT3.h
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

#ifndef CROUTINE_FTTOT3_H_
#define CROUTINE_FTTOT3_H_

#include "CRoutine.h"

class CRoutine_FTtoT3: public CRoutine
{
public:
	CRoutine_FTtoT3(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_FTtoT3();

	void Init(void);
	void FTtoT3(cl_mem ft_loc, cl_mem data_phasor, cl_mem uv_points, cl_mem data_sign, int n_t3, int n_v2, cl_mem output);
	void FTtoT3_CPU(cl_mem ft_loc, int n_uv, cl_mem data_phasor, cl_mem data_bsref, cl_mem data_sign, int n_t3, int n_v2, complex<float> * cpu_output);
	bool FTtoT3_Test(cl_mem ft_loc, int n_uv, cl_mem data_phasor, cl_mem uv_points, cl_mem data_sign, int n_t3, int n_v2, cl_mem output);
};

#endif /* CROUTINE_FTTOT3_H_ */
