cmake_minimum_required(VERSION 2.8)
project(getoifits)

# Add include directories and libs for compiled libraries.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../oifitslib)

set(SOURCES getoifits.cpp)

add_library(getoifits SHARED ${SOURCES})
target_link_libraries(getoifits oifits)
