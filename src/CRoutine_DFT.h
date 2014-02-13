/*
 * CRoutine_DFT.h
 *
 *  Created on: Dec 2, 2011
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

#ifndef CROUTINE_DFT_H_
#define CROUTINE_DFT_H_

#include "CRoutine_FT.h"

namespace liboi
{

class CRoutine_DFT: public CRoutine_FT
{
	float mImageScale;
public:
	CRoutine_DFT(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_DFT();

	void Init(float image_scale);
	void FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output);

	void FT(cl_float2 uv_point,
			valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
			cl_float2 & cpu_output);

	void FT(valarray<cl_float2> & uv_points, unsigned int n_uv_points,
			valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
			valarray<cl_float2> & cpu_output);
};

} /* namespace liboi */

#endif /* CROUTINE_DFT_H_ */
