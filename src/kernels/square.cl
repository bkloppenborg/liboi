/*
 * square.cl
 *
 *  Created on: Feb 1, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL Kernel to compute the square of elements in an
 *      OpenCL input buffer.  Stores the output in the output buffer.
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
 
__kernel void square(
    __global float * input,
    __global float * output,
    __private int n)
{
    size_t i = get_global_id(0);
    float temp = 0;
    
    if(i < n)
    	temp = input[i];
    	
    output[i] = temp * temp;
}
