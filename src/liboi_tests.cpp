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

int main(int argc, char **argv)
{
	// Find the path to the current executable
	string exe = FindExecutable();
	size_t folder_end = exe.find_last_of("/\\");
	LIBOI_KERNEL_PATH = exe.substr(0,folder_end+1) + "kernels/";


	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
