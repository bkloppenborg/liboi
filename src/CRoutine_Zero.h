/*
 * CRoutine_Zero.h
 *
 *  Created on: Feb 1, 2012
 *      Author: bkloppenborg
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
