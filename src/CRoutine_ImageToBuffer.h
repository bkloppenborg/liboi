/*
 * CRoutine_ImageToBuffer.h
 *
 *  Created on: Nov 18, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_IMAGETOBUFFER_H_
#define CROUTINE_IMAGETOBUFFER_H_

#include "COpenCLRoutine.h"

class CRoutine_ImageToBuffer: public COpenCLRoutine
{
public:
	CRoutine_ImageToBuffer(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_ImageToBuffer();

	void CopyImage(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int depth);
	void Init();
};

#endif /* CROUTINE_IMAGETOBUFFER_H_ */
