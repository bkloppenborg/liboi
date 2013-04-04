/*
 * COpenCL.h
 *
 *  Created on: Nov 11, 2011
 *      Author: bkloppenborg
 * 
 *  Description:
 *      Primary class to initialize an OpenCL context.
 *      Manages the creation and deletion of OpenCL objects, programs.
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

#ifndef COPENCL_H_
#define COPENCL_H_

#pragma OPENCL EXTENSION CL_APPLE_gl_sharing : enable
#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable

// cl.hpp throws lot of warnings, but we have no control over these.  Tell GCC to ignore them.
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wall"
//#pragma GCC diagnostic ignored "-Wextra"
//#pragma GCC diagnostic ignored "-Wshadow"

// Enable OpenCL exceptions
#define __CL_ENABLE_EXCEPTIONS

// cl.hpp throws lot of warnings, but we have no control over these.  Tell GCC to ignore them.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wshadow"

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.hpp>
	#include <OpenGL/gl.h>
#else
	#include <CL/cl.hpp>
	#include <GL/gl.h>
	#include <GL/glx.h>
#endif

// Restore the GCC warning state
#pragma GCC diagnostic pop


#include <string>
#include <memory>

using namespace std;

class COpenCL;
typedef shared_ptr<COpenCL> COpenCLPtr;

// TODO: Make this class support multiple devices.

class COpenCL
{
protected:
	// Datamembers for the OpenCL device, context and queue.
	// At present we only support one device/context/queue, but could add more in the future
	cl_device_id mDevice;
	cl_context mContext;
	cl_command_queue mQueue;

	bool mCLGLInteropEnabled;


public:
	COpenCL(cl_device_id device, cl_context context, cl_command_queue queue, bool cl_gl_interop_enabled);
	COpenCL(cl_device_type type);
	virtual ~COpenCL();

public:
	static void CheckOCLError(string user_message, int error_code);
	bool CL_GLInteropEnabled() { return mCLGLInteropEnabled; };

	cl_context		GetContext();
	cl_device_id	GetDevice();
	cl_command_queue GetQueue();

	void 			GetDeviceList(cl_platform_id platform, vector<cl_device_id> * devices);
	cl_device_type  GetDeviceType(cl_device_id device);
	void 			GetPlatformList(vector<cl_platform_id> * platforms);
protected:
	static string 	GetOCLErrorString(cl_int err);

	void Init(cl_device_type type);
	void Init(cl_platform_id platform, cl_device_id device, cl_device_type type);

public:
	void FindDevice(cl_platform_id & platform, cl_device_id & device, cl_device_type type);

	void PrintDeviceInfo(cl_device_id device_id);
	void PrintPlatformInfo(cl_platform_id platform_id);
};

#endif /* COPENCL_H_ */
