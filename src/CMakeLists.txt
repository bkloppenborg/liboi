cmake_minimum_required(VERSION 2.8)
project(oi)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})

# Add include directories and libs for compiled libraries.
include_directories(${CMAKE_SOURCE_DIR}/lib/textio/include)

file(GLOB SOURCE *.cpp)

add_library(oi ${SOURCE})
target_link_libraries(oi textio oifits getoifits ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES})
