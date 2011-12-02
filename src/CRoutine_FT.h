/*
 * CRoutine_FT.h
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 *
 *  A purely virtual class representing a Fourier Transform Method
 */

#ifndef CROUTINE_FT_H_
#define CROUTINE_FT_H_

#include "COpenCLRoutine.h"

class CRoutine_FT: public COpenCLRoutine
{
public:
	CRoutine_FT(cl_device_id device, cl_context context, cl_command_queue queue);
	~CRoutine_FT();

	virtual void Init(float image_scale) = 0;
	virtual void FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem output) = 0;
};

#endif /* CROUTINE_FT_H_ */
