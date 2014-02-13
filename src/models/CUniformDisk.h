/*
 * CUniformDisk.h
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#ifndef CUNIFORMDISK_H_
#define CUNIFORMDISK_H_

#include "CModel.h"

namespace liboi
{

class CUniformDisk: public CModel
{
protected:
	double mAlpha;
	double mDelta;
	double mRadius; // in angular units
	double mScale;	// in angular units per pixel

public:
	CUniformDisk(unsigned int image_width, unsigned int image_height, double image_scale);
	CUniformDisk(unsigned int image_width, unsigned int image_height, double image_scale,
			double radius, double alpha, double delta);
	virtual ~CUniformDisk();

	virtual complex<double> GetVis(pair<double,double> & uv);
	virtual valarray<double> GetImage();
};

} // namespace liboi

#endif /* CUNIFORMDISK_H_ */
