/*
 * CLibOI.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *
 * TODO: Permit multiple OpenCL devices to be used at the same time.  Might be useful to use 2-3 GPUs!
 *
 */

#ifndef CLIBOI_H_
#define CLIBOI_H_

//#include "COILibData.h"

#include <string>

#include "COpenCL.h"
#include "LibOIEnumerations.h"
#include "CRoutine_Reduce.h"
#include "CRoutine_Normalize.h"
#include "CRoutine_ImageToBuffer.h"

class CLibOI
{
protected:
	// Datamembers:
//	vector<COILibData*> DataList;

	eFTMethods mFTMethod;
	cl_device_type mDeviceType;

	// OpenCL Context, manager, etc.
	COpenCL * mOCL;

	// Routines:
	string mKernelSourcePath;
	CRoutine_Reduce * mrTotalFlux;
	CRoutine_ImageToBuffer * mrCopyImage;
	CRoutine_Normalize * mrNormalize;

	// Memory objects (OpenCL or otherwise)
	eImageTypes mImageType;
	cl_mem mCLImage;
	cl_mem mGLImage;
	int mImageWidth;
	int mImageHeight;
	int mImageDepth;

	cl_mem mFluxBuffer;


public:
	CLibOI();
	virtual ~CLibOI();

public:

	void CopyImageToBuffer(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int layer);

	void FreeOpenCLMem();

	void Init(cl_device_type, int image_width, int image_height, int image_depth);
	void InitMemory();
	void InitRoutines();

	float TotalFlux(bool return_value);

	void RegisterImageSize(int width, int height, int depth);
	void RegisterImage_CLMEM(cl_mem image);
	void RegisterImage_GLFB(GLuint framebuffer);
	void RegisterImage_GLTB(GLuint texturebuffer);

	void SetKernelSoucePath(string path_to_kernels);

//	void ComputeChi2_V2(cl_mem v2_sim_data, cl_mem v2_real_data, int v2_size);
//	void ComputeChi2_T3(cl_mem t3_sim_data, cl_mem t3_real_data, int t3_size);
//	float ComputeFlux(cl_mem image_location, int width, int height);

//	void FT(cl_mem image_location, int image_width, int image_height, cl_mem output_buffer);


//	void LoadData(string filename);
//
//	void MakeData_V2(cl_mem ft_buffer, int width, int height, cl_mem v2_uv, int v2_size, cl_mem output_buffer);
//	void MakeData_T3(cl_mem ft_buffer, int width, int height, cl_mem t3_uv, int t3_size, cl_mem output_buffer);

	//	void Normalize(cl_mem image, cl_mem divisor, int width, int height, int depth);

	//	void UnloadData(string filename);

	//	void SetFTMethod(eFTMethods FTMethod);

		// TODO: rename these as it's clunky.
	//	cl_mem SetImageFromTexture(cl_context OCLContext, GLuint texture);
	//	cl_mem SetImageFromRenderbuffer(cl_context OCLContext, GLuint renderbuffer);


};

#endif /* CLIBOI_H_ */
