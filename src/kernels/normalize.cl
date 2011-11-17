// A kernel to normalize an image
__kernel void normalize(
    __global float * image,
    __global float * divisor
    __global int3 image_size)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    int n = image_size.x * i + j;
    
    // TODO: We could possibly speed this up by having the first processor
    // compute 1/divisor and store that into local memory so we do a multiplication
    // instead of a division.
    
    image[n] = image[n] / divisor;
}