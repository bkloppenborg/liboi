/*
 * CRoutine_Sum.cpp
 *
 *  Created on: Jan 28, 2014
 *      Author: bkloppenborg
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

#include "CRoutine_Sum.h"

namespace liboi {

CRoutine_Sum::CRoutine_Sum(cl_device_id device, cl_context context, cl_command_queue queue, CRoutine_Zero * rZero)
	: CRoutine(device, context, queue)
{
	// Specify the source location, set temporary buffers to null
	mInputSize = 0;

	// External routines, do not delete/deallocate here.
	mrZero = rZero;
}

CRoutine_Sum::~CRoutine_Sum()
{
	// TODO Auto-generated destructor stub
}

bool CRoutine_Sum::isPow2(unsigned int x)
{
    return ((x&(x-1))==0);
}

unsigned int CRoutine_Sum::nextPow2( unsigned int x )
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

float CRoutine_Sum::Sum(cl_mem input_buffer, cl_mem final_buffer)
{
	// Compute the sum on the OpenCL device:
	float result = Sum(input_buffer);

	int status = CL_SUCCESS;

    // Copy the summed value to the OpenCL device:
	status = clEnqueueWriteBuffer(mQueue, final_buffer, CL_TRUE, 0, sizeof(cl_float), &result, 0, NULL, NULL);
	COpenCL::CheckOCLError("Unable to copy summed value from the CPU to the OpenCL device. CRoutine_Sum::Sum", status);

	return result;
}

} /* namespace liboi */
