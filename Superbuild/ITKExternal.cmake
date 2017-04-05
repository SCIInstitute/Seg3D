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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

SET(itk_ARGS
  "-DBUILD_SHARED_LIBS:BOOL=OFF"
  "-DITK_BUILD_SHARED_LIBS:BOOL=OFF"
  "-DBUILD_EXAMPLES:BOOL=OFF"
  "-DBUILD_TESTING:BOOL=OFF"
)

IF(TRAVIS_BUILD)
  LIST(APPEND itk_ARGS
    "-Wno-dev"
    "-Wno-deprecated"
  )

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
ENDIF()

SET(itk_CACHE_ARGS
  "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
  "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
  "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
  "-DITK_BUILD_DEFAULT_MODULES:BOOL=OFF"
  "-DITKV3_COMPATIBILITY:BOOL=ON"
#  "-DITK_INSTALL_NO_LIBRARIES:BOOL=ON"
#  "-DITK_INSTALL_NO_DEVELOPMENT:BOOL=ON"
#  "-DITK_INSTALL_NO_RUNTIME:BOOL=ON"
#  "-DITK_INSTALL_NO_DOCUMENTATION:BOOL=ON"
#  "-DGDCM_INSTALL_NO_DEVELOPMENT:BOOL=ON"
#  "-DGDCM_INSTALL_NO_RUNTIME:BOOL=ON"
#  "-DGDCM_INSTALL_NO_DOCUMENTATION:BOOL=ON"
#  "-DHDF5_INSTALL_NO_DEVELOPMENT:BOOL=ON"
#  "-DModule_ITKReview:BOOL=ON"
  "-DModule_ITKRegistrationCommon:BOOL=ON"
  "-DModule_ITKSmoothing:BOOL=ON"
  "-DModule_ITKAnisotropicSmoothing:BOOL=ON"
  "-DModule_ITKRegionGrowing:BOOL=ON"
  "-DModule_ITKMathematicalMorphology:BOOL=ON"
  "-DModule_ITKBinaryMathematicalMorphology:BOOL=ON"
  "-DModule_ITKDistanceMap:BOOL=ON"
  "-DModule_ITKLevelSets:BOOL=ON"
  "-DModule_ITKWatersheds:BOOL=ON"
  "-DModule_ITKImageFusion:BOOL=ON"
  "-DModule_ITKHDF5:BOOL=ON"
  "-DModule_ITKIOHDF5:BOOL=ON"
  "-DModule_ITKIOLSM:BOOL=ON"
  "-DModule_ITKIOJPEG:BOOL=ON"
  "-DModule_ITKIOGIPL:BOOL=ON"
  "-DModule_IITKIOMeta:BOOL=ON"
  "-DModule_ITKIONRRD:BOOL=ON"
  "-DModule_ITKDeprecated:BOOL=ON"
  "-DCMAKE_CXX_FLAGS:STATIC=${CMAKE_CXX_FLAGS}"
  "-DCMAKE_CXX_FLAGS_DEBUG:STATIC=${CMAKE_CXX_FLAGS_DEBUG}"
  "-DCMAKE_C_FLAGS:STATIC=${CMAKE_C_FLAGS}"
  "-DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD:STATIC=${CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD}"
  "-DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY:STATIC=${CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY}"
)

IF(BUILD_MOSAIC_TOOLS)
  LIST(APPEND itk_CACHE_ARGS
    "-DITK_USE_FFTWD:BOOL=ON"
    "-DITK_USE_FFTWF:BOOL=ON"
    "-DModule_ITKImageIntensity:BOOL=ON"
    "-DModule_ITKThresholding:BOOL=ON"
    "-DModule_ITKTransformFactory:BOOL=ON"
  )
ENDIF()

SET(itk_GIT_TAG "origin/master")

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(ITK_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/itk.git"
  GIT_TAG ${itk_GIT_TAG}
  PATCH_COMMAND ""
  CMAKE_ARGS ${itk_ARGS}
  CMAKE_CACHE_ARGS ${itk_CACHE_ARGS}
)

# hardcoded, since we need this before ITK's configure step
ExternalProject_Get_Property(ITK_external INSTALL_DIR)
SET(ITK_DIR "${INSTALL_DIR}/lib/cmake/ITK-4.10" CACHE PATH "")

MESSAGE(STATUS "ITK_DIR=${ITK_DIR}")
