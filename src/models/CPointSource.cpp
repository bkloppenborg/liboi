/*
 * CPointSource.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "CPointSource.h"

CPointSource::CPointSource(unsigned int image_width, unsigned int image_height, double image_scale)
	: CModel(image_width, image_height, image_scale)
{
	// Point source in the center of the image.
}

CPointSource::~CPointSource()
{
	// TODO Auto-generated destructor stub
}

complex<double> CPointSource::GetVis(pair<double,double> & uv)
{
	// Calculate the visibility and return
	double dx = 0; //(mImageCenterX) * mImageScale * RPMAS;
	double dy = 0; //(mImageCenterY) * mImageScale * RPMAS;
	double phi = -2 * PI * (dx * uv.first + dy * uv.second);
	return complex<double>(cos(phi), sin(phi));
}

valarray<double> CPointSource::GetImage(unsigned int image_width, unsigned int image_height, float image_scale)
{
	// Create a blank image:
	unsigned int image_size = image_width * image_height;
	valarray<double> output(image_size);

	// Now activate the center most pixel:
	//output[mImageCenterID] = 1;
	output[0] = 1;

	return output;
}
