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
    
For OpenCL capabilities you need to install drivers for your device. The
proprietary drivers for both NVidia and ATI GPUs distributed through the 
package manager *should* supply everything that is needed.

* For NVIDIA GPUs: `sudo apt-get install nvidia-current` or [NVidia drivers](www.nvidia.com/drivers)
* For AMD GPUs or AMD CPUs: `sudo apt-get install fglrx` or [AMD graphic drivers](http://support.amd.com/us/gpudownload/Pages/index.aspx), and then install the [OpenCL SDK] (http://developer.amd.com/tools/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/downloads/)
* For Intel CPUs: [Intel OpenCL SDK](http://software.intel.com/en-us/vcsource/tools/opencl-sdk)

After the OpenCL implementation is installed, ensure that the `cl.hpp` file got installed
along with the OpenCL drivers. If it was not installed copy the `liboi/include/cl.hpp`
into your system's OpenCL include directory.

After these are installed follow the building instructions below.

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
    
If you have any errors in the compilation steps, please contact us. (If you
are on a Apple machine be sure to set the `export` lines mentioned above!)

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

## Licensing and Acknowledgements

LibOI is free software, distributed under the [GNU Lesser General Public License (Version 3)](<http://www.gnu.org/licenses/lgpl.html). 

If you use this software as part of a scientific publication, please cite as:

Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library"
(Version X). Available from  <https://github.com/bkloppenborg/liboi>.

