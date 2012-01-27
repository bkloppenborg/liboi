/*
 * CRoutine_ImageToBuffer.cpp
 *
 *  Created on: Nov 18, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_ImageToBuffer.h"
#include <cstdio>

CRoutine_ImageToBuffer::CRoutine_ImageToBuffer(cl_device_id device, cl_context context, cl_command_queue queue)
	:COpenCLRoutine(device, context, queue)
{
	mSource.push_back("image2buf_GL_R.cl");

}

CRoutine_ImageToBuffer::~CRoutine_ImageToBuffer()
{
	// TODO Auto-generated destructor stub
}

// Read in the kernel source and build program object.
void CRoutine_ImageToBuffer::Init()
{
#ifdef DEBUG
    string message = "Loading and Compiling program " + mSource[0] + "\n";
	printf("%s\n", message.c_str());
#endif //DEBUG

	string source = ReadSource(mSource[0]);
	BuildKernel(source, "image2buf_GL_R");
}

void CRoutine_ImageToBuffer::CopyImage(cl_mem gl_image, cl_mem cl_buffer, int width, int height, int depth)
{
	// TODO: We need to redo this for 3D data sets and for non-square images.
	// TODO: Figure out how to determine these sizes dynamically.
	size_t * global = new size_t[2];
	global[0] = global[1] = width;
	size_t * local = new size_t[2];
	local[0] = local[1] = 16;

	// Enqueue the kernel.
	int err = CL_SUCCESS;
    err |= clSetKernelArg(mKernels[0],  0, sizeof(cl_mem), &gl_image);
    err |= clSetKernelArg(mKernels[0],  1, sizeof(cl_mem), &cl_buffer);
    err |= clSetKernelArg(mKernels[0],  2, sizeof(int), &width);
	COpenCL::CheckOCLError("Failed to set gl_image to cl_buffer kernel arguments.", err);


    err = CL_SUCCESS;
    err |= clEnqueueNDRangeKernel(mQueue, mKernels[0], 2, NULL, global, local, 0, NULL, NULL);
    COpenCL::CheckOCLError("Failed to enqueue image copying kernel.", err);

#ifdef DEBUG_VERBOSE
        	// Copy back the input/output buffers.
        	float tmp_sum = 0;
        	int num_elements = width * height;
        	cl_float * tmp = new cl_float[num_elements];
        	err = clEnqueueReadBuffer(mQueue, cl_buffer, CL_TRUE, 0, num_elements * sizeof(cl_float), tmp, 0, NULL, NULL);

        	for(int i = 0; i < num_elements; i++)
        	{
        		if(i % 10 == 0)
        			printf("%f ", tmp[i]);

        		tmp_sum += tmp[i];
        	}

        	printf("\n");
        	printf("Sum of Image copied to CPU: %f \n", tmp_sum);
#endif //DEBUG_VERBOSE

    // Free memory
    delete[] global;
    delete[] local;
}
