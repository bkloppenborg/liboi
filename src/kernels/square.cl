__kernel void square(
    __global float * input,
    __global float * output,
    __private int n)
{
    int i = get_global_id(0);
    float temp = 0;
    
    if(i < n)
    	temp = input[i];
    	
    output[i] = temp * temp;
}