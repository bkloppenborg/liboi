cmake_minimum_required(VERSION 2.8) 
project(liboi)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE DEBUG)
# Default debug level (mostly status messages, some copy-back numbers):
set(CMAKE_CXX_FLAGS_DEBUG "-g -D DEBUG")
# For even more verbose debugging output:
#set(CMAKE_CXX_FLAGS_DEBUG "-g -D DEBUG -D DEBUG_VERBOSE")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Call CMake on the subdirectories
if(NOT TARGET textio)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/textio)
	include_directories(${CMAKE_SOURCE_DIR}/lib/textio/include)
endif(NOT TARGET textio)

# Call CMake on the subdirectories
if(NOT TARGET oifits)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/oifitslib)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/oifitslib)
	include_directories(${OIFITS_INCLUDE_DIRS})
endif(NOT TARGET oifits)

# Call CMake on the subdirectories
if(NOT TARGET getoifits)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/getoifits)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/getoifits)
endif(NOT TARGET getoifits)

# TODO: Remove this silly requirement thanks to oifitslib.
# Export the inclusion directories.
set(OI_INCLUDE_DIRS ${OIFITS_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} CACHE INTERNAL "")

add_subdirectory(src)
