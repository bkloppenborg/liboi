/*
 * COpenCLRoutine.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#ifndef COPENCLROUTINE_H_
#define COPENCLROUTINE_H_

#include <string>
#include <vector>
#include "ReadTextFile.h"
#include "COpenCL.h"

#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;



class COpenCLRoutine
{
protected:
	cl_device_id mDeviceID;
	cl_context mContext;
	cl_command_queue mQueue;
	vector<cl_program> mPrograms;
	vector<cl_kernel> mKernels;
	vector<string> mSource;	// For storing the filenames of source files.

	string mKernelPath;

public:
	COpenCLRoutine(cl_device_id mDevice, cl_context mContext, cl_command_queue mQueue);
	~COpenCLRoutine();

	int BuildKernel(string source, string kernel_name);

	string ReadSource(string filename);

	void SetSourcePath(string path_to_kernels);
};

#endif /* COPENCLROUTINE_H_ */
