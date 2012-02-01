/*
 * CRoutine_Normalize.h
 *
 *  Created on: Nov 17, 2011
 *      Author: bkloppenborg
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
};

#endif /* CROUTINE_NORMALIZE_H_ */
