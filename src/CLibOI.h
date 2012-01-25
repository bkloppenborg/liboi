/*
 * CLibOI.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *
 * C++ interface layer to the OpenCL Interferometry Library
 *
 * TODO: Permit multiple OpenCL devices to be used at the same time.  Might be useful to use 2-3 GPUs!
 * TODO: Implement spectral layers.
 *       Probably need to add an ImageToChi2(int data_num, int layer) function.
 *
 * NOTE: To use copy an image to the OpenCL buffer using CopyImageToBuffer then call required functions.
 */

#ifndef CLIBOI_H_
#define CLIBOI_H_

//#include "COILibData.h"

#include <string>

#include "COpenCL.h"
#include "LibOIEnumerations.h"
#include "COILibDataList.h"
#include "CRoutine_Reduce.h"
#include "CRoutine_Normalize.h"
#include "CRoutine_ImageToBuffer.h"
#include "CRoutine_FT.h"
#include "CRoutine_DFT.h"
#include "CRoutine_FTtoV2.h"
#include "CRoutine_FTtoT3.h"
#include "CRoutine_Chi2.h"

class CLibOI
{
protected:
	// Datamembers:
	COILibDataList mDataList;

	eFTMethods mFTMethod;
	cl_device_type mDeviceType;

	// OpenCL Context, manager, etc.
	COpenCL * mOCL;

	// Routines:
	string mKernelSourcePath;
	CRoutine_Reduce * mrTotalFlux;
	CRoutine_ImageToBuffer * mrCopyImage;
	CRoutine_Normalize * mrNormalize;
	CRoutine_FT * mrFT;
	CRoutine_FTtoV2 * mrV2;
	CRoutine_FTtoT3 * mrT3;
	CRoutine_Chi2 * mrChi2;

	// Memory objects (OpenCL or otherwise)
	eImageTypes mImageType;
	cl_mem mCLImage;
	cl_mem mGLImage;
	int mImageWidth;
	int mImageHeight;
	int mImageDepth;
	float mImageScale;

	int mMaxData;
	int mMaxUV;

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
	float DataToChi2(COILibData * data);

public:
	void FreeOpenCLMem();
	void FTToData(COILibData * data);

	float ImageToChi2(COILibData * data);
	float ImageToChi2(int data_num);
	void Init();
	void InitMemory();
	void InitRoutines();

	void LoadData(string filename);

	void Normalize();

	float TotalFlux(int layer, bool return_value);

	void SetImageInfo(int width, int height, int depth, float scale);
	void SetImage_CLMEM(cl_mem image);
	void SetImage_GLFB(GLuint framebuffer);
	void SetImage_GLTB(GLuint texturebuffer);
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
