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
	// TODO Auto-generated constructor stub

}

CRoutine_FFT_clFFT::~CRoutine_FFT_clFFT()
{
	// TODO Auto-generated destructor stub
}

CRoutine_FFT_clFFT::CRoutine_FFT_clFFT(cl_device_id device, cl_context context, cl_command_queue queue);


void CRoutine_FFT_clFFT::Init(float image_scale)
{

}

void CRoutine_FFT_clFFT::FT(cl_mem uv_points, int n_uv_points,
		cl_mem image, int image_width, int image_height, cl_mem image_flux, cl_mem output)
{

}

void CRoutine_FFT_clFFT::FT(valarray<cl_float2> & uv_points, unsigned int n_uv_points,
	valarray<cl_float> & image, unsigned int image_width, unsigned int image_height, float image_scale,
	valarray<cl_float2> & cpu_output)
{

}

} /* namespace liboi */
