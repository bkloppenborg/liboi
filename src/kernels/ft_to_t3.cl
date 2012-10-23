/*
 * ft_to_t3.cl
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL Kernel for computing triple products
 *      from Fourier transform output
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
float2 MultComplex2(float2 A, float2 B);
float2 MultComplex3(float2 A, float2 B);

// Multiply two complex numbers
float2 MultComplex2(float2 A, float2 B)
{
    // There is the obvious way to do this:
/*    float2 temp;*/
/*    temp.s0 = A.s0*B.s0 - A.s1*B.s1;*/
/*    temp.s1 = A.s0*B.s1 + A.s1*B.s0;  */
/*    */
/*    return temp;*/
    
    // We can trade off one multiplication for three additional additions
    float k1 = A.s0 * B.s0;
    float k2 = A.s1 * B.s1;
    float k3 = (A.s0 + A.s1) * (B.s0 + B.s1);
    
    float2 temp;
    temp.s0 = k1 - k2;
    temp.s1 = k3 - k1 - k2;
    return temp;
}

float2 MultComplex3(float2 A, float2 B, float2 C)
{
    A = MultComplex2(A, B);
    return MultComplex2(A, C);
}

// The actual kernel function.
__kernel void ft_to_t3(
    __global float2 * FT_output,
    __global long4 * data_bsref,
    __global short4 * data_sign,
    __private int n_v2,
    __private int n_t3,
    __global float * T3_output)
{   
    int i = get_global_id(0);
    
    // Pull some data from global memory:
    long4 uvpnt = data_bsref[i];
    float2 vab = FT_output[uvpnt.s0];
    float2 vbc = FT_output[uvpnt.s1];
    float2 vca = FT_output[uvpnt.s2];
    
    short4 sign = data_sign[i];
    vab.s1 *= sign.s0;
    vbc.s1 *= sign.s1;
    vca.s1 *= sign.s2;
    
    float2 temp = MultComplex3(vab, vbc, vca);
    
    if(i < n_t3)
    {
        output[n_v2 + i] = temp.s0;
        output[n_v2 + n_t3 + i = temp.s1;
    }
}
