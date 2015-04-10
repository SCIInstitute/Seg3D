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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# TODO: create variable for python module library
#SET(PYTHON_MODULE_SEARCH_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pythonlib.zip" CACHE INTERNAL "Python modules." FORCE)
#SET(PYTHON_MODULE_SEARCH_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" CACHE INTERNAL "Python modules." FORCE)

#IF(WIN32)
#  SET(python_LIB_PREFIX "")
#ELSE()
#  SET(python_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
#ENDIF()

#SET(python_LIBRARY_PATH "${python_BINARY_DIR}/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
# other configuration options are RELWITHDEBINFO and MINSIZEREL
##SET(python_LIBRARY_PATH_RELEASE "${python_BINARY_DIR}/Release/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
##SET(python_LIBRARY_PATH_DEBUG "${python_BINARY_DIR}/Debug/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")

SET(python_GIT_TAG "origin/seg3d_external_test")
SET(python_DEPENDENCIES "Zlib_external")

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Python_external
  DEPENDS ${python_DEPENDENCIES}
  GIT_REPOSITORY "https://github.com/CIBC-Internal/python.git"
  GIT_TAG ${python_GIT_TAG}
  PATCH_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    #-DPYTHONLIB_DST_PATH:STRING=${CMAKE_BINARY_DIR}
    #-DPYTHON_MODULE_SEARCH_PATH:INTERNAL=${PYTHON_MODULE_SEARCH_PATH}
    #-DSCI_ZLIB_MANGLE:BOOL=${SCI_ZLIB_MANGLE}
    -DZlib_DIR:PATH=${Zlib_DIR}
)

ExternalProject_Get_Property(Python_external BINARY_DIR)
SET(Python_DIR ${BINARY_DIR} CACHE PATH "")

MESSAGE(STATUS "Python_DIR: ${Python_DIR}")
