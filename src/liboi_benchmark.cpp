/*
 * liboi_benchmark.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: bkloppenborg
 */

#include "liboi_benchmark.h"

#include <stdexcept>
// This probably isn't cross-platform compatible.
#include <sys/timeb.h>
using namespace std;

#include "PathFind.hpp"
#include "models/CPointSource.h"
#include "models/CUniformDisk.h"

using namespace liboi;

int main(int argc, char **argv)
{
	// Find the path to the current executable
	string exe = FindExecutable();
	size_t folder_end = exe.find_last_of("/\\");
	string exe_path = exe.substr(0,folder_end+1);

	// Setup properties of the image
	int image_width = 128;
	int image_height = 128;
	int image_depth = 1;
	float image_scale = 0.025;	// mas/pixel
	cl_device_type device_type = CL_DEVICE_TYPE_GPU;
	int n_iterations = 1000;
	int n_uv = 500;

	for(int i = 0; i < argc; i++)
	{
		if (string(argv[i]) == "-h")
		{
			PrintHelp();
			return 0;
		}

		if (string(argv[i]) == "-cpu")
		{
			device_type = CL_DEVICE_TYPE_CPU;
		}

		if (string(argv[i]) == "-width" && i + 1 < argc)
		{
			image_width = atoi(argv[i+1]);
			if(image_width < 1)
				throw runtime_error("Image width must be greater than 0");
		}

		if (string(argv[i]) == "-height" && i + 1 < argc)
		{
			image_height = atoi(argv[i+1]);
			if(image_height < 1)
				throw runtime_error("Image height must be greater than 0");
		}

		if (string(argv[i]) == "-s" && i + 1 < argc)
		{
			image_scale = atof(argv[i+1]);
			if(image_scale < 1)
				throw runtime_error("Image scale must be greater than 0");
		}

		if (string(argv[i]) == "-iter" && i + 1 < argc)
		{
			n_iterations = atoi(argv[i+1]);
			if(n_iterations < 1)
				throw runtime_error("Number of iterations to time must be greater than 0");
		}

		if (string(argv[i]) == "-nuv" && i + 1 < argc)
		{
			n_uv = atoi(argv[i+1]);
			if(n_uv < 1)
				throw runtime_error("Number of UV points must be greater than 0");
		}

	}

	RunBenchmark(device_type, exe_path, image_width, image_height, image_depth, image_scale, n_iterations, n_uv);
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

void PrintHelp()
{
	cout << "Running liboi_benchmark:" << endl;
	cout << " liboi_benchmark [...]" << endl;
	cout << endl;
	cout << "Options:" << endl;
	cout << " -cpu       Runs benchmark on the CPU [default: run on GPU]" << endl;
	cout << " -width N   Sets the image width (int, N > 0) [default: 128 pixel]" << endl;
	cout << " -height N  Sets the image width (int, N > 0) [default: 128 pixel]" << endl;
	cout << " -s N       Sets the image scale (float, N > 0) [default: 0.025 mas/pixel]" << endl;
	cout << " -iter N    Number of iterations to time (int, N > 0) [default: 1000]" << endl;
//	cout << " -nuv N     Sets the number of UV points (int, N > 0) [default: 500]" << endl;
}

int RunBenchmark(cl_device_type device_type, string exe_path,
		unsigned int image_width, unsigned int image_height, unsigned int image_depth, float image_scale,
		unsigned int n_iterations, unsigned int n_uv)
{
//	CUniformDisk ps(image_width, image_height, image_scale, 1, 0, 0);
	CPointSource ps(image_width, image_height, image_scale);
	// Setup the model, make an image and copy it over to a float buffer.
	//CPointSource ps(image_width, image_height, image_scale);
	valarray<double> temp = ps.GetImage();
	valarray<float> image(image_width * image_height * image_depth);
	for(int i = 0; i < temp.size(); i++)
		image[i] = float(temp[i]);

	// Get and OpenCL device:
	CLibOI liboi(device_type);

	// Print some nice information
	cout << "Running Benchmark with: " << endl << endl;
	liboi.PrintDeviceInfo();
	cout << endl << endl;

	// Setup some information:
	liboi.SetKernelSourcePath(exe_path + "kernels/");
	liboi.SetImageInfo(image_width, image_height, image_depth, image_scale);
	liboi.SetImageSource(&image[0]);

	// Load some data
	liboi.LoadData(exe_path + "../samples/PointSource_noise.oifits");

	// Init the device.
	cout << "Building kernels." << endl;
	liboi.Init();

	// Now run liboi as fast as possible:
	float chi2 = 0;
	double time = 0;

	cout << "Starting benchmark." << endl;

	int start = GetMilliCount();

	// Run the iterations
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

	liboi.ExportImage("!/tmp/circle.fits");

	return 0;
}
