/*
 * CModel.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "CModel.h"

CModel::CModel()
{
	// TODO Auto-generated constructor stub

}

CModel::~CModel()
{
	// TODO Auto-generated destructor stub
}

double CModel::GetV2(pair<double,double> uv)
{
	return norm(GetVis(uv));
}

complex<double> CModel::GetT3(pair<double,double> uv_ab, pair<double,double> uv_bc, pair<double,double> uv_ca)
{
	return GetVis(uv_ab) * GetVis(uv_bc) * GetVis(uv_ca);
}
