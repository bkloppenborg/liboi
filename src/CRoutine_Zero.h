/*
 * CRoutine_Zero.h
 *
 *  Created on: Feb 1, 2012
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

#ifndef CROUTINE_ZERO_H_
#define CROUTINE_ZERO_H_

#include "CRoutine.h"

namespace liboi
{

class CRoutine_Zero: public CRoutine
{
public:
	CRoutine_Zero(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Zero();

	void Init();
	void Zero(cl_mem input, int buffer_size);

	template <typename T>
	void Zero(valarray<T> & buffer, unsigned int buffer_size)
	{
		for(int i = 0; i < buffer_size; i++)
			buffer[i] = 0;
	}
};

} /* namespace liboi */

#endif /* CROUTINE_ZERO_H_ */
