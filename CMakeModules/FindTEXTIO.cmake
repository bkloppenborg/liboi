# - Try to find TEXTIO.
# Once executed, this module will define:
# Variables defined by this module:
#  TEXTIO_FOUND        - system has TEXTIO
#  TEXTIO_INCLUDE_DIR  - the TEXTIO include directory (cached)
#  TEXTIO_INCLUDE_DIRS - the TEXTIO include directories
#                         (identical to TEXTIO_INCLUDE_DIR)
#  TEXTIO_LIBRARY      - the TEXTIO library (cached)
#  TEXTIO_LIBRARIES    - the TEXTIO libraries
#                         (identical to TEXTIO_LIBRARY)
# 
# This module will use the following enviornmental variable
# when searching for TEXTIO:
#  TEXTIO_ROOT_DIR     - TEXTIO root directory
#

# 
#  Copyright (c) 2012 Brian Kloppenborg
# 
#  This file is part of the TextIO library
#  
#  TextIO is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License 
#  as published by the Free Software Foundation, either version 3 
#  of the License, or (at your option) any later version.
#  
#  TextIO is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#  
#  You should have received a copy of the GNU Lesser General Public 
#  License along with TextIO.  If not, see <http://www.gnu.org/licenses/>.
# 

if(NOT TEXTIO_FOUND)

    find_path(TEXTIO_INCLUDE_DIR 
        NAMES textio.h
        HINTS $ENV{TEXTIO_ROOT_DIR} 
        PATH_SUFFIXES include)
    find_library(TEXTIO_LIBRARY 
        NAMES textio
        HINTS $ENV{TEXTIO_ROOT_DIR} 
        PATH_SUFFIXES lib)
  
    mark_as_advanced(TEXTIO_INCLUDE_DIR TEXTIO_LIBRARY)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(TEXTIO DEFAULT_MSG
        TEXTIO_LIBRARY TEXTIO_INCLUDE_DIR)

    set(TEXTIO_INCLUDE_DIRS ${TEXTIO_INCLUDE_DIR})
    set(TEXTIO_LIBRARIES ${TEXTIO_LIBRARY})

endif(NOT TEXTIO_FOUND)
