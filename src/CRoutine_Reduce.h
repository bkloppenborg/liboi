/*
 * CRoutine_Reduce.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_REDUCE_H_
#define CROUTINE_REDUCE_H_

// Define a few constants for use in the OpenCL Kernels
#define MAX_GROUPS      (64)
#define MAX_WORK_ITEMS  (64)

#include "COpenCLRoutine.h"

class CRoutine_Reduce: public COpenCLRoutine
{
protected:
	int num_elements;
	int pass_count;

	vector<size_t> group_counts;
	vector<size_t> work_item_counts;
	vector<int> operation_counts;
	vector<int> entry_counts;

	cl_mem tmp_buff1;
	cl_mem tmp_buff2;

public:
	CRoutine_Reduce(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_Reduce();

protected:
	void AllocateInternalBuffers();
	void CreateReductionPasscounts(int max_group_size, int max_groups, int max_work_items);
	void BuildKernels();
public:

	float ComputeSum(bool copy_back, cl_mem final_buffer, cl_mem input_buffer, cl_mem output_buffer, cl_mem partial_sum_buffer);
	void Init(int num_elements, bool allocate_temp_buffers);


};

#endif /* CROUTINE_REDUCE_H_ */
