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

#ifndef CLIBOI_H_
#define CLIBOI_H_

//#include "COILibData.h"

#include <string>
#include <memory>

#include "COpenCL.h"
#include "LibOIEnumerations.h"
#include "COILibDataList.h"

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

class CLibOI
{
protected:
	// Datamembers:
	COILibDataList mDataList;

	// OpenCL Context, manager, etc.
	COpenCL * mOCL;

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
	CLibOI(cl_device_type type);
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

	double GetDataAveJD(int data_num) { return mDataList[data_num]->GetAveJD(); };
	int GetNData() { return mDataList.GetNData(); };
	int GetNDataAllocated() { return mDataList.GetNDataAllocated(); };
	int GetNDataAllocated(int data_num) { return mDataList.GetNDataAllocated(data_num); };
	int GetNDataSets() { return mDataList.size(); };
	int GetNT3(int data_num);
	int GetNV2(int data_num);
	int GetMaxDataSize() { return mMaxData; };
//	void GetSimulatedData(unsigned int data_set, float * output_buffer, unsigned int buffer_size);
//	void GetT3(unsigned int data_set, vector<CT3DataPtr> & t3);
//	void GetV2(unsigned int data_set, vector<CV2DataPtr> & v2);

	void ImageToChi(COILibDataPtr data, float * output, int & n);
	bool ImageToChi(int data_num, float * output, int & n);
	float ImageToChi2(COILibDataPtr data);
	float ImageToChi2(int data_num);
	void ImageToChi2(COILibDataPtr data, float * output, int & n);
	bool ImageToChi2(int data_num, float * output, int & n);
	void ImageToData(int data_num);
	void ImageToData(COILibDataPtr data);
	float ImageToLogLike(COILibDataPtr data);
	float ImageToLogLike(int data_num);
	void Init();
	void InitMemory();
	void InitRoutines();

	void LoadData(string filename);

	void Normalize();

	float TotalFlux(bool return_value);

	void RemoveData(int data_num);
	void RunVerification(int data_num);

	void SaveImage(string filename);
	void SetImageInfo(unsigned int width, unsigned int height, unsigned int depth, float scale);
	void SetImageSource(float * host_memory);
	void SetImageSource(cl_mem cl_device_memory);
	void SetImageSource(GLuint gl_device_memory, LibOIEnums::ImageTypes type);
	void SetKernelSourcePath(string path_to_kernels);


//
//	void MakeData_V2(cl_mem ft_buffer, int width, int height, cl_mem v2_uv, int v2_size, cl_mem output_buffer);
//	void MakeData_T3(cl_mem ft_buffer, int width, int height, cl_mem t3_uv, int t3_size, cl_mem output_buffer);

	//	void UnloadData(string filename);

	//	void SetFTMethod(eFTMethods FTMethod);

		// TODO: rename these as it's clunky.
	//	cl_mem SetImageFromTexture(cl_context OCLContext, GLuint texture);
	//	cl_mem SetImageFromRenderbuffer(cl_context OCLContext, GLuint renderbuffer);


};

#endif /* CLIBOI_H_ */
