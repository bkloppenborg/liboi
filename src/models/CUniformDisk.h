/*
 * CUniformDisk.h
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#ifndef CPOINTSOURCE_H_
#define CPOINTSOURCE_H_

#include "CModel.h"

class CUniformDisk: public CModel
{
protected:
	double mAlpha;
	double mDelta;
	double mRadius; // in angular units
	double mScale;	// in angular units per pixel

public:
	CUniformDisk(double image_scale);
	CUniformDisk(double alpha, double delta, double image_scale, double radius);
	virtual ~CUniformDisk();

	virtual complex<double> GetVis(pair<double,double> & uv);
	virtual valarray<double> GetImage(unsigned int image_width, unsigned int image_height, float image_scale);
};

#endif /* CPOINTSOURCE_H_ */
