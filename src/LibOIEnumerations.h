/*
 * enumerations.h
 *
 *  Created on: Nov 17, 2011
 *      Author: bkloppenborg
 */

/* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * The authors request, but do not require, that you acknowledge the
 * use of this software in any publications.  See 
 * https://github.com/bkloppenborg/liboi/wiki
 * for example citations
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

#ifndef LIBOI_ENUMERATIONS_H_
#define LIBOI_ENUMERATIONS_H_

namespace LibOIEnums
{
	enum FTMethods
	{
		DFT = 0,	// Discrete Fourier Transform O(N^2)
		NFFT = 1	// Nonuniform Fast Fourier Transform O(N log(N))
	};

	enum ImageTypes
	{
		OpenCLBuffer = 0,
		OpenGLBuffer = 1
	};

	enum OpMode
	{
		OPENCL_ONLY,	// Runs all calculations on OpenCL devices (default)
		VERIFY			// Runs all calculations on OpenCL devices, then checks them using CPU calculations.
	};


}

#endif /* LIBOI_ENUMERATIONS_H_ */
