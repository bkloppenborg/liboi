__kernel void zero_buffer(
    __global float * buffer,
    __private int n)
{
    int i = get_global_id(0);
    
    if(i < n)
		buffer[i] = 0;
}