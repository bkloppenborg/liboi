/*
 * liboi_benchmark.h
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */

#ifndef LIBOI_BENCHMARK_H_
#define LIBOI_BENCHMARK_H_

#include <string>

using namespace std;

int main(int argc, char **argv);

int GetMilliCount();
int GetMilliSpan( int nTimeStart );

#endif /* LIBOI_BENCHMARK_H_ */
