/*
 * COpenCL.cpp
 *
 *  Created on: Nov 11, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Class for finding, initializing and managing an OpenCL device.
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

#include <cstdio>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "COpenCL.hpp"

using namespace std;

COpenCL::COpenCL(cl_device_type type)
{
	mDevice = 0;
	mContext = 0;
	mQueue = 0;

	mCLGLInteropEnabled = false;

	Init(type);

	mCLVersion = FindOpenCLVersion();
}

COpenCL::COpenCL(cl_device_id device, cl_context context, cl_command_queue queue, bool cl_gl_interop_enabled)
{
	mDevice = device;
	mContext = context;
	mQueue = queue;

	mCLGLInteropEnabled = cl_gl_interop_enabled;

	mCLVersion = FindOpenCLVersion();
}

COpenCL::~COpenCL()
{
	// Free OpenCL memory:
	if(mQueue) clReleaseCommandQueue(mQueue);
	if(mContext) clReleaseContext(mContext);
}

/// Prints error message.
void COpenCL::error(std::string errorMsg)
{
    std::cout <<"Error: "<<errorMsg<<std::endl;
}

/// Searches through the available platforms and devices and returns the first
/// instance of the specified type.
void COpenCL::FindDevice(cl_platform_id & platform, cl_device_id & device, cl_device_type type)
{
	vector<cl_platform_id> platforms;
	GetPlatformList(&platforms);
	unsigned int i, j;
	vector<cl_device_id> devices;
	platform = 0;
	device = 0;

	for(i = 0; i < platforms.size(); i++)
	{
		GetDeviceList(platforms[i], &devices);

		for(j = 0; j < devices.size(); j++)
		{
			if(GetDeviceType(devices[j]) == type)
			{
				platform = platforms[i];
				device = devices[j];
				break;
			}
		}

		// Clear the vector
		devices.clear();
	}
}

/// Returns the OpenCL version as a three-digit unsigned integer.
///
/// The versions are as follows:
///  1.0 -> 100
///  1.1 -> 110
///  1.2 -> 120
///  2.0 -> 200
///  other -> 000
unsigned int COpenCL::FindOpenCLVersion()
{
	int err = CL_SUCCESS;
	cl_char device_cl_version[1024] = {0};
	size_t returned_size;

	unsigned int cl_version = 0;

	// Extract the OpenCL device version number. The OpenCL specifications
	// guarentee the following format: "OpenCL X.Y EXTRA_STUFF"
	err |= clGetDeviceInfo(mDevice, CL_DEVICE_VERSION, sizeof(device_cl_version), device_cl_version, &returned_size);
	string temp((char*) device_cl_version);
	unsigned int start = temp.find(' ', 0);
	unsigned int end = temp.find(' ', start + 1);
	string device_cl_version_number = temp.substr(start, end - start);

	if(device_cl_version_number == "1.0")
		cl_version = 100;
	else if(device_cl_version_number == "1.1")
		cl_version = 110;
	else if(device_cl_version_number == "1.2")
		cl_version = 120;
	else if(device_cl_version_number == "2.0")
		cl_version = 200;

	return cl_version;
}

/// Returns the context
cl_context COpenCL::GetContext()
{
	return mContext;
}

cl_device_id	COpenCL::GetDevice()
{
	return this->mDevice;
}

cl_command_queue COpenCL::GetQueue()
{
	return this->mQueue;
}

/// Returns a list of platforms on this machine
void COpenCL::GetPlatformList(vector<cl_platform_id> * platforms)
{
	// init a few variables
	cl_uint n = 0;
	int status = CL_SUCCESS;

	// First query to find out how many platforms exist:
	status |= clGetPlatformIDs(0, NULL, &n);
	CHECK_OPENCL_ERROR(status, "clGetPlatformIDs failed.");

	// Now allocate memory and pull out the platform IDs
	cl_platform_id * tmp = new cl_platform_id[n];
	status |= clGetPlatformIDs(n, tmp, &n);
	CHECK_OPENCL_ERROR(status, "clGetPlatformIDs failed.");

	platforms->assign(&tmp[0], &tmp[n]);
	delete[] tmp;
}

/// Returns a list of devices on the specified platform.
void COpenCL::GetDeviceList(cl_platform_id platform, vector<cl_device_id> * devices)
{
	cl_uint n = 0;
	int status = CL_SUCCESS;

	// Query to determine the number of devices:
	status |= clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &n);
	CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed.");

	// Now allocate memory and pull out the device IDs
	cl_device_id * tmp = new cl_device_id[n];
	status |= clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, n, tmp, &n);
	CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed.");
	devices->assign(&tmp[0], &tmp[n]);
}

/// Gets the type of device corresponding to device_id
cl_device_type COpenCL::GetDeviceType(cl_device_id device_id)
{
	cl_device_type type;
	int status = CL_SUCCESS;
	status = clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
	CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed.");

	return type;
}

/// Initializes the class using the first device found with the specified type.
void COpenCL::Init(cl_device_type type)
{
	cl_platform_id platform = 0;
	cl_device_id device = 0;

	FindDevice(platform, device, type);

//	PrintPlatformInfo(platform);

	if(platform != 0 && device != 0)
		this->Init(platform, device, type);
	else
		throw runtime_error("Could not find specified OpenCL platform.");
}

/// Initializes the class.  Creates contexts and command queues.
void COpenCL::Init(cl_platform_id platform, cl_device_id device, cl_device_type type)
{
	int status = CL_SUCCESS;
	this->mDevice = device;

	// Each operating system has a different routine for OpenCL-OpenGL interoperability
	// initialization.  Here we try to catch all of them.  Each OS-specific block below
	// shall define the properties required of the context for
	// (a) OpenCL + OpenGL and (b) OpenCL only.
	// This does lead to some code duplication, but makes each instance more clear.

	// Allocate enough space for defining the parameters below:
	cl_context_properties properties[7];

#if defined (__APPLE__) || defined(MACOSX)	// Apple / OSX

    CGLContextObj context = CGLGetCurrentContext();
    CGLShareGroupObj share_group = CGLGetShareGroup(context);

    if(context != NULL && share_group != NULL)
    {
      properties[0] = CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
      properties[1] = (cl_context_properties) share_group;
      properties[2] = CL_CONTEXT_PLATFORM;
      properties[3] = (cl_context_properties) platform;
      properties[4] = 0;
      mCLGLInteropEnabled = true;
    }

#elif defined WIN32 // Windows

    HGLRC WINAPI context = wglGetCurrentContext();
    HDC WINAPI dc = wglGetCurrentDC();

	if(context != NULL && dc != NULL)
	{
		properties[0] = CL_GL_CONTEXT_KHR;
		properties[1] = (cl_context_properties) context;
		properties[2] = CL_WGL_HDC_KHR;
		properties[3] = (cl_context_properties) dc;
		properties[4] = CL_CONTEXT_PLATFORM;
		properties[5] = (cl_context_properties) platform;
		properties[6] = 0;
		mCLGLInteropEnabled = true;
	}

#else	// Linux

	GLXContext context = glXGetCurrentContext();

	if(context != NULL && glXGetCurrentDisplay() != NULL)
	{
		// Enable an OpenCL - OpenGL interop session.
		// This works for an X11 OpenGL session on Linux.
		properties[0] = CL_GL_CONTEXT_KHR;
		properties[1] = (cl_context_properties) context;
		properties[2] = CL_GLX_DISPLAY_KHR;
		properties[3] = (cl_context_properties) glXGetCurrentDisplay();
		properties[4] = CL_CONTEXT_PLATFORM;
		properties[5] = (cl_context_properties) platform;
		properties[6] = 0;
		mCLGLInteropEnabled = true;
	}

#endif

	// If OpenCL - OpenGL interop was not detected, enable a plain OpenCL-only context:
	if(!mCLGLInteropEnabled)
	{
//		cout << "OpenCL-OpenGL interoperability NOT detected and NOT ENABLED." << endl;
		// enable a plain OpenCL-only context.
		properties[0] = CL_CONTEXT_PLATFORM;
		properties[1] = (cl_context_properties) platform;
		properties[2] = 0;
	}
	else
	{
//		cout << "OpenCL-OpenGL interoperability DETECTED and ENABLED." << endl;
	}

	// Creates a context with the above properties.
    this->mContext = clCreateContextFromType(properties, type, NULL, NULL, &status);
	CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed.");

    // Create a command queue
    this->mQueue = clCreateCommandQueue(mContext, mDevice, 0, &status);
	CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");

	// I wasn't able to get the C++ headers to init the context correctly, we'll just use the C-versions for now
//	// TODO: Register a callback error function.
//	this->mContext = cl::Context(mDevices, CL_CONTEXT_PLATFORM, NULL, NULL, &err);
//	CheckOCLError("Unable to create context.", err);
//
//	// TODO: If we enable multiple devices, we should use something other than the zeroth entry here:
//	this->mQueue = cl::CommandQueue(mContext, mDevices[0], 0, &err);
//	CheckOCLError("Unable to create command queue.", err);
}

/// Convert the OpenCL error code into a string.
string COpenCL::getOpenCLErrorCodeStr(cl_int err)
{
    switch (err)
    {
    case CL_DEVICE_NOT_FOUND:        					return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE:        				return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE:        				return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:        		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES:        					return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY:        					return "CL_OUT_OF_HOST_MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE:        		return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case CL_MEM_COPY_OVERLAP:        					return "CL_MEM_COPY_OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH:        				return "CL_IMAGE_FORMAT_MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:        			return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE:        				return "CL_BUILD_PROGRAM_FAILURE";
    case CL_MAP_FAILURE:        						return "CL_MAP_FAILURE";
    case CL_MISALIGNED_SUB_BUFFER_OFFSET:				return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:	return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case CL_INVALID_VALUE:        						return "CL_INVALID_VALUE";
    case CL_INVALID_DEVICE_TYPE:        				return "CL_INVALID_DEVICE_TYPE";
    case CL_INVALID_PLATFORM:        					return "CL_INVALID_PLATFORM";
    case CL_INVALID_DEVICE:        						return "CL_INVALID_DEVICE";
    case CL_INVALID_CONTEXT:        					return "CL_INVALID_CONTEXT";
    case CL_INVALID_QUEUE_PROPERTIES:        			return "CL_INVALID_QUEUE_PROPERTIES";
    case CL_INVALID_COMMAND_QUEUE:        				return "CL_INVALID_COMMAND_QUEUE";
    case CL_INVALID_HOST_PTR:        					return "CL_INVALID_HOST_PTR";
    case CL_INVALID_MEM_OBJECT:        					return "CL_INVALID_MEM_OBJECT";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:        	return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case CL_INVALID_IMAGE_SIZE:        					return "CL_INVALID_IMAGE_SIZE";
    case CL_INVALID_SAMPLER:        					return "CL_INVALID_SAMPLER";
    case CL_INVALID_BINARY:        						return "CL_INVALID_BINARY";
    case CL_INVALID_BUILD_OPTIONS:        				return "CL_INVALID_BUILD_OPTIONS";
    case CL_INVALID_PROGRAM:        					return "CL_INVALID_PROGRAM";
    case CL_INVALID_PROGRAM_EXECUTABLE:        			return "CL_INVALID_PROGRAM_EXECUTABLE";
    case CL_INVALID_KERNEL_NAME:        				return "CL_INVALID_KERNEL_NAME";
    case CL_INVALID_KERNEL_DEFINITION:        			return "CL_INVALID_KERNEL_DEFINITION";
    case CL_INVALID_KERNEL:        						return "CL_INVALID_KERNEL";
    case CL_INVALID_ARG_INDEX:        					return "CL_INVALID_ARG_INDEX";
    case CL_INVALID_ARG_VALUE:        					return "CL_INVALID_ARG_VALUE";
    case CL_INVALID_ARG_SIZE:        					return "CL_INVALID_ARG_SIZE";
    case CL_INVALID_KERNEL_ARGS:        				return "CL_INVALID_KERNEL_ARGS";
    case CL_INVALID_WORK_DIMENSION:        				return "CL_INVALID_WORK_DIMENSION";
    case CL_INVALID_WORK_GROUP_SIZE:        			return "CL_INVALID_WORK_GROUP_SIZE";
    case CL_INVALID_WORK_ITEM_SIZE:        				return "CL_INVALID_WORK_ITEM_SIZE";
    case CL_INVALID_GLOBAL_OFFSET:        				return "CL_INVALID_GLOBAL_OFFSET";
    case CL_INVALID_EVENT_WAIT_LIST:        			return "CL_INVALID_EVENT_WAIT_LIST";
    case CL_INVALID_EVENT:        						return "CL_INVALID_EVENT";
    case CL_INVALID_OPERATION:        					return "CL_INVALID_OPERATION";
    case CL_INVALID_GL_OBJECT:        					return "CL_INVALID_GL_OBJECT";
    case CL_INVALID_BUFFER_SIZE:        				return "CL_INVALID_BUFFER_SIZE";
    case CL_INVALID_MIP_LEVEL:        					return "CL_INVALID_MIP_LEVEL";
    case CL_INVALID_GLOBAL_WORK_SIZE:        			return "CL_INVALID_GLOBAL_WORK_SIZE";
    // The following five lines cause issues on old NVidia and new Apple systems.
    //case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:        return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    //case CL_PLATFORM_NOT_FOUND_KHR:        				return "CL_PLATFORM_NOT_FOUND_KHR";
    //case CL_INVALID_PROPERTY_EXT:        //    return "CL_INVALID_PROPERTY_EXT";
    //case CL_DEVICE_PARTITION_FAILED_EXT:        		return "CL_DEVICE_PARTITION_FAILED_EXT";
    //case CL_INVALID_PARTITION_COUNT_EXT:        		return "CL_INVALID_PARTITION_COUNT_EXT";
    default:        return "unknown error code";
    }
}

/// Returns the OpenCL version as a three-digit unsigned integer.
///
/// The versions are as follows:
///  1.0 -> 100
///  1.1 -> 110
///  1.2 -> 120
///  2.0 -> 200
///  other -> 000
unsigned int COpenCL::GetOpenCLVersion()
{
	return mCLVersion;
}

void COpenCL::PrintDeviceInfo(cl_device_id device_id)
{
	int err = CL_SUCCESS;
	int i;
	size_t j;
	size_t returned_size;

	// Report the device vendor and device name
	cl_char vendor_name[1024] = {0};
	cl_char device_name[1024] = {0};
	cl_char device_profile[1024] = {0};
	cl_char device_cl_version[1024] = {0};
	cl_char driver_cl_version[1024] = {0};
	cl_char device_extensions[1024] = {0};
	cl_device_local_mem_type local_mem_type;

	cl_ulong global_mem_size, global_mem_cache_size, local_mem_size;
	cl_ulong max_mem_alloc_size;

	cl_uint clock_frequency, vector_width, max_compute_units, max_samplers, max_work_item_dimensions;

	cl_uint vector_types[] = {CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE};
	string vector_type_names[] = {"char","short","int","long","float","double"};
	cl_bool has_image_support;
	size_t max_2Dimage_height = 0;
	size_t max_2Dimage_width = 0;
	size_t max_3Dimage_height = 0;
	size_t max_3Dimage_width = 0;
	size_t max_3Dimage_depth = 0;

	// basic device information
	err = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_PROFILE, sizeof(device_profile), device_profile, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, sizeof(device_extensions), device_extensions, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(device_cl_version), device_cl_version, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DRIVER_VERSION, sizeof(driver_cl_version), driver_cl_version, &returned_size);

	// information about memory
	err|= clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(global_mem_cache_size), &global_mem_cache_size, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, &returned_size);

	// Device specifics:
	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(max_work_item_dimensions), &max_work_item_dimensions, &returned_size);

	size_t max_work_group_size, max_work_item_sizes[max_work_item_dimensions];

	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_work_item_sizes), max_work_item_sizes, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units), &max_compute_units, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_SAMPLERS, sizeof(max_samplers), &max_samplers, &returned_size);

	// Image information:
	err|= clGetDeviceInfo(device_id, CL_DEVICE_IMAGE_SUPPORT, sizeof(has_image_support), &has_image_support, &returned_size);
	// 2D specific:
	err|= clGetDeviceInfo(device_id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(max_2Dimage_height), &max_2Dimage_height, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(max_2Dimage_width), &max_2Dimage_width, &returned_size);
	// 3D specific
	err|= clGetDeviceInfo(device_id, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(max_3Dimage_height), &max_3Dimage_height, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(max_3Dimage_width), &max_3Dimage_width, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(max_3Dimage_width), &max_3Dimage_width, &returned_size);

	// Print out some information about the hardware
	cout << "Device information: " << endl;
	cout << "Device Name: " << device_name << endl;
	cout << "Vendor: " << vendor_name << endl;
	cout << "Device OpenCL version (full): " << device_cl_version << endl;
	cout << "Device OpenCL version (number): " << FindOpenCLVersion() << endl;
	cout << "Driver OpenCL version: " << driver_cl_version << endl;
	cout << "Profile: " << device_profile << endl;
	cout << "Supported Extensions: " << device_extensions << endl;

	cout << endl;
	cout << "Device specifics:" << endl;
	cout << "Clock Frequency (MHz): " << clock_frequency << endl;

	for(i = 0; i < 6; i++)
	{
		err|= clGetDeviceInfo(device_id, vector_types[i], sizeof(clock_frequency), &vector_width, &returned_size);
		cout << "Vector type width for " << vector_type_names[i] << ": " << vector_width << endl;
	}
	cout << "Max Work Dimensions: " << max_work_item_dimensions << endl;
	cout << "Max Work Group Size: " << max_work_group_size << endl;
	cout << "Max Work Item Dimensions: " << max_work_item_dimensions << endl;
	for(j = 0; j < max_work_item_dimensions; j++)
		cout << "Max Work Items in Dimension " << (long unsigned)(j+1) << ": " << (long unsigned)max_work_item_sizes[j] << endl;

	cout << "Max Compute Units: " << max_compute_units << endl;

	cout << endl;
	cout << "Device image information:" << endl;
	cout << "Has image support: (0 = no, 1 = yes): " << int(has_image_support) << endl;
	cout << "Maximum image samplers per kernel: " << max_samplers << endl;
	cout << "Maximum 2D image size (w,h) (pixels): " << max_2Dimage_width << ", " << max_2Dimage_height << endl;
	cout << "Maximum 2D image size (w,h,d) (pixels): " << max_3Dimage_width << ", " << max_3Dimage_height << ", " << max_3Dimage_depth << endl;;

	cout << endl;
	cout << "Device memory information:" << endl;
	cout << "Local Mem Type (Local=1, Global=2): " << local_mem_type << endl;
	cout << "Local Mem Size (kB): " << local_mem_size / (1024) << endl;
	cout << "Global Mem Size (MB): " << global_mem_size/(1024*1024) << endl;
	cout << "Global Mem Cache Size (Bytes): " << global_mem_cache_size << endl;
	cout << "Max Mem Alloc Size (MB): " << max_mem_alloc_size/(1024*1024) << endl;
	cout << endl;

}

void COpenCL::PrintPlatformInfo(cl_platform_id platform_id)
{
	string * tmp = new string();
	cl::Platform * platform = new cl::Platform(platform_id);

	platform->getInfo(CL_PLATFORM_PROFILE, tmp);
	cout << "Profile: " << tmp << endl;
	platform->getInfo(CL_PLATFORM_VERSION, tmp);
	cout << "Version: " << tmp << endl;
	platform->getInfo(CL_PLATFORM_NAME, tmp);
	cout << "Name: " << tmp << endl;
	platform->getInfo(CL_PLATFORM_VENDOR, tmp);
	cout << "Vendor: " << tmp << endl;
	platform->getInfo(CL_PLATFORM_EXTENSIONS, tmp);
	cout << "Extensions: " << tmp << endl;

	delete tmp;
}
