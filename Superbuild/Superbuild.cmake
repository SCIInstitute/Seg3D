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

###########################################
# TODO: build from archive - Git not used
###########################################

set(compress_type "GIT" CACHE INTERNAL "")
set(ep_base "${CMAKE_BINARY_DIR}/Externals" CACHE INTERNAL "")

###########################################
# DETERMINE ARCHITECTURE
# In order for the code to depend on the architecture settings
###########################################

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(SEG3D_BITS 64)
else()
  set(SEG3D_BITS 32)
endif()

# Hardcode (unfortunately) minumum OS X version for
# productbuild's Distribution.xml
set(OSX_MINIMUM_OS_VERSION "10.12" CACHE STRING "Set the minimum Mac OS X version for the installer package XML configuration file.")
mark_as_advanced(OSX_MINIMUM_OS_VERSION)

###########################################
# Set default CMAKE_BUILD_TYPE
# if empty for Unix Makefile builds
###########################################

if(CMAKE_GENERATOR MATCHES "Unix Makefiles" OR CMAKE_GENERATOR MATCHES "Ninja" AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

find_package(Git)

if(NOT GIT_FOUND)
  message(ERROR "Cannot find Git. Git is required for Seg3D's Superbuild")
endif()


###########################################
# Configure advanced features:
#   * large volume (bricked dataset) support
#   * mosaicing tools
###########################################

option(BUILD_LARGE_VOLUME_TOOLS "Build with large volume (bricked) dataset support." ON)
set(DEFAULT_MOSAIC_SETTING ON)
if(WIN32)
  # still highly experimental on Windows...
  set(DEFAULT_MOSAIC_SETTING OFF)
endif()
option(BUILD_MOSAIC_TOOLS "Build with mosaicing tool support." ${DEFAULT_MOSAIC_SETTING})

include( ExternalProject )

# Compute -G arg for configuring external projects with the same CMake generator:
#if(CMAKE_EXTRA_GENERATOR)
#  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
#else()
#  set(gen "${CMAKE_GENERATOR}" )
#endif()


###########################################
# Options for console, headless mode
###########################################

option(BUILD_TESTING "Build with tests." OFF)

option(SEG3D_BUILD_INTERFACE "Build the GUI interface to Seg3D" ON)
if(WIN32)
  option(SEG3D_SHOW_CONSOLE "Show console for debugging (Windows GUI build only)" OFF)
endif()


###########################################
# Configure python
###########################################

option(BUILD_WITH_PYTHON "Build with python support." ON)

###########################################
# Configure Seg3D library build
###########################################

if((WIN32) AND (MSVC_VERSION GREATER 1900))

  option(BUILD_STANDALONE_LIBRARY "Build with a Seg3D library build." OFF)

  if(BUILD_STANDALONE_LIBRARY)
    set(BUILD_TESTING OFF)
    set(BUILD_WITH_PYTHON OFF)
    set(SUPERBUILD_LIBS_SOURCE_DIR ${CMAKE_BINARY_DIR})
  endif()

endif()

option(BUILD_MANUAL_TOOLS_ONLY "Build Seg3D library with only manual tools." OFF)

set(ENABLED_WARNINGS "-Wall")

###########################################
# Configure Qt
###########################################

if(WIN32)
  option(DO_ZLIB_MANGLE "Mangle Zlib names" OFF)
else()
  option(DO_ZLIB_MANGLE "Mangle Zlib names to avoid conflicts with Qt5 or other external libraries" ON)
endif()

if(SEG3D_BUILD_INTERFACE)

  if(NOT DEFINED QT_MIN_VERSION)
    set(QT_MIN_VERSION "5.12")
  endif()

  set(Qt5_PATH "" CACHE PATH "Path to directory where Qt 5 is installed. Directory should contain lib and bin subdirectories.")
  #set(CMAKE_AUTOMOC ON)

  find_package(Qt5 COMPONENTS Core Gui OpenGL Svg REQUIRED HINTS ${Qt5_PATH})

  if(Qt5_FOUND)
    message(STATUS "Found Qt version: ${Qt5_VERSION}")

    if(${Qt5_VERSION} VERSION_LESS QT_MIN_VERSION)
      message(FATAL_ERROR "Qt ${QT_MIN_VERSION} or greater is required for building the Seg3D GUI")
    endif()
  else()
    message(FATAL_ERROR "Qt5 is required for building the Seg3D GUI. Set Qt5_PATH to directory where Qt 5 is installed (containing lib and bin subdirectories) or set SEG3D_BUILD_INTERFACE to OFF.")
  endif()

  if(APPLE)
    set(MACDEPLOYQT_OUTPUT_LEVEL 0 CACHE STRING "Set macdeployqt output level (0-3)")
    mark_as_advanced(MACDEPLOYQT_OUTPUT_LEVEL)
  endif()

endif()

###########################################
# Configure sample data download
###########################################

option(DOWNLOAD_DATA "Download Seg3D sample and test data repository." ON)

###########################################
# *Nix C++ compiler flags
###########################################

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${ENABLED_WARNINGS}")
  if(APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -ftemplate-depth=256 ${DISABLED_WARNINGS_CLANG}")
    set(CMAKE_CXX_FLAGS_DEBUG "-Wshorten-64-to-32 ${CMAKE_CXX_FLAGS_DEBUG}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive ${DISABLED_WARNINGS_GCC}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -ldl -lrt")
  endif()
endif()

###########################################
# Microsoft VC compiler flags
###########################################

if(WIN32 AND MSVC)
  # Enable Intrinsic Functions
  set(CMAKE_CXX_FLAGS "/Oi ${CMAKE_CXX_FLAGS}")
  # Build with multiple processes -- speeds up compilation on multi-processor machines.
  set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}")
endif()

###########################################
# Configure LaTeX and Doxygen documentation
###########################################

option(BUILD_DOCUMENTATION "Build documentation" OFF)
mark_as_advanced(BUILD_DOCUMENTATION)

find_package(LATEX)

if(BUILD_DOCUMENTATION AND NOT PDFLATEX_COMPILER)
  message(WARNING "LaTeX compiler not found. Disabling documentation build.")
  set(BUILD_DOCUMENTATION OFF)
endif()

if(BUILD_DOCUMENTATION)
  option(BUILD_DOXYGEN_DOCUMENTATION "Generate doxygen-based documentation." OFF)
  mark_as_advanced(BUILD_DOXYGEN_DOCUMENTATION)

  if(BUILD_DOXYGEN_DOCUMENTATION)
    find_package(Doxygen)

    if(NOT DOXYGEN_FOUND)
      message(WARNING "Doxygen not found. Disabling Doxygen documentation build.")
      set(BUILD_DOXYGEN_DOCUMENTATION OFF CACHE BOOL "Generate doxygen-based documentation." FORCE)
    endif()
  endif()
endif()

###########################################
# Configure externals
###########################################

set( Seg3D_DEPENDENCIES )

macro(ADD_EXTERNAL cmake_file external)
  include( ${cmake_file} )
  list(APPEND Seg3D_DEPENDENCIES ${external})
endmacro()

set(SUPERBUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
set(SEG3D_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src CACHE INTERNAL "" FORCE)
set(SEG3D_BINARY_DIR ${CMAKE_BINARY_DIR}/Seg3D CACHE INTERNAL "" FORCE)

if(DOWNLOAD_DATA)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/DataExternal.cmake Data_external )
endif()

ADD_EXTERNAL( ${SUPERBUILD_DIR}/ZlibExternal.cmake Zlib_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/GlewExternal.cmake Glew_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/FreetypeExternal.cmake Freetype_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/SQLiteExternal.cmake SQLite_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/LibPNGExternal.cmake LibPNG_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/TeemExternal.cmake Teem_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/ITKExternal.cmake ITK_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/TetgenExternal.cmake Tetgen_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/EigenExternal.cmake Eigen_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/ImplicitFunctionExternal.cmake ImplicitFunction_external )

if(BUILD_WITH_PYTHON)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/PythonExternal.cmake Python_external )
endif()

ADD_EXTERNAL( ${SUPERBUILD_DIR}/BoostExternal.cmake Boost_external )

set(SEG3D_CACHE_ARGS
    "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}"
    "-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    "-DCMAKE_OSX_SYSROOT:STRING=${CMAKE_OSX_SYSROOT}"
    "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}"
    "-DOSX_MINIMUM_OS_VERSION:STRING=${OSX_MINIMUM_OS_VERSION}"
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
    "-DBUILD_STANDALONE_LIBRARY:BOOL=${BUILD_STANDALONE_LIBRARY}"
    "-DBUILD_MANUAL_TOOLS_ONLY:BOOL=${BUILD_MANUAL_TOOLS_ONLY}"
    "-DDO_ZLIB_MANGLE:BOOL=${DO_ZLIB_MANGLE}"
    "-DSUPERBUILD_LIBS_SOURCE_DIR:PATH=${SUPERBUILD_LIBS_SOURCE_DIR}"
    "-DZlib_DIR:PATH=${Zlib_DIR}"
    "-DLibPNG_DIR:PATH=${LibPNG_DIR}"
    "-DSQLite_DIR:PATH=${SQLite_DIR}"
    "-DITK_DIR:PATH=${ITK_DIR}"
    "-DBoost_DIR:PATH=${Boost_DIR}"
    "-DFreetype_DIR:PATH=${Freetype_DIR}"
    "-DGlew_DIR:PATH=${Glew_DIR}"
    "-DTeem_DIR:PATH=${Teem_DIR}"
    "-DImplicitFunction_DIR:PATH=${ImplicitFunction_DIR}"
    "-DTetgen_DIR:PATH=${Tetgen_DIR}"
)

if(BUILD_WITH_PYTHON)
  # python executable will be release version in IDE builds
  list(APPEND SEG3D_CACHE_ARGS
    "-DPython_DIR:PATH=${Python_DIR}"
    "-DPYTHON_EXECUTABLE:FILEPATH=${SCI_PYTHON_EXE}"
  )
endif()

if(BUILD_DOCUMENTATION)
  list(APPEND SEG3D_CACHE_ARGS
    "-DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}"
    "-DBUILD_DOXYGEN_DOCUMENTATION:BOOL=${BUILD_DOXYGEN_DOCUMENTATION}"
    "-DPDFLATEX_COMPILER:FILEPATH=${PDFLATEX_COMPILER}"
    "-DBIBTEX_COMPILER:FILEPATH=${BIBTEX_COMPILER}"
    "-DDOXYGEN_EXECUTABLE:FILEPATH=${DOXYGEN_EXECUTABLE}"
  )
endif()


if(SEG3D_BUILD_INTERFACE)
  list(APPEND SEG3D_CACHE_ARGS
    "-DQt5_PATH:PATH=${Qt5_PATH}"
    "-DQt5Core_DIR:PATH=${Qt5Core_DIR}"
    "-DQt5Gui_DIR:PATH=${Qt5Gui_DIR}"
    "-DQt5OpenGL_DIR:PATH=${Qt5OpenGL_DIR}"
    "-DQt5Svg_DIR:PATH=${Qt5Svg_DIR}"
    "-DMACDEPLOYQT_OUTPUT_LEVEL:STRING=${MACDEPLOYQT_OUTPUT_LEVEL}"
  )
endif()

ExternalProject_Add( Seg3D_external
  DEPENDS ${Seg3D_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SEG3D_SOURCE_DIR}
  BINARY_DIR ${SEG3D_BINARY_DIR}
  CMAKE_CACHE_ARGS ${SEG3D_CACHE_ARGS}
  INSTALL_COMMAND ""
)
