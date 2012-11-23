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

CUniformDisk::CUniformDisk(double image_scale)
	:CModel(0, 0, image_scale)
{

}

CUniformDisk::CUniformDisk(double alpha, double delta, double image_scale, double radius)
: CModel(alpha, delta, image_scale)
{
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
	double V = 2 * j0(bess)/bess;
	return V * phase;
}

valarray<double> CUniformDisk::GetImage(unsigned int image_width, unsigned int image_height, float image_scale)
{
	unsigned int radius = MasToPixel(mRadius);

	// Create a (normalized) image with a point source at the center:
	unsigned int image_size = image_width * image_height;
	valarray<double> image(image_size);

	unsigned int x_center = image_width/2;
	unsigned int y_center = image_height/2;
	double rad_sq = radius * radius;
	int di = 0;
	int dj = 0;

	for(int i = 0; i < image_width; i++)
	{
		di = abs(i - x_center);
		for(int j = 0; j < image_height; j++)
		{
			dj = abs(j - y_center);

			if(di*di + dj*dj < rad_sq)
				image[image_height * j + i] = 1;
		}
	}

	// Now normalize the image
	CRoutine_Normalize::Normalize(image, image_size);

	return image;
}
