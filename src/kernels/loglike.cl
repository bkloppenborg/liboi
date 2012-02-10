__kernel void loglike(
    __global float * data,
    __global float * data_err,
    __global float * mock_data,
    __global float * output,
    __private int n)
{
    int i = get_global_id(0);
    float temp = 0;
    
   	// The loglikelihood kernel executes on the entire output buffer
   	// because the reduce_sum_float kernel uses the entire buffer as input.
   	// Therefore we zero out the elements not directly involved in this computation.
    if(i < n)
	{
		temp = (data[i] - mock_data[i]) / data_err[i];
	    temp = -2 * native_log(data_err[i]) - temp * temp;
    }
    
    output[i] = temp;
}