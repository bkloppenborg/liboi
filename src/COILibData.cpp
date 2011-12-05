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
void COILibData::CopyToOpenCLDevice(cl_command_queue * queue)
{
	// First free any allocated GPU memory if it was allocated before.
	if(v2_loc) clReleaseMemObject(v2_loc);
	if(v2_uv) clReleaseMemObject(v2_uv);
	if(t3_loc) clReleaseMemObject(t3_loc);
	if(t3_phasor) clReleaseMemObject(t3_phasor);
	if(t3_uv) clReleaseMemObject(t3_uv);

	// Now copy the data from OIFITS structs into

}

/// Reads in an OIFITS file, returns a COILibData object.
COILibData * COILibData::FromFile(string filename)
{
	// TODO: Right now this routine uses getoifits (Fabien Baron) and oifitslib (John Young) to read in the data
	// we can probably get a performance increase on the GPU by sorting the data intelligently on load.
	// We'll need to implement a new reading function to do this.
	// Note: If we reorder the data, we'll need to make sure the V2 and T3 kernels still understand where their data is at.

	// From GPAIR, Allocate storage for OIFITS data
	oi_usersel usersel;
	oi_data * tmp = new oi_data();
	int status;

	// From GPAIR, read_oifits
	strcpy(usersel.file, filename.c_str());
	get_oi_fits_selection(&usersel, &status);
	get_oi_fits_data(&usersel, tmp, &status);
	printf("OIFITS File read\n");

	return new COILibData(tmp);
}
