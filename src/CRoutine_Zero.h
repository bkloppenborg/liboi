/*
 * CRoutine_Zero.h
 *
 *  Created on: Feb 1, 2012
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

#ifndef CROUTINE_ZERO_H_
#define CROUTINE_ZERO_H_

#include "CRoutine.h"

class CRoutine_Zero: public CRoutine
{
public:
	CRoutine_Zero(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Zero();

	void Init();
	void Zero(cl_mem input, int buffer_size);
};

#endif /* CROUTINE_ZERO_H_ */
