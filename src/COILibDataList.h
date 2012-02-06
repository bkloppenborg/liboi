/*
 * COILibDataList.h
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
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
	// Operator overloads:
	//COILibData * operator[](int i) { return mList[i]; }

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
};

#endif /* COILIBDATALIST_H_ */
