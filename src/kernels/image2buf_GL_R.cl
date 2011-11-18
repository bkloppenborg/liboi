// A kernel to copy a GL_R8 OpenGL image into a floating point cl_mem buffer
// Here we expect the data to be in the format:
//  colors = (r, 0.0, 0.0, 1.0)

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void normalize(__read_only  image2d_t image,
    __global float * buffer
    __global int image_width)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    float4 colors = readimagef(image, sampler, float2 {x, y});
    buffer[y * image_width + x] = colors[0];
}