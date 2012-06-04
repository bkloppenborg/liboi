cmake_minimum_required(VERSION 2.8)
project(liboifits)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

find_package(CFITSIO REQUIRED)
include_directories(${CFITSIO_INCLUDE_DIR})

# Export the inclusion directories:
set(OIFITS_INCLUDE_DIRS "${CFITSIO_INCLUDE_DIR}" CACHE INTERNAL "")

# This is all that is required for getoifits
set(SOURCES read_fits.c write_fits.c free_fits.c)
#set(SOURCES read_fits.c write_fits.c free_fits.c oifile.c oifilter.c oicheck.c oimerge.c)
add_library(oifits SHARED ${SOURCES} ${CFITSIO_LIBRARIES})
