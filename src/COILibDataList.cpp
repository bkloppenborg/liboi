/*
 * COILibDataList.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 *
 *  Description:
 *      Storage list for OIFITS data.
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

#include "COILibDataList.h"

namespace liboi
{

COILibDataList::COILibDataList()
{
	// TODO Auto-generated constructor stub

}

COILibDataList::~COILibDataList()
{

}

COILibDataPtr COILibDataList::at(unsigned int id)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	return mDataList.at(id);
}

void COILibDataList::ExportData(unsigned int data_num, string file_basename, cl_mem simulated_data)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	try
	{
		mDataList[data_num]->ExportData(file_basename, simulated_data);
	}
	catch(...)
	{
		// do nothing.
	}
}

OIDataList COILibDataList::GetData(unsigned int data_num)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	try
	{
		return mDataList[data_num]->GetData();
	}
	catch(...)
	{
		return OIDataList();
	}
}

void COILibDataList::GetData(int data_num, float * output, unsigned int & n)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	try
	{
		mDataList[data_num]->GetData(data_num, output, n);
	}
	catch(...)
	{
		return;
	}
}

void COILibDataList::GetDataUncertainties(int data_num, float * output, unsigned int & n)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	try
	{
		mDataList[data_num]->GetDataUncertainties(data_num, output, n);
	}
	catch(...)
	{
		return;
	}
}

/// Returns the total number of data points (UV + T3) in all data sets
int COILibDataList::GetNData()
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	// Calculate the total sum of the number of data points.
	int tmp = 0;
	for(auto data: mDataList)
		tmp += data->GetNumData();

    return tmp;
}

/// Returns the total number of float/double entries allocated on the OpenCL device
/// for all data sets.
int COILibDataList::GetNDataAllocated()
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	int tmp = 0;
	for(auto data: mDataList)
		tmp += data->GetNumData();

    return tmp;
}

/// Returns the size of the data_num's allocated data block.
int COILibDataList::GetNDataAllocated(unsigned int data_num)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	unsigned int n_data = 0;

	try
	{
		return mDataList[data_num]->GetNumData();
	}
	catch(...)
	{
		return 0;
	}
}

/// Reads in an OIFITS file, returns a COILibData object.
void COILibDataList::LoadData(string filename, cl_context context, cl_command_queue queue)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	COILibDataPtr tmp(new COILibData(filename, context, queue));
	mDataList.push_back(tmp);
}

/// Reads in an OIFITS file, returns a COILibData object.
void COILibDataList::LoadData(const OIDataList & data, cl_context context, cl_command_queue queue)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	COILibDataPtr tmp(new COILibData(data, context, queue));
	mDataList.push_back(tmp);
}

/// Finds the maximum number of data points (Vis2 + T3) and returns that number.
int COILibDataList::MaxNumData()
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	int tmp;
	int max = 0;
    for(auto data: mDataList)
    {
    	tmp = data->GetNumData();
    	if(tmp > max)
    		max = tmp;
    }

    return max;
}

/// Finds the maximum number of data points (Vis2 + T3) and returns that number.
int COILibDataList::MaxUVPoints()
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	int tmp;
	int max = 0;
    for(auto data: mDataList)
    {
    	tmp = data->GetNumUV();
    	if(tmp > max)
    		max = tmp;
    }

    return max;
}

/// Removes the specified data file from device and host memory
void COILibDataList::RemoveData(unsigned int data_num)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	try
	{
		mDataList.erase(mDataList.begin() + data_num);
	}
	catch(...)
	{
		// do nothing
	}
}

// Replaces the data stored in old_data_id with the new_data. The data must be of the same eize
void COILibDataList::ReplaceData(unsigned int old_data_id, const OIDataList & new_data, cl_context context, cl_command_queue queue)
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	COILibDataPtr temp = mDataList[old_data_id];
	temp->Replace(new_data);
}

unsigned int COILibDataList::size()
{
	// Lock the data, automatically unlocks
	lock_guard<mutex> lock(mDataMutex);

	return mDataList.size();
}

} /* namespace liboi */
