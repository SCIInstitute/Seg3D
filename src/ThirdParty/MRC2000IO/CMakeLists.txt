#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

###########################################
# MRC2000 IO project

cmake_minimum_required(VERSION 2.6)
project(MRC2000IO)

set(MRC2000IO_SRCS
  mrcheader.h
  MRCUtil.h
  MRCUtil.cc
)

add_library(MRC2000IO STATIC ${MRC2000IO_SRCS})

###########################################
# Set the name of the library

set(SCI_MRC2000IO_LIBRARY MRC2000IO CACHE INTERNAL "MRC2000IO library name." FORCE)
set(SCI_MRC2000IO_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "MRC2000IO include directories." FORCE)
