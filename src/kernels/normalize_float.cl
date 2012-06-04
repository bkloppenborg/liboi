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
    __global float * image,
    __global float * divisor,
    __private int2 image_size)
{
    int i = get_global_id(0);
    int j = get_global_id(1);
    int n = image_size.x * i + j;
    
    // TODO: We could possibly speed this up by having the first processor
    // compute 1/divisor and store that into local memory so we do a multiplication
    // instead of a division.
    
    image[n] = image[n] / divisor[0];
    
    if(image[n] < 0)
    	image[n] = 0;
}
