/*
 * chi_bispectra_convex.cl
 *
 *  Created on: Oct 24, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL kernel that computes the chi elements for bispectra under
 *      the non-convex assumption.
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

// The following define is created during the kernel compilation on the host.
// #define TWO_PI 6.283185307179586

#typedef float2 cfloat;

/// The chi_bispectra_convex kernel computes the chi elements for the amplitude and
/// phase of the bispectra.  
__kernel void chi_bispectra_convex(
    __global float * data,
    __global float * data_err,
    __global float * model,
    __global float * output,
    __private int n)
{
    int i = get_global_id(0);
    float tmp = 0;
    
    // Lookup the phase and other data quantities
    cfloat tmp_data(data[i], data[2*i]);
    cfloat tmp_data_err(data_err[i], data_err[2*i]);
    cfloat tmp_model(model[i], model[2*i]);
    
    // Compute the residual, then chi elements
    cfloat error = tmp_data - tmp_model;
    error.s0 = error.s0 / tmp_data_err.s0;
    error.s1 = remainder(error.s1, TWO_PI) / tmp_data_err.s1;    

    // Store the result:
    if(i < n)
    {
        output[i] = error.s0;
        output[2*i] = error.s1;
    }  
}
