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

###########################################
# TODO: build from archive - Git not used
###########################################

SET(compress_type "GIT" CACHE INTERNAL "")
SET(ep_base "${CMAKE_BINARY_DIR}/Externals" CACHE INTERNAL "")

###########################################
# DETERMINE ARCHITECTURE
# In order for the code to depend on the architecture settings
###########################################

IF(CMAKE_SIZEOF_VOID_P MATCHES 8)
  SET(SEG3D_BITS 64)
ELSE()
  SET(SEG3D_BITS 32)
ENDIF()

###########################################
# Set default CMAKE_BUILD_TYPE
# if empty for Unix Makefile builds
###########################################

IF(CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()

FIND_PACKAGE(Git)

IF(NOT GIT_FOUND)
  MESSAGE(ERROR "Cannot find Git. Git is required for Seg3D's Superbuild")
ENDIF()

###########################################
# Configure advanced features:
#   * large volume (bricked dataset) support
#   * mosaicing tools
###########################################

OPTION(BUILD_LARGE_VOLUME_TOOLS "Build with large volume (bricked) dataset support." ON)
SET(DEFAULT_MOSAIC_SETTING ON)
IF(WIN32)
  # still highly experimental on Windows...
  SET(DEFAULT_MOSAIC_SETTING OFF)
ENDIF()
OPTION(BUILD_MOSAIC_TOOLS "Build with mosaicing tool support." ${DEFAULT_MOSAIC_SETTING})

INCLUDE( ExternalProject )

# Compute -G arg for configuring external projects with the same CMake generator:
#if(CMAKE_EXTRA_GENERATOR)
#  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
#else()
#  set(gen "${CMAKE_GENERATOR}" )
#endif()

###########################################
# Options for console, headless mode
###########################################

OPTION(BUILD_TESTING "Build with tests." ON)

OPTION(SEG3D_BUILD_INTERFACE "Build the GUI interface to Seg3D" ON)
IF(WIN32)
  OPTION(SEG3D_SHOW_CONSOLE "Show console for debugging (Windows GUI build only)" OFF)
ENDIF()

###########################################
# Configure python
###########################################

OPTION(BUILD_WITH_PYTHON "Build with python support." ON)

###########################################
# Configure Qt
###########################################

IF(SEG3D_BUILD_INTERFACE)
  SET(QT_MIN_VERSION "4.6.0")
  INCLUDE(FindQt4)

  IF(QT4_FOUND)
    MESSAGE(STATUS "QTVERSION=${QTVERSION}")
    MESSAGE(STATUS "Found use file: ${QT_USE_FILE}")
    IF(APPLE AND ${QTVERSION} VERSION_EQUAL 4.8 AND ${QTVERSION} VERSION_LESS 4.8.5)
      MESSAGE(WARNING "Qt 4.8 versions earlier than 4.8.3 contain a bug that disables menu items under some circumstances. Upgrade to a more recent version.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "QT ${QT_MIN_VERSION} or later is required for building the Seg3D GUI")
  ENDIF()

  IF(APPLE)
    SET(MACDEPLOYQT_OUTPUT_LEVEL 0 CACHE STRING "Set macdeployqt output level (0-3)")
    MARK_AS_ADVANCED(MACDEPLOYQT_OUTPUT_LEVEL)
  ENDIF()

ENDIF()


###########################################
# *Nix C++ compiler flags
###########################################

IF(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
  IF(APPLE)
    SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -ftemplate-depth=256")
    SET(CMAKE_CXX_FLAGS_DEBUG "-Wshorten-64-to-32 ${CMAKE_CXX_FLAGS_DEBUG}")
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -ldl -lrt")
  ENDIF()
ENDIF()

###########################################
# Microsoft VC compiler flags
###########################################

IF(WIN32 AND MSVC)
  # Enable Intrinsic Functions
  SET(CMAKE_CXX_FLAGS "/Oi ${CMAKE_CXX_FLAGS}")
  # Build with multiple processes -- speeds up compilation on multi-processor machines.
  SET(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}")
ENDIF()

###########################################
# Configure LaTeX and Doxygen documentation
###########################################

OPTION(BUILD_DOCUMENTATION "Build documentation" OFF)
MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

FIND_PACKAGE(LATEX)

IF(BUILD_DOCUMENTATION AND NOT PDFLATEX_COMPILER)
  MESSAGE(WARNING "LaTeX compiler not found. Disabling documentation build.")
  SET(BUILD_DOCUMENTATION OFF)
ENDIF()

IF(BUILD_DOCUMENTATION)
  OPTION(BUILD_DOXYGEN_DOCUMENTATION "Generate doxygen-based documentation." OFF)
  MARK_AS_ADVANCED(BUILD_DOXYGEN_DOCUMENTATION)

  IF(BUILD_DOXYGEN_DOCUMENTATION)
    FIND_PACKAGE(Doxygen)

    IF(NOT DOXYGEN_FOUND)
      MESSAGE(WARNING "Doxygen not found. Disabling Doxygen documentation build.")
      SET(BUILD_DOXYGEN_DOCUMENTATION OFF CACHE BOOL "Generate doxygen-based documentation." FORCE)
    ENDIF()
  ENDIF()
ENDIF()

###########################################
# Configure externals
###########################################

SET( Seg3D_DEPENDENCIES )

MACRO(ADD_EXTERNAL cmake_file external)
  INCLUDE( ${cmake_file} )
  LIST(APPEND Seg3D_DEPENDENCIES ${external})
ENDMACRO()

SET(SUPERBUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
SET(SEG3D_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src CACHE INTERNAL "" FORCE)
SET(SEG3D_BINARY_DIR ${CMAKE_BINARY_DIR}/Seg3D CACHE INTERNAL "" FORCE)

ADD_EXTERNAL( ${SUPERBUILD_DIR}/DataExternal.cmake Data_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/ZlibExternal.cmake Zlib_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/GlewExternal.cmake Glew_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/FreetypeExternal.cmake Freetype_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/SQLiteExternal.cmake SQLite_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/LibPNGExternal.cmake LibPNG_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/TeemExternal.cmake Teem_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/ITKExternal.cmake ITK_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/ImplicitFunctionExternal.cmake ImplicitFunction_external )

IF(BUILD_WITH_PYTHON)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/PythonExternal.cmake Python_external )
ENDIF()

ADD_EXTERNAL( ${SUPERBUILD_DIR}/BoostExternal.cmake Boost_external )

SET(SEG3D_CACHE_ARGS
    "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-DSEG3D_SOURCE_DIR:PATH=${SEG3D_SOURCE_DIR}"
    "-DSEG3D_BINARY_DIR:PATH=${SEG3D_BINARY_DIR}"
    "-DBUILD_LARGE_VOLUME_TOOLS:BOOL=${BUILD_LARGE_VOLUME_TOOLS}"
    "-DBUILD_MOSAIC_TOOLS:BOOL=${BUILD_MOSAIC_TOOLS}"
    "-DSEG3D_BITS:STRING=${SEG3D_BITS}"
    "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
    "-DTEST_INPUT_PATH:PATH=${TEST_INPUT_PATH}"
    "-DCMAKE_CXX_FLAGS:STATIC=${CMAKE_CXX_FLAGS}"
    "-DCMAKE_CXX_FLAGS_DEBUG:STATIC=${CMAKE_CXX_FLAGS_DEBUG}"
    "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}"
    "-DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD:STATIC=${CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD}"
    "-DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY:STATIC=${CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY}"
    "-DSEG3D_BUILD_INTERFACE:BOOL=${SEG3D_BUILD_INTERFACE}"
    "-DSEG3D_SHOW_CONSOLE:BOOL=${SEG3D_SHOW_CONSOLE}"
    "-DBUILD_WITH_PYTHON:BOOL=${BUILD_WITH_PYTHON}"
    "-DZlib_DIR:PATH=${Zlib_DIR}"
    "-DLibPNG_DIR:PATH=${LibPNG_DIR}"
    "-DSQLite_DIR:PATH=${SQLite_DIR}"
    "-DITK_DIR:PATH=${ITK_DIR}"
    "-DBoost_DIR:PATH=${Boost_DIR}"
    "-DFreetype_DIR:PATH=${Freetype_DIR}"
    "-DGlew_DIR:PATH=${Glew_DIR}"
    "-DTeem_DIR:PATH=${Teem_DIR}"
    "-DImplicitFunction_DIR:PATH=${ImplicitFunction_DIR}"
)

IF(BUILD_WITH_PYTHON)
  # python executable will be release version in IDE builds
  LIST(APPEND SEG3D_CACHE_ARGS
    "-DPython_DIR:PATH=${Python_DIR}"
    "-DPYTHON_EXECUTABLE:FILEPATH=${SCI_PYTHON_EXE}"
  )
ENDIF()

IF(BUILD_DOCUMENTATION)
  LIST(APPEND SEG3D_CACHE_ARGS
    "-DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}"
    "-DBUILD_DOXYGEN_DOCUMENTATION:BOOL=${BUILD_DOXYGEN_DOCUMENTATION}"
    "-DPDFLATEX_COMPILER:FILEPATH=${PDFLATEX_COMPILER}"
    "-DBIBTEX_COMPILER:FILEPATH=${BIBTEX_COMPILER}"
    "-DDOXYGEN_EXECUTABLE:FILEPATH=${DOXYGEN_EXECUTABLE}"
  )
ENDIF()

IF(SEG3D_BUILD_INTERFACE)
  LIST(APPEND SEG3D_CACHE_ARGS
    "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
    "-DMACDEPLOYQT_OUTPUT_LEVEL:STRING=${MACDEPLOYQT_OUTPUT_LEVEL}"
  )
ENDIF()

ExternalProject_Add( Seg3D_external
  DEPENDS ${Seg3D_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SEG3D_SOURCE_DIR}
  BINARY_DIR ${SEG3D_BINARY_DIR}
  CMAKE_CACHE_ARGS ${SEG3D_CACHE_ARGS}
  INSTALL_COMMAND ""
)
