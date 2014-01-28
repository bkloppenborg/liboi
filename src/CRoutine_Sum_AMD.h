/*
 * CRoutine_Sum_AMD.h
 *
 *  Created on: Jan 28, 2014
 *      Author: bkloppenborg
 *
 *
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

#ifndef CROUTINE_SUM_AMD_H_
#define CROUTINE_SUM_AMD_H_

#include "CRoutine_Sum.h"

namespace liboi {

class CRoutine_Sum_AMD: public liboi::CRoutine_Sum
{
protected:
	cl_mem output_buffer;
	unsigned int mBufferSize;

public:
	CRoutine_Sum_AMD(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero);
	virtual ~CRoutine_Sum_AMD();

	float ComputeSum(cl_mem input_buffer, cl_mem final_buffer, bool return_value);

	void Init(int n);

};

} /* namespace liboi */
#endif /* CROUTINE_SUM_AMD_H_ */
