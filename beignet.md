
# Compiling liboi for use with Beignet (OpenCL on Intel GPUs on Linux)

Under Windows and Mac the Intel GPU drivers include OpenCL support; however, on
Linux OpenCL on Intel GPUs is implemented through an open source project called 
[Beignet](http://www.freedesktop.org/wiki/Software/Beignet/). To get 
[liboi](https://github.com/bkloppenborg/liboi) functioning on these devices
will be a several stage process:

1. Install the prerequisites for compiling `liboi`
2. Install the prerequisites for compiling `Beignet` (some of which overlaps
   with `liboi` so those steps will not be repeated)
3. Compile `Beignet` with the stock libraries in Ubuntu 14.04
4. Compile `liboi` linking with `Beignet`
5. Test `liboi`'s functionality (some of it may not work)

If all of the tests pass (see notes below) and OpenCL-OpenGL interop is not
required for your program, then no further work is required. If you do need
OpenCL-OpenGL interop or some of the tests fail (discussed in further detail
below), you may need to 

6. Re-compile your kernel to enable OpenCL shared local memory 
7. Compile the latest version of `Mesa` from the git master branch.

## Operating system

The following instructions were written for Ubuntu 14.04. Instructions may be
similar on other operating systems.

## Check installation requirements for liboi
    
Default requirements for `liboi`:

    sudo apt-get install build-essential g++ cmake libccfits0 libccfits-dev
    
To enable OpenCL-OpenGL interoperability you should also install an OpenGL
library. This is normally installed with your graphics drivers. In the case of
Intel hardware, you will want to install the following `Mesa` packages:
    
    sudo apt-get install libglu1-mesa libglu1-mesa-dev

Lastly, `liboi` uses the OpenCL Installable Client Driver (ICD) architecture.
The following packages enable this capability:

    sudo apt-get install opencl-headers ocl-icd-dev ocl-icd-libopencl1
    
(strictly speaking I don't think ocl-icd-dev is needed for `liboi`, but it is
needed for Beignet below). Lastly, it is useful to install the `clinfo` program
to get some information about your hardware:

    sudo apt-get install clinfo

## Check requirements for Beignet

I had to install the following packages beyond what liboi requires

    sudo apt-get install clang libclang-3.4-dev libclang-dev libclang1


## Install links to llvm-link and llvm-as version 3.4

	sudo update-alternatives --install /usr/bin/llvm-link llvm-link /usr/bin/llvm-link-3.4 34
	sudo update-alternatives --install /usr/bin/llvm-as llvm-as /usr/bin/llvm-as-3.4 34

where llvm-as-X.Y matches the version you installed.

## Compile Beignet

    mkdir build
    cd build
    cmake ..
    
During the CMake stage, ensure the following items are found otherwise the
library will not compile with the required components:

* OCL ICD header file
    
If everything went fine, go ahead and compile the source and install the library

    make
    sudo make install

(Note, you can do make -jN with N > 1 to do a parallel build.)

## Test the `Beignet` installation

In a terminal run the `clinfo` program. You should see something like this:

    $ clinfo
    ...      
    Platform Name:				Intel Gen OCL Driver
    Number of devices:			1
    Device Type:				CL_DEVICE_TYPE_GPU
    ...
    Name:						Intel(R) HD Graphics Haswell GT2 Desktop
    Vendor:					    Intel
    Device OpenCL C version:    OpenCL C 1.2 beignet 0.9.1
    Driver version:				0.9.1
    Profile:					FULL_PROFILE
    Version:					OpenCL 1.2 beignet 0.9.1
    Extensions:					cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store cl_khr_icd

Take note that the Intel OpenCL driver is loaded for a GPU device using
`beignet`. This indicates the Installable Client Driver (ICD) is correctly
installed. Also note the extensions listed. If you need OpenCL-OpenGL interop
the `cl_khr_gl_sharing` extension must be listed. If `cl_khr_gl_sharing` is
absent, you may need to compile a more-recent version of `Mesa` (see below).

## Test `liboi`

Run the test suite in the bin directory:

    cd ../bin
    ./liboi_tests
    
At the end of the test there should be some information like the following:

    ...
    [==========] 34 tests from 12 test cases ran. (6339 ms total)
    [  PASSED  ] 27 tests.
    [  FAILED  ] 7 tests, listed below:
    [  FAILED  ] CRoutine_DFT.CL_PointSource
    [  FAILED  ] CRoutine_DFT.CL_UniformDisk
    [  FAILED  ] CRoutine_DFT.CL_DFT_LARGE_UNEVEN_N_UV_POINTS
    [  FAILED  ] CRoutine_Sum_NVidia.CL_Sum_CPU_CHECK
    [  FAILED  ] CRoutine_Sum_AMD.CL_Sum_CPU_CHECK

Not all of the failures are unexpected. The NVidia Sum kernel (tested by
`CRoutine_Sum_NVidia.CL_Sum_CPU_CHECK`) has a known bug which causes it to fail
on non-NVidia hardware. Because we are running the tests on an Intel GPU, this
test failure is expected.

As of the writing of this document (2014 July 02), the failure of the three
DFT tests:

    CRoutine_DFT.CL_PointSource
    CRoutine_DFT.CL_UniformDisk
    CRoutine_DFT.CL_DFT_LARGE_UNEVEN_N_UV_POINTS
    
and the one AMD sum test:

    CRoutine_Sum_AMD.CL_Sum_CPU_CHECK

Is most likely due to Intel GPU OpenCL shared local memory being disabled in
your kernel. Please see below for instructions. 

## Benchmark liboi

If all is well (and, I suppose, even if things aren't working), you can run 
the benchmark program to see an example throughput on your system. For example,
this is with my Intel HD Graphics 4000 inside a Intel Core i7-4770K:

    $ ./liboi_benchmark 
    Running Benchmark with: 

    Device information: 
    Device Name: Intel(R) HD Graphics Haswell GT2 Desktop
    Vendor: Intel
    Device OpenCL version: OpenCL 1.2 beignet 0.9.1
    Driver OpenCL version: 0.9.1
    Profile: FULL_PROFILE
    Supported Extensions: cl_khr_global_int32_base_atomics
        cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics
        cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store
        cl_khr_icd
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
    Iteration 0 Chi2: 1993.02551
    Iteration 100 Chi2: 1993.02551
    Iteration 200 Chi2: 1993.02551
    Iteration 300 Chi2: 1993.02551
    Iteration 400 Chi2: 1993.02551
    Iteration 500 Chi2: 1993.02551
    Iteration 600 Chi2: 1993.02551
    Iteration 700 Chi2: 1993.02551
    Iteration 800 Chi2: 1993.02551
    Iteration 900 Chi2: 1993.02551
    Benchmark Test completed!
    1000 iterations in 7.33000 seconds. Throughput 136.42565 iterations/sec.

The chi-squared should be about what is listed below. If it deviates 
significantly (i.e. by more than a few percent), there may be some other issue
with your system.

## Applying the kernel patch to enable OpenCL shared local memory.

If the following tests failed:

    CRoutine_DFT.CL_PointSource
    CRoutine_DFT.CL_UniformDisk
    CRoutine_DFT.CL_DFT_LARGE_UNEVEN_N_UV_POINTS
    CRoutine_Sum_AMD.CL_Sum_CPU_CHECK
    
it is likely that your kernel does not support Intel OpenCL shared local memory.
We need to apply and install a custom kernel. Please read over the 
[instructions for compiling a kernel on Ubuntu](https://help.ubuntu.com/community/Kernel/Compile). Here is a summary for Ubuntu 14.04.

Install prerequisites:

    sudo apt-get build-dep linux-image-`uname -r`
    
Make a working directory for the kernel somewhere and download the source

    mkdir kernel
    cd kernel
    sudo apt-get source linux-image-`uname -r`

And take ownership of the source files (replace `your_username` with your 
username, of course):

    sudo chown -R your_username *
    
Next we see if we can automatically apply the patch. First download the
patch:

    wget https://01.org/sites/default/files/disable-batchbuffer-security.patch
    
Now `cd` into the kernel directory:

    cd linux-3.13.0
    
Check some details about the patch

    git apply --stat ../disable-batchbuffer-security.patch

Try automatically applying the patch:

    git apply --check ../disable-batchbuffer-security.patch
    
If it automatically applies with no errors, just apply it:

    git apply ../disable-batchbuffer-security.patch

If it did not work (as was the case on my system), the change is fairly
simple. Open the `drivers/gpu/drm/i915/i915_gem_execbuffer.c` file in an
editor:

    gedit drivers/gpu/drm/i915/i915_gem_execbuffer.c
    
In the `i915_gem_do_execbuffer` function, find the following code:

 	flags = 0;
 	if (args->flags & I915_EXEC_SECURE) {
 		if (!file->is_master || !capable(CAP_SYS_ADMIN))
 		    return -EPERM;

And add the line `flags |= I915_DISPATCH_SECURE;` as follows:

 	flags = 0;
	flags |= I915_DISPATCH_SECURE;
 	if (args->flags & I915_EXEC_SECURE) {
 		if (!file->is_master || !capable(CAP_SYS_ADMIN))
 		    return -EPERM;

This was inserted on line 998 for me. Save the file. Now we need to compile and
install the modified kernel.

Now we build the new kernel. Run the following commands from within the kernel
source directory:

    fakeroot debian/rules clean
    AUTOBUILD=1 fakeroot debian/rules binary-debs
    
If it looks like everything is compiling correctly, stop the compilation and
run it in parallel. I have four cores with 2 threads each so I built with
8 threads in parallel:

    DEB_BUILD_OPTIONS=parallel=8 AUTOBUILD=1 fakeroot debian/rules binary-debs

Now we install the kernel. If you want to see the Ubuntu splash screen (or use
text mode) before you get to X instead of just a black screen, you'll want to
make sure the framebuffer driver loads. Execute one of these two lines:

    echo vesafb | sudo tee -a /etc/initramfs-tools/modules
    echo fbcon | sudo tee -a /etc/initramfs-tools/modules
    
    

