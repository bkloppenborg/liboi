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
	double bess = 2 * PI * radius * baseline;

	// Handle divide by zero special case:
	if(bess < 1E-8)
		return 1;

	complex<double> phase(cos(phi), sin(phi));
	double V = 2 * j1(bess)/bess;
	return V * phase;
}

valarray<double> CUniformDisk::GetImage()
{
	double radius = mRadius / mImageScale;

	// Create a (normalized) image with a point source at the center:
	unsigned int image_size = mImageWidth * mImageHeight;
	valarray<double> image(image_size);

	// Account for the image indexing by shifting by one pixel:
	double center_col = double(mImageWidth - 1) / 2;
	double center_row = double(mImageHeight - 1) / 2;
	double rad_sq = radius * radius;
//	double dx = 0;
//	double dy = 0;

		for(unsigned int row = 0; row < mImageHeight; row++)
		{
			//dy = row - center_row;
			for(unsigned int col = 0; col < mImageWidth; col++)
			{
				//dx = col - center_col;

				if(col > (center_col - radius) && col < (center_col + radius) &&
						row > (center_row - radius) && row < (center_row + radius))
					image[mImageWidth * row + col] = 1;
				else
					image[mImageWidth * row + col] = 0;
			}
		}


//	for(unsigned int row = 0; row < mImageHeight; row++)
//	{
//		dy = row - center_row;
//		for(unsigned int col = 0; col < mImageWidth; col++)
//		{
//			dx = col - center_col;
//
//			if(dx*dx + dy*dy <= rad_sq)
//				image[mImageWidth * row + col] = 1;
//			else
//				image[mImageWidth * row + col] = 0;
//		}
//	}

	// Now normalize the image
	CRoutine_Normalize::Normalize(image, image_size);

	return image;
}

} // namespace liboi
