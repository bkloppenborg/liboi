cmake_minimum_required(VERSION 2.8)
project(oi)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})

# Temporary, required for oifitslib.  Don't forget to remove from below too!
find_package(CFITSIO REQUIRED)
include_directories(${CFITSIO_INCLUDE_DIRS})
find_package(GTK2 REQUIRED)
include_directories(${GTK2_INCLUDE_DIRS})

# Add include directories and libs for compiled libraries.
include_directories(${CMAKE_SOURCE_DIR}/lib/textio/include)
include_directories(${OIFITS_INCLUDE_DIRECTORIES})
include_directories(${GETOIFITS_INCLUDE_DIRECTORIES})

file(GLOB SOURCE *.cpp)

add_library(oi ${SOURCE})
target_link_libraries(oi textio oifits getoifits ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES} ${CFITSIO_LIBRARIES} ${GTK2_LIBRARIES})
