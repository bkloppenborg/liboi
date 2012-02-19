/*
 * CRoutine_Square.h
 *
 *  Created on: Feb 1, 2012
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_SQUARE_H_
#define CROUTINE_SQUARE_H_

#include "CRoutine.h"

class CRoutine_Square: public CRoutine
{
public:
	CRoutine_Square(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Square();

	void Init();
	void Square(cl_mem input, cl_mem output, int buffer_size, int data_size);
};

#endif /* CROUTINE_SQUARE_H_ */
