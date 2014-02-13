/*
 * CUniformDisk.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "CUniformDisk.h"
#include "CRoutine_Normalize.h"
#include <cmath>
using namespace std;

namespace liboi
{

/// Creates a uniform disk of 1.0 [image_scale units] centered at the origin.
CUniformDisk::CUniformDisk(unsigned int image_width, unsigned int image_height, double image_scale)
	:CModel(image_width, image_height, image_scale)
{
	mAlpha = 0;
	mDelta = 0;
	mRadius = 1.0;
}

/// Creates a uniform disk of radius centered at (alpha, delta).
CUniformDisk::CUniformDisk(unsigned int image_width, unsigned int image_height, double image_scale,
		double radius, double alpha, double delta)
:CModel(image_width, image_height, image_scale)
{
	mAlpha = alpha;
	mDelta = delta;
	mRadius = radius;
}

CUniformDisk::~CUniformDisk()
{
	// TODO Auto-generated destructor stub
}

complex<double> CUniformDisk::GetVis(pair<double,double> & uv)
{
	double radius = MasToRad(mRadius);
	// Calculate the visibility and return
	double baseline = sqrt(uv.first * uv.first + uv.second * uv.second);
	double phi = -2 * PI * (mAlpha * uv.first + mDelta * uv.second);
	double bess = PI * radius * baseline;

	complex<double> phase(cos(phi), sin(phi));
	double V = 2 * j1(bess)/bess;
	return V * phase;
}

valarray<double> CUniformDisk::GetImage()
{
	unsigned int radius = MasToPixel(mRadius);

	// Create a (normalized) image with a point source at the center:
	unsigned int image_size = mImageWidth * mImageHeight;
	valarray<double> image(image_size);

	unsigned int x_center = mImageWidth/2;
	unsigned int y_center = mImageHeight/2;
	double rad_sq = radius * radius;
	int dx = 0;
	int dy = 0;

	for(int x = 0; x < mImageWidth; x++)
	{
		dx = abs(x - x_center);
		for(int y = 0; y < mImageHeight; y++)
		{
			dy = abs(y - y_center);

			if(dx*dx + dy*dy < rad_sq)
				image[mImageHeight * y + x] = 1;
			else
				image[mImageHeight * y + x] = 0;
		}
	}

	// Now normalize the image
	CRoutine_Normalize::Normalize(image, image_size);

	return image;
}

} // namespace liboi
