/*
 * CRoutine_FT.cpp
 *
 *  Created on: Dec 2, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Abstract base class implementation for all Fourier transform routines
 *      used in LIBOI.
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

#include "CRoutine_FT.h"

namespace liboi
{

double CRoutine_FT::RPMAS = (M_PI / 180.0) / 3600000.0; // Number of radians per milliarcsecond

CRoutine_FT::CRoutine_FT(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine(device, context, queue)
{
	// TODO Auto-generated constructor stub

}

CRoutine_FT::~CRoutine_FT()
{
	// TODO Auto-generated destructor stub
}

} /* namespace liboi */
