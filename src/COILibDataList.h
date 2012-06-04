/*
 * COILibDataList.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
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

#ifndef COILIBDATALIST_H_
#define COILIBDATALIST_H_

#include <string>

#include "COILibData.h"
#include "CVectorList.h"

using namespace std;

class COILibDataList : public CVectorList<COILibData*>
{

public:


public:
	COILibDataList();
	~COILibDataList();

	void CopyToOpenCLDevice(cl_context context, cl_command_queue queue);

	int GetNData();
	int GetNDataAllocated();
	int GetNDataAllocated(unsigned int data_num);

	int MaxNumData();
	int MaxUVPoints();

	void ReadFile(string filename);
	void RemoveData(unsigned int data_num);
};

#endif /* COILIBDATALIST_H_ */
