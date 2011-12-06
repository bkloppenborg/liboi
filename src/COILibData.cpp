/*
 * COILibData.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "COILibData.h"

COILibData::COILibData(oi_data * data)
{
	mData = data;

	// Read out information about the data.
	n_uv = data->nuv;
	n_v2 = data->npow;
	n_t3 = data->nbis;

	v2_loc = NULL;
	v2_uv = NULL;
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

	// Free local memory:
	free_oi_data(mData);
}

/// Copies the data from CPU memory over to the OpenCL device memory, creating memory objects when necessary.
//void COILibData::CopyToOpenCLDevice(cl_command_queue * queue)
//{
//	// First free any allocated GPU memory if it was allocated before.
//	if(v2_loc) clReleaseMemObject(v2_loc);
//	if(v2_uv) clReleaseMemObject(v2_uv);
//	if(t3_loc) clReleaseMemObject(t3_loc);
//	if(t3_phasor) clReleaseMemObject(t3_phasor);
//	if(t3_uv) clReleaseMemObject(t3_uv);
//
//	// Now copy the data from OIFITS structs into
//	// TODO:
//}

int COILibData::GetNumV2()
{
	return n_v2;
}

int COILibData::GetNumT3()
{
	return n_t3;
}

int COILibData::GetNumUV()
{
	return n_uv;
}
