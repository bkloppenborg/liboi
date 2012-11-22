/*
 * CModel.h
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 *
 *  Base class for analytic-only visibility models. Inheriting classes simply
 *  need to define the GetVis function.
 */

#ifndef CMODEL_H_
#define CMODEL_H_

#include <complex>

#ifndef PI
#include <cmath>
#define PI M_PI
#endif

using namespace std;

class CModel
{
public:
	CModel();
	virtual ~CModel();

	virtual complex<double> GetVis(pair<double,double> uv) = 0;
	double GetV2(pair<double,double> uv);
	complex<double> GetT3(pair<double,double> uv_ab, pair<double,double> uv_bc, pair<double,double> uv_ca);
};

#endif /* CMODEL_H_ */
