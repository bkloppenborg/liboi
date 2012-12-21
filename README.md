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

`ccfits` will need to be compiled and installed manually.
[Download and install ccfits from here](http://heasarc.gsfc.nasa.gov/fitsio/CCfits/)
*NOTE:* When you compile `ccfits` be sure to specify

    export CC=/path/to/gcc
    export CXX=/path/to/g++

so that `gcc` compiles and links the libraries. After these are installed follow the 
building instructions below.

If you wish to keep up with the development version, it is suggested that you also
install [git](http://git-scm.com/).

&dagger; It is possible that Xcode 4.6 (which includes Apple `clang` 4.2 which is based on 
`llvm-clang` 3.2svn) could compile `liboi`, but we haven't tested this yet.

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

    sudo apt-get install build-essentials g++ cmake libccfits0 libccfits-dev 
    
To enable OpenCL-OpenGL interoperability you should also install an OpenGL
library. This *should* install the prerequisites:
    
    sudo apt-get install libglumesa1 libglumesa1-dev
    
For OpenCL capabilities you need to install drivers for your device. The
proprietary drivers for both NVidia and ATI GPUs distributed through the 
package manager *should* supply everything that is needed.

* `sudo apt-get install nvidia-current` or [NVidia drivers](www.nvidia.com/Drivers) (only for NVidia GPUs)
* `sudo apt-get install fglrx` [ATI drivers](http://support.amd.com/us/gpudownload/Pages/index.aspx) (only for ATI/AMD GPUs, also enables CPU computations)
* [Intel OpenCL SDK](http://software.intel.com/en-us/vcsource/tools/opencl-sdk) (only for Intel CPUs)

After the OpenCL implementation is installed, ensure that the `cl.hpp` file got installed
along with the OpenCL drivers. If it was not installed copy the `liboi/includes/cl.hpp`
into your system's OpenCL include directory.

After these are installed follow the building instructions below.

If you wish to keep up with the development version, it is suggested that you also
install [git](http://git-scm.com/) via:

    sudo apt-get install git

## Checkout / getting a copy of LibOI source code

After installing the aforementioned prerequisites, you simply need to checkout
a copy of LibOI

    git clone https://github.com/bkloppenborg/liboi.git
    cd liboi
    git submodule update --init

## Building instructions

After you have obtained a copy of the source and initialized the submodules, simply

    cd build
    cmake ..
    make
    
If you have any errors in the compilation steps, please contact us.

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

