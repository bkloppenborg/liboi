/*
 * chi_bispectra_convex.cl
 *
 *  Created on: Oct 24, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL kernel that computes the chi elements for bispectra under
 *      the convex assumption.
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

#typedef float2 cfloat;

// Function prototypes:
float2 MultComplex2(cfloat A, cfloat B);

// Multiply two complex numbers
float2 MultComplex2(cfloat A, cfloat B)
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
    
    // Lookup the phase and other data quantities
    cfloat tmp_data(data[i], data[2*i]);
    cfloat tmp_data_err(data_err[i], tmp_data.s0 * data_err[2*i]);
    cfloat tmp_model(model[i], model[2*i]);
    
    cfloat phasor(cos(tmp_data.s1), -sin(tmp_data.s1));
    
    // Rotate the phase and data
    tmp_data = MultComplex2(tmp_data, phasor);
    tmp_model = MultComplex2(tmp_model, phasor);
    
    // Compute the chi, store the result:
    float2 tmp = (tmp_data - tmp_model) / tmp_data_err;
    if(i < n)
    {
        output[i] = tmp.s0;
        output[2*i] = tmp.s1;
    }  
}
