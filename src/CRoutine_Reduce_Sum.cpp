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
#ifdef DEBUG_VERBOSE
	printf("Computing Parallel Sum. \n");
#endif // DEBUG
	return Compute(copy_back, final_buffer, input_buffer, output_buffer, partial_sum_buffer);
}

float CRoutine_Reduce_Sum::ComputeSum_CPU(cl_mem input_buffer)
{
	int err = CL_SUCCESS;
	cl_float tmp[num_elements];
	err |= clEnqueueReadBuffer(mQueue, input_buffer, CL_TRUE, 0, num_elements * sizeof(cl_float), tmp, 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy buffer back to CPU, CRoutine_Reduce_Sum::Compute_CPU() ", err);


	float sum = 0;
	for(int i = 0; i < num_elements; i++)
	{
		sum += float(tmp[i]);
	}

	return sum;
}

/// Tests that the CPU and OpenCL versions of ComputeSum return the same value.
bool CRoutine_Reduce_Sum::ComputeSum_Test(bool copy_back, cl_mem final_buffer, cl_mem input_buffer, cl_mem output_buffer, cl_mem partial_sum_buffer)
{
	// First run the CPU version as the CL version modifies the buffers.
	float cpu_sum = ComputeSum_CPU(input_buffer);
	float cl_sum = ComputeSum(true, final_buffer, input_buffer, output_buffer, partial_sum_buffer);

	bool sum_pass = bool(fabs(cpu_sum - cl_sum)/cpu_sum < MAX_REL_ERROR);
	printf("  CPU Value:  %0.4f\n", cpu_sum);
	printf("  CL  Value:  %0.4f\n", cl_sum);
	printf("  Difference: %0.4f\n", cpu_sum - cl_sum);
	PassFail(sum_pass);
}

/// Initializes the parallel sum object to sum num_element entries from a cl_mem buffer.
/// allocate_temp_buffers: if true will automatically allocate/deallocate buffers. Otherwise you need to do this elsewhere
void CRoutine_Reduce_Sum::Init(int n)
{
	// Set the number of elements on which this kernel will operate.
	this->num_elements = n;
	BuildKernels();
}
