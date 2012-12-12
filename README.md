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

If you are building LibOI from scratch, you will need a C and C++ compilers
along with the CMake build system. On a Debian-based system these can be 
installed using `apt-get`:

    sudo apt-get install build-essentials g++ cmake

If you intend to use Git to checkout the repository (suggested), you also need to 
install git:

    sudo apt-get install git

LibOI also requires ccfits, cfitsio, and an OpenGL library. These can all be 
installed via. `apt-get`:

    sudo apt-get install libccfits0 libccfits-dev libqt4-dev libglumesa1 libglumesa1-dev

In order to run LibOI you must also have an installation of OpenCL for your graphics
card. If you have an NVidia GPU simply install the video card drivers. On ATI GPUs
you must install *both* the drivers *and* the [AMD APP SDK](http://developer.amd.com/tools/heterogeneous-computing/amd-accelerated-parallel-processing-app-sdk/).

After the OpenCL implementation is installed, ensure that the `cl.hpp` file got installed
along with the OpenCL drivers. If it was not installed copy the `liboi/includes/cl.hpp`
into your system's OpenCL directory.

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

## Licensing and Acknowledgements

LibOI is free software, distributed under the [GNU Lesser General Public License (Version 3)](<http://www.gnu.org/licenses/lgpl.html). 

If you use this software as part of a scientific publication, please cite as:

Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library"
(Version X). Available from  <https://github.com/bkloppenborg/liboi>.

