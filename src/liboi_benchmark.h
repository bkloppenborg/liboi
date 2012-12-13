/*
 * liboi_benchmark.h
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */

#ifndef LIBOI_BENCHMARK_H_
#define LIBOI_BENCHMARK_H_

#include <string>
#include "liboi.hpp"

using namespace std;

int GetMilliCount();
int GetMilliSpan( int nTimeStart );

int main(int argc, char **argv);

void PrintHelp();

int RunBenchmark(cl_device_type device_type, string exe_path,
		unsigned int image_width, unsigned int image_height, unsigned int image_depth,
		float image_scale);

#endif /* LIBOI_BENCHMARK_H_ */
