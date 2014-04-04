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
	mOversampledImageBuffer = NULL;

	// Init the oversampled image buffer size to zero:
	mOversampledImageLengths.resize(3);
	mOversampledImageLengths[0] = 0;
	mOversampledImageLengths[1] = 0;
	mOversampledImageLengths[2] = 0;
}

CRoutine_FFT_clFFT::~CRoutine_FFT_clFFT()
{
	int status = CL_SUCCESS;
	 /* Release the plan. */
	status = clfftDestroyPlan( &mPlanHandle );

	/* Release clFFT library. */
	clfftTeardown();

	// Relase local resources
	if(mOversampledImageBuffer) clReleaseMemObject(mOversampledImageBuffer);
	if(mTempBuffer) clReleaseMemObject(mTempBuffer);
	if(mOutputBuffer) clReleaseMemObject(mOutputBuffer);
}

void CRoutine_FFT_clFFT::Init(float image_scale)
{
	// TODO: This function should be removed from the base class.
}


void CRoutine_FFT_clFFT::Init(unsigned int image_width, unsigned int image_height, unsigned int oversampling_factor)
{
	int status = CL_SUCCESS;

	mOversamplingFactor = oversampling_factor;
	// Force sampling to be at least 1
	if(mOversamplingFactor < 1)
		mOversamplingFactor = 1;

	mOversampledImageLengths[0] = nextPow2(image_width * mOversamplingFactor);
	mOversampledImageLengths[1] = nextPow2(image_height * mOversamplingFactor);

    // init the oversampled image buffer and temporary data buffers:
    unsigned int oversampled_image_size = mOversampledImageLengths[0] * mOversampledImageLengths[1];
    // oversampled image buffer
    mOversampledImageBuffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, oversampled_image_size * sizeof(cl_float), NULL, &status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.");

    mTempBuffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, oversampled_image_size * sizeof(cl_float2), NULL, &status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.");

	mOutputBuffer = clCreateBuffer(mContext, CL_MEM_READ_WRITE, oversampled_image_size * sizeof(cl_float2), NULL, &status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.");


	// Init the clFFT library.
	status = clfftInitSetupData(&mFFTSetup);
	CHECK_OPENCL_ERROR(status, "clfftInitSetupData failed.");
	status = clfftSetup(&mFFTSetup);
	CHECK_OPENCL_ERROR(status, "clfftSetup failed.");

	status = clfftCreateDefaultPlan(&mPlanHandle, mContext, mDimentions, &mOversampledImageLengths[0]);

    // Set plan parameters.
    status = clfftSetPlanPrecision(mPlanHandle, CLFFT_SINGLE);
	CHECK_OPENCL_ERROR(status, "clfftSetPlanPrecision failed.");
    status = clfftSetLayout(mPlanHandle, CLFFT_REAL , CLFFT_HERMITIAN_INTERLEAVED);
	CHECK_OPENCL_ERROR(status, "clfftSetLayout failed.");
    status = clfftSetResultLocation(mPlanHandle, CLFFT_OUTOFPLACE);
	CHECK_OPENCL_ERROR(status, "clfftSetResultLocation failed.");

    // Bake the plan.
    status = clfftBakePlan(mPlanHandle, 1, &mQueue, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clfftBakePlan failed.");
}

void CRoutine_FFT_clFFT::FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem output)
{
	int status = CL_SUCCESS;
    cl_event copyCompleteEvent;

	// Copy the image into the center of the (potentially oversampled) image buffer

	// Copy everything from the source image:
    vector<size_t> src_origin(3, 0);
	cout << "Source origin" << src_origin[0] << " " << src_origin[1] << " " << src_origin[2] << endl;

    vector<size_t> copy_region(3, 0);
	copy_region[0] = image_width * sizeof(cl_float);
	copy_region[1] = image_height * sizeof(cl_float);
	copy_region[2] = 1;

	cout << "Copy region" << copy_region[0] << " " << copy_region[1] << " " << copy_region[2] << endl;

	cout << "Destination region" << mOversampledImageLengths[0] << " " << mOversampledImageLengths[1] << " " << mOversampledImageLengths[2] << endl;

	// Copy the data into center of the destination image:
    vector<size_t> dst_origin(3, 0);
	dst_origin[0] = mOversampledImageLengths[0] / 2 - image_width / 2;
	dst_origin[1] = mOversampledImageLengths[1] / 2 - image_height / 2;

	cout << "Destination origin" << dst_origin[0] << " " << dst_origin[1] << " " << dst_origin[2] << endl;

	// Execute the copy
	status = clEnqueueCopyBufferRect(mQueue, image, mOversampledImageBuffer, &src_origin[0], &dst_origin[0], &copy_region[0],
				image_width * sizeof(cl_float), image_height * sizeof(cl_float),
				mOversampledImageLengths[0] * sizeof(cl_float), mOversampledImageLengths[1] * sizeof(cl_float),
				0, NULL, &copyCompleteEvent);
	CHECK_OPENCL_ERROR(status, "clEnqueueCopyBufferRect failed.");

	// Wait for the copy to complete before continuing.
	status = waitForEventAndRelease(&copyCompleteEvent);
	CHECK_OPENCL_ERROR(status, "waitForEventAndRelease failed.");

	// Execute the plan.
	status = clfftEnqueueTransform(mPlanHandle, CLFFT_FORWARD, 1, &mQueue, 0, NULL, NULL, &mOversampledImageBuffer, &mOutputBuffer, mTempBuffer);

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
