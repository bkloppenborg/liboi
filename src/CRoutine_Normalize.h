/*
 * CRoutine_Normalize.h
 *
 *  Created on: Nov 17, 2011
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

#ifndef CROUTINE_NORMALIZE_H_
#define CROUTINE_NORMALIZE_H_

#include "CRoutine.h"

class CRoutine_Normalize: public CRoutine
{
public:
	CRoutine_Normalize(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Normalize();

	void Init();

	void Normalize(cl_mem image, int image_width, int image_height, cl_mem divisor);
	void Normalize_CPU(cl_mem image, int image_width, int image_height, cl_mem divisor, valarray<cl_float> & output);
	bool Normalize_Test(cl_mem image, int image_width, int image_height, cl_mem divisor);
};

#endif /* CROUTINE_NORMALIZE_H_ */
