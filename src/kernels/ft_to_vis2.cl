// Kernel that converts output from Fourier Transforms into Visibility Squares

__kernel void ft_to_vis2(
    __global float2 * ft_input,
    __global float * vis2)
{
    int i = get_global_id(0);
    float2 temp = ft_input[i];
    
    temp.s0 = temp.s0 * temp.s0 + temp.s1 * temp.s1;
    temp.s1 = 0;
    vis2[i] = temp.s0;
}