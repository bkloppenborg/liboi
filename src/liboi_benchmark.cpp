/*
 * liboi_benchmark.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */

#include "liboi_benchmark.h"

#include "liboi.hpp"
#include "PathFinding.h"
#include "models/CPointSource.h"

// This probably isn't cross-platform compatable.
#include <sys/timeb.h>
using namespace std;

using namespace liboi;

int main(int argc, char **argv)
{
	// Find the path to the current executable
	string exe = FindExecutable();
	// Find the directory (the name of this program is "liboi_benchmark", so just strip off 16 characters)
	string exe_path = exe.substr(0, exe.size() - 15);

	// Setup properties of the image
	unsigned int image_width = 128;
	unsigned int image_height = 128;
	unsigned int image_depth = 1;
	float image_scale = 0.025;	// mas/pixel

	// Setup the model, make an image and copy it over to a float buffer.
	CPointSource ps(image_width, image_height, image_scale);
	valarray<double> temp = ps.GetImage(image_width, image_height, image_scale);
	valarray<float> image(image_width * image_height * image_depth);
	for(int i = 0; i < temp.size(); i++)
		image[i] = float(temp[i]);

	// Init the OpenCL device
	CLibOI liboi(CL_DEVICE_TYPE_GPU);
	liboi.SetKernelSourcePath(exe_path + "kernels/");
	liboi.SetImageInfo(image_width, image_height, image_depth, image_scale);
	liboi.SetImageSource(&image[0]);

	// Load some data
	liboi.LoadData(exe_path + "../samples/PointSource_noise.oifits");

	// Init the device.
	liboi.Init();

	// Now run liboi as fast as possible:
	float chi2 = 0;
	int n_iterations = 1000;
	double time = 0;

	int start = GetMilliCount();

	for(int i = 0; i < n_iterations; i++)
	{
		liboi.CopyImageToBuffer(0);
		chi2 = liboi.ImageToChi2(0);

		if(i % 100 == 0)
			cout << "Iteration " << i << " Chi2: " << chi2 << endl;
	}

	// Calculate the time, print out a nice message.
	time = double(GetMilliSpan(start)) / 1000;
	cout << "Benchmark Test completed!" << endl;
	cout << n_iterations << " iterations in " << time << " seconds. Throughput " << n_iterations/time << " iterations/sec.\n" << endl;

	return 0;
}

int GetMilliCount()
{
	// Something like GetTickCount but portable
	// It rolls over every ~ 12.1 days (0x100000/24/60/60)
	// Use GetMilliSpan to correct for rollover
	timeb tb;
	ftime( &tb );
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

int GetMilliSpan( int nTimeStart )
{
	int nSpan = GetMilliCount() - nTimeStart;
	if ( nSpan < 0 )
		nSpan += 0x100000 * 1000;
	return nSpan;
}
