/*
 * CRoutine_FTtoT3.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_FTTOT3_H_
#define CROUTINE_FTTOT3_H_

#include "COpenCLRoutine.h"

class CRoutine_FTtoT3: public COpenCLRoutine
{
public:
	CRoutine_FTtoT3(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_FTtoT3();

	void Init(float image_scale);
	void FTtoT3(cl_mem ft_loc, cl_mem data_phasor, cl_mem uv_points, cl_mem data_sign, int n_t3, cl_mem output);
};

#endif /* CROUTINE_FTTOT3_H_ */
