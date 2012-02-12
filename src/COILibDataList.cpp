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
//	// Deallocate the models
//	for(int i = data.size() - 1; i > -1; i--)
//		delete data[i];
}

//void COILibDataList::Append(COILibData * data)
//{
//	this->data.push_back(data);
//}

/// Copies all data sources to the OpenCL device using the specified command queue.
void COILibDataList::CopyToOpenCLDevice(cl_context context, cl_command_queue queue)
{
    for(vector<COILibData*>::iterator it = mList.begin(); it != mList.end(); ++it)
    {
    	(*it)->CopyToOpenCLDevice(context, queue);
    }
}

/// Returns the total number of data points (UV + T3) in all data sets
int COILibDataList::GetNData()
{
	int tmp = 0;
    for(vector<COILibData*>::iterator it = mList.begin(); it != mList.end(); ++it)
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
	for(unsigned int i = 0; i < mList.size(); i++)
		tmp += GetNDataAllocated(i);

    return tmp;
}

/// Returns the size of the data_num's allocated data block.
int COILibDataList::GetNDataAllocated(unsigned int data_num)
{
	if(data_num < mList.size())
		return mList[data_num]->GetNumV2() + 2 * mList[data_num]->GetNumT3();

	return 0;
}

/// Finds the maximum number of data points (Vis2 + T3) and returns that number.
int COILibDataList::MaxNumData()
{
	int tmp;
	int max = 0;
    for(vector<COILibData*>::iterator it = mList.begin(); it != mList.end(); ++it)
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
    for(vector<COILibData*>::iterator it = mList.begin(); it != mList.end(); ++it)
    {
    	tmp = (*it)->GetNumUV();
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

	Append(new COILibData(tmp, filename));
}
