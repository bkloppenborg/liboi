/*
 * COpenCL.h
 *
 *  Created on: Nov 11, 2011
 *      Author: bkloppenborg
 *
 * Primary class to initialize an OpenCL context.
 * Manages the creation and deletion of OpenCL objects, programs.
 * Also implements fast-call routines to get commonly needed interferometric quantities.
 *
 */

#ifndef COPENCL_H_
#define COPENCL_H_

#pragma OPENCL EXTENSION CL_APPLE_gl_sharing : enable
#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable

// Enable OpenCL exceptions
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.hpp>
#else
	#include <CL/cl.hpp>
#endif

#include <GL/gl.h>
#include <GL/glx.h>
#include <string>

using namespace std;

// TODO: Make this class support multiple devices.
// TODO: Permit the user to specify a specific OpenCL device to use.

class COpenCL
{
protected:
	// Datamembers for the OpenCL device, context and queue.
	// At present we only support one device/context/queue, but could add more in the future
	// TODO: Make this class support multiple OpenCL devices.
	cl_device_id mDevice;
	cl_context mContext;
	cl_command_queue mQueue;


public:
	COpenCL(cl_device_type type);
	virtual ~COpenCL();

public:
	static void CheckOCLError(string user_message, int error_code);

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
