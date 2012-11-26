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
float2 MultComplex3(float2 A, float2 B, float2 C);

// Multiply two complex numbers
float2 MultComplex2(float2 A, float2 B)
{
    // There is the obvious way to do this:
    // (a + bi) * (c + di) = (ac - bd) + (bc + ad)i
    float2 temp;
    // ac - bd
    temp.s0 = A.s0*B.s0 - A.s1*B.s1;
    // bc + ad
    temp.s1 = A.s1*B.s0 + A.s0*B.s1;

    return temp;
}

float2 MultComplex3(float2 A, float2 B, float2 C)
{
    A = MultComplex2(A, B);
    return MultComplex2(A, C);
}

// The actual kernel function.
__kernel void ft_to_t3(
    __global float2 * ft_input,
    __global uint4 * uv_ref,
    __global short4 * uv_sign,
    __private unsigned int offset,
    __private unsigned int n_t3,
    __global float * output)
{   
    int i = get_global_id(0);
    
    // Pull some data from global memory:
    uint4 uvpnt = uv_ref[i];
    float2 vab = ft_input[uvpnt.s0];
    float2 vbc = ft_input[uvpnt.s1];
    float2 vca = ft_input[uvpnt.s2];
    
    short4 sign = uv_sign[i];
    vab.s1 *= sign.s0;
    vbc.s1 *= sign.s1;
    vca.s1 *= sign.s2;
    
    float2 temp = MultComplex3(vab, vbc, vca);
    
    if(i < n_t3)
    {
        output[offset + i] = temp.s0;
        output[offset + n_t3 + i] = temp.s1;
    }
}
