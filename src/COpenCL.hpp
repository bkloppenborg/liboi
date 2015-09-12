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

#include <stdexcept>
#include <iostream>
#include <string>
#include <memory>

using namespace std;

#define OPENCL_SUCCESS 0
#define OPENCL_FAILURE 1

class COpenCL;
/// Macro which checks and marks the source of an OpenCL error.
#define CHECK_OPENCL_ERROR(actual, msg) \
    if(COpenCL::CheckError(actual, CL_SUCCESS, msg)) \
    { \
        std::cout << "Location : " << __FILE__ << ":" << __LINE__<< std::endl; \
        throw runtime_error("OpenCL error detected."); \
    }

// Enable OpenCL exceptions
#define __CL_ENABLE_EXCEPTIONS

// cl.hpp throws lot of warnings, but we have no control over these.  Tell GCC to ignore them.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wshadow"

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.hpp>
	#include <OpenGL/gl3.h>
#else
	#include <CL/cl.hpp>
	#include <GL/gl.h>
	#include <GL/glx.h>
#endif

// Restore the GCC warning state
#pragma GCC diagnostic pop

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

	unsigned int mCLVersion;

public:
	COpenCL(cl_device_id device, cl_context context, cl_command_queue queue);
	COpenCL(cl_device_type type);
	virtual ~COpenCL();

public:

	static bool checkExtensionAvailability(const cl_device_id device_id, std::string ext_name);

	static void error(std::string errorMsg);

protected:
	unsigned int  FindOpenCLVersion();
public:
	cl_context		GetContext();
	cl_device_id	GetDevice();
	cl_command_queue GetQueue();

	void 			GetDeviceList(cl_platform_id platform, vector<cl_device_id> * devices);
	cl_device_type  GetDeviceType(cl_device_id device);
	void 			GetPlatformList(vector<cl_platform_id> * platforms);
protected:
	static string 	getOpenCLErrorCodeStr(cl_int err);
public:
	unsigned int	GetOpenCLVersion();

	bool isCLGLInteropEnabled();

	void Init(cl_device_type type);
	void Init(cl_platform_id platform, cl_device_id device, cl_device_type type);

public:
	void FindDevice(cl_platform_id & platform, cl_device_id & device, cl_device_type type);

	void PrintDeviceInfo(cl_device_id device_id);
	void PrintPlatformInfo(cl_platform_id platform_id);

	/// Checks for the status of an OpenCL error. Generates an informative error message if one is detected.
	template<typename T>
	static int CheckError(T input, T reference,  std::string message, bool isAPIerror = true)
	{
	    if(input==reference)
	    {
	        return OPENCL_SUCCESS;
	    }
	    else
	    {
	        if(isAPIerror)
	        {
	            std::cout << "Error: "<< message << " Error code : ";
	            int error_code = (int) input;
	            std::cout << getOpenCLErrorCodeStr(error_code) << std::endl;
	        }
	        else
	        {
	            error(message);
	        }
	        return OPENCL_FAILURE;
	    }
	}
};

#endif /* COPENCL_H_ */
