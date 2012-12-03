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

COILibDataList::COILibDataList()
{
	// TODO Auto-generated constructor stub

}

COILibDataList::~COILibDataList()
{

}

OIDataList COILibDataList::GetData(unsigned int data_num)
{
	if(data_num < this->size())
		return this->at(data_num)->GetData();

	return OIDataList();
}

/// Returns the total number of data points (UV + T3) in all data sets
int COILibDataList::GetNData()
{
	int tmp = 0;
    for(vector<COILibDataPtr>::iterator it = this->begin(); it != this->end(); ++it)
    {
    	tmp += (*it)->GetNumV2() + 2 * (*it)->GetNumT3();
    }

    return tmp;
}

/// Returns the total number of float/double entries allocated on the OpenCL device
/// for all data sets.
int COILibDataList::GetNDataAllocated()
{
	int tmp = 0;
	for(unsigned int i = 0; i < this->size(); i++)
		tmp += GetNDataAllocated(i);

    return tmp;
}

/// Returns the size of the data_num's allocated data block.
int COILibDataList::GetNDataAllocated(unsigned int data_num)
{
	if(data_num < this->size())
		return this->at(data_num)->GetNumV2() + 2 * this->at(data_num)->GetNumT3();

	return 0;
}

/// Finds the maximum number of data points (Vis2 + T3) and returns that number.
int COILibDataList::MaxNumData()
{
	int tmp;
	int max = 0;
    for(vector<COILibDataPtr>::iterator it = this->begin(); it != this->end(); ++it)
    {
    	tmp = (*it)->GetNumV2() + 2 * (*it)->GetNumT3();
    	if(tmp > max)
    		max = tmp;
    }

    return max;
}

/// Finds the maximum number of data points (Vis2 + T3) and returns that number.
int COILibDataList::MaxUVPoints()
{
	int tmp;
	int max = 0;
    for(vector<COILibDataPtr>::iterator it = this->begin(); it != this->end(); ++it)
    {
    	tmp = (*it)->GetNumUV();
    	if(tmp > max)
    		max = tmp;
    }

    return max;
}


/// Reads in an OIFITS file, returns a COILibData object.
void COILibDataList::ReadFile(string filename, cl_context context, cl_command_queue queue)
{
	COILibDataPtr tmp(new COILibData(filename, context, queue));
	this->push_back(tmp);
}

/// Removes the specified data file from device and host memory
void COILibDataList::RemoveData(unsigned int data_num)
{
	if(data_num < this->size())
		this->erase(this->begin() + data_num);
}
