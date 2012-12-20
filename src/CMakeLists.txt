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
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})

if("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
	MESSAGE(STATUS "NOTICE: clang detected, switching to boost for threads/mutexes/locks/shared_ptr.")
	FIND_PACKAGE(Boost COMPONENTS system thread REQUIRED)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
endif("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

set(LIBOI_INCLUDE_DIRS ${OPENCL_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} CACHE INTERNAL "")

include_directories(. models/)
file(GLOB SOURCE *.cpp models/*.cpp)
file(GLOB TESTS *_test.cpp)
file(GLOB BENCHMARK *_benchmark.cpp)
list(REMOVE_ITEM SOURCE main.cpp ${TESTS} ${BENCHMARK})

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
