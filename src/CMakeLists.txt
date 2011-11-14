cmake_minimum_required(VERSION 2.8)
project(liboi)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})

file(GLOB SOURCE *.cpp)

add_library(liboi ${SOURCE} ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES})
add_executable(test main.cpp)
target_link_libraries(test liboi ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES})
