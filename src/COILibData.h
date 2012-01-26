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

// TODO: This class interfaces with getoifits (Fabien Baron) and oifitslib (John Young) which are
// are both written in C.  This forces us to use <complex.h> rather than <complex>.  We'll probably
// need to write a new OIFITS interface to alleviate this issue.

// TODO: This class should be able to manipulate the order of the data BEFORE uploading
// to the OpenCL device to promote coalesced memory loads.

#ifndef COILIBDATA_H_
#define COILIBDATA_H_

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.hpp>
#else
	#include <CL/cl.hpp>
#endif

extern "C" {
    #include "exchange.h"
    #include "oifile.h"
}

#include "getoifits.h"
#include <string>
#include <complex>
#include "COpenCL.h"


using namespace std;

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
	int mNVis2;
	int mNT3;
	int mNUV;
	int mNData;

	// Storage containers for OIFITS data.
	oi_data * mOIData;

	string mFileName;

public:
	COILibData(oi_data * data, string filename);
	~COILibData();

	void CopyToOpenCLDevice(cl_context context, cl_command_queue queue);

	// Inline the get location functions
	string GetFilename(void) { return mFileName; };
	cl_mem GetLoc_Data() {return mData_cl; };
	cl_mem GetLoc_DataErr() { return mData_err_cl; };
	cl_mem GetLoc_DataBSRef() { return mData_bsref_cl; };
	cl_mem GetLoc_DataT3Phi() { return mData_phasor_cl; };
	cl_mem GetLoc_DataT3Sign() { return mData_sign_cl; };
	cl_mem GetLoc_DataUVPoints() { return mData_uvpnt_cl; };

	int GetNumData() { return mNData; };
	int GetNumT3() { return mNT3; };
	int GetNumUV() { return mNUV; };
	int GetNumV2() { return mNVis2; };

	void InitData(bool do_extrapolation);

	float square(float number) {return number*number; };
};

#endif /* COILIBDATA_H_ */
