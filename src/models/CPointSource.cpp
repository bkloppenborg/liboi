/*
 * CPointSource.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "CPointSource.h"

CPointSource::CPointSource()
{
	mAlpha = 0;
	mDelta = 0;
}

CPointSource::CPointSource(double alpha, double delta)
{
	mAlpha = alpha;
	mDelta = delta;
}

CPointSource::~CPointSource()
{
	// TODO Auto-generated destructor stub
}

virtual complex<double> CPointSource::GetVis(pair<double,double> uv)
{
	// Calculate the visibility and return
	double arg = -2 * PI *(uv.first * mAlpha + uv.second * mDelta);
	return complex<double>(cos(arg), sin(arg));
}
