#
# Copyright (c) 2012 Brian Kloppenborg
#
# If you use this software as part of a scientific publication, please cite as:
#
# Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library" 
# (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
#
# This file is part of the OpenCL Interferometry Library (LIBOI).
#  
#  LIBOI is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License 
#  as published by the Free Software Foundation, either version 3 
#  of the License, or (at your option) any later version.
#  
#  LIBOI is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#  
#  You should have received a copy of the GNU Lesser General Public 
#  License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 2.8)
project(oi)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
find_package(TEXTIO REQUIRED)
include_directories(${TEXTIO_INCLUDE_DIRS})
find_package(CCOIFITS REQUIRED)
include_directories(${CCOIFITS_INCLUDE_DIRS})

file(GLOB SOURCE *.cpp)
list(REMOVE_ITEM SOURCE main.cpp)

add_library(oi SHARED ${SOURCE})
target_link_libraries(oi ${TEXTIO_LIBRARIES} ${CCOIFITS_LIBRARIES} ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES})

add_executable(liboi_tests main.cpp)
target_link_libraries(liboi_tests oi)

install(TARGETS oi DESTINATION lib)
install(FILES CLibOI.h DESTINATION include RENAME oi.hpp)
