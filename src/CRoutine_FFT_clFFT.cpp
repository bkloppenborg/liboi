/*
 * CRoutine_FFT_clFFT.cpp
 *
 *  Created on: Jan 30, 2014
 *      Author: bkloppenborg
 */

#include "CRoutine_FFT_clFFT.h"
#include <fstream>


namespace liboi
{

CRoutine_FFT_clFFT::CRoutine_FFT_clFFT(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_FT(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("ft_clfft_nearest.cl");

	mDimentions = CLFFT_2D;

	mTempBuffer = NULL;
	mOutputBuffer = NULL;
	mOversampledImageBuffer = NULL;

	// Init the oversampled image buffer size to zero:
	mOversampledImageLengths.resize(3);
	mOversampledImageLengths[0] = 0;
	mOversampledImageLengths[1] = 0;
	mOversampledImageLengths[2] = 0;
	mImageScale = 1;
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


void CRoutine_FFT_clFFT::Init(float image_scale, unsigned int image_width, unsigned int image_height, unsigned int oversampling_factor,
		CRoutine_Zero * zero_routine)
{
	assert(image_scale > 0);
	mImageScale = image_scale;

	mrZero = zero_routine;

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

	// zero out the buffers:
	mrZero->Zero(mOversampledImageBuffer, oversampled_image_size);
	mrZero->Zero(mTempBuffer, 2 * oversampled_image_size);
	mrZero->Zero(mOutputBuffer, 2 * oversampled_image_size);

	// Init the clFFT library.
	status = clfftInitSetupData(&mFFTSetup);
	CHECK_OPENCL_ERROR(status, "clfftInitSetupData failed.");
	status = clfftSetup(&mFFTSetup);
	CHECK_OPENCL_ERROR(status, "clfftSetup failed.");

	vector<size_t> clLengths(3, 0);
	clLengths[0] = mOversampledImageLengths[0];
	clLengths[1] = mOversampledImageLengths[1];
	clLengths[2] = mOversampledImageLengths[2];

	status = clfftCreateDefaultPlan(&mPlanHandle, mContext, mDimentions, &clLengths[0]);

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

	// Build the fft_nearest kernel
	string source = ReadSource(mSource[0]);
    BuildKernel(source, "ft_clfft_nearest", mSource[0]);
}

/// Computes an (oversampled) FFT of the image and copies the nearest UV points
/// from the FFT into output.
void CRoutine_FFT_clFFT::FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem output)
{
	int status = CL_SUCCESS;
    cl_event copyCompleteEvent;

    // First copy the image into the center of the oversampled buffer
    // Source image origin. Start at (0, 0, 0)
    vector<size_t> src_origin(3, 0);

    // Define the size of a row and 2D slice of the source image.
    size_t src_row_pitch = image_width * sizeof(cl_float);
    size_t src_slice_pitch = (image_width * image_height) * sizeof(cl_float);

    // Destination image origin. Center the source image into the destination image:
    vector<size_t> dst_origin(3, 0);
    dst_origin[0] = mOversampledImageLengths[0] / 2 - image_width / 2;
    dst_origin[0] = mOversampledImageLengths[1] / 2 - image_height / 2;

    // Define the size of a row and 2D slice of the destination image.
    size_t dst_row_pitch = mOversampledImageLengths[0] * sizeof(cl_float);
    size_t dst_slice_pitch =  (mOversampledImageLengths[0] * mOversampledImageLengths[1]) * sizeof(cl_float);

    // The size of the region to be copied, expressed in bytes.
    vector<size_t> region(3, 0);
    region[0] = image_width * sizeof(cl_float);
    region[1] = image_height;
    region[2] = 1;

    // Execute the copy
    status = clEnqueueCopyBufferRect(mQueue, image, mOversampledImageBuffer,
    		&src_origin[0], &dst_origin[0], &region[0],
    		src_row_pitch,src_slice_pitch,
    		dst_row_pitch, dst_slice_pitch,
    		0, NULL, &copyCompleteEvent);
	CHECK_OPENCL_ERROR(status, "clEnqueueCopyBufferRect failed.");

	// Wait for the copy to complete before continuing.
	status = waitForEventAndRelease(&copyCompleteEvent);
	CHECK_OPENCL_ERROR(status, "waitForEventAndRelease failed.");

	// Execute the FFT and wait for it to complete.
	status = clfftEnqueueTransform(mPlanHandle, CLFFT_FORWARD, 1, &mQueue, 0, NULL, NULL, &mOversampledImageBuffer, &mOutputBuffer, mTempBuffer);
	status = clFinish(mQueue);

	// Next, find the closest matching FFT coordinates to the true UV pair.
	// Copy the FFT value into output.
    size_t global = (size_t) n_uv_points;

    // Determine the best workgroup size for this kernel.
    // Init the local threads to something large
    size_t local = 2048;
    status = clGetKernelWorkGroupInfo(mKernels[0], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");

	// Round the global workgroup size to the next greatest multiple of the local workgroup size
	global = next_multiple(global, local);

	// Set the kernel arguments and enqueue the kernel
	status = clSetKernelArg(mKernels[0], 0, sizeof(cl_mem), &mOutputBuffer);
	status |= clSetKernelArg(mKernels[0], 1, sizeof(uint), &mOversampledImageLengths[0]);
	status |= clSetKernelArg(mKernels[0], 2, sizeof(uint), &mOversampledImageLengths[1]);
	status |= clSetKernelArg(mKernels[0], 3, sizeof(cl_float), &mImageScale);
	status |= clSetKernelArg(mKernels[0], 4, sizeof(cl_mem), &uv_points);
	status |= clSetKernelArg(mKernels[0], 5, sizeof(int), &n_uv_points);
	status |= clSetKernelArg(mKernels[0], 6, sizeof(cl_mem), &output);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    // Execute the kernel over the entire range of the data set
	status = clEnqueueNDRangeKernel(mQueue, mKernels[0], 1, NULL, &global, &local, 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	// wait for the operation to finish
	status = clFinish(mQueue);

    unsigned int oversampled_image_size = mOversampledImageLengths[0] * mOversampledImageLengths[1];
	vector<cl_float2> temp(oversampled_image_size);
	status = clEnqueueReadBuffer(mQueue, mOutputBuffer, CL_TRUE, 0, oversampled_image_size * sizeof(cl_float2), &temp[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");

	ofstream temp_file;
	temp_file.open("/tmp/fft_output.txt");
	float x;
	float y;
	int k, j;

	float RPMAS = (PI/180.0)/3600000.0; // rad/mas
	float scale = 0.025; // mas/pixel
	float u, v;

	cout << mOversampledImageLengths[0] / 2 << endl;

	for(unsigned int i = 0; i < temp.size(); i++)
	{
		// Convert the i-th entry into an equivalent k/j frequency
		k = i % mOversampledImageLengths[0];
		j = i / mOversampledImageLengths[0];

		if(j > mOversampledImageLengths[0] / 2)
		{
			j -= mOversampledImageLengths[0];
			j *= -1;
			k *= -1;
		}

		// now convert to UV coordinates
		u = k / (mOversampledImageLengths[0] * RPMAS * scale) / 1E6;
		v = j / (mOversampledImageLengths[1] * RPMAS * scale) / 1E6;

		temp_file << k << " " << j << " " << u << " " << v << " " << temp[i].s[0] << " " << temp[i].s[1] << endl;
	}

	temp_file.close();


	vector<cl_float2> temp_output(n_uv_points);
	status = clEnqueueReadBuffer(mQueue, output, CL_TRUE, 0, n_uv_points * sizeof(cl_float2), &temp_output[0], 0, NULL, NULL);
	vector<cl_float2> temp_uv_points(n_uv_points);
	status = clEnqueueReadBuffer(mQueue, uv_points, CL_TRUE, 0, n_uv_points * sizeof(cl_float2), &temp_uv_points[0], 0, NULL, NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");

	temp_file.open("/tmp/temp_output.txt");

	for(unsigned int i = 0; i < temp_output.size(); i++)
	{
		temp_file << i << " " << temp_uv_points[i].s[0]/1E6 << " " << temp_uv_points[i].s[1]/1E6 << " " << temp_output[i].s[0] << " " << temp_output[i].s[1] << endl;
	}

	temp_file.close();
}

void CRoutine_FFT_clFFT::FT(valarray<cl_float2> & uv_points, unsigned int n_uv_points,
	valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
	valarray<cl_float2> & cpu_output)
{

}

} /* namespace liboi */
