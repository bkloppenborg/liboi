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
 *      To use this kernel you must inline the following variable:
 *      ARG using a #define statement:
 *          float ARG = 2.0 * PI * RPMAS * image_scale
 *      where PI = 3.14159265358979323, RPMAS = (PI/180.0)/3600000.0
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
 * as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */




// Function prototypes:
float2 MultComplex3Special(float A, float B, float C);

// Multiply together three complex numbers.
// NOTE this function is optimized for A.imag = 0, mag(B) = mag(C) = 1
// And is done in polar coordinates
float2 MultComplex3Special(float magA, float argB, float argC)
{
    float2 temp;
    temp.s0 = magA * native_cos(argB + argC);
    temp.s1 = magA * native_sin(argB + argC);
    
    return temp;
}

__kernel void dft_2d(
	__global float2 * uv_points,
	__private int nuv,
	__global float * image,
	__private int image_width,
	__global float2 * output,
	__local float * sA,
	__local float * sB,
	__global float * flux
)
{
    float2 tmp;
    tmp.s0 = 0;
    tmp.s1 = 0;
    
    float arg_C;
      
    int tid = get_global_id(0);
    int lsize_x;
    int lid = get_local_id(0);
    int i = 0;
    int j = 0;
    int m = 0;    

    // Load up the UV information
    float2 uv = uv_points[tid];
    
    // Iterate over every pixel (i,j) in the image, calculating their contributions to the given UV point.
 
    for(j=0; j < image_width; j++)
    {       
        arg_C = -1 * ARG * uv.s1 * (float) j;
        
        // Reload the lsize_x, just in case it was modified by a previous loop.
        lsize_x = get_local_size(0);
        
        // Now iterate over the x-direction in the image.
        // We are using shared memory and therefore move in blocks of lsize_x
        for(i=0; i < image_width; i+= lsize_x)
        {
            if((i + lsize_x) > image_width)
                lsize_x = image_width - i;
                
            if((i + lid) < image_width)
            {
                sA[lid] = image[image_width * j + (i + lid)];
                // Save a little computation time by removing one multiplication and addition from each iteration.
                sB[lid] = ARG * (float) (i + lid);
            }
            barrier(CLK_LOCAL_MEM_FENCE);
            
            
            for(m = 0; m < lsize_x; m++)
            {
                tmp += MultComplex3Special(sA[m], sB[m] * uv.s0, arg_C);
            }
            barrier(CLK_LOCAL_MEM_FENCE);
        }
    }
        
    // Write the result to the output array
    output[tid] = tmp;
}

