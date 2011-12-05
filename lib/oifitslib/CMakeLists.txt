cmake_minimum_required(VERSION 2.8)
project(liboifits)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

find_package(CFITSIO REQUIRED)
include_directories(${CFITSIO_INCLUDE_DIRS})

find_package(GTK2 REQUIRED)
include_directories(${GTK2_INCLUDE_DIRS})

set(SOURCES read_fits.c write_fits.c free_fits.c oifile.c oifilter.c oicheck.c oimerge.c)

add_library(oifits ${SOURCES} ${CFITSIO_LIBRARIES} ${GTK2_LIBRARIES})
