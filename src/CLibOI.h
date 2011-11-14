/*
 * CLibOI.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#ifndef CLIBOI_H_
#define CLIBOI_H_

#include <string>
#include "COILibData.h"

#include "COpenCL.h"

class CLibOI
{
private:
	// Datamembers:
	vector<COILibData*> DataList;

public:
	CLibOI();
	virtual ~CLibOI();

public:
	void ComputeChi2_V2(cl_mem v2_sim_data, cl_mem v2_real_data, int v2_size);
	void ComputeChi2_T3(cl_mem t3_sim_data, cl_mem t3_real_data, int t3_size);
	float ComputeFlux(cl_mem image_location, int width, int height);

	void FT(cl_mem image_location, int image_width, int image_height, cl_mem output_buffer);

	void Init();

	void LoadData(string filename);

	void MakeData_V2(cl_mem ft_buffer, int width, int height, cl_mem v2_uv, int v2_size, cl_mem output_buffer);
	void MakeData_T3(cl_mem ft_buffer, int width, int height, cl_mem t3_uv, int t3_size, cl_mem output_buffer);

	void Normalize(cl_mem image_location, int width, int height, float total_flux);

	void UnloadData(string filename);
};

#endif /* CLIBOI_H_ */
