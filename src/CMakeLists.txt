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
ADD_DEFINITIONS(-DTEEM_STATIC)
ADD_DEFINITIONS(-DLIBXML_STATIC)
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

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
# SETUP FOR DEBUG BUILDS
# enable any extra compile warnings and 
# set any debug-specific variables
# NOTE: This needs to be here as ITK does
#  *NOT* compile with the DEBUG flag set.
#  It looks like ITK's vnl library is missing
#  includes that would allow it to compile
#  with this flag. Hence we only enable it
#  for our code.
###########################################

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  ADD_DEFINITIONS(-DDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

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
# Generate a variable for registering all
# the plugins
###########################################

INCLUDE(CMake/ActionLibrary.cmake)
INCLUDE(CMake/ToolLibrary.cmake)


###########################################
# Application layers:
#  Core
#  DataVolume
#  Application
#  Interface
###########################################

ADD_SUBDIRECTORY(Utils)
ADD_SUBDIRECTORY(Application)
ADD_SUBDIRECTORY(Interface)

###########################################
# Plugins:
# Plugin libraries need be generated before
# we build the registration objects as they
# may contain as well pieces that need to
# be registered in the main object 
# factories.
###########################################


###########################################
# Build the plugin generator files
# This code will automatically register all
# the components with the proper object
# factories
###########################################

BUILD_REGISTER_ACTIONS_LIBRARY()
BUILD_REGISTER_TOOLS_LIBRARY()

###########################################
# Init:
# This builds the function that glues all
# the plugins together for use in a static
# application. This mechanism ensures every
# action and tool is registered and 
# mentioned 
###########################################

ADD_SUBDIRECTORY(Init)

