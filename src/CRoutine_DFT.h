/*
 * CRoutine_DFT.h
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_DFT_H_
#define CROUTINE_DFT_H_

#include "COpenCLRoutine.h"

class CRoutine_DFT: public CRoutine_FT
{
public:
	CRoutine_DFT();
	~CRoutine_DFT();

	void Init(float image_scale);
	void FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem output);
};

#endif /* CROUTINE_DFT_H_ */
