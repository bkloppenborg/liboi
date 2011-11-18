/*
 * enumerations.h
 *
 *  Created on: Nov 17, 2011
 *      Author: bkloppenborg
 */

#ifndef LIBOI_ENUMERATIONS_H_
#define LIBOI_ENUMERATIONS_H_


enum eFTMethods
{
	DFT = 0,	// Discrete Fourier Transform O(N^2)
	NFFT = 1	// Nonuniform Fast Fourier Transform O(N log(N))
};

enum eImageTypes
{
	OpenCLBuffer = 0,
	OpenGLBuffer = 1
};

#endif /* LIBOI_ENUMERATIONS_H_ */
