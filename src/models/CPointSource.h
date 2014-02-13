/*
 * CPointSource.h
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#ifndef CPOINTSOURCE_H_
#define CPOINTSOURCE_H_

#include "CModel.h"

namespace liboi
{

class CPointSource: public CModel
{

public:
	CPointSource(unsigned int image_width, unsigned int image_height, double image_scale);
	virtual ~CPointSource();

	complex<double> GetVis(pair<double,double> & uv);
	valarray<double> GetImage();
};

} // namespace liboi

#endif /* CPOINTSOURCE_H_ */
