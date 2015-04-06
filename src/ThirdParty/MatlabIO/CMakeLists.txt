#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2009 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#

# CMakeLists.txt for Core/Matlab

###########################################
# ZLib project

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(MATLABIO)

SET(MatlabIO_SRCS
  matfile.h
  matfilebase.h
  matfiledefs.h
  matfile.cc
  matfiledata.h
  matfiledata.cc
  matlabarray.h
  matlabarray.cc
  matlabfile.h
  matlabfile.cc
)

INCLUDE_DIRECTORIES( ${SCI_ZLIB_INCLUDE} )

ADD_LIBRARY(MatlabIO STATIC ${MatlabIO_SRCS})
ADD_DEFINITIONS(-DZ_PREFIX)

TARGET_LINK_LIBRARIES(MatlabIO
  ${SCI_ZLIB_LIBRARY}
)

###########################################
# Set the name of the library

SET(SCI_MATLABIO_LIBRARY MatlabIO CACHE INTERNAL "MatlabIO library name." FORCE)
SET(SCI_MATLABIO_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "MatlabIO include directories." FORCE)
