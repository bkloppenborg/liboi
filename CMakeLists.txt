cmake_minimum_required(VERSION 2.8) 
project(liboi)

# Compile with C++11 support.  Disable __STRICT_ANSI__ to support 
# some nice OpenCL shortcuts (i.e. cl_float2's .s0, .s1 syntax).
ADD_DEFINITIONS("-std=c++0x -pthread -U__STRICT_ANSI__")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# We need to compile the following libraries
if(NOT TARGET textio)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/textio EXCLUDE_FROM_ALL)
    include_directories({CMAKE_SOURCE_DIR}/lib/textio/src)
endif(NOT TARGET textio)

if(NOT TARGET ccoifits)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/ccoifits EXCLUDE_FROM_ALL)
    include_directories(${CMAKE_SOURCE_DIR}/lib/ccoifits/src)
endif(NOT TARGET ccoifits)

add_subdirectory(src)

# Copy over kernel and shader sources:
file(GLOB KERNELS ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels/*.cl)
file(COPY ${KERNELS} DESTINATION ${EXECUTABLE_OUTPUT_PATH}/kernels/)
