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
 * How to Use:
 * 	1) Create a new CLibOI object with the device or type of device you wish to use, e.g.:
 * 		CL = new CLibOI(CL_DEVICE_TYPE_GPU);
 * 	   If you are using CL/GL interop, this must be done from the same thread that
 * 	   currently has access to the OpenGL context.
 * 	2) Call all required Set* functions to define the image location and kernel source
 * 	3) Load any data you wish to include.
 * 	   If no data is loaded, the Fourier Transform, V2, T3 and Chi2 routines will be unavailable.
 * 	4) Call Init().  This allocates the
 * 	5) Use the LibOI functions as needed
 * 	6) Free memory by deleting the CLibOI object
 * 	   (if you don't do this GPU memory will not be deallocated)
 */

#ifndef CLIBOI_H_
#define CLIBOI_H_

//#include "COILibData.h"

#include <string>

#include "COpenCL.h"
#include "LibOIEnumerations.h"
#include "COILibDataList.h"

class CRoutine_Reduce_Sum;
class CRoutine_ImageToBuffer;
class CRoutine_Normalize;
class CRoutine_FT;
class CRoutine_FTtoV2;
class CRoutine_FTtoT3;
class CRoutine_Chi;
class CRoutine_LogLike;
class CRoutine_Square;

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
	CRoutine_Reduce_Sum * mrTotalFlux;
	CRoutine_ImageToBuffer * mrCopyImage;
	CRoutine_Normalize * mrNormalize;
	CRoutine_FT * mrFT;
	CRoutine_FTtoV2 * mrV2;
	CRoutine_FTtoT3 * mrT3;
	CRoutine_Chi * mrChi;
	CRoutine_LogLike * mrLogLike;
	CRoutine_Square * mrSquare;

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
	float DataToLogLike(COILibData * data);

public:
	void FreeOpenCLMem();
	void FTToData(COILibData * data);

	float ImageToChi2(COILibData * data);
	float ImageToChi2(int data_num);
	float ImageToLogLike(COILibData * data);
	float ImageToLogLike(int data_num);
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
