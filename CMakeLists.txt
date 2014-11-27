cmake_minimum_required(VERSION 2.8) 
project(liboi)

INCLUDE("${CMAKE_SOURCE_DIR}/CMakeModules/SetVersion.cmake")

# Compile with C++11 support.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#if("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#endif("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# We need to compile the following libraries
if(NOT TARGET textio)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/textio EXCLUDE_FROM_ALL)
    include_directories(${CMAKE_SOURCE_DIR}/lib/textio/src)
endif(NOT TARGET textio)

if(NOT TARGET ccoifits)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/ccoifits EXCLUDE_FROM_ALL)
    include_directories(${CMAKE_SOURCE_DIR}/lib/ccoifits/src)
    include_directories(${CCOIFITS_INCLUDE_DIRS})
endif(NOT TARGET ccoifits)

# Include and build Google Test framework:
if(NOT TARGET gtest)
    ADD_SUBDIRECTORY (lib/gtest-1.6.0)
    enable_testing()
endif(NOT TARGET gtest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(src)

# Copy over kernel and shader sources:
file(GLOB KERNELS ${CMAKE_CURRENT_SOURCE_DIR}/src/kernels/*.cl)
file(COPY ${KERNELS} DESTINATION ${EXECUTABLE_OUTPUT_PATH}/kernels/)

# Automatic packaging
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "liboi")
SET(CPACK_PACKAGE_VERSION ${GIT_VERSION_TAG})
SET(CPACK_PACKAGE_CONTACT "Brian Kloppenborg <bkloppenborg@gmail.com>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCL Interferometry Library (https://github.com/bkloppenborg/liboi)")

# Debian-specific packaging options
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "textio ( >= 1.0.0 ), ccoifits ( >= 1.0.0 ), ocl-icd-libopencl1 ( >= 2.1.3 )")
 
INCLUDE(CPack)
