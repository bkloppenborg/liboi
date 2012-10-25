/*
 * main.cpp
 *
 *  Created on: Nov 7, 2011
 *      Author: bkloppenborg
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

#ifndef MAIN_CPP_
#define MAIN_CPP_

#include "main.h"
#include "CLibOI.h"

#include <string>

using namespace std;

// The main routine.
int main(int argc, char *argv[])
{
    // Create an OpenCL object from a GPU.  Initialize it.
    string kernel_source_dir = "./kernels/";
    CLibOI oCL(CL_DEVICE_TYPE_GPU);
    //oCL.SetImage_CLMEM(cl_image);

    return 0;
}


void PrintHelp()
{
	printf("This verification program has no options.  Simply run the executable.");
	exit(0);
}

#endif /* MAIN_CPP_ */
