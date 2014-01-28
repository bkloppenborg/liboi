/*
 * CRoutine_Sum.h
 *
 *  Created on: Jan 28, 2014
 *      Author: bkloppenborg
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
