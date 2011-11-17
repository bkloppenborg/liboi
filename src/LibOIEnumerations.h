/*
 * enumerations.h
 *
 *  Created on: Nov 17, 2011
 *      Author: bkloppenborg
 */

#ifndef ENUMERATIONS_H_
#define ENUMERATIONS_H_


enum eFTMethods
{
	DFT,	// Discrete Fourier Transform O(N^2)
	NFFT	// Nonuniform Fast Fourier Transform O(N log(N))
};

#endif /* ENUMERATIONS_H_ */
