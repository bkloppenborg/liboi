/*
 * CLibOI.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *
 * C++ interface layer to the OpenCL Interferometry Library
 *
 * TODO: Implement spectral layers.
 *       Probably need to add an ImageToChi2(int data_num, int layer) function.
 *
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

#ifndef LIBOI_H_
#define LIBOI_H_

#pragma OPENCL EXTENSION CL_APPLE_gl_sharing : enable
#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable

// cl.hpp throws lot of warnings, but we have no control over these.  Tell GCC to ignore them.
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wall"
//#pragma GCC diagnostic ignored "-Wextra"
//#pragma GCC diagnostic ignored "-Wshadow"

// Enable OpenCL exceptions
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.hpp>
#else
	#include <CL/cl.hpp>
#endif

#include <string>
#include <memory>
#include "oi_file.hpp"

using namespace std;
using namespace ccoifits;

// TODO: Switch to native C++11 when Apple clang is based on 3.2svn
// clang 3.1 does not (fully) support c++11 features so when compiled by clang
// we switch to boost for threads, mutexes, and locks.
#ifdef __clang__
#include <boost/smart_ptr/shared_ptr.hpp>
using namespace boost;
namespace ns = boost;
#else
namespace ns = std;
#endif

class COpenCL;
typedef shared_ptr<COpenCL> COpenCLPtr;

namespace liboi
{

class CRoutine_Sum;
class CRoutine_ImageToBuffer;
class CRoutine_Normalize;
class CRoutine_FT;
class CRoutine_FTtoV2;
class CRoutine_FTtoT3;
class CRoutine_Chi;
class CRoutine_LogLike;
class CRoutine_Square;
class CRoutine_Zero;

class COILibDataList;

class COILibData;
typedef ns::shared_ptr<COILibData> COILibDataPtr;

namespace LibOIEnums
{
	enum ImageTypes
	{
		OPENCL_BUFFER,
		OPENGL_FRAMEBUFFER,
		OPENGL_TEXTUREBUFFER,
		OPENGL_RENDERBUFFER,
		HOST_MEMORY
	};

	enum Chi2Types
	{
		CONVEX,
		NON_CONVEX
	};
}

class CLibOI
{
protected:
	// Datamembers:
	COILibDataList * mDataList;

	// OpenCL Context, manager, etc.
	COpenCLPtr mOCL;

	// Routines:
	bool mDataRoutinesInitialized;
	string mKernelSourcePath;
	CRoutine_Zero * mrZeroBuffer;
	CRoutine_Sum * mrTotalFlux;
	CRoutine_ImageToBuffer * mrCopyImage;
	CRoutine_Normalize * mrNormalize;
	CRoutine_FT * mrFT;
	CRoutine_FTtoV2 * mrV2;
	CRoutine_FTtoT3 * mrT3;
	CRoutine_Chi * mrChi;
	CRoutine_LogLike * mrLogLike;
	CRoutine_Square * mrSquare;

	// Memory objects (OpenCL or otherwise)
	LibOIEnums::ImageTypes mImageType;
	// Memory locations.
	cl_mem mImage_cl;
	cl_mem mImage_gl;	// OpenCL - OpenGL interop buffer.
	float * mImage_host;	// Allocated externally. DO NOT FREE
	// Image properties
	unsigned int mImageWidth;
	unsigned int mImageHeight;
	unsigned int mImageDepth;
	float mImageScale;

	unsigned int mMaxData;
	unsigned int mMaxUV;

	// Temporary buffers:
	cl_mem mFluxBuffer;
	cl_mem mFTBuffer;
	cl_mem mSimDataBuffer;


public:
	CLibOI(COpenCLPtr open_cl);
	CLibOI(cl_device_type type);
	CLibOI(cl_device_id device, cl_context context, cl_command_queue queue, bool cl_gl_interop_enabled);
	virtual ~CLibOI();

public:

	void CopyImageToBuffer(int layer);
	void CopyImageToBuffer(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int layer);
	void CopyImageToBuffer(float * host_mem, cl_mem cl_buffer, int width, int height, int layer);

	float DataToChi2(COILibDataPtr data);
	float DataToLogLike(COILibDataPtr data);

public:
	void ExportImage(float * image, unsigned int width, unsigned int height, unsigned int depth);
	void FreeOpenCLMem();
	void FTToData(COILibDataPtr data);

	OIDataList GetData(unsigned int data_num);
	double GetDataAveJD(int data_num);
	int GetNData();
	int GetNDataAllocated();
	int GetNDataAllocated(int data_num);
	int GetNDataSets();
	int GetNT3(int data_num);
	int GetNV2(int data_num);
	int GetMaxDataSize() { return mMaxData; };
//	void GetSimulatedData(unsigned int data_set, float * output_buffer, unsigned int buffer_size);
//	void GetT3(unsigned int data_set, vector<CT3DataPtr> & t3);
//	void GetV2(unsigned int data_set, vector<CV2DataPtr> & v2);

	void ImageToChi(COILibDataPtr data, float * output, unsigned int & n);
	bool ImageToChi(int data_num, float * output, unsigned int & n);
	float ImageToChi2(COILibDataPtr data);
	float ImageToChi2(int data_num);
	void ImageToChi2(COILibDataPtr data, float * output, unsigned int & n);
	bool ImageToChi2(int data_num, float * output, unsigned int & n);
	void ImageToData(int data_num);
	void ImageToData(COILibDataPtr data);
	float ImageToLogLike(COILibDataPtr data);
	float ImageToLogLike(int data_num);
	void Init();
private:
	void InitMembers();
public:
	void InitMemory();
	void InitRoutines();

	int LoadData(string filename);
	int LoadData(const OIDataList & data);

	void Normalize();

	void PrintDeviceInfo();

	void RemoveData(int data_num);
	void ReplaceData(unsigned int old_data_id, const OIDataList & new_data);
	void RunVerification(int data_num);

	void SaveImage(string filename);
	void SaveSimulatedData(int data_num, string savefile_dir);
	void SetImageInfo(unsigned int width, unsigned int height, unsigned int depth, float scale);
	void SetImageSource(float * host_memory);
	void SetImageSource(cl_mem cl_device_memory);
	void SetImageSource(GLuint gl_device_memory, LibOIEnums::ImageTypes type);
	void SetKernelSourcePath(string path_to_kernels);

	float TotalFlux(bool return_value);
};

} /* namespace liboi */

#endif /* LIBOI_H_ */
