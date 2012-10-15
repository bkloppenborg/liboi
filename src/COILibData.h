/*
 * COILibData.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *
 * A class for reading in and storing data in a convenient format for liboi operations.
 * NOTE: Right now this class is compatible with only Optical Interferometric Data.
 * It should be generalized if radio interferometry routines are implmenented in liboi.
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

// TODO: This class interfaces with getoifits (Fabien Baron) and oifitslib (John Young) which are
// are both written in C.  This forces us to use <complex.h> rather than <complex>.  We'll probably
// need to write a new OIFITS interface to alleviate this issue.

// TODO: This class should be able to manipulate the order of the data BEFORE uploading
// to the OpenCL device to promote coalesced memory loads.

#ifndef COILIBDATA_H_
#define COILIBDATA_H_

// cl.hpp throws lot of warnings, but we have no control over these.  Tell GCC to ignore them.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wshadow"

// Enable OpenCL exceptions
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.hpp>
#else
	#include <CL/cl.hpp>
#endif

// Restore the GCC warning state
#pragma GCC diagnostic pop

extern "C" {
    #include "exchange.h"
}

#include "getoifits.h"
#include <string>
#include <complex>
#include <memory>
#include "COpenCL.h"

using namespace std;

// Define storage classes for exporting data (used only in GetV2, GetT3)
class CV2Data
{
public:
	float u;
	float v;
	float v2;
	float v2_err;
};

class CT3Data
{
public:
	float u1;
	float u2;
	float u3;
	float v1;
	float v2;
	float v3;
	float t3_amp;
	float t3_amp_err;
	float t3_phi;
	float t3_phi_err;
};

typedef shared_ptr<CV2Data> CV2DataPtr;
typedef shared_ptr<CT3Data> CT3DataPtr;

class COILibData
{
	// NOTE: This class currently uses liboifits and getoifits and is compliant with those libraries
	// but it is our intention to change this.  I wouldn't suggest inheriting from this object as
	// datamembers and some functions are likely to change.
protected:
	// Location and size of data loaded into OpenCL memory objects.
	cl_mem mData_cl; // Vis2 + T3, concatinated as cl_float2's.
	cl_mem mData_err_cl;
	cl_mem mData_phasor_cl;	// Quantity required to rotate the T3 phase to Y=0
	cl_mem mData_uvpnt_cl;
	cl_mem mData_bsref_cl;
	cl_mem mData_sign_cl;

	// TODO: Temporary datamembers for use with getoifits and oifitslib.
	float * mData;
	float * mData_err;
	complex<float> * mData_phasor;
	unsigned int mNVis2;
	unsigned int mNT3;
	unsigned int mNUV;
	unsigned int mNData;
	double mAveJD;

	// Storage containers for OIFITS data.
	oi_data * mOIData;

	string mFileName;

public:
	COILibData(string filename);
	~COILibData();

	void CopyToOpenCLDevice(cl_context context, cl_command_queue queue);

	// Inline the get location functions
	double GetAveJD(void) { return mAveJD; };
	string GetFilename(void) { return mFileName; };
	cl_mem GetLoc_Data() { return mData_cl; };
	cl_mem GetLoc_DataErr() { return mData_err_cl; };
	cl_mem GetLoc_DataBSRef() { return mData_bsref_cl; };
	cl_mem GetLoc_DataT3Phi() { return mData_phasor_cl; };
	cl_mem GetLoc_DataT3Sign() { return mData_sign_cl; };
	cl_mem GetLoc_DataUVPoints() { return mData_uvpnt_cl; };
	unsigned int GetNumData() { return mNData; };
	unsigned int GetNumT3() { return mNT3; };
	unsigned int GetNumUV() { return mNUV; };
	unsigned int GetNumV2() { return mNVis2; };
	void GetT3(vector<CT3DataPtr> & t3);
	void GetV2(vector<CV2DataPtr> & v2);

	void InitData(bool do_extrapolation);

	void ReadFile(string filename);

	static float square(float number) { return number*number; };
};

#endif /* COILIBDATA_H_ */
