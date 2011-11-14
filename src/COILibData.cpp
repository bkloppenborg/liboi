/*
 * COILibData.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "COILibData.h"

COILibData::COILibData()
{
	n_v2 = 0;
	v2_loc = NULL;
	v2_uv = NULL;
	n_t3 = 0;
	t3_loc = NULL;
	t3_phasor = NULL;
	t3_uv = NULL;
}

COILibData::~COILibData()
{
	// Release memory on the GPU.
	if(v2_loc) clReleaseMemObject(v2_loc);
	if(v2_uv) clReleaseMemObject(v2_uv);
	if(t3_loc) clReleaseMemObject(t3_loc);
	if(t3_phasor) clReleaseMemObject(t3_phasor);
	if(t3_uv) clReleaseMemObject(t3_uv);
}
