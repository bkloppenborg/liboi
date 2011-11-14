/*
 * CRoutine_Reduce.h
 *
 *  Created on: Nov 14, 2011
 *      Author: bkloppenborg
 */

#ifndef CROUTINE_REDUCE_H_
#define CROUTINE_REDUCE_H_

#include "COpenCLRoutine.h"

class CRoutine_Reduce: public COpenCLRoutine
{
protected:
	string kernel_source;
public:
	CRoutine_Reduce();
	virtual ~CRoutine_Reduce();

protected:
	void CreateReductionPasscounts();
	void BuildKernels();
public:
	float ComputeSum(bool copy_back);


};

#endif /* CROUTINE_REDUCE_H_ */
