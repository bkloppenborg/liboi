cmake_minimum_required(VERSION 2.8)
project(liboi)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})

# Add include directories and libs for compiled libraries.
include_directories(${CMAKE_SOURCE_DIR}/lib/textio/include)

file(GLOB SOURCE *.cpp)

add_library(liboi ${SOURCE} ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES})
add_executable(test main.cpp)
target_link_libraries(test liboi textio ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES})
