/*
 * CRoutine_Sum.cpp
 *
 *  Created on: Jan 28, 2014
 *      Author: bkloppenborg
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

} /* namespace liboi */
