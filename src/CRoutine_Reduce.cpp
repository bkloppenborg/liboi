/*
 * CRoutine_Reduce.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_Reduce.h"

CRoutine_Reduce::CRoutine_Reduce()
{
	// TODO Auto-generated constructor stub
	kernel_source = "reduce_float.cl";

}

CRoutine_Reduce::~CRoutine_Reduce()
{
	// TODO Auto-generated destructor stub
}

void CRoutine_Reduce::CreateReductionPasscounts(
    int count,
    int max_group_size,
    int max_groups,
    int max_work_items,
    int *pass_count,
    size_t **group_counts,
    size_t **work_item_counts,
    int **operation_counts,
    int **entry_counts)
{
    int work_items = (count < max_work_items * 2) ? count / 2 : max_work_items;
    if(count < 1)
        work_items = 1;

    int groups = count / (work_items * 2);
    groups = max_groups < groups ? max_groups : groups;

    int max_levels = 1;
    int s = groups;

    while(s > 1)
    {
        int work_items = (s < max_work_items * 2) ? s / 2 : max_work_items;
        s = s / (work_items*2);
        max_levels++;
    }

    *group_counts = (size_t*)malloc(max_levels * sizeof(size_t));
    *work_item_counts = (size_t*)malloc(max_levels * sizeof(size_t));
    *operation_counts = (int*)malloc(max_levels * sizeof(int));
    *entry_counts = (int*)malloc(max_levels * sizeof(int));

    (*pass_count) = max_levels;
    (*group_counts)[0] = groups;
    (*work_item_counts)[0] = work_items;
    (*operation_counts)[0] = 1;
    (*entry_counts)[0] = count;
    if(max_group_size < work_items)
    {
        (*operation_counts)[0] = work_items;
        (*work_item_counts)[0] = max_group_size;
    }

    s = groups;
    int level = 1;

    while(s > 1)
    {
        int work_items = (s < max_work_items * 2) ? s / 2 : max_work_items;
        int groups = s / (work_items * 2);
        groups = (max_groups < groups) ? max_groups : groups;

        (*group_counts)[level] = groups;
        (*work_item_counts)[level] = work_items;
        (*operation_counts)[level] = 1;
        (*entry_counts)[level] = s;
        if(max_group_size < work_items)
        {
            (*operation_counts)[level] = work_items;
            (*work_item_counts)[level] = max_group_size;
        }

        s = s / (work_items*2);
        level++;
    }
}

void CRoutine_Reduce::BuildKernels(int data_size,
    int * pass_count, size_t ** group_counts, size_t ** work_item_counts,
    int ** operation_counts, int ** entry_counts)
{
    // Init a few variables:
    int err = 0;
    int i;
    cl_program program;
    cl_kernel kernel;

#ifdef DEBUG
	printf("Loading and Compiling program " + this->kernel_source + "\n");
#endif //DEBUG

	source = this->ReadSource(this->kernel_source);

    size_t returned_size = 0;
    size_t max_workgroup_size = 0;
    err |= clGetDeviceInfo(mDeviceID, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_workgroup_size, &returned_size);
    CheckOCLError("Couldn't get maximum work group size from the device.", err);

    // Determine the reduction pass configuration for each level in the pyramid
    create_reduction_pass_counts(data_size, max_workgroup_size, MAX_GROUPS, MAX_WORK_ITEMS, pass_count, group_counts, work_item_counts, operation_counts, entry_counts);

    // TODO: We might need to set format flags here:
    stringstream tmp;

    for(i = 0; i < pass_count; i++)
    {
        // Insert macro definitions to specialize the kernel to a particular group size
        tmp.clear();
        tmp << "#define GROUP_SIZE " << group_counts[i] << "\n" << "#define OPERATIONS " << operation_counts << "\n\n";
        tmp << mSource[0];

        // Create the compute program from the source buffer
        program = clCreateProgramWithSource(mContext, 1, tmp.str().c_str(), NULL, &err);
        if (!program || err != CL_SUCCESS)
        {
            size_t len;
            string tmp_err;
            tmp_err.reserve(2048);
            printf("Error: Failed to create compute program!\n");
            printf("%s", tmp);
            clGetProgramBuildInfo(program, mDevice, CL_PROGRAM_BUILD_LOG, tmp_err.size(), &tmp_err, &len);
            printf("%s\n", tmp_err);
            // TODO: Throw an exception, cleanly exit.
        }

        // Build the program executable
        err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            size_t len;
            string tmp_err;
            tmp_err.reserve(2048);
            printf("Error: Failed to create compute program!\n");
            printf("%s", tmp);
            clGetProgramBuildInfo(program, mDevice, CL_PROGRAM_BUILD_LOG, tmp_err.size(), &tmp_err, &len);
            printf("%s\n", tmp_err);
            // TODO: Throw an exception, cleanly exit.
        }

        // Program built correctly, push it onto the vector
        mPrograms.push_back(program);

        // Create the compute kernel from within the program
        kernel = clCreateKernel(programs, "reduce", &err);
		CheckOCLError("Failed to create parallel sum kernel.", err);

		// All is well, push the kernel onto the vector
		mKernels.push_back(kernel);
    }

}

float ComputeSum(bool copy_back, cl_mem * input_buffer, cl_mem * output_buffer, cl_mem * partial_sum_buffer, cl_mem * final_buffer,
    cl_kernel * pKernels,
    int pass_count, size_t * group_counts, size_t * work_item_counts,
    int * operation_counts, int * entry_counts)
{
    if(gpu_enable_verbose || gpu_enable_debug)
        printf("Computing Parallel Sum. \n");

    if(input_buffer == NULL || output_buffer == NULL || partial_sum_buffer == NULL || final_buffer == NULL)
        print_opencl_error("Input to gpu_compute_sum is NULL!", 0);

    int i;
    int err;
    // Do the reduction for each level
    //
    cl_mem pass_swap;
    cl_mem pass_input = *output_buffer;
    cl_mem pass_output = *input_buffer;
    cl_mem partials_buffer = *partial_sum_buffer; // Partial sum buffer

    for(i = 0; i < pass_count; i++)
    {
        size_t global = group_counts[i] * work_item_counts[i];
        size_t local = work_item_counts[i];
        unsigned int operations = operation_counts[i];
        unsigned int entries = entry_counts[i];
        size_t shared_size = sizeof(float) * local * operations;

        if(gpu_enable_debug && gpu_enable_verbose)
        {
            printf("Pass[%4d] Global[%4d] Local[%4d] Groups[%4d] WorkItems[%4d] Operations[%d] Entries[%d]\n",  i,
                (int)global, (int)local, (int)group_counts[i], (int)work_item_counts[i], operations, entries);
        }

        // Swap the inputs and outputs for each pass
        //
        pass_swap = pass_input;
        pass_input = pass_output;
        pass_output = pass_swap;

        err = CL_SUCCESS;
        err |= clSetKernelArg(pKernels[i],  0, sizeof(cl_mem), &pass_output);
        err |= clSetKernelArg(pKernels[i],  1, sizeof(cl_mem), &pass_input);
        err |= clSetKernelArg(pKernels[i],  2, shared_size,    NULL);
        err |= clSetKernelArg(pKernels[i],  3, sizeof(int),    &entries);
        if (err != CL_SUCCESS)
            print_opencl_error("Failed to set partial sum kernel arguments.", err);

        // After the first pass, use the partial sums for the next input values
        //
        if(pass_input == *input_buffer)
            pass_input = partials_buffer;

        err = CL_SUCCESS;
        err |= clEnqueueNDRangeKernel(*pQueue, pKernels[i], 1, NULL, &global, &local, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            print_opencl_error("Failed to enqueue parallel sum kernels.", err);
    }

    // Let the queue complete.
    clFinish(*pQueue);

    // Copy the new chi2 value over to it's final place in GPU memory.
    err = clEnqueueCopyBuffer(*pQueue, pass_output, *final_buffer, 0, 0, sizeof(float), 0, NULL, NULL);
    if(err != CL_SUCCESS)
        print_opencl_error("Could not copy summed value to/from buffers on the GPU.", err);

    if(gpu_enable_debug && gpu_enable_verbose)
    {
        float sum = 0;
        err = clEnqueueReadBuffer(*pQueue, *final_buffer, CL_TRUE, 0, sizeof(float), &sum, 0, NULL, NULL );
        if(err != CL_SUCCESS)
            print_opencl_error("Could not read back GPU SUM value.", err);

        if(isnan(sum))
            print_opencl_error("Error: Calculation yielded NAN, aborting!", 0);

        printf("Sum: %f (copied value on GPU)\n", sum);
    }

    clFinish(*pQueue);
}
