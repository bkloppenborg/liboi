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
using namespace std;

class COpenCLRoutine
{
protected:
	cl_device_id mDeviceID;
	cl_context_id mContext;
	vector<cl_program> mPrograms;
	vector<cl_kernel> mKernels;
	vector<string> mSource;

public:
	COpenCLRoutine();
	~COpenCLRoutine();

	string ReadSource(string filename);
};

#endif /* COPENCLROUTINE_H_ */
