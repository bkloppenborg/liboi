/*
 * CRoutine_Reduce.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include <cstdio>
#include <sstream>
#include "CRoutine_Reduce.h"

using namespace std;

CRoutine_Reduce::CRoutine_Reduce(cl_device_id device, cl_context context, cl_command_queue queue)
	:COpenCLRoutine(device, context, queue)
{
	// Specify the source location, set temporary buffers to null
	mSource.push_back("reduce_float.cl");
	tmp_buff1 = NULL;
	tmp_buff2 = NULL;

}

CRoutine_Reduce::~CRoutine_Reduce()
{
	// Release temporary buffers:
	if(tmp_buff1) clReleaseMemObject(tmp_buff1);
	if(tmp_buff2) clReleaseMemObject(tmp_buff2);
}

void CRoutine_Reduce::AllocateInternalBuffers()
{
	int err;
	if(tmp_buff1 == NULL)
		tmp_buff1 = clCreateBuffer(mContext, CL_MEM_READ_WRITE, num_elements * sizeof(cl_float), NULL, &err);

	if(tmp_buff2 == NULL)
		tmp_buff2 = clCreateBuffer(mContext, CL_MEM_READ_WRITE, num_elements * sizeof(cl_float), NULL, &err);

	COpenCL::CheckOCLError("Could not allocate temporary buffers for sum kernel", err);

}

void CRoutine_Reduce::CreateReductionPasscounts(int max_group_size, int max_groups, int max_work_items)
{
    int work_items = (num_elements < max_work_items * 2) ? num_elements / 2 : max_work_items;
    if(num_elements < 1)
        work_items = 1;

    int groups = num_elements / (work_items * 2);
    groups = max_groups < groups ? max_groups : groups;

    int max_levels = 1;
    int s = groups;

    while(s > 1)
    {
        int work_items = (s < max_work_items * 2) ? s / 2 : max_work_items;
        s = s / (work_items*2);
        max_levels++;
    }

    pass_count = max_levels;
    group_counts.push_back(groups);
    work_item_counts.push_back(work_items);
    operation_counts.push_back(1);
    entry_counts.push_back(num_elements);
    if(max_group_size < work_items)
    {
        operation_counts.push_back(work_items);
        work_item_counts.push_back(max_group_size);
    }

    s = groups;
    while(s > 1)
    {
        int work_items = (s < max_work_items * 2) ? s / 2 : max_work_items;
        int groups = s / (work_items * 2);
        groups = (max_groups < groups) ? max_groups : groups;

        group_counts.push_back(groups);
        work_item_counts.push_back(work_items);
        operation_counts.push_back(1);
        entry_counts.push_back(s);

        if(max_group_size < work_items)
        {
            operation_counts.push_back(work_items);
            work_item_counts.push_back(max_group_size);
        }

        s = s / (work_items*2);
    }
}

/// Compiles the kernels optimized to handle num_elements entries per reduction.
void CRoutine_Reduce::BuildKernels()
{
    // Init a few variables:
    int err = 0;
    int i;
    cl_program program;
    cl_kernel kernel;

#ifdef DEBUG
    string message = "Loading and Compiling program " + mSource[0] + "\n";
	printf("%s\n", message.c_str());
#endif //DEBUG

	string source = ReadSource(mSource[0]);

	// Determine the limits of the OpenCL device:
    size_t returned_size = 0;
    size_t max_workgroup_size = 0;
    err |= clGetDeviceInfo(mDeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_workgroup_size, &returned_size);
    COpenCL::CheckOCLError("Couldn't get maximum work group size from the device.", err);

    // Determine the reduction pass configuration for each level in the pyramid
    CreateReductionPasscounts(max_workgroup_size, MAX_GROUPS, MAX_WORK_ITEMS);

    // TODO: We might need to set format flags here:
    stringstream tmp;

    for(i = 0; i < pass_count; i++)
    {
        // Insert macro definitions to specialize the kernel to a particular group size.  Then build the kernel.
        tmp.clear();
        tmp << "#define GROUP_SIZE " << group_counts[i] << "\n" << "#define OPERATIONS " << operation_counts[i] << "\n\n";
        tmp << source;

        BuildKernel(tmp.str());
    }

}

// Performs an out-of-plate sum storing temporary values in output_buffer and partial_sum_buffer.
float CRoutine_Reduce::ComputeSum(bool copy_back, cl_mem final_buffer, cl_mem input_buffer, cl_mem output_buffer, cl_mem partial_sum_buffer)
{
#ifdef DEBUG
	printf("Computing Parallel Sum. \n");
#endif // DEBUG

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
    int err;

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

#ifdef DEBUG
		printf("Pass[%4d] Global[%4d] Local[%4d] Groups[%4d] WorkItems[%4d] Operations[%d] Entries[%d]\n",  i,
			(int)global, (int)local, (int)group_counts[i], (int)work_item_counts[i], operations, entries);
#endif // DEBUG

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
    err = clEnqueueCopyBuffer(mQueue, pass_output, final_buffer, 0, 0, sizeof(float), 0, NULL, NULL);
    COpenCL::CheckOCLError("Could not copy summed value to/from buffers on the GPU.", err);


    // If we need to copy data back do so
    if(copy_back)
    {
        float sum = 0;
        err = clEnqueueReadBuffer(mQueue, final_buffer, CL_TRUE, 0, sizeof(float), &sum, 0, NULL, NULL );
        COpenCL::CheckOCLError("Could not read back GPU SUM value.", err);

        // Check for a NaN:
        if(sum != sum)
        	COpenCL::CheckOCLError("Error: Calculation yielded NAN.", 0);

        // Return the sum
        return sum;
    }

    // Return zero otherwise.
    return 0.0;
}

/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Reduce::Init(int num_elements, bool allocate_temp_buffers)
{
	// Set the number of elements on which this kernel will operate.
	this->num_elements = num_elements;
	BuildKernels();

	// Now allocate any required buffers:
	if(allocate_temp_buffers)
	{

	}

}
