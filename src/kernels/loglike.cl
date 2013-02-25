/*
 * loglike.cl
 *
 *  Created on: Jan 30, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL Kernel for computing the elements of log(likelihood)
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
 
__kernel void loglike(
    __global float * chi_buffer,
    __global float * data_err,
    __global float * output,
    __private unsigned int n)
{
    int i = get_global_id(0);
    
    // Computes the log of the likelihood
    if(i < n)
	{
	    output[i] = -1*native_log(data_err[i]) - chi_buffer[i] * chi_buffer[i] / 2;
    }
}
