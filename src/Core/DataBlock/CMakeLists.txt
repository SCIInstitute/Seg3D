#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

##################################################
# Set sources
##################################################

SET(CORE_DATABLOCK_SRCS
  DataBlock.h
  DataBlockFWD.h
  DataBlock.cc
  DataBlockManager.h
  DataBlockManager.cc
  DataSlice.h
  DataSlice.cc
  DataType.h
  DataType.cc
  Histogram.h
  Histogram.cc
  ITKDataBlock.h
  ITKDataBlock.cc
  ITKImageData.h
  ITKImageData.cc
  ITKImage2DData.h
  ITKImage2DData.cc
  MaskDataBlock.h
  MaskDataBlock.cc
  MaskDataBlockManager.h
  MaskDataBlockManager.cc
  MaskDataSlice.h
  MaskDataSlice.cc
  NrrdData.h
  NrrdData.cc
  NrrdDataBlock.h
  NrrdDataBlock.cc
  SliceType.h
  StdDataBlock.h
  StdDataBlock.cc
)

CORE_ADD_LIBRARY(Core_DataBlock ${CORE_DATABLOCK_SRCS} )

ITK_MODULE_LOAD(ITKCommon)

INCLUDE_DIRECTORIES(${ITKCommon_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(Core_DataBlock
  Core_Utils
  Core_Geometry
  ${ITKCommon_LIBRARIES}
  ${SCI_TEEM_LIBRARY}
  ${SCI_BOOST_LIBRARY})

#SET_TARGET_PROPERTIES(Core_DataBlock
#  PROPERTIES
#  INCLUDE_DIRECTORIES "${ITKCommon_INCLUDE_DIRS}"
#)
