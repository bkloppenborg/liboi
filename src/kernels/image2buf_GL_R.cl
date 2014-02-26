//#pragma OPENCL EXTENSION cl_amd_printf : enable
__kernel void image2buf_GL_R(__read_only image2d_t image, __global float * output)
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

		int2 coords = (int2){get_global_id(0), get_global_id(1)};

    int2 image_dim = get_image_dim(image);
		
//		if(coords.x == 0 && coords.y == 0)
//			printf("%0x \n", image);

    float4 colors = read_imagef(image, sampler, coords);
    output[coords.s1 * image_dim.s0 + coords.s0] = colors.s0;
}
