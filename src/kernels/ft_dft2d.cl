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




// Function prototrowpes:
float2 ft_pixel(float flux, float arg_x, float arg_row);

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

__kernel void dft_2d(
	__global float2 * uv_points,
	__private unsigned int nuv,
	__global float * image,
	__private unsigned image_width,
	__private unsigned image_height,
	__global float2 * output
)
{     
    size_t tid = get_global_id(0);
    size_t lid = get_local_id(0);

    float col_center = ((float) image_width) / 2.0;
    float row_center = ((float) image_height) / 2.0;

    float row_temp = 0;
    float col_temp = 0;
    float2 dft_output = (float2) (0.0f, 0.0f);

    float arg_u =  ARG * uv_points[tid].s0; // note, positive due to U definition in interferometrrow.
    float arg_v = -ARG * uv_points[tid].s1;

    for(unsigned int row = 0; row < image_height; row++)
    {
        row_temp = arg_v * (row - row_center);

        for(unsigned int col = 0; col < image_width; col++)
        {
            col_temp = arg_u * (col - col_center);

            dft_output += ft_pixel(image[col + image_width * row], row_temp, col_temp);
        }
    }

    // assign the output    
    output[tid] = dft_output;
}

