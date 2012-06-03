/*
 * CRoutine_FT.h
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 *
 *  A purely virtual class representing a Fourier Transform Method
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

#ifndef CROUTINE_FT_H_
#define CROUTINE_FT_H_

#include "CRoutine.h"
#include <complex>

class CRoutine_FT: public CRoutine
{
public:
	CRoutine_FT(cl_device_id device, cl_context context, cl_command_queue queue);
	~CRoutine_FT();

	virtual void Init(float image_scale) = 0;
	virtual void FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output) = 0;
	virtual void FT_CPU(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, complex<float> * cpu_output) = 0;
	virtual bool FT_Test(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output) = 0;
};

#endif /* CROUTINE_FT_H_ */
