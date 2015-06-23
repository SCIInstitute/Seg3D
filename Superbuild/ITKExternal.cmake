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

SET(itk_ARGS
  "-DBUILD_SHARED_LIBS:BOOL=OFF"
  "-DITK_BUILD_SHARED_LIBS:BOOL=OFF"
  "-DBUILD_EXAMPLES:BOOL=OFF"
  "-DBUILD_TESTING:BOOL=OFF"
)

SET(itk_CACHE_ARGS
  "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
  "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
  "-DITK_INSTALL_NO_LIBRARIES:BOOL=ON"
  "-DITK_INSTALL_NO_DEVELOPMENT:BOOL=ON"
  "-DITK_INSTALL_NO_RUNTIME:BOOL=ON"
  "-DITK_INSTALL_NO_DOCUMENTATION:BOOL=ON"
  "-DGDCM_INSTALL_NO_DEVELOPMENT:BOOL=ON"
  "-DGDCM_INSTALL_NO_RUNTIME:BOOL=ON"
  "-DGDCM_INSTALL_NO_DOCUMENTATION:BOOL=ON"
  "-DHDF5_INSTALL_NO_DEVELOPMENT:BOOL=ON"
  "-DModule_ITKReview:BOOL=ON"
  "-DModule_ITKDeprecated:BOOL=ON"
  "-DITKV3_COMPATIBILITY:BOOL=ON"
  "-DCMAKE_CXX_FLAGS:STATIC=${CMAKE_CXX_FLAGS}"
  "-DCMAKE_CXX_FLAGS_DEBUG:STATIC=${CMAKE_CXX_FLAGS_DEBUG}"
  "-DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD:STATIC=${CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD}"
  "-DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY:STATIC=${CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY}"
)

IF(BUILD_MOSAIC_TOOLS)
  #SET(ITK_USE_FFTWD ON CACHE BOOL "" FORCE)
  #SET(ITK_USE_FFTWF ON CACHE BOOL "" FORCE)
  LIST(APPEND itk_CACHE_ARGS
    "-DITK_USE_FFTWD:BOOL=ON"
    "-DITK_USE_FFTWF:BOOL=ON"
  )
ENDIF()

SET(itk_GIT_TAG "origin/seg3d_external_test")

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(ITK_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/itk.git"
  GIT_TAG ${itk_GIT_TAG}
  PATCH_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_ARGS ${itk_ARGS}
  CMAKE_CACHE_ARGS ${itk_CACHE_ARGS}
)

ExternalProject_Get_Property(ITK_external BINARY_DIR)
SET(ITK_DIR ${BINARY_DIR} CACHE PATH "")

MESSAGE(STATUS "ITK_DIR=${ITK_DIR}")
