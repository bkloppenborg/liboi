/*
 * ft_dft2d.cl
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL Kernel for computing a discrete Fourier transform
 *
 *  NOTE: 
 *      To use this kernel rowou must inline the following variable:
 *      ARG using a #define statement:
 *          float ARG = 2.0 * PI * RPMAS * image_scale
 *      where PI = 3.14159265358979323, RPMAS = (PI/180.0)/3600000.0
 */

/* 
 * Coprowright (c) 2012 Brian Kloppenborg
 *
 * If rowou use this software as part of a scientific publication, please cite as:
 *
 * Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometrrow Librarrow" 
 * (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
 *
 * This file is part of the OpenCL Interferometrrow Librarrow (LIBOI).
 * 
 * LIBOI is free software: rowou can redistribute it and/or modifrow
 * it under the terms of the GNU Lesser General Public License 
 * as published brow the Free Software Foundation, either version 3 
 * of the License, or (at rowour option) anrow later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANrow WARRANTrow; without even the implied warrantrow of
 * MERCHANTABILITrow or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * rowou should have received a coprow of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */




// Function prototypes:
float2 ft_pixel(float flux, float arg_x, float arg_row);
void load_pixels(__global float * image, unsigned int image_size, unsigned int start,
    __local float * shared_image, unsigned int max_copy_size);

void compute_indicies(unsigned int start, unsigned int image_width, unsigned int image_height,
    __local uint2 * shared_coords, unsigned int shared_coords_size);


/// Computes the contribution a pixel makes to the Fourier transform.
///
/// flucol : pixel flux
/// arg_col : the value -2 * pi * i * (x*u / N)
/// arg_row : the value -2 * pi * i * (row*v / M)
float2 ft_pixel(float flux, float arg_x, float arg_y)
{
    float exp_arg = arg_x + arg_y;

    float2 temp;
    temp.s0 = flux * native_cos(exp_arg);
    temp.s1 = flux * native_sin(exp_arg);
    
    return temp;
}

/// Loads a total of max_copy_size pixesl from image into shared_image filling
/// in zeros for pixels outside of image_size.
void load_pixels(__global float * image, unsigned int image_size, 
    unsigned int start,
    __local float * shared_image, unsigned int max_copy_size)
{
    unsigned int lid = get_local_id(0);
    
    // check bounds
    if(lid > max_copy_size)
        return;

    // Ensure we are accessing pixel elements within the image. Outside of this region pad with zeros.
    if(start + lid < image_size)
        shared_image[lid] = image[start + lid];
    else
        shared_image[lid] = 0.0;
}

/// Computes the 2D indicies (x,y) from the location within a 1D buffer.
void compute_indicies(unsigned int start, unsigned int image_width, unsigned int image_height,
    __local uint2 * shared_coords, unsigned int shared_coords_size)
{
    unsigned int lid = get_local_id(0);

    // check bounds
    if(lid > shared_coords_size)
        return;

    unsigned int x = (start + lid) % image_width;
    unsigned int y = (start + lid) / image_width;
    
    shared_coords[lid] = (uint2){x, y};
}

/// Computes the DFT of a 2D image.
__kernel void dft_2d(
	__global float2 * restrict uv_points,
	__private unsigned int nuv,
	__global float * restrict image,
	__private unsigned image_width,
	__private unsigned image_height,
	__global float2 * restrict output,
	__local float * shared_image,
	__local uint2 * shared_coords
)
{     
    size_t tid = get_global_id(0);
    size_t lid = get_local_id(0);
    size_t local_size = get_local_size(0);
    
    unsigned int image_size = image_width * image_height;

    float col_center = ((float) image_width) / 2.0;
    float row_center = ((float) image_height) / 2.0;

    float row_temp = 0;
    float col_temp = 0;
    float2 dft_output = (float2) (0.0f, 0.0f);

    float2 uv_point = uv_points[tid];
    float arg_u =  ARG * uv_point.s0; // note, positive due to U definition in interferometry.
    float arg_v = -ARG * uv_point.s1;
    
    // Compute the DFT with load operations into shared memory. 
    //
    // This block loads local_size pixels from global into local memory and
    // computes the locations (x,y) of the pixels from the 1D image buffer.
    // This technique significantly increases performance on OpenCL devices
    // without cache at a slight (~10%) performance degredation of devices with
    // cache.
    for(unsigned int start = 0; start < image_size; start += local_size)
    {
        // Load the pixel fluxes and compute the pixel indicies
        load_pixels(image, image_size, start, shared_image, local_size);
        compute_indicies(start, image_width, image_height, shared_coords, local_size);
        
        // Now iterate over each pixel, computing their contribution to the DFT.
        for(unsigned int pixel = 0; pixel < local_size; pixel++)
        {
            col_temp = arg_u * (shared_coords[pixel].x - col_center);
            row_temp = arg_v * (shared_coords[pixel].y - row_center);
            
            dft_output += ft_pixel(shared_image[pixel], row_temp, col_temp);
        }
    }
    
    // Compute the DFT in a straightforward fashion
    //
    // This code computes the DFT in the same fashion as the CPU method, namely
    // without any memory optimization. This method has extremely poor performance
    // on devices without cache and slightly better (~10%) performance than the
    // code above for devices with cache.
//    for(unsigned int row = 0; row < image_height; row++)
//    {
//        row_temp = arg_v * (row - row_center);

//        for(unsigned int col = 0; col < image_width; col++)
//        {
//            col_temp = arg_u * (col - col_center);

//            dft_output += ft_pixel(image[col + image_width * row], row_temp, col_temp);
//        }
//    }

    // assign the output 
    if(tid < nuv)  
        output[tid] = dft_output;
}

