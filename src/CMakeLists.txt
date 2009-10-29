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

###########################################
# PROJECT: Seg3D
###########################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(Seg3D)

###########################################
# WHERE TO COMPILE EXECUTABLES
###########################################

SET(SEG3D_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

###########################################
# SEG3D VERSION NUMBERS
###########################################

SET(SEG3D_MAJOR_VERSION 2)
SET(SEG3D_MINOR_VERSION 0)

###########################################
# DO STANDARD TESTS
###########################################

INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)

###########################################
# DETERMINE ARCHITECTURE
# In order for the code to depend on the
# architecture settings
###########################################

IF (CMAKE_SIZEOF_VOID_P MATCHES 8)
  SET(SEG3D_BITS "64bit")
  SET(MSVC90_BUILD "amd64")
ELSE (CMAKE_SIZEOF_VOID_P MATCHES 8)
  SET(SEG3D_BITS "32bit")
  SET(MSVC90_BUILD "x86")
ENDIF (CMAKE_SIZEOF_VOID_P MATCHES 8)

###########################################
# SETUP DEFINTIONS FOR EXTERNAL PACKAGES
# To force a static build these flags
# need to be set project wide.
###########################################

ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DGLEW_MX)
ADD_DEFINITIONS(-DTEEM_STATIC)
ADD_DEFINITIONS(-DLIBXML_STATIC)

###########################################
# Build Seg3DConfiguration.h
# this file contains information such as
# version numbers and architecture
# on which code can depend.
###########################################

CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/Configuration/Seg3DConfiguration.h.in 
                ${CMAKE_CURRENT_BINARY_DIR}/Seg3DConfiguration.h 
                @ONLY )
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

###########################################
# Find QT
# We need QT - We use a modified version of
# the FindQt4.cmake file that can be set
# to search for different versions of QT
# just point QT_DIR to the right directory
# to skip the use of the default installed
# QT distribution. 
###########################################

SET(QT_DIR "" CACHE PATH "Path to QT")
INCLUDE(CMake/FindQt4.cmake)
SET(QT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CMake/UseQt4.cmake)

IF (QT_FOUND)
  INCLUDE(${QT_USE_FILE})
ELSE (QT_FOUND)
  MESSAGE(SEND_ERROR "QT is required for building Seg3D")
ENDIF (QT_FOUND)

###########################################
# External packages other than Qt
# Include the External packages, these are
# the packages that are shipped with 
# Seg3D to lower the number of dependencies.
# QT is currently not included as the code
# is a huge repository and to allow it to
# be build against different licensed 
# versions of Qt.
###########################################

ADD_SUBDIRECTORY(Externals)

###########################################
# Set the include paths
###########################################

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${SCI_BOOST_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_ZLIB_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_LIBPNG_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_TEEM_INCLUDE})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

###########################################
# GUI Layer of the application
###########################################

ADD_SUBDIRECTORY(Core)
