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

# Borrowed CMake code from the MaidSafe Boost CMake build
# found at https://github.com/maidsafe/MaidSafe/blob/master/cmake_modules/add_boost.cmake
# and code borrowed from ITK4 HDFMacros.cmake

set_property(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# disable auto linking
# also set in Seg3D?
set(boost_CXX_Flags "-DBOOST_ALL_NO_LIB=1")
if(APPLE)
  list(APPEND boost_CXX_Flag "-DBOOST_LCAST_NO_WCHAR_T" "-DBOOST_THREAD_DONT_USE_ATOMIC")
endif()
if(WIN32)
  list(APPEND boost_CXX_Flag "-DBOOST_BIND_ENABLE_STDCALL")
endif()

set( boost_DEPENDENCIES )

# explicitly set library list
set(boost_Libraries
  "atomic"
  "date_time"
  "exception"
  "filesystem"
  "regex"
  "system"
  "thread"
  CACHE INTERNAL "Boost library name.")

if(BUILD_WITH_PYTHON)
  add_definitions(-DBOOST_PYTHON_STATIC_LIB=1)
  list(APPEND boost_Libraries python)
  list(APPEND boost_DEPENDENCIES Python_external)
  list(APPEND boost_CXX_Flag "-DBOOST_PYTHON_STATIC_MODULE" "-DBOOST_PYTHON_STATIC_LIB")
endif()

# TODO: set up 64-bit build detection
# Boost Jam needs to have 64-bit build explicitly configured
if(WIN32)
  set(FORCE_64BIT_BUILD ON)
  set(boost_GIT_TAG "origin/v1.67.0")
else()
    set(boost_GIT_TAG "origin/v1.58.0")
endif()

set(boost_GIT_URL "https://github.com/CIBC-Internal/boost.git")

# TODO: fix install step
#
# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Boost_external
  DEPENDS ${boost_DEPENDENCIES}
  GIT_REPOSITORY ${boost_GIT_URL}
  GIT_TAG ${boost_GIT_TAG}
  BUILD_IN_SOURCE ON
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DBUILD_PYTHON:BOOL=${BUILD_WITH_PYTHON}
    -DPython_DIR:PATH=${Python_DIR}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DFORCE_64BIT_BUILD:BOOL=${FORCE_64BIT_BUILD}
    -DSCI_BOOST_LIBRARIES:STATIC=${boost_Libraries}
    -DSCI_BOOST_CXX_FLAGS:STRING="${boost_CXX_Flags} ${INHIBIT_ALL_WARNINGS}"
)

ExternalProject_Get_Property(Boost_external INSTALL_DIR)
ExternalProject_Get_Property(Boost_external SOURCE_DIR)
set(SCI_BOOST_INCLUDE ${SOURCE_DIR})
set(SCI_BOOST_LIBRARY_DIR ${SOURCE_DIR}/lib)
set(SCI_BOOST_USE_FILE ${INSTALL_DIR}/UseBoost.cmake)

set(BOOST_PREFIX "boost_")
set(THREAD_POSTFIX "-mt")

set(SCI_BOOST_LIBRARY)

foreach(lib ${boost_Libraries})
  set(LIB_NAME "${BOOST_PREFIX}${lib}${THREAD_POSTFIX}")
  list(APPEND SCI_BOOST_LIBRARY ${LIB_NAME})
endforeach()

# Boost is special case - normally this should be handled in external library repo
configure_file(${SUPERBUILD_DIR}/BoostConfig.cmake.in ${INSTALL_DIR}/BoostConfig.cmake @ONLY)
configure_file(${SUPERBUILD_DIR}/UseBoost.cmake ${SCI_BOOST_USE_FILE} COPYONLY)

set(Boost_DIR ${INSTALL_DIR} CACHE PATH "")

message(STATUS "Boost_DIR: ${Boost_DIR}")
