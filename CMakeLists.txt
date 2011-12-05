cmake_minimum_required(VERSION 2.8) 
project(liboi)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "-g -D DEBUG")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Call CMake on the subdirectories
if(NOT TARGET textio)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/textio)
endif(NOT TARGET textio)

# Call CMake on the subdirectories
if(NOT TARGET liboifits)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/oifitslib)
endif(NOT TARGET liboifits)

# Call CMake on the subdirectories
if(NOT TARGET getoifits)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/getoifits)
endif(NOT TARGET getoifits)

add_subdirectory(src)
