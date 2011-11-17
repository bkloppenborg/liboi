/*
 * COpenCLRoutine.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include <cstdio>
#include "COpenCLRoutine.h"


COpenCLRoutine::COpenCLRoutine()
{
	mKernelPath = "";

}

COpenCLRoutine::~COpenCLRoutine()
{
	// Release any kernels or programs
	int i;
	for(i = mKernels.size(); i > 0; i--)
	{
		clReleaseKernel(mKernels[i]);
		mKernels.pop_back();
	}

	for(i = mPrograms.size(); i > 0; i--)
	{
		clReleaseProgram(mPrograms[i]);
		mPrograms.pop_back();
	}


}

/// Builds the kernel from the specified string.
/// Appends the compiled kernel to mKernels and mPrograms, returns the index at which this kernel is located.
int COpenCLRoutine::BuildKernel(string source)
{
    const char * tmp = source.c_str();
    cl_program program;
    cl_kernel kernel;
    size_t len;
    int err;
    string tmp_err;
    tmp_err.reserve(2048);

    // Create the program
    program = clCreateProgramWithSource(mContext, 1, &tmp, NULL, &err);
    if (!program || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute program!\n");
        printf("%s", tmp_err.c_str());
        clGetProgramBuildInfo(program, mDeviceID, CL_PROGRAM_BUILD_LOG, tmp_err.size(), &tmp_err, &len);
        printf("%s\n", tmp_err.c_str());
        // TODO: Throw an exception, cleanly exit.
    }

    // Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute program!\n");
        printf("%s", tmp_err.c_str());
        clGetProgramBuildInfo(program, mDeviceID, CL_PROGRAM_BUILD_LOG, tmp_err.size(), &tmp_err, &len);
        printf("%s\n", tmp_err.c_str());
        // TODO: Throw an exception, cleanly exit.
    }

    // Program built correctly, push it onto the vector
    mPrograms.push_back(program);

    // Create the compute kernel from within the program
    kernel = clCreateKernel(program, "reduce", &err);
	COpenCL::CheckOCLError("Failed to create parallel sum kernel.", err);

	// All is well, push the kernel onto the vector
	mKernels.push_back(kernel);

	// Free memory
	delete tmp;

	// Return the index of the latest kernel
	return mPrograms.size();
}

string COpenCLRoutine::ReadSource(string filename)
{
	return ReadFile(mKernelPath + '/' +  filename, "Could not read OpenCL kernel source " + mKernelPath + '/' +  filename);
}

void SetSourcePath(string path_to_kernels)
{
	mKernelPath = path_to_kernels;
}
