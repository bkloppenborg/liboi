/*
 * chi_complex_convex.cl
 *
 *  Created on: Oct 24, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL kernel that computes the chi elements for complex quantities
 *      under the assumption that the probability distribution is convex
 *      in Cartesian coordinates.
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
float cabs(float2 A);
float carg(float2 A);

// Multiply two complex numbers
float2 MultComplex2(float2 A, float2 B)
{
    // There is the obvious way to do this (which turns out to be faster)
    // (a + bi) * (c + di) = (ac - bd) + (bc + ad)i
    float2 temp;
    // ac - bd
    temp.s0 = A.s0*B.s0 - A.s1*B.s1;
    // bc + ad
    temp.s1 = A.s1*B.s0 + A.s0*B.s1;

    return temp;
}

float cabs(float2 A)
{
    return sqrt(A.s0 * A.s0 + A.s1 * A.s1);
}

float carg(float2 A)
{
    return atan2(A.s1, A.s0);
}

/// The chi_bispectra_convex kernel computes the chi elements for the amplitude and
/// phase of the bispectra.  
__kernel void chi_complex_convex(
    __global float * data,
    __global float * data_err,
    __global float * model,
    __global float * output,
    __private unsigned int start,
    __private unsigned int n)
{
    int i = get_global_id(0);
    unsigned int index = start + i;
    
    // Lookup the phase and other data quantities
    float2 tmp_data;
    tmp_data.s0 = data[index];
    tmp_data.s1 = data[n + index];
    
    float2 tmp_data_err;
    tmp_data_err.s0 = data_err[index];
    tmp_data_err.s1 = tmp_data.s0 * data_err[n + index];

    float2 tmp_model;
    tmp_model.s0 = model[index];
    tmp_model.s1 = model[n + index];
    
    float2 phasor;
    phasor.s0 = cos(tmp_data.s1);
    phasor.s1 = -sin(tmp_data.s1);
    
    // Rotate the phase and data
    tmp_data = MultComplex2(tmp_data, phasor);
    tmp_model = MultComplex2(tmp_model, phasor);
    
    // Compute the chi, store the result:
    if(i < n)
    {
        output[index] = (cabs(tmp_data) - cabs(tmp_model)) / tmp_data_err.s0;
        output[n + index] = (carg(tmp_data) - carg(tmp_model)) / (cabs(tmp_data) * tmp_data_err.s1);
    }  
}
