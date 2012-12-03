/*
 * CRoutine_Square.h
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

#ifndef CROUTINE_SQUARE_H_
#define CROUTINE_SQUARE_H_

#include "CRoutine.h"

namespace liboi
{

class CRoutine_Square: public CRoutine
{
public:
	CRoutine_Square(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Square();

	void Init();
	void Square(cl_mem input, cl_mem output, unsigned int buffer_size, unsigned int data_size);

	template <typename T>
	static void Square(valarray<T> & input_buffer, valarray<T> & output_buffer, unsigned int buffer_size, unsigned int data_size)
	{
		for(int i = 0; i < buffer_size && i < data_size; i++)
			output_buffer[i] = input_buffer[i] * input_buffer[i];
	}
};

} /* namespace liboi */

#endif /* CROUTINE_SQUARE_H_ */
