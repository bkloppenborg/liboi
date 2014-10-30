# OpenCL Interferometry Library (liboi)

## Description

The OpenCL Interferometry Library (liboi) is a C / C++ library that aims to 
provide software developers with access to routines that are commonly used 
in interferometry.  The software heavily relies on the heterogeneous computing environment targeted by the Open Compute Language (OpenCL) to target a wide range of traditional and multi-core CPUs; servers, hand-held/embedded devices, specialized hardware, and Graphical Processing Units (GPUs).

## Features

The library currently provides:
* OpenGL / OpenCL Interop (copy OpenGL image to OpenCL buffers)
* Image to Fourier transform via. a discrete Fourier Transform
* Fourier transform to interferometric data (visibility squared, bispectra)
* Image data to chi, chi squared, and log(likelihood).

## Installing prerequisites

### Apple / OS X

To use `liboi` on OS X, the following are required:

* OpenCL 1.1 support (OS X 10.7 or higher)
* gcc 4.7.4 or later &dagger;
* cmake 2.8 or higher
* cfitsio and ccfits

The OS can be installed/upgraded through the Appstore. Several of the additional 
required libraries can be installed through [MacPorts](http://www.macports.org/):

    sudo port install gcc47
    sudo port install cmake
    sudo port install cfitsio
    sudo port install git

Please note that Apple may have installed a fake version of `gcc` located at
`/opt/local/bin/gcc` that is a wrapper for clang. Besure to specify that you
want to use the macport-installed compiler using

    export CC=/opt/local/bin/gcc-mp-4.7
    export CXX=/opt/local/bin/g++-mp-4.7

before running and `./configure` or `cmake` commands!

Next you will need to download and install 
[ccfits](http://heasarc.gsfc.nasa.gov/fitsio/CCfits/).
You can do this using commands similar to the following

    wget http://heasarc.gsfc.nasa.gov/fitsio/CCfits/CCfits-2.4.tar.gz
    tar xvzf CCfits-2.4.tar.gz
    cd CCfits
    ./configure --prefix=/opt/local --with-cfitsio=/opt/local
    make
    sudo make install

After this follow the installation instructions below.

&dagger; When we last attempted to compile `liboi` on a Mac the machine had
Xcode 4.6 (which included apple `clang` 4.2 which is based on `llvm-clang` 3.2svn)
that did not have full C++11 support. We do not have access to an Apple system
for development, thus we cannot try compiling on a more recent system. If you
wish to help get `liboi` running on a Mac please contact us!

### Debian/Ubuntu

To use `liboi` on a Debian / Ubuntu system, the following are should be 
installed:

* gcc and g++ v4.6.3 (or later)
* cmake v2.8
* cfitsio, ccfits
* An OpenGL library (optional, enables OpenCL-OpenGL interoperability)
* An OpenCL 1.1 compliant device and library

Most of these packages can be easily installed through `apt-get`. First the
compiler, cmake, cfitsio, and ccfits:

    sudo apt-get install build-essential g++ cmake libccfits0 libccfits-dev git
    
To enable OpenCL-OpenGL interoperability you should also install an OpenGL
library. This *should* install the prerequisites:
    
    sudo apt-get install libglu1-mesa libglu1-mesa-dev
    
For OpenCL capabilities you need to install drivers for your device, OpenCL
headers, and an OpenCL Installable Client Driver (ICD) loader. These *should*
supply everything you need to compile `liboi`. Unfortunately each vendor has
specific installation instructions which we list below. After the OpenCL 
implementation is installed, ensure that the `cl.hpp` file was installed
along with the OpenCL drivers. If it was not installed copy the `liboi/include/cl.hpp`
into your system's OpenCL include directory.

After OpenCL support are installed follow the building instructions below.

#### NVIDIA GPUS:

On systems that do not have NVIDIA unified virtual memory (e.g. Ubuntu 13.10
and earlier) you only need the display drivers (e.g. one of
`nvidia-current`, `nvidia-304`, or `nvidia-331`), OpenCL headers, and an OpenCL
ICD loader:

    sudo apt-get install nvidia-319 opencl-headers nvidia-opencl-dev
    
On later Ubuntu systems you will need to add the UVM package. You may also
need to install special modprobe rules for the NVIDIA UVM drivers. These can be
found in the following packages:

    sudo apt-get install nvidia-331 nvidia-331-uvm nvidia-modprobe opencl-headers nvidia-opencl-dev

If you prefer, you can install the [drivers from NVIDIA](www.nvidia.com/drivers) instead.

#### AMD GPUs and AMD CPUs:

    sudo apt-get install fglrx opencl-headers
    
or if you so choose you can install the 
[AMD graphic drivers](http://support.amd.com/us/gpudownload/Pages/index.aspx), 
and then install the 
[OpenCL SDK] (http://developer.amd.com/tools/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/downloads/)

#### Intel CPUs and GPUs

For Intel CPUs you need to install the [Intel OpenCL SDK](http://software.intel.com/en-us/vcsource/tools/opencl-sdk)

Unlike on Windows and Apple systems, Intel does not provide support for OpenCL
on their GPUs with their display drivers. Instead an open source project
called [Beignet](http://www.freedesktop.org/wiki/Software/Beignet/) is filling
the gap.

We have successfully compiled `liboi` and verified that it functions with 
`Beignet`, however doing so was not a straightforward process.
See the [beignet.md](beignet.md) document in this directory for further details.


## Checkout / getting a copy of LibOI source code

After installing the aforementioned prerequisites, you simply need to checkout
a copy of LibOI

    git clone https://github.com/bkloppenborg/liboi.git
    cd liboi
    git submodule update --init

If you wish to have the bleeding-edge development version of `liboi` (which
often includes the latest features and bugfixes) checkout the development version:

    git checkout develop

otherwise you can stay on the default `master` branch.

## Building instructions

After you have obtained a copy of the source and initialized the submodules, 
simply 

    cd build
    cmake ..
    make

If you have installed a library in a non-standard location, please see the
[Overriding library locations][] section below. If you have any errors in the
compilation steps, please contact us. 
(If you are on a Apple machine be sure to set the `export` lines mentioned above!)

## Overriding library locations

If you have installed a library in a non-standard location, it may be necessary
to override the library installation location. The following environmental 
variables are checked by CMake when building:

```
CFITSIO_ROOT_DIR    - path to directory above cfitstio.h and libcfitsio.*
CCFITS_ROOT_DIR     - path to directory above CCfits/ (the folder)
OPENCL_ROOT_DIR     - path to directory containing OpenCL
                      that is OpenCL/cl.hpp (Apple) or CL/cl.hpp (everyone else)
```
   
    
These can be set by typing `export VARIABLE=/path/to/directory` before calling
`cmake` in the building instructions above. CMake should indicate that the
directory you specified is used, rather than the default search path on your
computer.

## Testing and benchmarking

After compiling `liboi` it is useful to test that `liboi` is functioning correcly
on your hardware. For this purpose we have packaged the `liboi_tests` and
`liboi_benchmark` program in the `liboi/bin` directory.

The `liboi_tests` program executes a series of unit tests and compares the result
of `liboi`'s calculations with analytic results. 
By default `liboi_tests` runs on the first OpenCL-compatabile GPU it can find.
You can also have it run on an OpenCL-supporting CPU by specifying the `-cpu` option.
See `liboi_tests -h` for more information.

When you execute `liboi_tests`you should see something like this:

    liboi/bin$ ./liboi_tests 
    [==========] Running 34 tests from 12 test cases.
    [----------] Global test environment set-up.
    [----------] 14 tests from ChiTest
    [ RUN      ] ChiTest.CPU_Chi_ZERO
    [       OK ] ChiTest.CPU_Chi_ZERO (3 ms)
    ...
    [----------] Global test environment tear-down
    [==========] 34 tests from 12 test cases ran. (5987 ms total)
    [  PASSED  ] 34 tests.
    
In the ideal situation all tests will pass, but frequently a few tests will fail.
In partcular, the following tests often fail for the following reasons:

* `CRoutine_Sum_NVidia.CL_Sum_CPU_CHECK` executes a parallel sum. Due to some
  optimizations specific to NVidia GPUs, this test seems to always fail on
  non-NVidia hardware. Thus if you have an Intel or AMD GPU, don't worry if this
  test fails.
* `CRoutine_DFT.CL_UniformDisk` this test compares the real (indexed as `.s[0]`)
  and imaginary (indexed `.s[1]`) components of an analytical uniform disk
  Fourier transform with a discrete Fourier transform of an image. The analytic
  and DFT answers are required to match at a 3% or better level. Out of the 10
  UV points tested, it is common for a few to fail, particularly on older
  hardware.
* `CRoutine_DFT.CL_DFT_LARGE_UNEVEN_N_UV_POINTS` compares an analytical and DFT
  result when there are a large uneven number of UV points (10221 in total). 
  This test often fails on older hardware where shared memory limitations occur.

The `liboi_benchmark` program accesses how fast your hardware can execute
the following sequence:

1. Copy a 128x128 image from RAM to OpenCL device memory
2. Compute the DFT on a reference data set
3. Compute a chi-squared
4. Sum the chi-squared
5. Copy the summed chi-squared back to the host.

By default this will execute on a GPU. You can, specify the `-cpu` option to run
the benchmark program on your OpenCL-compatible CPU. 
You can also change the image size, image scale, and number of iterations used 
in the benchmark. 
See `liboi_benchmark -h` for this and other options.

`liboi_benchmark` will print some information about your hardware, print metadata
about the data file being used, and then benchmark liboi.
When you run `liboi_benchmark` you should see something like the following:

    liboi/bin$ ./liboi_benchmark 
    Running Benchmark with: 

    Device information: 
    Device Name: Tahiti

    ...

    Data set information for: 
     /home/bkloppenborg/workspace/liboi/bin/../samples/PointSource_noise.oifits
    N Vis: 0
    N V2 : 525
    N T3 : 700
    N UV : 529
    Average JD: 2456253.90049
    Building kernels.
    Starting benchmark.
    Iteration 0 Chi2: 1993.02527
    Iteration 100 Chi2: 1993.02527
    Iteration 200 Chi2: 1993.02527
    Iteration 300 Chi2: 1993.02527
    Iteration 400 Chi2: 1993.02527
    Iteration 500 Chi2: 1993.02527
    Iteration 600 Chi2: 1993.02527
    Iteration 700 Chi2: 1993.02527
    Iteration 800 Chi2: 1993.02527
    Iteration 900 Chi2: 1993.02527
    Benchmark Test completed!
    1000 iterations in 6.33000 seconds. Throughput 157.97788 iterations/sec.

The important aspects to note are that (1) the chi-squared is constant as a
function of iteration number, (2) the chi-squared is near the reference number
above, and (2) your throughput.
The performance of `liboi` is limited by the DFT which is linearly dependent
on the product of the number of UV points and number of pixels.
In terms of what you expect, here are some representative test values from
`liboi_benchmark` on various hardware:

| OpenCL device             | Iterations/sec    | Other information            |
|:--------------------------|:------------------|------------------------------|
| NVidia GeForce GTX 570    | 260               | |
| Intel i7-3520M (GPU)      | 210               | Apple driver, HD Graphics 4000 |
| AMD Radeon R9 280x        | 155               | |
| Intel i7-4770K (GPU)      | 135               | Beignet, HD Graphics 4600 |
| NVidia GeForce 8600m GT   | 60                | |
| NVidia GeForce 8400 GS    | 50                | |
| i7-4770K (CPU)            | 5                 | Running on 4 physical cores. |

All tests were performed on various Linux distributions using manufacturer 
drivers unless otherwise noted. 
Any additional benchmarks would be appreciated.

## Licensing and Acknowledgements

LibOI is free software, distributed under the [GNU Lesser General Public License (Version 3)](<http://www.gnu.org/licenses/lgpl.html). 

If you use this software as part of a scientific publication, please cite as:

Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library"
(Version X). Available from  <https://github.com/bkloppenborg/liboi>.

