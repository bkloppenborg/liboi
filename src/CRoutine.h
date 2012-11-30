/*
 * CRoutine.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
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

/*  Base class for all LibOI routines.  Each inheriting class should implement at least
 *  three functions:
 *
 *    Foo(...)	 		// Default function for LibOI to call
 *    Foo_CL(...)  		// Called by Function, runs OpenCL routine
 *    Foo_CPU(...) 		// Takes same input as Foo_CL, calculates values on CPU.
 *    Foo_Verify(...)	// Checks OpenCL results against CPU calculations
 *    Init(...)			// Initializes the routine, building kernels, allocating any memory.
 *
 *  Foo() is just a common interface to the function, it should check CRoutine::mOpMode and
 *  route to the appropriate kernel function.
 *
 *  The *_CL function should run the OpenCL kernels on the OpenCL device identified
 *  by mDeviceID, mContext, and mQueue.  The OpenCL routines should be fully optimized.
 *  Any memory access optimizations in the OpenCL kernels should be documented in the
 *  kernels source.  With _CL kernels, most cl_mem objects should be in DEVICE memory.
 *
 *  The *_CPU function is a nearly UNOPTIMIZED version of the *_CL kernels that is used to
 *  test the validity of the *_CL kernel results.  Ideally the *_CPU kernel should be well
 *  documented, closely mirror what is done on the OpenCL device and be easy to follow.  These
 *  functions should copy over OpenCL memory and use it as input and store the calculations in
 *  class-level members (if needed in a later step).
 *
 *  Lastly, the *_Verify function should execute the *_CL and *_CPU versions of the functions
 *  and compare intermediate results using one of the CRoutine::Compare* functions.  Assume
 *  that all cl_mem objects are in DEVICE memory and that they need to be copied to the host
 *  before using them in the _CPU functions.  This is a slow process, but guarantees that
 *  the CL and CPU functions produce the same output given the same input.
 *
 */
 


#ifndef CROUTINE_H_
#define CROUTINE_H_

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <valarray>
#include <cassert>

#include "textio.hpp"
#include "liboi.hpp"

#include "COpenCL.h"

#define MAX_REL_ERROR 0.001	// Maximum of 0.1% relative error permitted

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>

#ifndef PI
#ifdef M_PI
#define PI M_PI
#else
#define PI 3.1415926535897932384626433832795028841968
#endif
#endif

using namespace std;

class CRoutine
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
	CRoutine(cl_device_id mDevice, cl_context mContext, cl_command_queue mQueue);
	virtual ~CRoutine();

	int BuildKernel(string source, string kernel_name);
	int BuildKernel(string source, string kernel_name, string kernel_filename);

	void DumpFloatBuffer(cl_mem buffer, unsigned int size);

	void PassFail(bool);

	string ReadSource(string filename);

	void SetSourcePath(string path_to_kernels);

	bool Verify(valarray<cl_float> & cpu_buffer, cl_mem device_buffer, int n_elements, size_t offset);
	bool Verify(valarray<complex<float>> & cpu_buffer, cl_mem device_buffer, int num_elements, size_t offset);
};

#endif /* COPENCLROUTINE_H_ */
