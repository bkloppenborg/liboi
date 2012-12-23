/*
 * image2buf_GL_R.cl
 *
 *  Created on: Nov 8, 2011
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL Kernel that copies the red channel from a GL_R8 OpenGL
 *      image buffer into a cl_mem floating point buffer.  Here the data
 *      is expected to be in 
 *          colors = (r, 0.0, 0.0, 1.0)
 *      format.
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

// A kernel to copy a GL_R8 OpenGL image into a floating point cl_mem buffer
// Here we expect the data to be in the format:
//  colors = (r, 0.0, 0.0, 1.0)

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void image2buf_GL_R(__read_only image2d_t image,
    __global float * buffer,
    __private int image_width)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    int2 coords;
    coords.x = x;
    coords.y = y;
    
    float4 colors = read_imagef(image, sampler, coords);
    buffer[y * image_width + x] = colors.s0;
}
