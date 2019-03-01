#
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
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9 FATAL_ERROR)
#CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12 FATAL_ERROR)
#CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)
IF(POLICY CMP0020)
  CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()
IF(POLICY CMP0049)
  CMAKE_POLICY(SET CMP0049 NEW)
ENDIF()
IF(POLICY CMP0043)
  CMAKE_POLICY(SET CMP0043 OLD)
ENDIF()
IF(POLICY CMP0053)
  CMAKE_POLICY(SET CMP0053 OLD)
ENDIF()
IF(POLICY CMP0054)
  CMAKE_POLICY(SET CMP0054 NEW)
ENDIF()


###########################################
# Verify build directory
###########################################

IF( ${CMAKE_BINARY_DIR} MATCHES ${CMAKE_SOURCE_DIR} )
  MESSAGE( FATAL_ERROR "Please delete the CMakeCache.txt from the src directory and build Seg3D in the bin directory. Run 'rm CMakeCache.txt; cd ../bin; cmake ../src' from the binary directory." )
ENDIF()


###########################################
# PROJECT: Seg3D
###########################################

PROJECT(Seg3D)

###########################################
# Where to compile executables
###########################################

IF(NOT SEG3D_BINARY_DIR)
  SET(SEG3D_BINARY_DIR ${CMAKE_BINARY_DIR})
ENDIF()


###########################################
# DO STANDARD TESTS
###########################################

INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckCSourceCompiles)


###########################################
# Configure testing support.
# This should be done before any
# MESSAGE(FATAL_ERROR ...) commands are
# invoked.
###########################################

INCLUDE( CTest )
IF(BUILD_TESTING)
  ENABLE_TESTING()
  IF(WIN32)
    ADD_DEFINITIONS(-DGTEST_USE_OWN_TR1_TUPLE=0 -D_VARIADIC_MAX=10)
  ENDIF()
ENDIF()

INCLUDE(CMake/RegisterUnitTest.cmake)

###########################################
# SEG3D VERSION NUMBERS
###########################################

SET(SEG3D_MAJOR_VERSION 2 CACHE INTERNAL "Major version number" FORCE)
SET(SEG3D_MINOR_VERSION 4 CACHE INTERNAL "Minor version number" FORCE)
SET(SEG3D_PATCH_VERSION 4 CACHE INTERNAL "Patch version number" FORCE)


###########################################
# APPLICATION Information
###########################################

SET(APPLICATION_NAME "Seg3D2" CACHE INTERNAL "Name of the application" FORCE)
SET(APPLICATION_EXTENSION ".s3d" CACHE INTERNAL "Name of the extension" FORCE)
SET(APPLICATION_URL "www.seg3d.org" CACHE INTERNAL "URL where more information can be found" FORCE)
SET(APPLICATION_VENDOR "SCI" CACHE INTERNAL "Who generated the applicatio" FORCE)
SET(APPLICATION_CONTACT "seg3d-users@sci.utah.edu" CACHE INTERNAL "Contact information" FORCE)
SET(APPLICATION_WINDOWS_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons/seg3d2_32.ico" CACHE INTERNAL "Windows icon" FORCE)
SET(APPLICATION_WINDOWS_RC "${CMAKE_SOURCE_DIR}/Resources/Icons/Seg3D.rc" CACHE INTERNAL "Windows icon" FORCE)
SET(APPLICATION_OSX_ICON_FULL_PATH "${CMAKE_SOURCE_DIR}/Resources/Icons/Seg3D.icns" CACHE INTERNAL "OSX icon" FORCE)
SET(APPLICATION_OSX_ICON "Seg3D.icns" CACHE INTERNAL "OSX icon" FORCE)


###########################################
# PACKAGING Information
###########################################

SET(PACKAGING_README "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ReadMe.txt" CACHE INTERNAL "Where to find the readme file." FORCE )
SET(PACKAGING_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/License.txt" CACHE INTERNAL "Where to find the license file." FORCE )
SET(PACKAGING_RELEASENOTES "${CMAKE_CURRENT_SOURCE_DIR}/ReleaseNotes.txt" CACHE INTERNAL "Where to find the releasenotes file." FORCE )
SET(PACKAGING_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Welcome.txt" CACHE INTERNAL "Where to find the welcome file." FORCE )
SET(PACKAGING_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\seg3d2_256.png" CACHE INTERNAL "Packing icon." FORCE )

###########################################
# DETERMINE ARCHITECTURE
# In order for the code to depend on the
# architecture settings
###########################################

IF(SEG3D_BITS MATCHES 64)
  SET(MSVC90_BUILD "amd64")
  ADD_DEFINITIONS(-DSCI_64BITS)
ELSE()
  SET(MSVC90_BUILD "x86")
  ADD_DEFINITIONS(-DSCI_32BITS)
ENDIF()

IF(BUILD_STANDALONE_LIBRARY)
  MESSAGE("Library build successful")
  ADD_DEFINITIONS(-DBUILD_STANDALONE_LIBRARY)
ENDIF()

IF(BUILD_MANUAL_TOOLS_ONLY)
  ADD_DEFINITIONS(-DBUILD_MANUAL_TOOLS_ONLY)
ENDIF()

###########################################
# Microsoft VC compiler flags
###########################################

IF(WIN32 AND MSVC)
  ADD_DEFINITIONS(-D_ALLOW_KEYWORD_MACROS)
  ADD_DEFINITIONS(-D_WIN32_WINNT=_WIN32_WINNT_WS03 -DNTDDI_VERSION=NTDDI_WS03SP2)
  ADD_DEFINITIONS(-DPSAPI_VERSION=1)
  # Disable Visual C++ Secure Warnings
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)
  ADD_DEFINITIONS(-D_BIND_TO_CURRENT_CRT_VERSION=1)
ENDIF()


###########################################
# Where to install command line tools, docs
# and other extras on OS X
###########################################

SET(OSX_UTILS_DIR "Seg3DUtils")


############################################
# Set Up External Projects
############################################

MACRO(CONFIG_STANDARD_EXTERNAL name config_file directory)
  FIND_PACKAGE(${name} CONFIGS ${config_file} HINTS ${directory} NO_SYSTEM_ENVIRONMENT_PATH)
  IF(NOT ${name}_FOUND)
    MESSAGE(FATAL_ERROR "${name} library not found in ${directory}")
  ENDIF()
ENDMACRO()

CONFIG_STANDARD_EXTERNAL( Zlib ZlibConfig.cmake ${Zlib_DIR} )
INCLUDE(${ZLIB_USE_FILE})

CONFIG_STANDARD_EXTERNAL( LibPNG LibPNGConfig.cmake ${LibPNG_DIR} )
INCLUDE(${PNG_USE_FILE})

CONFIG_STANDARD_EXTERNAL( SQLite SQLiteConfig.cmake ${SQLite_DIR} )
INCLUDE(${SQLITE_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Teem TeemConfig.cmake ${Teem_DIR} )
INCLUDE(${Teem_USE_FILE})

IF(BUILD_WITH_PYTHON)
  CONFIG_STANDARD_EXTERNAL( Python PythonConfig.cmake ${Python_DIR} )
  INCLUDE(${SCI_PYTHON_USE_FILE})
ENDIF()

CONFIG_STANDARD_EXTERNAL( Boost BoostConfig.cmake ${Boost_DIR} )
INCLUDE(${SCI_BOOST_USE_FILE})

CONFIG_STANDARD_EXTERNAL( ITK ITKConfig.cmake ${ITK_DIR} )
INCLUDE(${ITK_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Glew GlewConfig.cmake ${Glew_DIR} )
INCLUDE(${GLEW_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Freetype FreetypeConfig.cmake ${Freetype_DIR} )
INCLUDE(${FREETYPE_USE_FILE})

CONFIG_STANDARD_EXTERNAL( ImplicitFunction ImplicitFunctionConfig.cmake ${ImplicitFunction_DIR} )
INCLUDE(${ImplicitFunction_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Tetgen TetgenConfig.cmake ${Tetgen_DIR} )
INCLUDE(${TETGEN_USE_FILE})


###########################################
# Global defines
###########################################

# To force a static build these flags need to be set project wide.
ADD_DEFINITIONS(-DTINYXML_STATIC)
ADD_DEFINITIONS(-DTIXML_USE_STL)

# define z_prefix project wide if zlib was mangled
IF (DO_ZLIB_MANGLE)
  ADD_DEFINITIONS(-DZ_PREFIX)
ENDIF()


###########################################
# RPATH settings for Linux
###########################################

IF(UNIX)
  MESSAGE(STATUS "Configuring RPATH settings")
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF()

#  ###########################################
#  # Find QT
#  # We need QT - We use a modified version of
#  # the FindQt4.cmake file that can be set
#  # to search for different versions of QT
#  # just point QT_DIR to the right directory
#  # to skip the use of the default installed
#  # QT distribution.
#  ###########################################
#
IF(SEG3D_BUILD_INTERFACE)
  # see http://doc.qt.io/qt-5/cmake-manual.html
  FIND_PACKAGE(Qt5Core REQUIRED HINTS ${Qt5_PATH})
  GET_TARGET_PROPERTY(QtCore_location_Release Qt5::Core LOCATION_Release)
  #GET_TARGET_PROPERTY(QtCore_location_Debug Qt5::Core LOCATION_Debug)

  FIND_PACKAGE(Qt5Gui REQUIRED HINTS ${Qt5_PATH})
  GET_TARGET_PROPERTY(QtGui_location_Release Qt5::Gui LOCATION_Release)

  FIND_PACKAGE(Qt5OpenGL REQUIRED HINTS ${Qt5_PATH})
  GET_TARGET_PROPERTY(QtOpenGL_location_Release Qt5::OpenGL LOCATION_Release)

  FIND_PACKAGE(Qt5Widgets REQUIRED HINTS ${Qt5_PATH})
  GET_TARGET_PROPERTY(QtWidgets_location_Release Qt5::Widgets LOCATION_Release)

  FIND_PACKAGE(Qt5Svg REQUIRED HINTS ${Qt5_PATH})
  GET_TARGET_PROPERTY(QtSvg_location_Release Qt5::Svg LOCATION_Release)

  IF(NOT Qt5Widgets_FOUND OR NOT Qt5Gui_FOUND)
    MESSAGE(FATAL_ERROR "Qt5 is required for building the Seg3D GUI")
  ENDIF()

  ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

  # should be in ${Qt5_PATH}/bin
  IF(APPLE)
    FIND_PROGRAM(MACDEPLOYQT_EXE
      HINTS ${Qt5_PATH} ${Qt5_PATH}/bin
      NAMES macdeployqt
      NO_DEFAULT_PATH
    )
    IF(MACDEPLOYQT_EXE MATCHES "NOTFOUND")
      MESSAGE(FATAL_ERROR "macdeployqt is required to build the Seg3D app bundle. Please ensure it is installed and in your path.")
    ENDIF()
    SET(MACDEPLOYQT ${MACDEPLOYQT_EXE} CACHE INTERNAL "macdeployqt executable")
    MARK_AS_ADVANCED(MACDEPLOYQT)
  ELSE()
    IF(WIN32)
      FIND_PROGRAM(WINDEPLOYQT_EXE
        HINTS ${Qt5_PATH} ${Qt5_PATH}/bin
        NAMES windeployqt.exe
      )
      IF (WINDEPLOYQT_EXE MATCHES "NOTFOUND")
        MESSAGE(FATAL_ERROR "windeployqt.exe is required to build the Seg3D installer.  Please ensure that it is installed and in your path.  It should be found within the Qt5 install path.")
      ENDIF()
      SET(WINDEPLOYQT ${WINDEPLOYQT_EXE} CACHE INTERNAL "windeployqt executable")
      MARK_AS_ADVANCED(WINDEPLOYQT)
    ENDIF()
  ENDIF()

ENDIF()

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

ADD_SUBDIRECTORY(ThirdParty)

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
    ENDIF()
  ENDIF()
ENDIF()

###########################################
# Set Revision information for GLP
###########################################

INCLUDE(${CMAKE_SOURCE_DIR}/CMake/RevisionInfo.cmake)
GENERATE_REVISION_INFO()

###########################################
# Build ApplicationConfiguration.h
# this file contains information such as
# version numbers and architecture
# on which code can depend.
###########################################

CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/Configuration/ApplicationConfiguration.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/ApplicationConfiguration.h
  @ONLY )

CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/Configuration/InterfaceConfiguration.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/InterfaceConfiguration.h
  @ONLY )

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

###########################################
# Set the include paths
###########################################

INCLUDE_DIRECTORIES(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5OpenGL_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

# TODO: make use files for thirdparty projects
INCLUDE_DIRECTORIES(${SCI_MATLABIO_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_MRC2000IO_INCLUDE})
INCLUDE_DIRECTORIES(${SCI_TINYXML_INCLUDE})

IF(BUILD_WITH_PYTHON)
  INCLUDE(CMake/GenerateActionPythonWrapper.cmake)
ENDIF()

IF(BUILD_TESTING)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/submodules/googletest/googletest/include)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/submodules/googletest/googlemock/include)
ENDIF()

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

IF(BUILD_LARGE_VOLUME_TOOLS)
  MESSAGE(STATUS "Configuring Large Volume tools")
  ADD_SUBDIRECTORY(LargeVolumeUtils)
ENDIF()

MESSAGE(STATUS "Configuring Seg3D Interface")
IF(SEG3D_BUILD_INTERFACE)
  ADD_SUBDIRECTORY(QtUtils)
  ADD_SUBDIRECTORY(Interface)
ENDIF()

###########################################
# Plugins:
# Plugin libraries need be generated before
# we build the registration objects as they
# may contain as well pieces that need to
# be registered in the main object
# factories.
###########################################

SET(IGNORE_RESOURCES OFF CACHE INTERNAL "Ignore default resources library.")
IF(NOT SEG3D_BUILD_INTERFACE)
  SET(IGNORE_RESOURCES ON CACHE INTERNAL "Ignore default resources library.")
ENDIF()

MESSAGE(STATUS "Configuring Seg3D Plugins")
FILE(GLOB PLUGIN_DIRS "Plugins/*" )

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/Plugins")

FOREACH(DIR ${PLUGIN_DIRS})
  IF(IS_DIRECTORY ${DIR})
    ADD_SUBDIRECTORY(${DIR})
  ENDIF()
ENDFOREACH()

###########################################
# Resources:
# images + icons
###########################################

IF(NOT IGNORE_RESOURCES)
  MESSAGE(STATUS "Configuring Seg3D Resources")
  IF(SEG3D_BUILD_INTERFACE)
    ADD_SUBDIRECTORY(Resources)
  ENDIF()
ENDIF()


###########################################
# Main:
# Main needs to be included after custom
# plugins
###########################################

IF(SEG3D_BUILD_INTERFACE)
  MESSAGE(STATUS "Configuring Seg3D main")
  ADD_SUBDIRECTORY(Main)
ELSE()
  MESSAGE(STATUS "Configuring Seg3D headless main")
  ADD_SUBDIRECTORY(HeadlessMain)
ENDIF()


###########################################
# Build the plugin generator files
# This code will automatically register all
# the components with the proper object
# factories
###########################################

MESSAGE(STATUS "Configuring Seg3D registered functions")
GENERATE_REGISTRATION_FILES()


###########################################
# Generate the python functions registration file
###########################################

IF(BUILD_WITH_PYTHON)
  MESSAGE(STATUS "Configuring action python wrapper functions")
  GENERATE_ACTION_PYTHON_WRAPPER_REGISTRATION_FILE()
ENDIF()


###########################################
# Configure LaTeX and Doxygen documentation
###########################################

IF(BUILD_DOCUMENTATION)
  ADD_SUBDIRECTORY(Documentation)
ENDIF()

IF(BUILD_DOXYGEN_DOCUMENTATION)
  ADD_SUBDIRECTORY(Documentation/Doxygen)
ENDIF()


###########################################
# Bootstrap
# Add code for packaging shaders and other
# objects into the executable
###########################################

ADD_SUBDIRECTORY(Bootstrap)


###########################################
# Configure testing code
###########################################

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()


###########################################
# Packaging:
# This part sets up the packager using
# CPack.
###########################################

MESSAGE(STATUS "Configuring Seg3D packaging")

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "${APPLICATION_NAME}")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME}")
SET(CPACK_PACKAGE_VENDOR "${APPLICATION_VENDOR}")

SET(CPACK_PACKAGE_DESCRIPTION_FILE ${PACKAGING_README} )
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME} ${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}.${SEG3D_PATCH_VERSION} is a segmentation program")

SET(CPACK_RESOURCE_FILE_LICENSE ${PACKAGING_LICENSE})
SET(CPACK_RESOURCE_FILE_README ${PACKAGING_RELEASENOTES})
SET(CPACK_RESOURCE_FILE_WELCOME ${PACKAGING_WELCOME})

SET(CPACK_PACKAGE_VERSION_MAJOR "${SEG3D_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${SEG3D_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${SEG3D_PATCH_VERSION}")
SET(CPACK_PACKAGE_VERSION "${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}.${SEG3D_PATCH_VERSION}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${APPLICATION_NAME}_${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}")

IF(WIN32 AND NOT UNIX)
  # There is a bug in NSIS that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  SET(CPACK_PACKAGE_ICON "${PACKAGING_ICON}")

  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  ENDIF()

  IF(SEG3D_BITS MATCHES 64)
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  ELSE()
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
  ENDIF()

  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${APPLICATION_NAME}")
  SET(CPACK_NSIS_MUI_ICON "${APPLICATION_WINDOWS_ICON}")
  SET(CPACK_NSIS_DISPLAY_NAME "${APPLICATION_NAME} ${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\${APPLICATION_URL}")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\${APPLICATION_URL}")
  SET(CPACK_NSIS_CONTACT ${APPLICATION_CONTACT} )
  SET(CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}" "${APPLICATION_NAME}")
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    WriteRegStr HKCR '.nrrd' '' '${APPLICATION_NAME}-nrrd'
    WriteRegStr HKCR '${APPLICATION_NAME}-nrrd' '' '${APPLICATION_NAME} NRRD File'
    WriteRegStr HKCR '${APPLICATION_NAME}-nrrd\\\\shell' '' 'open'
    WriteRegStr HKCR '${APPLICATION_NAME}-nrrd\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe,0'
    WriteRegStr HKCR '${APPLICATION_NAME}-nrrd\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
    WriteRegStr HKCR '${APPLICATION_NAME}-nrrd\\\\shell\\\\edit' '' 'Open NRRD Data File'
    WriteRegStr HKCR '${APPLICATION_NAME}-nrrd\\\\shell\\\\edit\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
    #System::Call 'Shell32::SHChangeNotify( i 0x800000, i 0, i 0, i 0)'
    WriteRegStr HKCR '.nhdr' '' '${APPLICATION_NAME}-nhdr'
    WriteRegStr HKCR '${APPLICATION_NAME}-nhdr' '' '${APPLICATION_NAME} NHDR File'
    WriteRegStr HKCR '${APPLICATION_NAME}-nhdr\\\\shell' '' 'open'
    WriteRegStr HKCR '${APPLICATION_NAME}-nhdr\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe,0'
    WriteRegStr HKCR '${APPLICATION_NAME}-nhdr\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
    WriteRegStr HKCR '${APPLICATION_NAME}-nhdr\\\\shell\\\\edit' '' 'Open NHDR Data File'
    WriteRegStr HKCR '${APPLICATION_NAME}-nhdr\\\\shell\\\\edit\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
    #System::Call 'Shell32::SHChangeNotify( i 0x800000, i 0, i 0, i 0)'
    WriteRegStr HKCR '${APPLICATION_EXTENSION}' '' '${APPLICATION_NAME}'
    WriteRegStr HKCR '${APPLICATION_NAME}' '' '${APPLICATION_NAME} Project File'
    WriteRegStr HKCR '${APPLICATION_NAME}\\\\shell' '' 'open'
    WriteRegStr HKCR '${APPLICATION_NAME}\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe,0'
    WriteRegStr HKCR '${APPLICATION_NAME}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
    WriteRegStr HKCR '${APPLICATION_NAME}\\\\shell\\\\edit' '' 'Open ${APPLICATION_NAME} Project File'
    WriteRegStr HKCR '${APPLICATION_NAME}\\\\shell\\\\edit\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
    System::Call 'Shell32::SHChangeNotify( i 0x800000, i 0, i 0, i 0)'
  ")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    DeleteRegKey HKCR '.nrrd'
    DeleteRegKey HKCR '.nhdr'
    DeleteRegKey HKCR '${APPLICATION_EXTENSION}'
    DeleteRegKey HKCR '${APPLICATION_NAME}'
  ")
ELSE()
  IF(APPLE)
    SET(CPACK_GENERATOR "productbuild;TGZ")
    SET(CPACK_TOPLEVEL_TAG "")
    IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
      SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    ENDIF()
    # productbuild packaging repair hack
    # call repair_package.sh after running package target in build
    # Tip: can inspect packages using the pkgutil --expand command
    IF(CPACK_GENERATOR MATCHES "productbuild")
      # make sure there are paths to packaging tools
      FIND_PROGRAM(PKGBUILD_EXE pkgbuild)
      FIND_PROGRAM(PRODUCTBUILD_EXE productbuild)
      FIND_PROGRAM(PKGUTIL_EXE pkgutil)
      SET(PKG_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Darwin")
      SET(PKG_BUILD_DIR "${CMAKE_BINARY_DIR}/_CPack_Packages/productbuild/${PKG_FILE_NAME}")
      SET(PKG_ROOT_DIR "${PKG_BUILD_DIR}/Applications")

      CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/Modules/CPack.Distribution.dist.in
                     ${CMAKE_BINARY_DIR}/CPack.Distribution.dist)
      CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/Scripts/repair_package.sh.in
                     ${CMAKE_BINARY_DIR}/repair_package.sh)
    ENDIF()
  ELSE()
    SET(CPACK_STRIP_FILES TRUE)
    SET(CPACK_SOURCE_STRIP_FILES TRUE)
    SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}" "${APPLICATION_NAME}")
  ENDIF()
ENDIF()

INCLUDE(CPack)
