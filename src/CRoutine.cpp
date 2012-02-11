/*
 * CRoutine.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include <cstdio>
#include "CRoutine.h"


CRoutine::CRoutine(cl_device_id device, cl_context context, cl_command_queue queue)
{
	mDeviceID = device;
	mContext = context;
	mQueue = queue;
	mKernelPath = "";

}

CRoutine::~CRoutine()
{
	// Release any kernels or programs
	int i;
	for(i = mKernels.size() - 1; i > 0; i--)
	{
		clReleaseKernel(mKernels[i]);
		mKernels.pop_back();
	}

	for(i = mPrograms.size() - 1; i > 0; i--)
	{
		clReleaseProgram(mPrograms[i]);
		mPrograms.pop_back();
	}


}

/// A more verbose wrapper for BuildKernel, emits the name of the kernel before compilation.
int CRoutine::BuildKernel(string source, string kernel_name, string kernel_filename)
{
#ifdef DEBUG
    string message = "Loading and Compiling program " +  kernel_filename + "\n";
	printf("%s", message.c_str());
#endif //DEBUG

	return BuildKernel(source, kernel_name);
}

/// Builds the kernel from the specified string.
/// Appends the compiled kernel to mKernels and mPrograms, returns the index at which this kernel is located.
int CRoutine::BuildKernel(string source, string kernel_name)
{
    const char * tmp = source.c_str();
    cl_program program;
    cl_kernel kernel;
    size_t len = 0;
    int err;
    //    string tmp_err;
    //    tmp_err.reserve(2048);

	// Create the program
	program = clCreateProgramWithSource(mContext, 1, &tmp, NULL, &err);
	if (!program || err != CL_SUCCESS)
	{
		size_t length;
		char build_log[2048];
		//printf("%s\n", block_source);
		printf("Error: Failed to create compute program!\n");
		clGetProgramBuildInfo(program, mDeviceID, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &length);
		printf("%s\n", build_log);


//        printf("Error: Failed to create compute program!\n");
//        printf("%s", tmp_err.c_str());
//        clGetProgramBuildInfo(program, mDeviceID, CL_PROGRAM_BUILD_LOG, tmp_err.size(), &tmp_err, &len);
//        printf("%s\n", tmp_err.c_str());
		// TODO: Throw an exception, cleanly exit.
	}

	// Build the program executable
	err = clBuildProgram(program, 1, &mDeviceID, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t length;
		char build_log[2048];
		//printf("%s\n", block_source);
		printf("Error: Failed to build compute program!\n");
		clGetProgramBuildInfo(program, mDeviceID, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &length);
		printf("%s\n", build_log);

		// TODO: Throw an exception, cleanly exit.
	}

    // Program built correctly, push it onto the vector
    mPrograms.push_back(program);

    // Create the compute kernel from within the program
    kernel = clCreateKernel(program, kernel_name.c_str(), &err);
	COpenCL::CheckOCLError("Failed to create kernel named " + kernel_name, err);

	// All is well, push the kernel onto the vector
	mKernels.push_back(kernel);

	// Return the index of the latest kernel
	return mPrograms.size();
}

/// Dumps the entire contents of the specified buffer to standard out.
void CRoutine::DumpFloatBuffer(cl_mem buffer, unsigned int size)
{
	int err = CL_SUCCESS;
	cl_float * tmp = new cl_float[size];
	err |= clEnqueueReadBuffer(mQueue, buffer, CL_TRUE, 0, size * sizeof(cl_float), tmp, 0, NULL, NULL);
	for(int i = 0; i < size; i++)
		printf(" %i %f\n", i, tmp[i]);
}

string CRoutine::ReadSource(string filename)
{
	return ReadFile(mKernelPath + '/' +  filename, "Could not read OpenCL kernel source " + mKernelPath + '/' +  filename);
}

void CRoutine::SetSourcePath(string path_to_kernels)
{
	mKernelPath = path_to_kernels;
}
