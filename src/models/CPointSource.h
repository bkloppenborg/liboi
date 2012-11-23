/*
 * CPointSource.h
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#ifndef CPOINTSOURCE_H_
#define CPOINTSOURCE_H_

#include "CModel.h"

class CPointSource: public CModel
{

public:
	CPointSource(double image_scale);
	CPointSource(double alpha, double delta, double image_scale);
	virtual ~CPointSource();

	complex<double> GetVis(pair<double,double> & uv);
	valarray<double> GetImage(unsigned int image_width, unsigned int image_height, float image_scale);
};

#endif /* CPOINTSOURCE_H_ */
