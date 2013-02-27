/*
 * COILibData.cpp
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      C++ Wrapper class for OIFITS data.  Handles copying data to/from OpenCL devices.
 */
 
/* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * If you use this software as part of a scientific publication, please cite as:
 *
 * Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library"
 * (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
 *
 * This file is part of the OpenCL Interferometry Library (LIBOI).
 * 
 * LIBOI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "COILibData.h"
#include <cassert>
#include <stdexcept>
#include "oi_tools.hpp"
#include "oi_export.hpp"

//temp
#include <limits>

using namespace std;

namespace liboi
{

#define MJD 2400000.5

COILibData::COILibData(string filename, cl_context context, cl_command_queue queue)
{
	mContext = context;
	mQueue = queue;
	mFileName = filename;

	// Read in the data.
	COIFile tmp;
	try
	{
		tmp.open(filename);
		mData = tmp.read();
	}
	catch(CCfits::FITS::CantOpen)
	{

	}

	// Set the OpenCL buffers to NULL
	mData_cl = 0;
	mData_err_cl = 0;
	mData_uv_cl = 0;
	mData_Vis_uv_ref = 0;
	mData_V2_uv_ref = 0;
	mData_T3_uv_ref = 0;
	mData_T3_sign = 0;

	InitData();
}

COILibData::COILibData(const OIDataList & data, cl_context context, cl_command_queue queue)
{
	mContext = context;
	mQueue = queue;
	mData = data;

	// Set the OpenCL buffers to NULL
	mData_cl = 0;
	mData_err_cl = 0;
	mData_uv_cl = 0;
	mData_Vis_uv_ref = 0;
	mData_V2_uv_ref = 0;
	mData_T3_uv_ref = 0;
	mData_T3_sign = 0;

	InitData();
}

COILibData::~COILibData()
{
	DeallocateMemory();
}

/// Initializes statistics on the data set and uploads the data to the OpenCL device.
void COILibData::AllocateMemory()
{
	int err = CL_SUCCESS;

	// In case it has been called before, clean up memory.
	DeallocateMemory();

	// Main data buffer
	// An array of cl_floats arranged as follows: [vis_real, vis_imag, v2, t3_amp, t3_phi]
	// The number of data must always be greater than zero.
	assert(mNData > 0);
	mData_cl = clCreateBuffer(mContext, CL_MEM_READ_ONLY, sizeof(cl_float) * mNData, NULL, NULL);
	mData_err_cl = clCreateBuffer(mContext, CL_MEM_READ_ONLY, sizeof(cl_float) * mNData, NULL, NULL);

	// Copy over the UV points.  We MUST always have at least one UV point (otherwise the data would be nonsense).
	assert(mNUV > 0);

	// Allocate the optimial buffer size for the UV points
	// For NVidia Fermi, there are 16 processors per multiprocessor
	// For ATI hardware, there are 20 processors per multiprocessor
// TODO: We need to dynamically determine the vendor
	mNUV = NextHighestMultiple(16, mNUV);

	mData_uv_cl = clCreateBuffer(mContext, CL_MEM_READ_ONLY, sizeof(cl_float2) * mNUV, NULL, NULL);

	mData_Vis_uv_ref = 0;
	if(mNVis > 0)
		mData_Vis_uv_ref = clCreateBuffer(mContext, CL_MEM_READ_ONLY, sizeof(cl_uint) * mNVis, NULL, NULL);

	mData_V2_uv_ref = 0;
	if(mNV2 > 0)
		mData_V2_uv_ref = clCreateBuffer(mContext, CL_MEM_READ_ONLY, sizeof(cl_uint) * mNV2, NULL, NULL);

	mData_T3_uv_ref = 0;
	mData_T3_sign = 0;
	if(mNT3 > 0)
	{
		mData_T3_uv_ref = clCreateBuffer(mContext, CL_MEM_READ_ONLY, sizeof(cl_uint4) * mNT3, NULL, NULL);
		mData_T3_sign = clCreateBuffer(mContext, CL_MEM_READ_ONLY, sizeof(cl_short4) * mNT3, NULL, NULL);
	}

	// Wait for the queue to process
	clFinish(mQueue);
}

/// Deallocates memory allocated on the OpenCL device.
void COILibData::DeallocateMemory()
{
	// Free OpenCL memory
	if(mData_cl) clReleaseMemObject(mData_cl);
	if(mData_err_cl) clReleaseMemObject(mData_err_cl);

	if(mData_uv_cl) clReleaseMemObject(mData_uv_cl);
	if(mData_Vis_uv_ref) clReleaseMemObject(mData_Vis_uv_ref);
	if(mData_V2_uv_ref) clReleaseMemObject(mData_V2_uv_ref);
	if(mData_T3_uv_ref) clReleaseMemObject(mData_T3_uv_ref);

	if(mData_T3_sign) clReleaseMemObject(mData_T3_sign);
}

/// \brief Exports the real and current simulated data to a file beginning with base_filename;
///
/// Here we use ccoifits::ExportToText(...) function to export data to a well-defined format
/// The format of the text file matches CCOIFITS, namely:
///  *_vis.txt contains visibility data
///	 *_v2.txt  contains V2 data
///  *_t3.txt  contains T3 data.
/// The model data has the word "model" inserted before the suffixes listed above.
void COILibData::ExportData(string base_filename, cl_mem simulated_data)
{
	// Init locals in which we store the data:
	vector<pair<double,double> > uv_points;
	valarray<complex<double>> vis;
	valarray<pair<double,double>> vis_err;
	vector<unsigned int> vis_uv_ref;
	valarray<double> vis2;
	valarray<double> vis2_err;
	vector<unsigned int> vis2_uv_ref;
	valarray<complex<double>> t3;
	valarray<pair<double,double> > t3_err;
	vector<tuple<unsigned int, unsigned int, unsigned int>> t3_uv_ref;
	vector<tuple<short, short, short>> t3_uv_sign;

	// First get a copy of the real data, save it to a file
	CopyFromDevice(uv_points, mData_uv_cl,
			vis, mData_cl,
			vis_err, mData_err_cl,
			vis_uv_ref, mData_Vis_uv_ref,
			vis2, mData_cl,
			vis2_err, mData_err_cl,
			vis2_uv_ref, mData_V2_uv_ref,
			t3, mData_cl,
			t3_err, mData_err_cl,
			t3_uv_ref, mData_T3_uv_ref,
			t3_uv_sign, mData_T3_sign);

	ccoifits::Export_ToText(base_filename, uv_points, vis, vis_err, vis_uv_ref, vis2, vis2_err, vis2_uv_ref, t3, t3_err, t3_uv_ref, t3_uv_sign);

	if(simulated_data != NULL)
	{
		// Now get a copy of the simulated data, save it to a file with "_model" in the filename:
		CopyFromDevice(uv_points, mData_uv_cl,
				vis, simulated_data,
				vis_err, mData_err_cl,
				vis_uv_ref, mData_Vis_uv_ref,
				vis2, simulated_data,
				vis2_err, mData_err_cl,
				vis2_uv_ref, mData_V2_uv_ref,
				t3, simulated_data,
				t3_err, mData_err_cl,
				t3_uv_ref, mData_T3_uv_ref,
				t3_uv_sign, mData_T3_sign);

		ccoifits::Export_ToText(base_filename + "_model", uv_points, vis, vis_err, vis_uv_ref, vis2, vis2_err, vis2_uv_ref, t3, t3_err, t3_uv_ref, t3_uv_sign);
	}
}

/// Initializes statistics on the data set and uploads the data to the OpenCL device.
void COILibData::InitData()
{
	int err = CL_SUCCESS;

	// Export the data from OIDataList to something we can use here.
	vector<pair<double,double> > uv_points;
	valarray<complex<double>> vis;
	valarray<pair<double,double>> vis_err;
	vector<unsigned int> vis_uv_ref;
	valarray<double> vis2;
	valarray<double> vis2_err;
	vector<unsigned int> vis2_uv_ref;
	valarray<complex<double>> t3;
	valarray<pair<double,double> > t3_err;
	vector<tuple<unsigned int, unsigned int, unsigned int>> t3_uv_ref;
	vector<tuple<short, short, short>> t3_uv_sign;

	ccoifits::Export_MinUV(mData, uv_points, vis, vis_err, vis_uv_ref, vis2, vis2_err, vis2_uv_ref, t3, t3_err, t3_uv_ref, t3_uv_sign);

	// Generate some statistics on the data set:
	mNVis = vis.size();
	mNV2 = vis2.size();
	mNT3 = t3.size();
	mNUV = uv_points.size();
	// Average JD (notice we need to add in MJD)
	mAveJD = AverageMJD(mData) + MJD;
	// Total number of double/floats allocated for storage on the OpenCL context:
	mNData = TotalBufferSize(mNVis, mNV2, mNT3);

	cout << "Data set information: " << endl;
	cout << " " << mNVis << " Vis" << endl;
	cout << " " << mNV2 << " V2" << endl;
	cout << " " << mNT3 << " T3" << endl;
	cout << " " << mNUV << " UV points" << endl;
	cout << " " << mAveJD << " Average JD" << endl;

	// Copy data over to the OpenCL device.
	AllocateMemory();
	CopyToDevice(uv_points, vis, vis_err, vis_uv_ref, vis2, vis2_err, vis2_uv_ref, t3, t3_err, t3_uv_ref, t3_uv_sign);
}

unsigned int COILibData::CalculateOffset_Vis(void)
{
	return 0;
}

// Calculates the number of floats before the V2 data segment following the definition in COILibData.h
unsigned int COILibData::CalculateOffset_T3(unsigned int n_vis, unsigned int n_v2)
{
	return 2*n_vis + n_v2;
}

/// Calculates the number of floats before the V2 data segment following the definition in COILibData.h
unsigned int COILibData::CalculateOffset_V2(unsigned int n_vis)
{
	return 2*n_vis;
}

/// Copies the data which resides in OpenCL device memory into the specified buffers
void COILibData::CopyFromDevice(vector<pair<double,double> > & uv_points, cl_mem uv_buffer,
		valarray<complex<double>> & vis, cl_mem vis_buffer,
		valarray<pair<double,double>> & vis_err, cl_mem vis_err_buffer,
		vector<unsigned int> & vis_uv_ref, cl_mem vis_uv_ref_buffer,
		valarray<double> & vis2, cl_mem vis2_buffer,
		valarray<double> & vis2_err, cl_mem vis2_err_buffer,
		vector<unsigned int> & vis2_uv_ref, cl_mem vis2_uv_ref_buffer,
		valarray<complex<double>> & t3, cl_mem t3_buffer,
		valarray<pair<double,double> > & t3_err, cl_mem t3_err_buffer,
		vector<tuple<unsigned int, unsigned int, unsigned int>> & t3_uv_ref, cl_mem t3_uv_ref_buffer,
		vector<tuple<short, short, short>> & t3_uv_sign, cl_mem t3_uv_sign_buffer)
{

	int err = CL_SUCCESS;

	//
	// Procedure here:
	// 1) Allocate room and copy back OpenCL values to the CPU
	// 2) Copy values from OpenCL device to host memory, accounting for offsets (defined in COILIbData::CopyToDevice)
	//    Do so asynchronously and synchronize on a clFinish.
	// 3) Repackage OpenCL values into C++ data types specified in the function call
	//

	// If there is no data, simply return.
	if(!(mNData > 0))
		return;

	// #####
	// UV points
	valarray<cl_float2> t_uv_points(mNUV);
	err  = clEnqueueReadBuffer(mQueue, uv_buffer, CL_FALSE, 0, sizeof(cl_float2) * mNUV, &t_uv_points[0], 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy OI UV points from OpenCL device. COILibData::InitData.", err);

	// #####
	// Visibilities
	valarray<cl_float> t_vis(2*mNVis);
	valarray<cl_float> t_vis_err(2*mNVis);
	valarray<cl_uint> t_vis_uvref(mNVis);
	if(mNVis > 0)
	{
		// Copy the data.  No offset, this is always at the start of the buffer.
		err  = clEnqueueReadBuffer(mQueue, vis_buffer, CL_FALSE, 0, sizeof(cl_float) * 2*mNVis, &t_vis[0], 0, NULL, NULL);
		err |= clEnqueueReadBuffer(mQueue, vis_err_buffer, CL_FALSE, 0, sizeof(cl_float) * 2*mNVis, &t_vis_err[0], 0, NULL, NULL);
		err |= clEnqueueReadBuffer(mQueue, vis_uv_ref_buffer, CL_FALSE, 0, sizeof(cl_uint) * mNVis, &t_vis_uvref[0], 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not copy OI_VIS data from OpenCL device. COILibData::InitData", err);
	}

	// #####
	// V2
	valarray<cl_float> t_vis2(mNV2);
	valarray<cl_float> t_vis2_err(mNV2);
	valarray<cl_uint> t_vis2_uvref(mNV2);

	if(mNV2 > 0)
	{
		int offset = CalculateOffset_V2(mNVis);
		err  = clEnqueueReadBuffer(mQueue, vis2_buffer, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * mNV2, &t_vis2[0], 0, NULL, NULL);
		err |= clEnqueueReadBuffer(mQueue, vis2_err_buffer, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * mNV2, &t_vis2_err[0], 0, NULL, NULL);
		err |= clEnqueueReadBuffer(mQueue, vis2_uv_ref_buffer, CL_FALSE, 0, sizeof(cl_uint) * mNV2, &t_vis2_uvref[0], 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not copy OI_VIS2 data from OpenCL device. COILibData::InitData", err);
	}

	// #####
	// T3
	valarray<cl_float> t_t3(2*mNT3);
	valarray<cl_float> t_t3_err(2*mNT3);
	valarray<cl_uint4> t_t3_uvref(mNT3);
	valarray<cl_short4> t_t3_sign(mNT3);
	if(mNT3 > 0)
	{
		int offset = CalculateOffset_T3(mNVis, mNV2);
		err  = clEnqueueReadBuffer(mQueue, t3_buffer, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * 2*mNT3, &t_t3[0], 0, NULL, NULL);
		err |= clEnqueueReadBuffer(mQueue, t3_err_buffer, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * 2*mNT3, &t_t3_err[0], 0, NULL, NULL);
		err |= clEnqueueReadBuffer(mQueue, t3_uv_ref_buffer, CL_FALSE, 0, sizeof(cl_uint4) * mNT3, &t_t3_uvref[0], 0, NULL, NULL);
		err |= clEnqueueReadBuffer(mQueue, t3_uv_sign_buffer, CL_FALSE, 0, sizeof(cl_short4) * mNT3, &t_t3_sign[0], 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not copy OI_T3 data from OpenCL device. COILibData::InitData", err);
	}

	// Wait for the queue to process
	clFinish(mQueue);

	//
	// Now repackage the data into C++ data types.
	//

	// #####
	// UV points
	uv_points.resize(mNUV);
	for(int i = 0; i < mNUV; i++)
	{
		uv_points[i].first = t_uv_points[i].s0;
		uv_points[i].second = t_uv_points[i].s1;
	}

	// #####
	// Visibilities
	// Stored as [real(vis1), ..., real(visN), imag(vis1), ..., imag(visN)] within the mData_cl buffer
	vis.resize(mNVis);
	vis_err.resize(mNVis);
	for(int i = 0; i < mNVis; i++)
	{
		vis[i] = complex<double>(t_vis[i], t_vis[mNVis + i]);
		vis_err[i].first = t_vis_err[i];
		vis_err[i].second = t_vis_err[mNVis + i];
		vis_uv_ref[i] = t_vis_uvref[i];
	}

	// #####
	// V2:
	// Stored in linear order: [Vis2_1, ..., Vis2_N)] within the mData_cl buffer
	vis2.resize(mNV2);
	vis2_err.resize(mNV2);
	vis2_uv_ref.resize(mNV2);
	for(int i = 0; i < mNV2; i++)
	{
		vis2[i] = t_vis2[i];
		vis2_err[i] = t_vis2_err[i];
		vis2_uv_ref[i] = t_vis2_uvref[i];
	}

	// #####
	// T3:
	// Stored as [real(T3_1), ..., real(T3_N), imag(T3_1), ..., imag(T3_N)] within the mData_cl buffer
	// This choice encourages sequential memory access patterns in OpenCL
	t3.resize(mNT3);
	t3_err.resize(mNT3);
	t3_uv_ref.resize(mNT3);
	t3_uv_sign.resize(mNT3);
	for(int i = 0; i < mNT3; i++)
	{
		t3[i] = complex<double>(t_t3[i], t_t3[mNT3 + i]);

		t3_err[i].first = t_t3_err[i];
		t3_err[i].second = t_t3_err[mNT3 + i];

		t3_uv_ref[i] = make_tuple(t_t3_uvref[i].s0, t_t3_uvref[i].s1, t_t3_uvref[i].s2);
		t3_uv_sign[i] = make_tuple(t_t3_sign[i].s0, t_t3_sign[i].s1, t_t3_sign[i].s2);
	}
}


/// Copies the data which resides in host memory to the OpenCL device
void COILibData::CopyToDevice(const vector<pair<double,double> > & uv_points,
		const valarray<complex<double>> & vis, const valarray<pair<double,double>> & vis_err, const vector<unsigned int> & vis_uv_ref,
		const valarray<double> & vis2, const valarray<double> & vis2_err, const vector<unsigned int> & vis2_uv_ref,
		const valarray<complex<double>> & t3, const valarray<pair<double,double> > & t3_err,
		const vector<tuple<unsigned int, unsigned int, unsigned int>> & t3_uv_ref,
		const vector<tuple<short, short, short>> & t3_uv_sign)
{

	int err = CL_SUCCESS;

	//
	// Start uploading data to the OpenCL device. We need to copy the above data into
	// OpenCL data types to ensure things are moved correctly.
	//

	// Main data buffer
	// An array of cl_floats arranged as follows: [vis_real, vis_imag, v2, t3_amp, t3_phi]
	// The number of data must always be greater than zero.
	assert(mNData > 0);

	// #####
	// UV points:
	// Stored as pair of floats: [(u,v)_0, ..., (u,v)_N]

	// Allocate a buffer, fill it with UV points
	valarray<cl_float2> t_uv_points(mNUV);
	for(int i = 0; i < uv_points.size(); i++)
	{
		t_uv_points[i].s0 = uv_points[i].first;
		t_uv_points[i].s1 = uv_points[i].second;
	}
	// Pad with infinities after this (results in UV points having value of (0 + 0i))
	for(int i = uv_points.size(); i < mNUV; i++)
	{
		t_uv_points[i].s0 = numeric_limits<float>::infinity();
		t_uv_points[i].s1 = numeric_limits<float>::infinity();
	}

	// Copy over the UV points.  We MUST always have at least one UV point (otherwise the data would be nonsense).
	assert(mNUV > 0);
	err  = clEnqueueWriteBuffer(mQueue, mData_uv_cl, CL_FALSE, 0, sizeof(cl_float2) * mNUV, &t_uv_points[0], 0, NULL, NULL);
	COpenCL::CheckOCLError("Could not copy OI UV points to OpenCL device. COILibData::InitData.", err);

	// #####
	// Vis.
	// Stored as [real(vis1), ..., real(visN), imag(vis1), ..., imag(visN)] within the mData_cl buffer
	// This choice encourages sequential memory access patterns in OpenCL
	valarray<cl_float> t_vis(2*mNVis);
	valarray<cl_float> t_vis_err(2*mNVis);
	valarray<cl_uint> t_vis_uvref(mNVis);
	for(int i = 0; i < mNVis; i++)
	{
		t_vis[i] = real(vis[i]);
		t_vis[mNVis + i] = imag(vis[i]);
		t_vis_err[i] = vis_err[i].first;
		t_vis_err[mNVis + i] = vis_err[i].second;
		t_vis_uvref[i] = vis_uv_ref[i];
	}

	if(mNVis > 0)
	{
		// Copy the data.  No offset, this is always at the start of the buffer.
		err  = clEnqueueWriteBuffer(mQueue, mData_cl, CL_FALSE, 0, sizeof(cl_float) * 2*mNVis, &t_vis[0], 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(mQueue, mData_err_cl, CL_FALSE, 0, sizeof(cl_float) * 2*mNVis, &t_vis_err[0], 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(mQueue, mData_Vis_uv_ref, CL_FALSE, 0, sizeof(cl_uint) * mNVis, &t_vis_uvref[0], 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not copy OI_VIS data to OpenCL device. COILibData::InitData", err);
	}

	// #####
	// V2:
	// Stored in linear order: [Vis2_1, ..., Vis2_N)] within the mData_cl buffer
	valarray<cl_float> t_vis2(mNV2);
	valarray<cl_float> t_vis2_err(mNV2);
	valarray<cl_uint> t_vis2_uvref(mNV2);
	for(int i = 0; i < mNV2; i++)
	{
		t_vis2[i] = vis2[i];
		t_vis2_err[i] = vis2_err[i];
		t_vis2_uvref[i] = vis2_uv_ref[i];
	}

	if(mNV2 > 0)
	{
		int offset = CalculateOffset_V2(mNVis);
		err  = clEnqueueWriteBuffer(mQueue, mData_cl, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * mNV2, &t_vis2[0], 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(mQueue, mData_err_cl, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * mNV2, &t_vis2_err[0], 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(mQueue, mData_V2_uv_ref, CL_FALSE, 0, sizeof(cl_uint) * mNV2, &t_vis2_uvref[0], 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not copy OI_VIS2 data to OpenCL device. COILibData::InitData", err);
	}


	// #####
	// T3:
	// Stored as [real(T3_1), ..., real(T3_N), imag(T3_1), ..., imag(T3_N)] within the mData_cl buffer
	// This choice encourages sequential memory access patterns in OpenCL
	valarray<cl_float> t_t3(2*mNT3);
	valarray<cl_float> t_t3_err(2*mNT3);
	valarray<cl_uint4> t_t3_uvref(mNT3);
	valarray<cl_short4> t_t3_sign(mNT3);
	for(int i = 0; i < mNT3; i++)
	{
		t_t3[i] = real(t3[i]);
		t_t3[mNT3 + i] = imag(t3[i]);
		t_t3_err[i] = t3_err[i].first;
		t_t3_err[mNT3 + i] = t3_err[i].second;

		// UV references
		t_t3_uvref[i].s0 = get<0>(t3_uv_ref[i]);
		t_t3_uvref[i].s1 = get<1>(t3_uv_ref[i]);
		t_t3_uvref[i].s2 = get<2>(t3_uv_ref[i]);
		t_t3_uvref[i].s3 = 0;

		// T3 uv signs:
		t_t3_sign[i].s0 = get<0>(t3_uv_sign[i]);
		t_t3_sign[i].s1 = get<1>(t3_uv_sign[i]);
		t_t3_sign[i].s2 = get<2>(t3_uv_sign[i]);
		t_t3_sign[i].s3 = 0;
	}

	if(mNT3 > 0)
	{
		int offset = CalculateOffset_T3(mNVis, mNV2);
		err  = clEnqueueWriteBuffer(mQueue, mData_cl, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * 2*mNT3, &t_t3[0], 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(mQueue, mData_err_cl, CL_FALSE, sizeof(cl_float) * offset, sizeof(cl_float) * 2*mNT3, &t_t3_err[0], 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(mQueue, mData_T3_uv_ref, CL_FALSE, 0, sizeof(cl_uint4) * mNT3, &t_t3_uvref[0], 0, NULL, NULL);
		err |= clEnqueueWriteBuffer(mQueue, mData_T3_sign, CL_FALSE, 0, sizeof(cl_short4) * mNT3, &t_t3_sign[0], 0, NULL, NULL);
		COpenCL::CheckOCLError("Could not copy OI_T3 data to OpenCL device. COILibData::InitData", err);
	}

	// Wait for the queue to process
	clFinish(mQueue);
}

/// Copies up to n of the data from the OpenCL device to output.
void COILibData::GetData(int data_num, float * output, unsigned int & n)
{
	int err = CL_SUCCESS;

	unsigned int n_data = TotalBufferSize(mNVis, mNV2, mNT3);

	// Copy the minimum of the output buffer size and n_data
	n = min(n, n_data);
	err |= clEnqueueReadBuffer(mQueue, mData_cl, CL_FALSE, 0, sizeof(cl_float) * n, output, 0, NULL, NULL);
	clFinish(mQueue);
}

/// Copies up to n of the data uncertainties from the OpenCL device to output.
void COILibData::GetDataUncertainties(int data_num, float * output, unsigned int & n)
{
	int err = CL_SUCCESS;

	unsigned int n_data = TotalBufferSize(mNVis, mNV2, mNT3);

	// Copy the minimum of the output buffer size and n_data
	n = min(n, n_data);
	err |= clEnqueueReadBuffer(mQueue, mData_err_cl, CL_FALSE, 0, sizeof(cl_float) * n, output, 0, NULL, NULL);
	clFinish(mQueue);
}

// Calculate the total number of elements in the data buffer following the data storage definition in COILibData.h
unsigned int COILibData::TotalBufferSize(unsigned int n_vis, unsigned int n_v2, unsigned int n_t3)
{
	return 2*n_vis + n_v2 + 2*n_t3;
}

/// Replaces the currently loaded data set with another of the exact same size stored in new_data
/// this function is useful for bootstrapping.
/// Function throws exceptions if new_data does not match the size of the existing data exactly.
void COILibData::Replace(const OIDataList & new_data)
{
	// This is essentially a repeat of the InitData function, except we check that the total number of data
	// doesn't change.

	// Export the data from OIDataList to something we can use here.
	vector<pair<double,double> > uv_points;
	valarray<complex<double>> vis;
	valarray<pair<double,double>> vis_err;
	vector<unsigned int> vis_uv_ref;
	valarray<double> vis2;
	valarray<double> vis2_err;
	vector<unsigned int> vis2_uv_ref;
	valarray<complex<double>> t3;
	valarray<pair<double,double> > t3_err;
	vector<tuple<unsigned int, unsigned int, unsigned int>> t3_uv_ref;
	vector<tuple<short, short, short>> t3_uv_sign;

	ccoifits::Export_MinUV(new_data, uv_points, vis, vis_err, vis_uv_ref, vis2, vis2_err, vis2_uv_ref, t3, t3_err, t3_uv_ref, t3_uv_sign);

	unsigned int n_vis = vis.size();
	unsigned int n_v2 = vis2.size();
	unsigned int n_t3 = t3.size();
	unsigned int n_uv = uv_points.size();
	unsigned int total_size = TotalBufferSize(n_vis, n_v2, n_t3);

	// When data is replaced (as is often done in bootstrapping), the number of UV points can be smaller
	if(n_uv > mNUV)
	{
		throw length_error("Number of UV points exceeds allocated size.");
	}

	// But for statistical information to make sense, the total number of data must match exactly.
	if(n_vis != mNVis)
	{
		throw length_error("Number of Vis points does not match allocated size");
	}

	if(n_v2 != mNV2)
	{
		throw length_error("Number of V2 points does not match allocated size");
	}

	if(n_t3 != mNT3)
	{
		throw length_error("Number of T3 points does not match allocated size");
	}

	if(total_size != mNData)
	{
		throw length_error("Size of data allocation does not match allocated size");
	}

	// Copy data over to the OpenCL device.
	CopyToDevice(uv_points, vis, vis_err, vis_uv_ref, vis2, vis2_err, vis2_uv_ref, t3, t3_err, t3_uv_ref, t3_uv_sign);
}

} // namespace liboi
