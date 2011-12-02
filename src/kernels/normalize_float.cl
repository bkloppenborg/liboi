// A kernel to normalize a 2D array of floating point values
// the kernel dividies the entry (i,j) by divisor, the total of all of the arrays stored in OpenCL memory.
__kernel void normalize_float(
    __global float * image,
    __global float * divisor,
    __private int2 image_size)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    int n = image_size.x * i + j;
    
    // TODO: We could possibly speed this up by having the first processor
    // compute 1/divisor and store that into local memory so we do a multiplication
    // instead of a division.
    
    image[n] = image[n] / divisor[0];
}