cmake_minimum_required(VERSION 2.8) 
project(liboi)

# Compile with C++11 support.  Disable __STRICT_ANSI__ to support 
# some nice OpenCL shortcuts (i.e. cl_float2's .s0, .s1 syntax).
if("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
	ADD_DEFINITIONS("-stdlib=libc++")
endif("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

ADD_DEFINITIONS("-std=c++11 -U__STRICT_ANSI__")


SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# We need to compile the following libraries
if(NOT TARGET textio)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/textio EXCLUDE_FROM_ALL)
    include_directories(${CMAKE_SOURCE_DIR}/lib/textio/src)
endif(NOT TARGET textio)

if(NOT TARGET ccoifits)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/ccoifits EXCLUDE_FROM_ALL)
    include_directories(${CMAKE_SOURCE_DIR}/lib/ccoifits/src)
    include_directories(${CCOIFITS_INCLUDE_DIRS})
endif(NOT TARGET ccoifits)

ADD_SUBDIRECTORY (lib/gtest-1.6.0)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(src)

# Copy over kernel and shader sources:
file(GLOB KERNELS ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels/*.cl)
file(COPY ${KERNELS} DESTINATION ${EXECUTABLE_OUTPUT_PATH}/kernels/)
