/*
 * COILibData.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      C++ Wrapper class for OIFITS data.  Handles copying data to/from OpenCL devices.
 */
 
/* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * If you use this software as part of a scientific publication, please cite as:
 *
 * Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library"
 * (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
 *
 * This file is part of the OpenCL Interferometry Library (LIBOI).
 * 
 * LIBOI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "COILibData.h"
#include <cstdio>

#define MJD 2400000.5

using namespace std;

COILibData::COILibData(string filename)
{
	mOIData = NULL;
	mFileName = filename;
	ReadFile(mFileName);

	// TODO: Temporary for getoifits routines, allocate room:
	mNVis2 = mOIData->npow;
	mNT3 = mOIData->nbis;
	mNUV = mOIData->nuv;
	mNData = mNVis2 + 2*mNT3;
	mData = new float[mNData];
	mData_err = new float[mNData];
	mData_phasor = new complex<float>[mNT3];

	// Init all of the OpenCL memory locations to NULL
	mData_cl = NULL;
	mData_err_cl = NULL;
	mData_phasor_cl = NULL;
	mData_uvpnt_cl = NULL;
	mData_bsref_cl = NULL;
	mData_sign_cl = NULL;

	// Now compute the average time for the data set.  We accumulate the times
	// in a long double to minimize a loss of precision.
	long double time = 0;
	for(unsigned int i = 0; i < mNVis2; i++)
		time += mOIData->powmjd[i];

	for(unsigned int i = 0; i < mNT3; i++)
		time += mOIData->bismjd[i];

	// Convert the average time to a JD, casting back to a double in the process.
	mAveJD = double(time / (mNVis2 + mNT3) + MJD);
	printf("Data Average Time: %f\n", mAveJD);

	InitData(true);
}

COILibData::~COILibData()
{
	// Release memory on the GPU.
	if(mData_cl) clReleaseMemObject(mData_cl);
	if(mData_err_cl) clReleaseMemObject(mData_err_cl);
	if(mData_phasor_cl) clReleaseMemObject(mData_phasor_cl);
	if(mData_uvpnt_cl) clReleaseMemObject(mData_uvpnt_cl);
	if(mData_sign_cl) clReleaseMemObject(mData_sign_cl);
	if(mData_bsref_cl) clReleaseMemObject(mData_bsref_cl);

	// Free local memory:
	free_oi_data(mOIData);

	delete[] mData;
	delete[] mData_err;
	delete[] mData_phasor;
}

/// Copies the data from CPU memory over to the OpenCL device memory, creating memory objects when necessary.
void COILibData::CopyToOpenCLDevice(cl_context context, cl_command_queue queue)
{
	// TODO: If we change away from getoifits and oifitslib we'll need to rewrite this function entirely.
	int i = 0;
	int err = CL_SUCCESS;

	// Convert the biphasor over to a cl_float2 in format <real, imaginary>
	cl_float2 * phasor = new cl_float2[mNT3];
	for(i = 0; i < mNT3; i++)
	{
		phasor[i].s0 = mData_phasor[i].real();
		phasor[i].s1 = mData_phasor[i].imag();
	}

	// We will also need the uvpnt and sign information for bispectrum computations.
	// Although we waste a little space, we use cl_long4 and cl_float4 so that we may have
	// coalesced loads on the GPU.
	cl_long4 * bsref_uvpnt = new cl_long4[mNT3];
	cl_short4 * bsref_sign = new cl_short4[mNT3];
	for(i = 0; i < mNT3; i++)
	{
		bsref_uvpnt[i].s0 = mOIData->bsref[i].ab.uvpnt;
		bsref_uvpnt[i].s1 = mOIData->bsref[i].bc.uvpnt;
		bsref_uvpnt[i].s2 = mOIData->bsref[i].ca.uvpnt;
		bsref_uvpnt[i].s3 = 0;

		bsref_sign[i].s0 = mOIData->bsref[i].ab.sign;
		bsref_sign[i].s1 = mOIData->bsref[i].bc.sign;
		bsref_sign[i].s2 = mOIData->bsref[i].ca.sign;
		bsref_sign[i].s3 = 0;
	}

	cl_float2 * uv_info = new cl_float2[mNUV];
	for(i = 0; i < mNUV; i++)
	{
		uv_info[i].s0 = mOIData->uv[i].u;
		uv_info[i].s1 = mOIData->uv[i].v;
	}

	// Now create buffers on the OpenCL device
    mData_cl = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(float) * mNData, NULL, NULL);
    mData_err_cl = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(float) * mNData, NULL, NULL);
    mData_uvpnt_cl = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float2) * mNUV, NULL, NULL);
    if(mNT3 > 0)
    {
		mData_phasor_cl = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(cl_float2) * mNT3, NULL, NULL);
		mData_bsref_cl = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_long4) * mNT3, NULL, NULL);
		mData_sign_cl = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_short4) * mNT3, NULL, NULL);
    }

    // Now move the data over:
    err  = clEnqueueWriteBuffer(queue, mData_cl, CL_FALSE, 0, sizeof(float) * mNData, mData, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, mData_err_cl, CL_FALSE, 0, sizeof(float) * mNData, mData_err, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, mData_uvpnt_cl, CL_FALSE, 0, sizeof(cl_float2) * mNUV, uv_info, 0, NULL, NULL);
    if(mNT3 > 0)
    {
		err |= clEnqueueWriteBuffer(queue, mData_phasor_cl, CL_FALSE, 0, sizeof(cl_float2) * mNT3, mData_phasor, 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(queue, mData_bsref_cl, CL_FALSE, 0, sizeof(cl_long4) * mNT3, bsref_uvpnt, 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(queue, mData_sign_cl, CL_FALSE, 0, sizeof(cl_short4) * mNT3, bsref_sign, 0, NULL, NULL);
    }
	COpenCL::CheckOCLError("Failed to copy OIFITS data over to the OpenCL Device.", err);

	// Wait for the queue to process
	clFinish(queue);

	// Free memory
	delete[] phasor;
	delete[] bsref_uvpnt;
	delete[] bsref_sign;
}

/// Initalize the OIFITS data, copying over to standard float memory objects
/// TODO: This is a routine pulled directly from GPAIR, if we replace getoifits and liboifits we should
/// also strike this routine.
void COILibData::InitData(bool do_extrapolation)
{
	register int ii;
	int ndof = mNData;
	int warning_extrapolation = 0;
	float pow1, powerr1, pow2, powerr2, pow3, powerr3, sqamp1, sqamp2, sqamp3, sqamperr1, sqamperr2, sqamperr3;

	// Set elements [0, npow - 1] equal to the powerspectra
	for (ii = 0; ii < mNVis2; ii++)
	{
		mData[ii] = mOIData->pow[ii];
		mData_err[ii] = fabs(mOIData->powerr[ii]);
	}

	// Let j = npow, set elements [j, j + nbis - 1] to the bispectra data.
	for (ii = 0; ii < mNT3; ii++)
	{
		if ((do_extrapolation) && ((mOIData->bisamperr[ii] <= 0.) || (mOIData->bisamperr[ii] > 1e3))) // Missing triple amplitudes
		{
			if ((mOIData->bsref[ii].ab.uvpnt < mNVis2) && (mOIData->bsref[ii].bc.uvpnt < mNVis2) && (mOIData->bsref[ii].ca.uvpnt < mNVis2))
			{
				// if corresponding powerspectrum points are available
				// Derive pseudo-triple amplitudes from powerspectrum data
				// First select the relevant powerspectra
				pow1 = mOIData->pow[mOIData->bsref[ii].ab.uvpnt];
				powerr1 = mOIData->powerr[mOIData->bsref[ii].ab.uvpnt];
				pow2 = mOIData->pow[mOIData->bsref[ii].bc.uvpnt];
				powerr2 = mOIData->powerr[mOIData->bsref[ii].bc.uvpnt];
				pow3 = mOIData->pow[mOIData->bsref[ii].ca.uvpnt];
				powerr3 = mOIData->powerr[mOIData->bsref[ii].ca.uvpnt];

				// Derive optimal visibility amplitudes + noise variance
				sqamp1 = (pow1 + sqrt(square(pow1) + 2.0 * square(powerr1))) / 2.;
				sqamperr1 = 1. / (1. / sqamp1 + 2. * (3. * sqamp1 - pow1) / square(powerr1));
				sqamp2 = (pow2 + sqrt(square(pow2) + 2.0 * square(powerr2))) / 2.;
				sqamperr2 = 1. / (1. / sqamp2 + 2. * (3. * sqamp2 - pow2) / square(powerr2));
				sqamp3 = (pow3 + sqrt(square(pow3) + 2.0 * square(powerr3))) / 2.;
				sqamperr3 = 1. / (1. / sqamp3 + 2. * (3. * sqamp3 - pow3) / square(powerr3));

				// And form the triple amplitude statistics
				mOIData->bisamp[ii] = sqrt(sqamp1 * sqamp2 * sqamp3);
				mOIData->bisamperr[ii] = fabs(mOIData->bisamp[ii] * sqrt(sqamperr1 / sqamp1 + sqamperr2 / sqamp2 + sqamperr3 / sqamp3));

				if(!warning_extrapolation)
				{
					printf("*************************  Warning - Recalculating T3amp from Powerspectra - Check this is wanted ********************\n");
					warning_extrapolation = true;
				}
			}
			else // missing powerspectrum points -> cannot extrapolate bispectrum
			{
				printf("WARNING: triple amplitude extrapolation from powerspectrum failed because of missing powerspectrum\n");
				mOIData->bisamp[ii] = 1.0;
				mOIData->bisamperr[ii] = 1e38; // close to max value for a float
				ndof--;
			}
		}

		// A weird workaround here between Fabien's original code and the C++ conversion.
		// Evidently we can't do exp(-1 * I * mOIData->bisphs[ii] * PI / 180) so instead we do this:
		float angle = mOIData->bisphs[ii] * PI / 180;
		mData_phasor[ii] = complex<float> (cos(angle), -sin(angle));

		mData[mNVis2 + 2 * ii] = fabs(mOIData->bisamp[ii]);
		mData[mNVis2 + 2 * ii + 1] = 0.;
		mData_err[mNVis2 + 2 * ii] = fabs(mOIData->bisamperr[ii]);
		mData_err[mNVis2 + 2 * ii + 1] = fabs(mOIData->bisamp[ii] * mOIData->bisphserr[ii] * PI / 180. );

		//printf("ii %d err1 %f err2 %f \n ", ii, data_err[npow + 2 * ii], data_err[npow + 2 * ii + 1]);
	}

}

/// Exports the T3 data as a vector of CT3Data.
void COILibData::GetT3(vector<CT3DataPtr> & t3)
{
	t3.clear();

	for(int i = 0; i < mNT3; i++)
	{
		CT3DataPtr tmp(new CT3Data());
		tmp->u1 = mOIData->uv[ mOIData->bsref[i].ab.uvpnt ].u;
		tmp->v1 = mOIData->uv[ mOIData->bsref[i].ab.uvpnt ].v;;
		tmp->u2 = mOIData->uv[ mOIData->bsref[i].bc.uvpnt ].u;
		tmp->v2 = mOIData->uv[ mOIData->bsref[i].bc.uvpnt ].v;;
		tmp->u3 = mOIData->uv[ mOIData->bsref[i].ca.uvpnt ].u;
		tmp->v3 = mOIData->uv[ mOIData->bsref[i].ca.uvpnt ].v;;
		tmp->t3_amp = mData[mNVis2 + 2*i];
		tmp->t3_amp_err = mData_err[mNVis2 + 2*i];
		tmp->t3_phi = mOIData->bisphs[i];
		tmp->t3_phi_err = mOIData->bisphserr[i];

		t3.push_back(tmp);
	}
}

/// Exports the V2 data as a vector of CV2Data
void COILibData::GetV2(vector<CV2DataPtr> & v2)
{
	v2.clear();

	for(int i = 0; i < mNVis2; i++)
	{
		CV2DataPtr tmp(new CV2Data());
		tmp->u = mOIData->uv[i].u;
		tmp->v = mOIData->uv[i].v;
		tmp->v2 = mData[i];
		tmp->v2_err = mData_err[i];

		v2.push_back(tmp);
	}
}

/// Reads in the specified data file.
void COILibData::ReadFile(string filename)
{
	// TODO: Right now this routine uses getoifits (Fabien Baron) and oifitslib (John Young) to read in the data
	// we can probably get a performance increase on the GPU by sorting the data intelligently on load.
	// We'll need to implement a new reading function to do this.
	// Note: If we reorder the data, we'll need to make sure the V2 and T3 kernels still understand where their data is at.


	// From GPAIR, Allocate storage for OIFITS data
	oi_usersel usersel;
	mOIData = new oi_data();
	int status = 0;

	// From GPAIR, read_oifits.  Notice we unwrap the shared_ptr<oi_data> object
	// to interface with getoifits.
	strcpy(usersel.file, filename.c_str());
	get_oi_fits_selection(&usersel, &status);
	get_oi_fits_data(&usersel, mOIData, &status);
	printf("OIFITS File read\n");
}


