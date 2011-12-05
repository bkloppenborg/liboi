cmake_minimum_required(VERSION 2.8)
project(getoifits)

# Add include directories and libs for compiled libraries.
include_directories(${CMAKE_SOURCE_DIR}/lib/oifitslib)

set(SOURCES getoifits.c)

add_library(getoifits ${SOURCES})
