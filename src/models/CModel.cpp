/*
 * CModel.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppen
 */

#include "CModel.h"
#include <cassert>

using namespace std;

namespace liboi
{

double CModel::RPMAS = (M_PI / 180.0) / 3600000.0; // Number of radians per milliarcsecond

CModel::CModel(unsigned int image_width, unsigned int image_height, double image_scale)
{
	assert(image_scale > 0);
	assert(image_width >= 0);
	assert(image_height >= 0);

	mImageWidth = image_width;
	mImageHeight = image_height;
	mImageScale = image_scale;

	mImageCenterX = mImageWidth / 2;
	mImageCenterY = mImageHeight / 2;
	mImageCenterID = mImageWidth * mImageCenterY + mImageCenterX;

	mShiftX = 0;
	mShiftY = 0;
}

CModel::~CModel()
{
	// TODO Auto-generated destructor stub
}

/// Returns the visibility of the model at the UV point uv converted into a cl_float2
cl_float2 CModel::GetVis_CL(cl_float2 & uv)
{
	pair<double,double> t_uv(uv.s0, uv.s1);
	complex<double> t_val = GetVis(t_uv);

	cl_float2 output;
	output.s0 = real(t_val);
	output.s1 = imag(t_val);

	return output;
}

/// Computes the visibility returning OpenCL data types.
valarray<cl_float2> CModel::GetVis_CL(valarray<cl_float2> & uv_list)
{
	unsigned int n_uv = uv_list.size();
	valarray<cl_float2> output(n_uv);
	for(int i = 0; i < n_uv; i++)
	{
		output[i] = GetVis_CL(uv_list[i]);
	}

	return output;
}

/// Computes the V2
double CModel::GetV2(pair<double,double> & uv)
{
	return norm(GetVis(uv));
}

/// Computes the V2, returns OpenCL data types
cl_float CModel::GetV2_CL(cl_float2 & uv)
{
	pair<double,double> t_uv(uv.s0, uv.s1);

	return cl_float(GetV2(t_uv));
}

/// Computes the V2 from a list of UV points.
valarray<cl_float> CModel::GetV2_CL(valarray<cl_float2> & uv_list)
{
	int n_uv = uv_list.size();
	valarray<cl_float> output(n_uv);
	for(int i = 0; i < n_uv; i++)
	{
		cl_float2 uv = uv_list[i];
		output[i] = GetV2_CL(uv);
	}

	return output;
}

/// Computes the T3
complex<double> CModel::GetT3(pair<double,double> & uv_ab, pair<double,double> & uv_bc, pair<double,double> & uv_ca)
{
	return GetVis(uv_ab) * GetVis(uv_bc) * GetVis(uv_ca);
}

/// Computes T3 from OpenCL values
cl_float2 CModel::GetT3_CL(cl_float2 & uv_ab, cl_float2 & uv_bc, cl_float2 & uv_ca)
{
	pair<double,double> t_uv_ab(uv_ab.s0, uv_ab.s1);
	pair<double,double> t_uv_bc(uv_bc.s0, uv_bc.s1);
	pair<double,double> t_uv_ca(uv_ca.s0, uv_ca.s1);

	complex<double> t_val = GetT3(t_uv_ab, t_uv_bc, t_uv_ca);

	cl_float2 output;
	output.s0 = real(t_val);
	output.s1 = imag(t_val);

	return output;
}

/// Computes T3 from OpenCL values
valarray<cl_float2> CModel::GetT3_CL(valarray<cl_float2> & uv_points, valarray<cl_uint4> & uv_ref)
{
	int n_t3 = uv_ref.size();

	valarray<cl_float2> output(n_t3);

	for(int i = 0; i < n_t3; i++)
	{
		cl_float2 uv_ab = uv_points[uv_ref[i].s0];
		cl_float2 uv_bc = uv_points[uv_ref[i].s1];
		cl_float2 uv_ca = uv_points[uv_ref[i].s2];

		output[i] = GetT3_CL(uv_ab, uv_bc, uv_ca);
	}

	return output;
}

valarray<pair<double,double>> CModel::GenerateUVSpiral(unsigned int n_uv)
{
	// Init a buffer to store the UV points in:
	valarray<pair<double,double>> uv_points(n_uv);

	// Here we make an Archimedean spiral that covers a wide range of UV points
	// We should get 4 full revolutions and end up with r ~ 400 mega-lambda.
	double a = 1;	// 1 mega-lambda
	double b = 20;
	double dt = 4*2*PI / n_uv;	// We want to have four full circles
	double r = 0;
	double theta = 0;
	for(int i = 0; i < n_uv; i++)
	{
		r = (a + b * theta) * 1E6;
		uv_points[i] = pair<float,float>(r*cos(theta), r*sin(theta));

		// Increment theta
		theta += dt;
	}

	return uv_points;
}

valarray<cl_float2> CModel::GenerateUVSpiral_CL(unsigned int n_uv)
{
	valarray<cl_float2> output(n_uv);

	valarray<pair<double,double>> input = GenerateUVSpiral(n_uv);
	for(int i = 0; i < n_uv; i++)
	{
		output[i] = cl_float2();
		output[i].s0 = cl_float(input[i].first);
		output[i].s1 = cl_float(input[i].second);
	}

	return output;
}

valarray<cl_float> CModel::GetImage_CL()
{
	valarray<double> input = GetImage();
	valarray<cl_float> output(input.size());
	for(int i = 0; i < input.size(); i++)
		output[i] = cl_float(input[i]);

	return output;
}

unsigned int CModel::MasToPixel(double value)
{
	return floor(value / mImageScale);
}

double CModel::MasToRad(double value)
{
	return value * RPMAS;
}

} // namespace liboi
