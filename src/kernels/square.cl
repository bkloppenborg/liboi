__kernel void square(
    __global float * input,
    __global float * output
)
{
    int i = get_global_id(0);
    float temp = input[i];   
    output[i] = temp * temp;
}