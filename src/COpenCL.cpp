/*
 * COpenCL.cpp
 *
 *  Created on: Nov 11, 2011
 *      Author: bkloppenborg
 */

#include <cstdio>
#include <vector>
#include "COpenCL.h"

using namespace std;

COpenCL::COpenCL()
{
	// init datamembers
	mDevice = NULL;
	mContext = NULL;
	mQueue = NULL;
}

COpenCL::~COpenCL()
{
	// Free OpenCL memory:
	if(mQueue) clReleaseCommandQueue(mQueue);
	if(mContext) clReleaseContext(mContext);
}

/// Check error_code and output a detailed message if not CL_SUCCESS.
void COpenCL::CheckOCLError(string user_message, int error_code)
{
	if(error_code != CL_SUCCESS)
	{
		// Something bad happened.  Look up the error code:
		string error_string = GetOCLErrorString(error_code);
		// and print out a message:
		printf("Error Detected\n");
		printf("%s \n", user_message.c_str());
		printf("OpenCL Error: %s\n", error_string.c_str());
		//printf(SEP);
		//exit(0);
	}
}

/// Searches through the available platforms and devices and returns the first
/// instance of the specified type.
cl_device_id COpenCL::FindDevice(cl_device_type type)
{
	vector<cl_platform_id> platforms;
	GetPlatformList(&platforms);
	int i, j;
	vector<cl_device_id> devices;

	for(i = 0; i < platforms.size(); i++)
	{
		GetDeviceList(platforms[i], &devices);

		for(j = 0; j < devices.size(); j++)
		{
			if(GetDeviceType(devices[j]) == type)
				return devices[j];
		}

		// Clear the vector
		devices.clear();
	}

	return NULL;
}

/// Returns a list of platforms on this machine
void COpenCL::GetPlatformList(vector<cl_platform_id> * platforms)
{
	// init a few variables
	cl_uint n = 0;
	int err = 0;

	// First query to find out how many platforms exist:
	err |= clGetPlatformIDs(0, NULL, &n);

	// Now allocate memory and pull out the platform IDs
	cl_platform_id * tmp = new cl_platform_id[n];
	err |= clGetPlatformIDs(n, tmp, &n);
	CheckOCLError("Unable to get a list of platforms on this computer.", err);

	platforms->assign(&tmp[0], &tmp[n]);
}

/// Returns a list of devices on the specified platform.
void COpenCL::GetDeviceList(cl_platform_id platform, vector<cl_device_id> * devices)
{
	cl_uint n = 0;
	int err = 0;

	// Query to determine the number of devices:
	err |= clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &n);

	// Now allocate memory and pull out the device IDs
	cl_device_id * tmp = new cl_device_id[n];
	err |= clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, n, tmp, &n);
	CheckOCLError("Unable to get a list of devices for the platform.", err);
	devices->assign(&tmp[0], &tmp[n]);
}

/// Gets the type of device corresponding to device_id
cl_device_type COpenCL::GetDeviceType(cl_device_id device_id)
{
	cl_device_type type;
	int err = clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
	CheckOCLError("Unable to determine the type of the specified device.", err);

	return type;
}

/// Initializes the class using the first device found with the specified type.
void COpenCL::Init(cl_device_type type)
{
	this->Init(this->FindDevice(type));
}

/// Initializes the class.  Creates contexts and command queues.
void COpenCL::Init(cl_device_id device_id)
{
	int err = 0;
	this->mDevice = device_id;
#ifdef DEBUG
	printf("Initializing using the following device:\n");
	this->PrintDeviceInfo(device_id);
#endif //DEBUG

	this->mContext = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	CheckOCLError("Unable to create context.", err);

	this->mQueue = clCreateCommandQueue(mContext, device_id, 0, &err);
	CheckOCLError("Unable to create command queue.", err);
}

/// Convert the OpenCL error code into a string.
string COpenCL::GetOCLErrorString(cl_int err)
{
    switch (err) {
        case CL_SUCCESS:                          return "Success!";
        case CL_DEVICE_NOT_FOUND:                 return "Device not found.";
        case CL_DEVICE_NOT_AVAILABLE:             return "Device not available";
        case CL_COMPILER_NOT_AVAILABLE:           return "Compiler not available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return "Memory object allocation failure";
        case CL_OUT_OF_RESOURCES:                 return "Out of resources";
        case CL_OUT_OF_HOST_MEMORY:               return "Out of host memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:     return "Profiling information not available";
        case CL_MEM_COPY_OVERLAP:                 return "Memory copy overlap";
        case CL_IMAGE_FORMAT_MISMATCH:            return "Image format mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return "Image format not supported";
        case CL_BUILD_PROGRAM_FAILURE:            return "Program build failure";
        case CL_MAP_FAILURE:                      return "Map failure";
        case CL_INVALID_VALUE:                    return "Invalid value";
        case CL_INVALID_DEVICE_TYPE:              return "Invalid device type";
        case CL_INVALID_PLATFORM:                 return "Invalid platform";
        case CL_INVALID_DEVICE:                   return "Invalid device";
        case CL_INVALID_CONTEXT:                  return "Invalid context";
        case CL_INVALID_QUEUE_PROPERTIES:         return "Invalid queue properties";
        case CL_INVALID_COMMAND_QUEUE:            return "Invalid command queue";
        case CL_INVALID_HOST_PTR:                 return "Invalid host pointer";
        case CL_INVALID_MEM_OBJECT:               return "Invalid memory object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return "Invalid image format descriptor";
        case CL_INVALID_IMAGE_SIZE:               return "Invalid image size";
        case CL_INVALID_SAMPLER:                  return "Invalid sampler";
        case CL_INVALID_BINARY:                   return "Invalid binary";
        case CL_INVALID_BUILD_OPTIONS:            return "Invalid build options";
        case CL_INVALID_PROGRAM:                  return "Invalid program";
        case CL_INVALID_PROGRAM_EXECUTABLE:       return "Invalid program executable";
        case CL_INVALID_KERNEL_NAME:              return "Invalid kernel name";
        case CL_INVALID_KERNEL_DEFINITION:        return "Invalid kernel definition";
        case CL_INVALID_KERNEL:                   return "Invalid kernel";
        case CL_INVALID_ARG_INDEX:                return "Invalid argument index";
        case CL_INVALID_ARG_VALUE:                return "Invalid argument value";
        case CL_INVALID_ARG_SIZE:                 return "Invalid argument size";
        case CL_INVALID_KERNEL_ARGS:              return "Invalid kernel arguments";
        case CL_INVALID_WORK_DIMENSION:           return "Invalid work dimension";
        case CL_INVALID_WORK_GROUP_SIZE:          return "Invalid work group size";
        case CL_INVALID_WORK_ITEM_SIZE:           return "Invalid work item size";
        case CL_INVALID_GLOBAL_OFFSET:            return "Invalid global offset";
        case CL_INVALID_EVENT_WAIT_LIST:          return "Invalid event wait list";
        case CL_INVALID_EVENT:                    return "Invalid event";
        case CL_INVALID_OPERATION:                return "Invalid operation";
        case CL_INVALID_GL_OBJECT:                return "Invalid OpenGL object";
        case CL_INVALID_BUFFER_SIZE:              return "Invalid buffer size";
        case CL_INVALID_MIP_LEVEL:                return "Invalid mip-map level";
        default:                                  return "Unknown";
    }
}

void COpenCL::PrintDeviceInfo(cl_device_id device_id)
{
	int err;
	int i;
	size_t j;
	size_t returned_size;

	printf("\n");
	// Report the device vendor and device name
	cl_char vendor_name[1024] = {0};
	cl_char device_name[1024] = {0};
	cl_char device_profile[1024] = {0};
	cl_char device_extensions[1024] = {0};
	cl_device_local_mem_type local_mem_type;

	cl_ulong global_mem_size, global_mem_cache_size;
	cl_ulong max_mem_alloc_size;

	cl_uint clock_frequency, vector_width, max_compute_units;

	size_t max_work_item_dims = 3;
	size_t max_work_group_size, max_work_item_sizes[3];

	cl_uint vector_types[] = {CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE};
	string vector_type_names[] = {"char","short","int","long","float","double"};

	err = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_PROFILE, sizeof(device_profile), device_profile, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, sizeof(device_extensions), device_extensions, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, &returned_size);

	err|= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(global_mem_cache_size), &global_mem_cache_size, &returned_size);
	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, &returned_size);

	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, &returned_size);

	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, &returned_size);

	// TODO: There be a bug here somewhere.  Upgrade to Nvidia driver 195 somehow screwed up this command.
	//err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(max_work_item_dims), &max_work_item_dims, &returned_size);

	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_work_item_sizes), max_work_item_sizes, &returned_size);

	err|= clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units), &max_compute_units, &returned_size);

	printf("Vendor: %s\n", vendor_name);
	printf("Device Name: %s\n", device_name);
	printf("Profile: %s\n", device_profile);
	printf("Supported Extensions: %s\n\n", device_extensions);

	printf("Local Mem Type (Local=1, Global=2): %i\n",(int) local_mem_type);
	printf("Global Mem Size (MB): %i\n",(int) global_mem_size/(1024*1024));
	printf("Global Mem Cache Size (Bytes): %i\n",(int) global_mem_cache_size);
	printf("Max Mem Alloc Size (MB): %ld\n",(long int) max_mem_alloc_size/(1024*1024));

	printf("Clock Frequency (MHz): %i\n\n",clock_frequency);

	for(i = 0; i < 6; i++)
	{
		err|= clGetDeviceInfo(device_id, vector_types[i], sizeof(clock_frequency), &vector_width, &returned_size);
		printf("Vector type width for: %s = %i\n",vector_type_names[i].c_str(),vector_width);
	}

	printf("\nMax Work Group Size: %lu\n",max_work_group_size);
	printf("Max Work Item Dims: %lu\n",max_work_item_dims);
	for(j = 0; j < max_work_item_dims; j++)
		printf("Max Work Items in Dim %lu: %lu\n",(long unsigned)(j+1),(long unsigned)max_work_item_sizes[j]);

	printf("Max Compute Units: %i\n",max_compute_units);
	printf("\n");
}

void COpenCL::PrintPlatformInfo(cl_platform_id platform_id)
{
	string * tmp = new string();
	cl::Platform * platform = new cl::Platform(platform_id);

	platform->getInfo(CL_PLATFORM_PROFILE, tmp);
	printf("Profile: %s\n", tmp->c_str());
	platform->getInfo(CL_PLATFORM_VERSION, tmp);
	printf("Version: %s\n", tmp->c_str());
	platform->getInfo(CL_PLATFORM_NAME, tmp);
	printf("Name: %s\n", tmp->c_str());
	platform->getInfo(CL_PLATFORM_VENDOR, tmp);
	printf("Vendor: %s\n", tmp->c_str());
	platform->getInfo(CL_PLATFORM_EXTENSIONS, tmp);
	printf("Extensions: %s\n", tmp->c_str());

	delete tmp;
}
