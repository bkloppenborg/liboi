cmake_minimum_required(VERSION 2.8) 
project(liboi)

# Compile with C++11 support.  Disable __STRICT_ANSI__ to support 
# some nice OpenCL shortcuts (i.e. cl_float2's .s0, .s1 syntax).
ADD_DEFINITIONS("-std=c++0x -pthread -U__STRICT_ANSI__")

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE DEBUG)
# Default debug level (mostly status messages, some copy-back numbers):
set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wshadow -D DEBUG")
# For even more verbose debugging output:
#set(CMAKE_CXX_FLAGS_DEBUG "-g -D DEBUG -D DEBUG_VERBOSE")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

include_directories(${CMAKE_SOURCE_DIR}/include)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Call CMake on the subdirectories
if(NOT TARGET ccoifits)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/ccoifits)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/ccoifits/src)
	include_directories(${CCOIFITS_INCLUDE_DIRS})
endif(NOT TARGET ccoifits)

add_subdirectory(src)

# Copy over kernel and shader sources:
file(GLOB KERNELS ${CMAKE_SOURCE_DIR}/src/kernels/*.cl)
file(COPY ${KERNELS} DESTINATION ${EXECUTABLE_OUTPUT_PATH}/kernels/)
