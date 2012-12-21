# - Try to find LIBOI.
# Once executed, this module will define:
# Variables defined by this module:
#  LIBOI_FOUND        - system has LIBOI
#  LIBOI_INCLUDE_DIR  - the LIBOI include directory (cached)
#  LIBOI_INCLUDE_DIRS - the LIBOI include directories
#                         (identical to LIBOI_INCLUDE_DIR)
#  LIBOI_LIBRARY      - the LIBOI library (cached)
#  LIBOI_LIBRARIES    - the LIBOI libraries
#                         (identical to LIBOI_LIBRARY)
# 
# This module will use the following enviornmental variable
# when searching for LIBOI:
#  LIBOI_ROOT_DIR     - LIBOI root directory
#

# 
#  Copyright (c) 2012 Brian Kloppenborg
# 
#  This file is part of the OpenCL Interferometry Library (LIBOI)
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
# 

if(NOT LIBOI_FOUND)

    find_path(LIBOI_INCLUDE_DIR 
        NAMES oi.hpp
        HINTS $ENV{LIBOI_ROOT_DIR} 
        PATH_SUFFIXES include)
        
    find_library(LIBOI_LIBRARY 
        NAMES oi
        HINTS $ENV{LIBOI_ROOT_DIR} 
        PATH_SUFFIXES lib)
  
    FIND_PACKAGE(OpenCL REQUIRED)
    FIND_PACKAGE(OpenGL)
    FIND_PACKAGE(CCOIFITS REQUIRED)
  
    mark_as_advanced(LIBOI_INCLUDE_DIR LIBOI_LIBRARY)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(LIBOI DEFAULT_MSG
        LIBOI_LIBRARY LIBOI_INCLUDE_DIR)

    set(LIBOI_LIBRARIES ${LIBOI_LIBRARY} ${OPENCL_LIBRARIES} ${OPENGL_LIBRARIES} ${CCOIFITS_LIBRARIES} ${LIBOI_LIBRARIES})
    set(LIBOI_INCLUDE_DIRS ${LIBOI_INCLUDE_DIR} ${OPENCL_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${CCOIFITS_INCLUDE_DIRS} ${LIBOI_INCLUDE_DIRS})


endif(NOT LIBOI_FOUND)
