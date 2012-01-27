/*
 * CRoutine_Reduce_Sum.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include <cstdio>
#include <sstream>
#include "CRoutine_Reduce_Sum.h"

using namespace std;

CRoutine_Reduce_Sum::CRoutine_Reduce_Sum(cl_device_id device, cl_context context, cl_command_queue queue)
	: CRoutine_Reduce(device, context, queue)
{
	// Specify the source location, set temporary buffers to null
	mSource.push_back("reduce_sum_float.cl");
}

CRoutine_Reduce_Sum::~CRoutine_Reduce_Sum()
{

}

// Performs an out-of-plate sum storing temporary values in output_buffer and partial_sum_buffer.
float CRoutine_Reduce_Sum::ComputeSum(bool copy_back, cl_mem final_buffer, cl_mem input_buffer, cl_mem output_buffer, cl_mem partial_sum_buffer)
{
#ifdef DEBUG
	printf("Computing Parallel Sum. \n");
#endif // DEBUG

#ifdef DEBUG_VERBOSE
	ComputeSum_CPU(input_buffer, num_elements);
#endif // DEBUG_VERBOSE

	// TODO: Perhaps we should just allocate temporary buffers if things are null and store them in the class?
    if(input_buffer == NULL || final_buffer == NULL)
    	COpenCL::CheckOCLError("Input / Output parallel sum buffers are NULL!", -1);

    // See if the temporary buffers are specified, if not use the internal buffers.
    if(partial_sum_buffer == NULL || output_buffer == NULL)
    {
    	if(tmp_buff1 == NULL || tmp_buff2 == NULL)
    		AllocateInternalBuffers();

    	partial_sum_buffer = tmp_buff1;
    	output_buffer = tmp_buff2;
    }


    int i;
    int err = CL_SUCCESS;

    // Do the reduction for each level
    cl_mem pass_swap;
    cl_mem pass_input = output_buffer;
    cl_mem pass_output = input_buffer;
    cl_mem partials_buffer = partial_sum_buffer; // Partial sum buffer

    for(i = 0; i < pass_count; i++)
    {
        size_t global = group_counts[i] * work_item_counts[i];
        size_t local = work_item_counts[i];
        unsigned int operations = operation_counts[i];
        unsigned int entries = entry_counts[i];
        size_t shared_size = sizeof(float) * local * operations;

#ifdef DEBUG_VERBOSE
		printf("Pass[%4d] Global[%4d] Local[%4d] Groups[%4d] WorkItems[%4d] Operations[%d] Entries[%d]\n",  i,
			(int)global, (int)local, (int)group_counts[i], (int)work_item_counts[i], operations, entries);
#endif // DEBUG_VERBOSE

        // Swap the inputs and outputs for each pass
        //
        pass_swap = pass_input;
        pass_input = pass_output;
        pass_output = pass_swap;

        err = CL_SUCCESS;
        err |= clSetKernelArg(mKernels[i],  0, sizeof(cl_mem), &pass_output);
        err |= clSetKernelArg(mKernels[i],  1, sizeof(cl_mem), &pass_input);
        err |= clSetKernelArg(mKernels[i],  2, shared_size,    NULL);
        err |= clSetKernelArg(mKernels[i],  3, sizeof(int),    &entries);
		COpenCL::CheckOCLError("Failed to set partial sum kernel arguments.", err);

        // After the first pass, use the partial sums for the next input values
        //
        if(pass_input == input_buffer)
            pass_input = partials_buffer;

        err = CL_SUCCESS;
        err |= clEnqueueNDRangeKernel(mQueue, mKernels[i], 1, NULL, &global, &local, 0, NULL, NULL);
        COpenCL::CheckOCLError("Failed to enqueue parallel sum kernels.", err);
    }

    // Copy the new chi2 value over to it's final place in GPU memory.
    err = clEnqueueCopyBuffer(mQueue, pass_output, final_buffer, 0, 0, sizeof(cl_float), 0, NULL, NULL);
    COpenCL::CheckOCLError("Could not copy summed value to/from buffers on the GPU.", err);


    // If we need to copy data back do so
    if(copy_back)
    {
        cl_float sum = 0;
        err = clEnqueueReadBuffer(mQueue, final_buffer, CL_TRUE, 0, sizeof(cl_float), &sum, 0, NULL, NULL );
        COpenCL::CheckOCLError("Could not read back OpenCL SUM value.", err);

        // Check for a NaN:
        if(sum != sum)
        {
        	COpenCL::CheckOCLError("Error: Calculation yielded NAN.", 1);
        }

        // Return the sum
        return (float) sum;
    }

    // Return zero otherwise.
    return 0.0;
}

float CRoutine_Reduce_Sum::ComputeSum_CPU(cl_mem input_buffer, int n)
{
	int err = 0;
	cl_float * tmp = new cl_float[n];
	err = clEnqueueReadBuffer(mQueue, input_buffer, CL_TRUE, 0, n * sizeof(cl_float), tmp, 0, NULL, NULL);

	float sum = 0;
	printf("Computing Sum on CPU from input_buffer\n");
	for(int i = 0; i < n; i++)
	{
		printf("\t%i %e\n", i, tmp[i]);
		sum += tmp[i];
	}

	printf("CPU Sum: %f\n", sum);

	delete[] tmp;
}

/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Reduce_Sum::Init(int num_elements, bool allocate_temp_buffers)
{
	// Set the number of elements on which this kernel will operate.
	this->num_elements = num_elements;
	BuildKernels();

	// Now allocate any required buffers:
//	if(allocate_temp_buffers)
//		AllocateInternalBuffers();

}
