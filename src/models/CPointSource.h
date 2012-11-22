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
protected:
	double mAlpha;
	double mDelta;

public:
	CPointSource();
	CPointSource(double alpha, double delta);
	virtual ~CPointSource();

	virtual complex<double> GetVis(pair<double,double> uv);
};

#endif /* CPOINTSOURCE_H_ */
