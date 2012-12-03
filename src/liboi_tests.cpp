/*
 * liboi_tests.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */

#include "liboi_tests.h"
#include "PathFinding.h"
#include "gtest/gtest.h"
#include "liboi.hpp"

using namespace liboi;

string LIBOI_KERNEL_PATH;

int main(int argc, char **argv)
{
	// Find the path to the current executable
	string exe = FindExecutable();
	// Find the directory (the name of this program is "liboi_tests", so just strip off five characters)
	LIBOI_KERNEL_PATH = exe.substr(0, exe.size() - 11) + "kernels/";

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
