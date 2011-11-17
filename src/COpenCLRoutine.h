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
	COpenCLRoutine();
	~COpenCLRoutine();

	int BuildKernel(string source);

	string ReadSource(string filename);

	void SetSourcePath(string path_to_kernels);
};

#endif /* COPENCLROUTINE_H_ */
