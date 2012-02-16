/*
 * CRoutine_FTtoT3.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_FTTOT3_H_
#define CROUTINE_FTTOT3_H_

#include "CRoutine.h"

class CRoutine_FTtoT3: public CRoutine
{
public:
	CRoutine_FTtoT3(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_FTtoT3();

	void Init(void);
	void FTtoT3(cl_mem ft_loc, cl_mem data_phasor, cl_mem uv_points, cl_mem data_sign, int n_t3, int n_v2, cl_mem output);
	void FTtoT3_CPU(cl_mem ft_loc, int n_uv, cl_mem data_phasor, cl_mem data_bsref, cl_mem data_sign, int n_t3, int n_v2, complex<float> * cpu_output);
	bool FTtoT3_Test(cl_mem ft_loc, int n_uv, cl_mem data_phasor, cl_mem uv_points, cl_mem data_sign, int n_t3, int n_v2, cl_mem output);
};

#endif /* CROUTINE_FTTOT3_H_ */
