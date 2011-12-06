/*
 * COILibDataList.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#include "COILibDataList.h"

COILibDataList::COILibDataList()
{
	// TODO Auto-generated constructor stub

}

COILibDataList::~COILibDataList()
{
	// Deallocate the models
	for(int i = data.size() - 1; i > -1; i--)
		delete data[i];
}

COILibDataList::Append(COILibData * data)
{
	this->data.push_back(data);
}

/// Finds the maximum number of data points (Vis2 + T3) and returns that number.
int COILibDataList::MaxNumData()
{
	int tmp;
	int max = 0;
    for(vector<COILibData*>::iterator it = data.begin(); it != data.end(); ++it)
    {
    	tmp = (*it)->GetNumV2() + (*it)->GetNumT3();
    	if(tmp > max)
    		max = tmp;
    }

    return max;
}


/// Reads in an OIFITS file, returns a COILibData object.
void COILibDataList::ReadFile(string filename)
{
	// TODO: Right now this routine uses getoifits (Fabien Baron) and oifitslib (John Young) to read in the data
	// we can probably get a performance increase on the GPU by sorting the data intelligently on load.
	// We'll need to implement a new reading function to do this.
	// Note: If we reorder the data, we'll need to make sure the V2 and T3 kernels still understand where their data is at.

	// From GPAIR, Allocate storage for OIFITS data
	oi_usersel usersel;
	oi_data * tmp = new oi_data();
	int status;

	// From GPAIR, read_oifits
	strcpy(usersel.file, filename.c_str());
	get_oi_fits_selection(&usersel, &status);
	get_oi_fits_data(&usersel, tmp, &status);
	printf("OIFITS File read\n");

	Append(new COILibData(tmp));
}
