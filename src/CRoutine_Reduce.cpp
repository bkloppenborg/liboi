/*
 * CRoutine_Reduce.cpp
 *
 *  Created on: Jan 27, 2012
 *      Author: bkloppenborg
 */

#include "CRoutine_Reduce.h"

CRoutine_Reduce::CRoutine_Reduce(cl_device_id device, cl_context context, cl_command_queue queue)
	: COpenCLRoutine(device, context, queue)
{
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
        tmp.str("");
        tmp << "#define GROUP_SIZE " << group_counts[i] << "\n" << "#define OPERATIONS " << operation_counts[i] << "\n\n";
        tmp << source;

        BuildKernel(tmp.str(), "reduce");
    }

}
