/*
 * COpenCLRoutine.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "COpenCLRoutine.h"

COpenCLRoutine::COpenCLRoutine()
{
	// TODO Auto-generated constructor stub

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

string COpenCLRoutine::ReadSource(string filename)
{
	return ReadFile(filename, "Could not read OpenCL kernel source" + filename);
}
