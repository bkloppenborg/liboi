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
#include <valarray>

#ifndef PI
#include <cmath>
#define PI M_PI
#endif

#include "cl.hpp"
//#if defined(__APPLE__) || defined(__MACOSX)
//	#include <OpenCL/cl.hpp>
//#else
//	#include <CL/cl.hpp>
//#endif

using namespace std;

namespace liboi
{

class CModel
{
protected:
	static double RPMAS;

	// Image information:
	unsigned int mImageWidth;
	unsigned int mImageHeight;
	double mImageScale;
	unsigned int mImageCenterX;
	unsigned int mImageCenterY;
	unsigned int mImageCenterID;

	// Object center location, relative to the center of the image.
	int mShiftX;
	int mShiftY;

public:
	CModel(unsigned int image_width, unsigned int image_height, double image_scale);
	virtual ~CModel();

	virtual complex<double> GetVis(pair<double,double> & uv) = 0;
	cl_float2 GetVis_CL(cl_float2 & uv);
	valarray<cl_float2> GetVis_CL(valarray<cl_float2> & uv_list);

	double GetV2(pair<double,double> & uv);
	cl_float GetV2_CL(cl_float2 & uv);
	valarray<cl_float> GetV2_CL(valarray<cl_float2> & uv_list);

	complex<double> GetT3(pair<double,double> & uv_ab, pair<double,double> & uv_bc, pair<double,double> & uv_ca);
	cl_float2 GetT3_CL(cl_float2 & uv_ab, cl_float2 & uv_bc, cl_float2 & uv_ca);
	valarray<cl_float2> GetT3_CL(valarray<cl_float2> & uv_points, valarray<cl_uint4> & uv_ref);

	static valarray<pair<double,double>> GenerateUVSpiral(unsigned int n_uv);
	static valarray<cl_float2> GenerateUVSpiral_CL(unsigned int n_uv);

	virtual valarray<double> GetImage() = 0;
	virtual valarray<cl_float> GetImage_CL();

	unsigned int MasToPixel(double value);
	double MasToRad(double value);

	valarray<double> ReadImage(string filename, unsigned int width, unsigned int height, double image_scale);

	void   WriteImage(valarray<double> & image, unsigned int image_width, unsigned int image_height, double image_scale, string filename);
};

} // namespace liboi

#endif /* CMODEL_H_ */
