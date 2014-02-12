/*
 * normalize_float.cl
 *
 *  Created on: Dec 1, 2011
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL Kernel for normalzing a 2D array of floating point
 *      numbers.  The kernel dividies entry (i,j) by divisor (the
 *      total of all of the array elements stored in OpenCL memory).
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

__kernel void normalize_float(
    __global float * buffer,
    __private unsigned int buffer_size,
    __private float one_over_sum)
{
    size_t i = get_global_id(0);
    
    // TODO: We could possibly speed this up by having the first processor
    // compute 1/divisor and store that into local memory so we do a multiplication
    // instead of a division.
    
    if(i < buffer_size)
    {    
        buffer[i] = buffer[i] * one_over_sum;
    
        // Force the buffer to be positive definite. All infinities and NaNs
        // are forced to zero.
        if(buffer[i] < 0 || !isfinite(buffer[i]) || isnan(buffer[i]))
        	buffer[i] = 0;
	}
}
