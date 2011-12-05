#!/bin/bash

# Updates all libraries needed in this project.  If you add a library
# list it in ../CMakeLists.txt and ../src/CMakeLists.txt rather than
# a CMakeLists.txt file in this directory.

# textio, SVN checkout
svn co -r 8 http://finiteline.homeip.net/svn/textio ./textio

### NOTE:
### The following two libraries are included in the subversion repository,
### but Brian eventually plans to replace them with someone new.
# Library: oifitslib from here and extracted:
# ftp://ftp.mrao.cam.ac.uk/pub/jsy1001/oifitslib/oifitslib.tar.gz
# with additions/modifications for CMake compatability added by Brian Kloppenborg
# Library: getoifits
# Fabien Baron's routines for accessing OIFITS files, easier to use than
# oifitslib directly.  Extracted from BSMEM.

