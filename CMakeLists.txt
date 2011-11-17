cmake_minimum_required(VERSION 2.8) 
project(liboi)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE DEBUG)

SET(CMAKE_CXX_FLAGS_DEBUG "-D DEBUG")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Call CMake on the subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/textio "${CMAKE_CURRENT_BINARY_DIR}/lib/textio")
add_subdirectory(src)
