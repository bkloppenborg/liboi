#
# Copyright (c) 2012 Brian Kloppenborg
#
# If you use this software as part of a scientific publication, please cite as:
#
# Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library" 
# (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
#
# This file is part of the OpenCL Interferometry Library (LIBOI).
#  
#  LIBOI is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License 
#  as published by the Free Software Foundation, either version 3 
#  of the License, or (at your option) any later version.
#  
#  LIBOI is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#  
#  You should have received a copy of the GNU Lesser General Public 
#  License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 2.8)
project(oi)

find_package(OpenGL REQUIRED)

# Find OpenCL. Set compiler flags if we find an old (1.0, 1.1) version
# to use clCreateFromGLTexture3D on these old devices.
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
message(STATUS "Found OpenCL Version: ${OPENCL_VERSION}")
if(${OPENCL_VERSION} VERSION_EQUAL 1.0)
    add_definitions(-DDETECTED_OPENCL_1_0)
elseif(${OPENCL_VERSION} VERSION_EQUAL 1.1)
    add_definitions(-DDETECTED_OPENCL_1_1)
elseif(${OPENCL_VERSION} VERSION_EQUAL 1.2)
    add_definitions(-DDETECTED_OPENCL_1_2)    
elseif(${OPENCL_VERSION} VERSION_EQUAL 2.0)
    add_definitions(-DDETECTED_OPENCL_2_0)
else(${OPENCL_VERSION} VERSION_EQUAL 2.0)
    add_definitions(-DDETECTED_OPENCL_UNKNOWN_VERSION)     
endif(${OPENCL_VERSION} VERSION_EQUAL 1.0)

set(LIBOI_INCLUDE_DIRS ${OPENCL_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} CACHE INTERNAL "")

include_directories(. models/)
file(GLOB SOURCE *.cpp models/*.cpp)
file(GLOB TESTS *_test.cpp PathFind.cpp)
file(GLOB BENCHMARK *_benchmark.cpp PathFind.cpp)
list(REMOVE_ITEM SOURCE main.cpp ${TESTS} ${BENCHMARK})

find_package(clFFT)
if(clFFT_FOUND)
    include_directories(${CLFFT_INCLUDE_DIRS})
    LIST(APPEND OPTIONAL_LIBS ${CLFFT_LIBRARIES})
    message(STATUS " clFFT found, support for FFT-based kernels enabled.")
    add_definitions(-D_ADD_CLFFT)
else(clFFT_FOUND)
    message(STATUS " clFFT not found. No FFT-based kernels will be enabled.")
    file(GLOB REMOVE_CLFFT "CRoutine_FFT_clFFT*")
    list(REMOVE_ITEM SOURCE ${REMOVE_CLFFT})
endif(clFFT_FOUND)

add_library(oi SHARED EXCLUDE_FROM_ALL ${SOURCE})
target_link_libraries(oi textio ccoifits ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES} ${Boost_LIBRARIES})

add_library(oi_static STATIC ${SOURCE})
target_link_libraries(oi_static textio_static ccoifits_static ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES} ${Boost_LIBRARIES})

# Build tests:
add_executable(liboi_tests ${TESTS})
target_link_libraries(liboi_tests gtest oi_static)
add_test(liboi_tests liboi_tests)

# Build benchmark
add_executable(liboi_benchmark ${BENCHMARK})
target_link_libraries(liboi_benchmark oi_static)

install(TARGETS oi_static DESTINATION lib)
install(FILES liboi.hpp DESTINATION include)
