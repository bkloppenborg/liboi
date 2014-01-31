/*
 * CRoutine_FFT_clFFT.cpp
 *
 *  Created on: Jan 30, 2014
 *      Author: bkloppenborg
 */

#include "CRoutine_FFT_clFFT.h"

namespace liboi
{

CRoutine_FFT_clFFT::CRoutine_FFT_clFFT(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_FT(device, context, queue)
{
	mDimentions = CLFFT_2D;

	mTempBuffer = NULL;
	mOutputBuffer = NULL;

}

CRoutine_FFT_clFFT::~CRoutine_FFT_clFFT()
{
	int status = CL_SUCCESS;
	 /* Release the plan. */
	status = clfftDestroyPlan( &mPlanHandle );

	/* Release clFFT library. */
	clfftTeardown();
}

CRoutine_FFT_clFFT::CRoutine_FFT_clFFT(cl_device_id device, cl_context context, cl_command_queue queue);


void CRoutine_FFT_clFFT::Init(float image_scale)
{
	int status = CL_SUCCESS;

	// Init the clFFT library.
	status = clfftInitSetupData(&mFFTSetup);
	status = clfftSetup(&mFFTSetup);

    // Set plan parameters.
    status = clfftSetPlanPrecision(mPlanHandle, CLFFT_SINGLE);
    status = clfftSetLayout(mPlanHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
    status = clfftSetResultLocation(mPlanHandle, CLFFT_INPLACE);

    // Bake the plan.
    status = clfftBakePlan(mPlanHandle, 1, &mQueue, NULL, NULL);

    // init the buffers
//    mTempBuffer = ...;
//    mOutputBuffer = ...;

}

void CRoutine_FFT_clFFT::FT(cl_mem uv_points, int n_uv_points,
		cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output)
{
	int status = CL_SUCCESS;

	// Execute the plan.
//	status = clfftEnqueueTransform(mPlanHandle, CLFFT_FORWARD, 1, &mQueue, 0, NULL, NULL, &image, &mOutputBuffer, &mTempBuffer);

	// Wait for calculations to be finished.
	status = clFinish(mQueue);

	// Fetch results of calculations.
	//status = clEnqueueReadBuffer( queue, bufX, CL_TRUE, 0, N * 2 * sizeof( *X ), X, 0, NULL, NULL );

}

void CRoutine_FFT_clFFT::FT(valarray<cl_float2> & uv_points, unsigned int n_uv_points,
	valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
	valarray<cl_float2> & cpu_output)
{

}

} /* namespace liboi */
