/*
 * main.cpp
 *
Created on: Nov 12, 2011
    Author: bkloppenborg
 */

#ifndef MAIN_CPP_
#define MAIN_CPP_

#include <cstdio>
#include "COpenCL.h"

using namespace std;

// The main routine.
int main(int argc, char *argv[])
{
	COpenCL tmp = COpenCL();
	tmp.Init(CL_DEVICE_TYPE_GPU);


	return 0;
}



#endif /* MAIN_CPP_ */
