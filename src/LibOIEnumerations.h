/*
 * enumerations.h
 *
 *  Created on: Nov 17, 2011
 *      Author: bkloppenborg
 */

#ifndef LIBOI_ENUMERATIONS_H_
#define LIBOI_ENUMERATIONS_H_

namespace LibOIEnums
{
	enum FTMethods
	{
		DFT = 0,	// Discrete Fourier Transform O(N^2)
		NFFT = 1	// Nonuniform Fast Fourier Transform O(N log(N))
	};

	enum ImageTypes
	{
		OpenCLBuffer = 0,
		OpenGLBuffer = 1
	};

	enum OpMode
	{
		OPENCL_ONLY,	// Runs all calculations on OpenCL devices (default)
		VERIFY			// Runs all calculations on OpenCL devices, then checks them using CPU calculations.
	};


}

#endif /* LIBOI_ENUMERATIONS_H_ */
