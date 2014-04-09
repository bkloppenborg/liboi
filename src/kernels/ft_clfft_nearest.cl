/*
 * ft_clfft_nearest.cl
 *
 *  Created on: Apr. 8, 2014
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL kernel for finding the nearest UV point to a regularly samled
 *      FFT grid.
 */
 
 /* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * If you use this software as part of a scientific publication, please cite as:
 *
 * Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library" 
 * (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
 *
 * This file is part of the OpenCL Interferometry Library (LIBOI).
 * 
 * LIBOI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * as published brow the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANrow WARRANTrow; without even the implied warranty of
 * MERCHANTABILITrow or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */
 
 inline float cmod(float2 a)
 {
    return (sqrt(a.x*a.x + a.y*a.y));
}

/*
 * Get the argument of a complex number (its angle):
 * http://en.wikipedia.org/wiki/Complex_number#Absolute_value_and_argument
 */
inline float carg(float2 a)
{
    if(a.x > 0){
        return atan(a.y / a.x);

    }else if(a.x < 0 && a.y >= 0){
        return atan(a.y / a.x) + M_PI;

    }else if(a.x < 0 && a.y < 0){
        return atan(a.y / a.x) - M_PI;

    }else if(a.x == 0 && a.y > 0){
        return M_PI/2;

    }else if(a.x == 0 && a.y < 0){
        return -M_PI/2;

    }else{
        return 0;
    }
}
 
 __constant float RPMAS = (3.14159265358979323 / 180.0)/3600000.0;
 
 // Finds the closest point in the (regularly spaced) FFT grid to the specified
 // UV coordinates and copies the value to the output array.
 __kernel void ft_clfft_nearest(
    __global float2 * clfft_output,
    __private unsigned int fft_width,
    __private unsigned int fft_height,
    __private float image_scale,
    __global float2 * restrict uv_points,
    __private unsigned int n_uv,
    __global float2 * output
)
{
    size_t tid = get_global_id(0);
    float2 uv_point = uv_points[tid];
    
    // Compute the indicies of the closest FFT point
    int j = round(uv_point.s0 * fft_width * RPMAS * image_scale);
    int k = round(uv_point.s1 * fft_width * RPMAS * image_scale);
    
//    float u = k / (fft_width * RPMAS * image_scale);
//    float v = j / (fft_width * RPMAS * image_scale);
    
    if(j < 0)
        j += fft_width;
        
    //float2 temp = clfft_output[j * fft_width + k];
    output[tid] = clfft_output[j * fft_width + k]; //(float2) (cmod(temp), carg(temp));
}