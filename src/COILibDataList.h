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

// TODO: Switch to native C++11 when Apple clang is based on 3.2svn
// clang 3.1 does not (fully) support c++11 features so when compiled by clang
// we switch to boost for threads, mutexes, and locks.
#ifdef __clang__
#include <boost/thread.hpp>
#include <boost/signals2/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
using namespace boost;
namespace ns = boost;

#else
#include <memory>
#include <mutex>
#include <thread>
namespace ns = std;

#endif

using namespace std;

namespace liboi
{

class COILibData;
typedef ns::shared_ptr<COILibData> COILibDataPtr;

class COILibDataList
{
protected:
	vector<COILibDataPtr> mDataList;
	mutex mDataMutex;

public:
	COILibDataList();
	virtual ~COILibDataList();

	COILibDataPtr at(unsigned int id);

	void ExportData(unsigned int data_num, string file_basename, cl_mem simulated_data);

	OIDataList GetData(unsigned int data_num);
	void GetData(int data_num, float * output, unsigned int & n);
	void GetDataUncertainties(int data_num, float * output, unsigned int & n);
	int GetNData();
	int GetNDataAllocated();
	int GetNDataAllocated(unsigned int data_num);

	void LoadData(string filename, cl_context context, cl_command_queue queue);
	void LoadData(const OIDataList & data, cl_context context, cl_command_queue queue);

	int MaxNumData();
	int MaxUVPoints();

	void RemoveData(unsigned int data_num);
	void ReplaceData(unsigned int old_data_id, const OIDataList & new_data, cl_context context, cl_command_queue queue);

	unsigned int size();
};
} /* namespace liboi */

#endif /* COILIBDATALIST_H_ */
