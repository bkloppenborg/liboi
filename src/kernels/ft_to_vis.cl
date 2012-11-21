/*
 * ft_to_vis.cl
 *
 *  Created on: Nov. 21, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL Kernel for computing visibilities from the Fourier
 *      transform output.
 *      As there is nothing to compute, this function simply moves
 *      values to conform to the data format specification in COILibData.h
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

/// Converts the fourier transform input (complex numbers) in ft_input into
/// squared visibilities.
__kernel void ft_to_vis(
    __global float2 * ft_input,
    __global unsigned int * uv_ref,
    __private unsigned int offset,
    __private unsigned int n_vis,
    __global float * output)
{
    int i = get_global_id(0);
    // Lookup the index of the UV point which corresponds to this data point.
    unsigned int uv_index = uv_ref[i];
    // Get the Complex values.
    float2 temp = ft_input[uv_index];
    
    // Simply rearrange the data to be in [real_0, ..., real_n, imag_0, ..., imag_n] format.
    if(i < n_vis)
    {
        output[offset + i] = temp.s0;
        output[offset + n_vis + i] = temp.s1;
    }
}
