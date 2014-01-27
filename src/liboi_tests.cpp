/*
 * liboi_tests.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */

#include "liboi_tests.h"
#include "PathFind.hpp"
#include "gtest/gtest.h"
#include "liboi.hpp"

using namespace liboi;

string LIBOI_KERNEL_PATH;
cl_device_type OPENCL_DEVICE_TYPE;

int main(int argc, char **argv)
{
	// Find the path to the current executable
	string exe = FindExecutable();
	size_t folder_end = exe.find_last_of("/\\");
	LIBOI_KERNEL_PATH = exe.substr(0,folder_end+1) + "kernels/";

	OPENCL_DEVICE_TYPE = CL_DEVICE_TYPE_GPU;

	for(int i = 0; i < argc; i++)
	{
		if (string(argv[i]) == "-h")
		{
			PrintHelp();
			return 0;
		}

		if (string(argv[i]) == "-cpu")
		{
			OPENCL_DEVICE_TYPE = CL_DEVICE_TYPE_CPU;
		}
	}

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

void PrintHelp()
{

	cout << "Running liboi_benchmark:" << endl;
	cout << " liboi_tests [...]" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << " -cpu       Runs tests using a CPU OpenCL device [default: use GPU]" << endl;

}
