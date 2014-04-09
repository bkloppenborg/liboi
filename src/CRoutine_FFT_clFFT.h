/*
 * CRoutine_FFT_clFFT.h
 *
 *  Created on: Jan 30, 2014
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_FFT_CLFFT_H_
#define CROUTINE_FFT_CLFFT_H_

#include "CRoutine_FT.h"
#include <clFFT.h>

#include "CRoutine_Zero.h"

namespace liboi
{

class CRoutine_FFT_clFFT : public CRoutine_FT
{
protected:
	clfftPlanHandle mPlanHandle;
	clfftDim mDimentions;
	clfftSetupData mFFTSetup;

	cl_mem mOutputBuffer;
	cl_mem mTempBuffer;

	cl_mem mOversampledImageBuffer;
	vector<unsigned int> mOversampledImageLengths;
	unsigned int mOversamplingFactor;
	float mImageScale;

	// External routines, deleted elsewhere
	CRoutine_Zero * mrZero;

public:
	CRoutine_FFT_clFFT(cl_device_id device, cl_context context, cl_command_queue queue);
	virtual ~CRoutine_FFT_clFFT();

	void Init(float image_scale);
	void Init(float image_scale, unsigned int image_width, unsigned int image_height, unsigned int oversampling_factor, CRoutine_Zero * zero_routine);
	void FT(cl_mem uv_points, int n_uv_points, cl_mem image, int image_width, int image_height, cl_mem output);
	void FT(valarray<cl_float2> & uv_points, unsigned int n_uv_points,
		valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
		valarray<cl_float2> & cpu_output);
};

} /* namespace liboi */
#endif /* CROUTINE_FFT_CLFFT_H_ */
