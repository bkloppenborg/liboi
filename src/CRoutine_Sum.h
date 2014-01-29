/*
 * CRoutine_Sum.h
 *
 *  Created on: Jan 28, 2014
 *      Author: bkloppenborg
 *
 *  Abstract base class for all routines which compute sums of buffers.
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

#ifndef CROUTINE_SUM_H_
#define CROUTINE_SUM_H_

#include "CRoutine.h"

namespace liboi {

class CRoutine_Sum: public CRoutine
{
protected:
	unsigned int mInputSize;

	// External routines, deleted elsewhere
	CRoutine_Zero * mrZero;

public:
	CRoutine_Sum(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero);
	virtual ~CRoutine_Sum();

	virtual float ComputeSum(cl_mem input_buffer, cl_mem final_buffer, bool return_value) = 0;

	virtual void Init(int n) = 0;

	static bool isPow2(unsigned int x);

	static unsigned int nextPow2( unsigned int x );

	template <typename T>
	static T Sum(valarray<T> & buffer)
	{
		// Use Kahan summation to minimize lost precision.
		// http://en.wikipedia.org/wiki/Kahan_summation_algorithm
		T sum = buffer[0];
		T c = T(0.0);
		for (int i = 1; i < buffer.size(); i++)
		{
			T y = buffer[i] - c;
			T t = sum + y;
			c = (t - sum) - y;
			sum = t;
		}

		return sum;
	}
};

} /* namespace liboi */
#endif /* CROUTINE_SUM_H_ */
