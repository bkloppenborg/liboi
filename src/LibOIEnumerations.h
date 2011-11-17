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
	DFT,	// Discrete Fourier Transform O(N^2)
	NFFT	// Nonuniform Fast Fourier Transform O(N log(N))
};

#endif /* LIBOI_ENUMERATIONS_H_ */
