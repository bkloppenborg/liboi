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

using namespace std;

class COILibDataList
{
protected:
	vector<COILibData *> data;

public:
	// Operator overloads:
	COILibData * operator[](int i) { return data[i]; }

public:
	COILibDataList();
	~COILibDataList();

	void Append(COILibData * data);

	void CopyToOpenCLDevice(cl_context context, cl_command_queue queue);

	int MaxNumData();

	void ReadFile(string filename);


};

#endif /* COILIBDATALIST_H_ */
