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
#include "liboi.hpp"
#include "PathFinding.h"

#include <iostream>
#include <algorithm>
using namespace std;

// The main routine.
int main(int argc, char *argv[])
{
	// Find the path to the current executable
	string exe = FindExecutable();
	// Find the directory (the name of this program is "liboi_tests", so just strip off 11 characters)
	string path = exe.substr(0, exe.size()-11);

	RunTests(path);

    return 0;
}


void PrintHelp()
{
	printf("This verification program has no options.  Simply run the executable.");
	exit(0);
}

void RunTests(string path)
{

    string kernel_source_dir = path + "kernels";

    // Create a point-source image, copy it to the GPU:
    unsigned int width = 128;
    unsigned int height = 128;
    unsigned int depth = 1;
    unsigned int size = width * height * depth;
    float scale = 0.01;

    // Initialize the image.
    float * image = new float[size];
    int pixel = 0;
    double radius = 2; // mas
    for(int i = 0; i < size; i++)
    	image[i] = 0;

    // Put a single flux element in the center, use a brightness of two.
    image[4096] = 2;

    // Create an OpenCL object from a GPU.  Initialize it.
    CLibOI LibOI(CL_DEVICE_TYPE_GPU);
    LibOI.SetKernelSourcePath(kernel_source_dir);
    LibOI.SetImageInfo(width, height, depth, scale);
    LibOI.SetImageSource(image);

    // Load sample data:
    LibOI.LoadData(path + "../samples/UDD-2_5mas_nonoise.oifits");

    // Init memory and routines.
    LibOI.Init();

    // Load the image
    LibOI.CopyImageToBuffer(0);

    // Now run tests.
    LibOI.RunVerification(0);

    delete[] image;
}

#endif /* MAIN_CPP_ */
