/*
 * CRoutine_FTtoV2.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_FTTOV2_H_
#define CROUTINE_FTTOV2_H_

#include "COpenCLRoutine.h"

class CRoutine_FTtoV2: public COpenCLRoutine
{
public:
	CRoutine_FTtoV2(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_FTtoV2();

	void Init(float image_scale);
	void FTtoV2(cl_mem ft_loc, int n_v2_points, cl_mem output);
	void FTtoV2_CPU(cl_mem ft_loc, int n_v2_points, cl_mem output);
};

#endif /* CROUTINE_FTTOV2_H_ */
