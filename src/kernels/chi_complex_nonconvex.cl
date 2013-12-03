/*
 * chi_complex_mpmconvex.cl
 *
 *  Created on: Oct 24, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL kernel that computes the chi elements for complex quantities
 *      under the assumption that the probability distributions is non-convex.
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

// The following define should be created during the kernel compilation on the host.
// but we initialize it here just in case.
#ifndef TWO_PI
#define TWO_PI 6.283185307179586
#endif

float cabs(float2 A);
float carg(float2 A);

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
__kernel void chi_complex_nonconvex(
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
    tmp_data.s1 = data[n+index];

    float2 tmp_data_err;
    tmp_data_err.s0 = data_err[index];  // amplitude error
    tmp_data_err.s1 = data_err[n+index];// phase error

    float2 tmp_model;
    tmp_model.s0 = model[index];
    tmp_model.s1 =  model[n+index];
    
    float data_amp = cabs(tmp_data);
    float data_phi = carg(tmp_data);
    float model_amp = cabs(tmp_model);
    float model_phi = carg(tmp_model);

    // Store the result:
    if(i < n)
    {
        output[index] = (data_amp - model_amp) / tmp_data_err.s0;
        output[n+index] = fmod(data_phi - model_phi, (float)TWO_PI) / tmp_data_err.s1;;    
    }   
}
