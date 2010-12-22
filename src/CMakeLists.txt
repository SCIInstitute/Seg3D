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

MESSAGE( STATUS "${CMAKE_ROOT}")

###########################################
# WHERE TO COMPILE EXECUTABLES
###########################################

SET(SEG3D_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/tmp)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/tmp)

###########################################
# COMPILE OPTIONS
###########################################

SET(SEG3D_BUILD_INTERFACE 1 CACHE BOOL "Build the interface to Seg3D")

###########################################
# SEG3D VERSION NUMBERS
###########################################

SET(SEG3D_MAJOR_VERSION 2)
SET(SEG3D_MINOR_VERSION 0)
SET(SEG3D_PATCH_VERSION 0)

###########################################
# DO STANDARD TESTS
###########################################

INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckCSourceCompiles)

###########################################
# DETERMINE ARCHITECTURE
# In order for the code to depend on the
# architecture settings
###########################################

IF (CMAKE_SIZEOF_VOID_P MATCHES 8)
  SET(SEG3D_BITS "64bit")
  SET(MSVC90_BUILD "amd64")
  ADD_DEFINITIONS(-DSCI_64BITS)
ELSE (CMAKE_SIZEOF_VOID_P MATCHES 8)
  SET(SEG3D_BITS "32bit")
  SET(MSVC90_BUILD "x86")
  ADD_DEFINITIONS(-DSCI_32BITS)
  IF (WIN32)
  ADD_DEFINITIONS(-DBOOST_BIND_ENABLE_STDCALL)
  ENDIF (WIN32)
ENDIF (CMAKE_SIZEOF_VOID_P MATCHES 8)

###########################################
# SETUP DEFINTIONS FOR EXTERNAL PACKAGES
# To force a static build these flags
# need to be set project wide.
###########################################

ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_DEFINITIONS(-DTEEM_STATIC)
ADD_DEFINITIONS(-DLIBXML_STATIC)
ADD_DEFINITIONS(-DTINYXML_STATIC)
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
ADD_DEFINITIONS(-DTIXML_USE_STL)
  
SET(ITK_BUILD_SHARED_LIBS "STATIC")

###########################################
# Flags for Microsoft Compiler
# Disable Visual C++ Secure Warnings
# Enable Intrinsic Functions
###########################################

IF (WIN32 AND MSVC)
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)
  ADD_DEFINITIONS(-DPSAPI_VERSION=1)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  SET(CMAKE_CXX_FLAGS "/Oi ${CMAKE_CXX_FLAGS}")
ENDIF (WIN32 AND MSVC)


###########################################
# RPATH settings for Linux
###########################################

IF(UNIX)
  MESSAGE(STATUS "Configuring RPATH settings")
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF(UNIX)

###########################################
# Find QT
# We need QT - We use a modified version of
# the FindQt4.cmake file that can be set
# to search for different versions of QT
# just point QT_DIR to the right directory
# to skip the use of the default installed
# QT distribution. 
###########################################

IF(SEG3D_BUILD_INTERFACE)

SET(QT_MIN_VERSION "4.6.0")
SET(QT_DIR "" CACHE PATH "Path to QT")
INCLUDE(CMake/FindQt4.cmake)
SET(QT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CMake/UseQt4.cmake)

IF (QT_FOUND)
  SET(QT_USE_QTOPENGL 1)
  INCLUDE(${QT_USE_FILE})
ELSE (QT_FOUND)
  MESSAGE(SEND_ERROR "QT 4.6.0 or later is required for building Seg3D")
ENDIF (QT_FOUND)

ENDIF(SEG3D_BUILD_INTERFACE)

###########################################
# Configure python
OPTION(BUILD_WITH_PYTHON "Build with python support." OFF)
IF(BUILD_WITH_PYTHON)
  ADD_DEFINITIONS(-DBUILD_WITH_PYTHON 
          -DPy_NO_ENABLE_SHARED 
          -DBOOST_PYTHON_STATIC_MODULE
          -DBOOST_PYTHON_STATIC_LIB 
          -DLANDMARK=L"lib\\\\pythonlib.zip"
          -DPYTHONPATH=L".\\\\lib\\\\pythonlib.zip")
  FILE(COPY Externals/python/pythonlib.zip DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
  FILE(COPY Externals/python/pythonlib.zip DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE})
ENDIF(BUILD_WITH_PYTHON)

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
  SET(SEG3D_DEBUG_VERSION "\"(Debug)\"")
ELSE(CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(SEG3D_DEBUG_VERSION "\"\"")
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

###########################################
# Check for "XInitThreads" on X-window
# systems.
###########################################

IF(NOT WIN32 AND NOT APPLE)
  SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} X11)

  CHECK_C_SOURCE_COMPILES(
  "
  #include <X11/Xlib.h>

  int main()
  {
    XInitThreads();
    return 1;
  }
  "
  X11_THREADSAFE
  )
  IF(${X11_THREADSAFE})
  MESSAGE(STATUS "XInitThreads found")
  OPTION(MULTITHREADED_RENDERING "Enable multi-threaded rendering." OFF)
  IF(MULTITHREADED_RENDERING)
    ADD_DEFINITIONS(-DX11_THREADSAFE)
  ENDIF(MULTITHREADED_RENDERING)
  ENDIF(${X11_THREADSAFE})
ENDIF(NOT WIN32 AND NOT APPLE)

###########################################
# Build ApplicationConfiguration.h
# this file contains information such as
# version numbers and architecture
# on which code can depend.
###########################################

CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/Configuration/ApplicationConfiguration.h.in 
                ${CMAKE_CURRENT_BINARY_DIR}/ApplicationConfiguration.h 
                @ONLY )
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

###########################################
# Set the include paths
###########################################

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${SCI_BOOST_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_ZLIB_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_LIBPNG_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_TEEM_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_GLEW_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_GDCM_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_FREETYPE_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_ITK_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_TINYXML_INCLUDE})

IF(BUILD_WITH_PYTHON)
  INCLUDE_DIRECTORIES(${SCI_PYTHON_INCLUDE})
  ADD_DEFINITIONS(-DPy_NO_ENABLE_SHARED)
ENDIF(BUILD_WITH_PYTHON)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

###########################################
# Generate a variable for registering all
# the plugins
###########################################

INCLUDE(CMake/Core.cmake)
INCLUDE(CMake/RegisterClass.cmake)

###########################################

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")

###########################################
# Application layers:
#  Core - Core functionality
#  Application - All pieces need for the
#          application specific to Seg3D
#  Interface - Seg3D interface
#  Main - Main function of the application
#
# Auxillary layers:
#  QtUtils - binding QtWidgets and abstract
#            class around Qt implementations
#  Resources - Icons, images, etc.
###########################################

MESSAGE(STATUS "Configuring Seg3D Core")
ADD_SUBDIRECTORY(Core)

MESSAGE(STATUS "Configuring Seg3D Application")
ADD_SUBDIRECTORY(Application)


MESSAGE(STATUS "Configuring Seg3D Interface")
IF(SEG3D_BUILD_INTERFACE)
  ADD_SUBDIRECTORY(Resources)
  ADD_SUBDIRECTORY(QtUtils)
  ADD_SUBDIRECTORY(Interface)
ENDIF(SEG3D_BUILD_INTERFACE)

###########################################
# Plugins:
# Plugin libraries need be generated before
# we build the registration objects as they
# may contain as well pieces that need to
# be registered in the main object 
# factories.
###########################################

MESSAGE(STATUS "Configuring Seg3D Plugins")
FILE(GLOB PLUGIN_DIRS "Plugins/*" )

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/Plugins")

FOREACH(DIR ${PLUGIN_DIRS})
  IF(NOT ${DIR} MATCHES "Plugins/.svn")
  IF(IS_DIRECTORY ${DIR}) 
      ADD_SUBDIRECTORY(${DIR})
  ENDIF(IS_DIRECTORY ${DIR}) 
  ENDIF(NOT ${DIR} MATCHES "Plugins/.svn")
ENDFOREACH(DIR ${PLUGIN_DIRS})

###########################################
# Main:
# Main needs to be included after custom 
# plugins
###########################################


IF(SEG3D_BUILD_INTERFACE)
  MESSAGE(STATUS "Configuring Seg3D main")
  ADD_SUBDIRECTORY(Main)
ENDIF(SEG3D_BUILD_INTERFACE)


###########################################
# Build the plugin generator files
# This code will automatically register all
# the components with the proper object
# factories
###########################################

MESSAGE(STATUS "Configuring Seg3D registered functions")
GENERATE_REGISTRATION_FILES()

###########################################
# Bootstrap
# Add code for packaging shaders and other
# objects into the executable
###########################################

ADD_SUBDIRECTORY(Bootstrap)

###########################################
# Packaging:
# This part sets up the packager using
# CPack.
###########################################

MESSAGE(STATUS "Configuring Seg3D packaging")

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Seg3D2")
SET(CPACK_PACKAGE_VENDOR "SCI")
      
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ReadMe.txt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Seg3D2 ${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION} is a segmentation program")

SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/License.txt")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Welcome.txt")

SET(CPACK_PACKAGE_VERSION_MAJOR "${SEG3D_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${SEG3D_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${SEG3D_PATCH_VERSION}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY 
    "Seg3D ${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}")
    
    
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSIS that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\Seg3D.bmp")
    IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
      SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Seg3D")
  SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\Seg3D.ico")
  SET(CPACK_NSIS_DISPLAY_NAME "Seg3D ${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.seg3d.org")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.seg3d.org")
  SET(CPACK_NSIS_CONTACT "seg3d-users@sci.utah.edu")
  SET(CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_PACKAGE_EXECUTABLES "Seg3D2" "Seg3D2")
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
  WriteRegStr HKCR '.nrrd' '' 'Seg3D2'
  WriteRegStr HKCR 'Seg3D2' '' 'Seg3D2 Data File'
  WriteRegStr HKCR 'Seg3D2\\\\shell' '' 'open'
  WriteRegStr HKCR 'Seg3D2\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\Seg3D2.exe,0'
  WriteRegStr HKCR 'Seg3D2\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\Seg3D2.exe \\\"%1\\\"'
  WriteRegStr HKCR 'Seg3D2\\\\shell\\\\edit' '' 'Edit Seg3D2 Data File'
  WriteRegStr HKCR 'Seg3D2\\\\shell\\\\edit\\\\command' '' '$INSTDIR\\\\bin\\\\Seg3D2.exe \\\"%1\\\"'
  System::Call 'Shell32::SHChangeNotify( i 0x800000, i 0, i 0, i 0)'
  ")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
  DeleteRegKey HKCR '.nrrd'
  DeleteRegKey HKCR 'Seg3D2'
  ")
ELSE(WIN32 AND NOT UNIX)
  IF(APPLE)
    SET(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications/")
    SET(CPACK_TOPLEVEL_TAG "")
    IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
      SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
    SET(CPACK_STRIP_FILES TRUE)
    SET(CPACK_SOURCE_STRIP_FILES TRUE)
    SET(CPACK_PACKAGE_EXECUTABLES "Seg3D2.app" "Seg3D2")
  ELSE(APPLE)
    SET(CPACK_STRIP_FILES TRUE)
    SET(CPACK_SOURCE_STRIP_FILES TRUE)
    SET(CPACK_PACKAGE_EXECUTABLES "Seg3D2" "Seg3D2")
  ENDIF(APPLE)
ENDIF(WIN32 AND NOT UNIX)

INCLUDE(CPack)

