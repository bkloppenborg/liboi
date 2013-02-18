/*
 * CRoutine_LogLike_test.cpp
 *
 *  Created on: Dec 6, 2012
 *      Author: bkloppen
 */

#include "gtest/gtest.h"
#include "liboi_tests.h"
#include "COpenCL.hpp"
#include "CRoutine_ImageToBuffer.h"

using namespace std;
using namespace liboi;

extern string LIBOI_KERNEL_PATH;

/// TODO: Without an active OpenGL context, we can't test much here. Initialing
/// OpenGL in a cross-platform fashion is difficult, so we won't throughly
/// check this routine. Instead, just make sure it builds. In the future we
/// should initialize an OpenGL context and verify that the copy routine works
/// correctly.

/// Checks that the ImageToBuffer kernel builds correctly
TEST(ImageToBuffer, KernelBuilds)
{
	COpenCL cl(CL_DEVICE_TYPE_GPU);
	CRoutine_ImageToBuffer r(cl.GetDevice(), cl.GetContext(), cl.GetQueue());
	r.SetSourcePath(LIBOI_KERNEL_PATH);
	r.Init();

	ASSERT_TRUE(true);
}



