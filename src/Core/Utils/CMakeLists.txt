
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

##################################################
# Set sources
##################################################

SET(CORE_UTILS_SRCS
  AtomicCounter.h
  ConnectionHandler.h
  ConnectionHandler.cc
  EnumClass.h
  Exception.h
  Exception.cc
  FilesystemUtil.h
  FilesystemUtil.cc
  GenerationNumber.h
  GenerationNumber.cc
  IntrusiveBase.h
  IntrusiveBase.cc
  Lockable.h
  Log.h
  Log.cc
  LogHistory.h
  LogHistory.cc
  LogStreamer.h
  LogStreamer.cc
  Notifier.h
  Notifier.cc
  Parallel.h
  Parallel.cc
  Runnable.h
  Runnable.cc
  ScopedCounter.h
  Singleton.h
  StackBasedVector.h
  StackVector.h
  StringContainer.h
  StringParser.h
  StringParser.cc
  StringUtil.h
  StringUtil.cc
  Timer.h
  Timer.cc
  Variant.h
  Variant.cc
)

##################################################
# Build static library
##################################################

CORE_ADD_LIBRARY(Core_Utils ${CORE_UTILS_SRCS})
TARGET_LINK_LIBRARIES(Core_Utils ${SCI_BOOST_LIBRARY})
