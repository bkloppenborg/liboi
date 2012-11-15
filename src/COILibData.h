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

#include <string>
#include <complex>
#include <memory>
#include "COpenCL.h"
#include "COIFile.h"

using namespace std;
using namespace ccoifits;

class COILibData
{
protected:
	// Data list
	OIDataList mData;

	// OpenCL memory objects for the data
	cl_mem mData_cl; 			// All data, stored in cl_floats in [vis_real, vis_imag, v2, t3_amp, t3_phi] order
	cl_mem mData_err_cl;

	cl_mem mData_uv_cl;			// UV points.  Ideally arranged in an optimal ordering for the OpenCL device (GPU).
	cl_mem mData_Vis_uv_ref;	// Contains the index of the UV point for creating the i-th Vis point
	cl_mem mData_V2_uv_ref;		// Contains the index of the UV point for creating the i-th V2 point
	cl_mem mData_T3_uv_ref;		// Contains the index of the UV point for creating the i-th T3 point.  A cl_uint4 in [uv_ab, uv_bc, uv_ca, empty]

	cl_mem mData_T3_sign;		// Contains signs indicating conjugation of uv points. A cl_short4 in [uv_ab, uv_bc, uv_ca, 0] order

	// A few things we will need to know about the data
	unsigned int mNVis;
	unsigned int mNV2;
	unsigned int mNT3;
	unsigned int mNUV;
	unsigned int mNData;
	double mAveJD;

	string mFileName;

public:
	COILibData(string filename, cl_context context, cl_command_queue queue);
	COILibData(OIDataList & data, cl_context context, cl_command_queue queue);
	~COILibData();

//	void CopyToOpenCLDevice(cl_context context, cl_command_queue queue);

	// Inline the get location functions
	double GetAveJD(void) { return mAveJD; };
	string GetFilename(void) { return mFileName; };
	cl_mem GetLoc_Data() { return mData_cl; };
	cl_mem GetLoc_DataErr() { return mData_err_cl; };
	cl_mem GetLoc_Vis_UVRef() { return mData_Vis_uv_ref; };
	cl_mem GetLoc_V2_UVRef() { return mData_V2_uv_ref; };
	cl_mem GetLoc_T3_UVRef() { return mData_T3_uv_ref; };
	cl_mem GetLoc_T3_sign() { return mData_T3_sign; };
	cl_mem GetLoc_DataUVPoints() { return mData_uv_cl; };
	unsigned int GetNumData() { return mNData; };
	unsigned int GetNumT3() { return mNT3; };
	unsigned int GetNumUV() { return mNUV; };
	unsigned int GetNumV2() { return mNV2; };
	unsigned int GetNumVis() { return mNVis; };
//	void GetT3(vector<CT3DataPtr> & t3);
//	void GetV2(vector<CV2DataPtr> & v2);

//	void ReadFile(string filename);

//	static float square(float number) { return number*number; };

protected:
	void InitData(cl_context context, cl_command_queue queue);
};

#endif /* COILIBDATA_H_ */
