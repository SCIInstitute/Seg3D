#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# disable auto linking
# also set in Seg3D?
SET(boost_CXX_Flags "-DBOOST_ALL_NO_LIB=1")
IF(APPLE)
  LIST(APPEND boost_CXX_Flag "-DBOOST_LCAST_NO_WCHAR_T" "-DBOOST_THREAD_DONT_USE_ATOMIC")
ENDIF()
IF(WIN32)
  LIST(APPEND boost_CXX_Flag "-DBOOST_BIND_ENABLE_STDCALL")
ENDIF()

SET( boost_DEPENDENCIES )

# explicitly set library list
SET(boost_Libraries
  "atomic"
  "date_time"
  "exception"
  "filesystem"
  "regex"
  "system"
  "thread"
  CACHE INTERNAL "Boost library name.")

IF(BUILD_WITH_PYTHON)
  ADD_DEFINITIONS(-DBOOST_PYTHON_STATIC_LIB=1)
  LIST(APPEND boost_Libraries python)
  LIST(APPEND boost_DEPENDENCIES Python_external)
  LIST(APPEND boost_CXX_Flag "-DBOOST_PYTHON_STATIC_MODULE" "-DBOOST_PYTHON_STATIC_LIB")
ENDIF()

# TODO: set up 64-bit build detection
# Boost Jam needs to have 64-bit build explicitly configured
IF(WIN32)
  SET(FORCE_64BIT_BUILD ON)
ENDIF()

# TODO: temporary - switch git tag back to master for all
#       builds once boost 1.56 is available and boost atomic
#       library fix is verified.
SET(boost_GIT_TAG "origin/python_fixes")

# TODO: fix install step
#
# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Boost_external
  DEPENDS ${boost_DEPENDENCIES}
  GIT_REPOSITORY "https://github.com/CIBC-Internal/boost.git"
  GIT_TAG ${boost_GIT_TAG}
  BUILD_IN_SOURCE ON
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  #CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${boost_BINARY_DIR}"
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DBUILD_PYTHON:BOOL=${BUILD_WITH_PYTHON}
    -DPython_DIR:PATH=${Python_DIR}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DFORCE_64BIT_BUILD:BOOL=${FORCE_64BIT_BUILD}
    -DSCI_BOOST_LIBRARIES:STATIC=${boost_Libraries}
    -DSCI_BOOST_CXX_FLAGS:STRING=${boost_CXX_Flags}
)
#-DSCI_PYTHON_INCLUDE:PATH=${SCI_PYTHON_INCLUDE}
#-DSCI_PYTHON_LIBRARY:FILEPATH=${SCI_PYTHON_LIBRARY}
#-DBUILD_SHARED_LIBS:BOOL=OFF
#-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
#-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#-DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
#-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
#-DCMAKE_ANSI_CFLAGS:STRING="${jpeg_pic}"

ExternalProject_Get_Property(Boost_external SOURCE_DIR)
SET(SCI_BOOST_INCLUDE ${SOURCE_DIR})

SET(BOOST_PREFIX "boost_")
SET(THREAD_POSTFIX "-mt")

# TODO: if static runtime link is supported, then ABI tag postfix must include s
# see:
# http://www.boost.org/doc/libs/1_57_0/more/getting_started/windows.html
# http://www.boost.org/doc/libs/1_57_0/more/getting_started/unix-variants.html

IF(WIN32)
  SET(DEBUG_POSTFIX "-gd")
ELSE()
  SET(DEBUG_POSTFIX "-d")
ENDIF()

IF(WIN32)
  SET(boost_LIB_PREFIX "lib")
ELSE()
  SET(boost_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
ENDIF()

# adding Boost as a build target and dependency
#
# TODO: how to make boost include dependent on Boost_external?
SET(SCI_BOOST_LIBRARY)
FOREACH(lib ${boost_Libraries})
  SET(FULL_LIB_NAME "${BOOST_PREFIX}${lib}${THREAD_POSTFIX}")
  LIST(APPEND SCI_BOOST_LIBRARY ${FULL_LIB_NAME})
  ADD_LIBRARY(${FULL_LIB_NAME} STATIC IMPORTED GLOBAL)
  ADD_DEPENDENCIES(${FULL_LIB_NAME} Boost_external)
  ADD_DEPENDENCIES(${FULL_LIB_NAME} ${SCI_BOOST_INCLUDE})

  MESSAGE(STATUS "Configure Boost library ${FULL_LIB_NAME}")

  #IF(CMAKE_GENERATOR MATCHES "Makefiles" AND CMAKE_BUILD_TYPE MATCHES "Debug")
  #  SET(boost_LIBRARY_PATH "${boost_BINARY_DIR}/lib/${boost_LIB_PREFIX}${FULL_LIB_NAME}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  #ELSE()
  #  SET(boost_LIBRARY_PATH "${boost_BINARY_DIR}/lib/${boost_LIB_PREFIX}${FULL_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  #ENDIF()

#  # other configuration options are RELWITHDEBINFO and MINSIZEREL
#  #
#  # TODO: debug and release builds are building in the same location - FIX THIS!!!
#  #SET(boost_LIBRARY_PATH_RELEASE "${boost_BINARY_DIR}/lib/${boost_LIB_PREFIX}${BOOST_PREFIX}${lib}${THREAD_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
#  #SET(boost_LIBRARY_PATH_DEBUG "${boost_BINARY_DIR}/lib/${boost_LIB_PREFIX}${BOOST_PREFIX}${lib}${THREAD_POSTFIX}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

  #IF (CMAKE_GENERATOR MATCHES "Makefiles")
  #  SET_TARGET_PROPERTIES(${lib}
  #    PROPERTIES
  #      IMPORTED_LOCATION ${boost_LIBRARY_PATH}
  #  )
  #ELSE() # IDEs: Xcode, VS, others...
  #  SET_TARGET_PROPERTIES(${lib}
  #    PROPERTIES
  #      IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
  #      IMPORTED_LOCATION_RELEASE ${boost_LIBRARY_PATH_RELEASE}
  #      IMPORTED_LOCATION_DEBUG ${boost_LIBRARY_PATH_DEBUG}
  #  )
  #ENDIF()
ENDFOREACH()

ExternalProject_Get_Property(Boost_external SOURCE_DIR)
ExternalProject_Get_Property(Boost_external INSTALL_DIR)

SET(SCI_BOOST_LIBRARY_DIR ${SOURCE_DIR}/lib)
SET(SCI_BOOST_USE_FILE ${INSTALL_DIR}/UseBoost.cmake)

# Boost is special case - normally this should be handled in external library repo
CONFIGURE_FILE(${SUPERBUILD_DIR}/BoostConfig.cmake.in ${INSTALL_DIR}/BoostConfig.cmake @ONLY)
CONFIGURE_FILE(${SUPERBUILD_DIR}/UseBoost.cmake ${SCI_BOOST_USE_FILE} COPYONLY)

SET(Boost_DIR ${INSTALL_DIR} CACHE PATH "")

MESSAGE(STATUS "Boost_DIR: ${Boost_DIR}")

