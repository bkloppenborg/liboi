/*
 * CPointSource.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "CPointSource.h"

CPointSource::CPointSource(double image_scale)
	: CModel(0, 0, image_scale)
{

}

CPointSource::CPointSource(double alpha, double delta, double image_scale)
	: CModel(alpha, delta, image_scale)
{

}

CPointSource::~CPointSource()
{
	// TODO Auto-generated destructor stub
}

complex<double> CPointSource::CPointSource::GetVis(pair<double,double> & uv)
{
	// Calculate the visibility and return
	double phi = -2 * PI * (mAlpha * uv.first + mDelta * uv.second);
	return complex<double>(cos(phi), sin(phi));
}

valarray<double> CPointSource::GetImage(unsigned int image_width, unsigned int image_height, float image_scale)
{
	// Create a blank image:
	unsigned int image_size = image_width * image_height;
	valarray<double> output(image_size);

	// Now activate the center most pixel:
	output[image_width * image_height/2 + image_width/2] = 1;

	return output;
}
