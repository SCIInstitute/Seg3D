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

cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)
#cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
#cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
if(POLICY CMP0049)
  cmake_policy(SET CMP0049 NEW)
endif()
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 OLD)
endif()
if(POLICY CMP0053)
  cmake_policy(SET CMP0053 OLD)
endif()
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

###########################################
# Verify build directory
###########################################

if( ${CMAKE_BINARY_DIR} MATCHES ${CMAKE_SOURCE_DIR} )
  message( FATAL_ERROR "Please delete the CMakeCache.txt from the src directory and build Seg3D in the bin directory. Run 'rm CMakeCache.txt; cd ../bin; cmake ../src' from the binary directory." )
endif()


###########################################
# PROJECT: Seg3D
###########################################

project(Seg3D)

###########################################
# Where to compile executables
###########################################

if(NOT SEG3D_BINARY_DIR)
  set(SEG3D_BINARY_DIR ${CMAKE_BINARY_DIR})
endif()


###########################################
# DO STANDARD TESTS
###########################################

include(CMakeBackwardCompatibilityC)
include(CMakeBackwardCompatibilityCXX)
include(CheckCSourceCompiles)


###########################################
# Configure testing support.
# This should be done before any
# message(FATAL_ERROR ...) commands are
# invoked.
###########################################

include( CTest )
if(BUILD_TESTING)
  enable_testing()
  if(WIN32)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=0 -D_VARIADIC_MAX=10)
  endif()
endif()

include(CMake/RegisterUnitTest.cmake)

###########################################
# SEG3D VERSION NUMBERS
###########################################

set(SEG3D_MAJOR_VERSION 2 CACHE INTERNAL "Major version number" FORCE)
set(SEG3D_MINOR_VERSION 5 CACHE INTERNAL "Minor version number" FORCE)
set(SEG3D_PATCH_VERSION 0 CACHE INTERNAL "Patch version number" FORCE)


###########################################
# APPLICATION Information
###########################################

set(APPLICATION_NAME "Seg3D2" CACHE INTERNAL "Name of the application" FORCE)
set(APPLICATION_EXTENSION ".s3d" CACHE INTERNAL "Name of the extension" FORCE)
set(APPLICATION_URL "www.seg3d.org" CACHE INTERNAL "URL where more information can be found" FORCE)
set(APPLICATION_VENDOR "SCI" CACHE INTERNAL "Who generated the applicatio" FORCE)
set(APPLICATION_CONTACT "seg3d-users@sci.utah.edu" CACHE INTERNAL "Contact information" FORCE)
set(APPLICATION_WINDOWS_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons/seg3d2_32.ico" CACHE INTERNAL "Windows icon" FORCE)
set(APPLICATION_WINDOWS_RC "${CMAKE_SOURCE_DIR}/Resources/Icons/Seg3D.rc" CACHE INTERNAL "Windows icon" FORCE)
set(APPLICATION_OSX_ICON_FULL_PATH "${CMAKE_SOURCE_DIR}/Resources/Icons/Seg3D.icns" CACHE INTERNAL "OSX icon" FORCE)
set(APPLICATION_OSX_ICON "Seg3D.icns" CACHE INTERNAL "OSX icon" FORCE)


###########################################
# PACKAGING Information
###########################################

set(PACKAGING_README "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ReadMe.txt" CACHE INTERNAL "Where to find the readme file." FORCE )
set(PACKAGING_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/CMake/License.txt" CACHE INTERNAL "Where to find the license file." FORCE )
set(PACKAGING_RELEASENOTES "${CMAKE_CURRENT_SOURCE_DIR}/ReleaseNotes.txt" CACHE INTERNAL "Where to find the releasenotes file." FORCE )
set(PACKAGING_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Welcome.txt" CACHE INTERNAL "Where to find the welcome file." FORCE )
set(PACKAGING_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\seg3d2_256.png" CACHE INTERNAL "Packing icon." FORCE )

###########################################
# DETERMINE ARCHITECTURE
# In order for the code to depend on the
# architecture settings
###########################################

if(SEG3D_BITS MATCHES 64)
  set(MSVC90_BUILD "amd64")
  add_definitions(-DSCI_64BITS)
else()
  set(MSVC90_BUILD "x86")
  add_definitions(-DSCI_32BITS)
endif()

if(BUILD_STANDALONE_LIBRARY)
  message("Library build successful")
  add_definitions(-DBUILD_STANDALONE_LIBRARY)
endif()

if(BUILD_MANUAL_TOOLS_ONLY)
  add_definitions(-DBUILD_MANUAL_TOOLS_ONLY)
endif()

###########################################
# Microsoft VC compiler flags
###########################################

if(WIN32 AND MSVC)
  add_definitions(-D_ALLOW_KEYWORD_MACROS)
  add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WS03 -DNTDDI_VERSION=NTDDI_WS03SP2)
  add_definitions(-DPSAPI_VERSION=1)
  # Disable Visual C++ Secure Warnings
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)
  add_definitions(-D_BIND_TO_CURRENT_CRT_VERSION=1)
endif()


###########################################
# Where to install command line tools, docs
# and other extras on OS X
###########################################

set(OSX_UTILS_DIR "Seg3DUtils")


############################################
# Set Up External Projects
############################################

macro(CONFIG_STANDARD_EXTERNAL name config_file directory)
  find_package(${name} CONFIGS ${config_file} HINTS ${directory} NO_SYSTEM_ENVIRONMENT_PATH)
  if(NOT ${name}_FOUND)
    message(FATAL_ERROR "${name} library not found in ${directory}")
  endif()
endmacro()

CONFIG_STANDARD_EXTERNAL( Zlib ZlibConfig.cmake ${Zlib_DIR} )
include(${ZLIB_USE_FILE})

CONFIG_STANDARD_EXTERNAL( LibPNG LibPNGConfig.cmake ${LibPNG_DIR} )
include(${PNG_USE_FILE})

CONFIG_STANDARD_EXTERNAL( SQLite SQLiteConfig.cmake ${SQLite_DIR} )
include(${SQLITE_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Teem TeemConfig.cmake ${Teem_DIR} )
include(${Teem_USE_FILE})

if(BUILD_WITH_PYTHON)
  CONFIG_STANDARD_EXTERNAL( Python PythonConfig.cmake ${Python_DIR} )
  include(${SCI_PYTHON_USE_FILE})
endif()

CONFIG_STANDARD_EXTERNAL( Boost BoostConfig.cmake ${Boost_DIR} )
include(${SCI_BOOST_USE_FILE})

CONFIG_STANDARD_EXTERNAL( ITK ITKConfig.cmake ${ITK_DIR} )
include(${ITK_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Glew GlewConfig.cmake ${Glew_DIR} )
include(${GLEW_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Freetype FreetypeConfig.cmake ${Freetype_DIR} )
include(${FREETYPE_USE_FILE})

CONFIG_STANDARD_EXTERNAL( ImplicitFunction ImplicitFunctionConfig.cmake ${ImplicitFunction_DIR} )
include(${ImplicitFunction_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Tetgen TetgenConfig.cmake ${Tetgen_DIR} )
include(${TETGEN_USE_FILE})


###########################################
# Global defines
###########################################

# To force a static build these flags need to be set project wide.
add_definitions(-DTINYXML_STATIC)
add_definitions(-DTIXML_USE_STL)

# define z_prefix project wide if zlib was mangled
if (DO_ZLIB_MANGLE)
  add_definitions(-DZ_PREFIX)
endif()


###########################################
# RPATH settings for Linux
###########################################

if(UNIX)
  message(STATUS "Configuring RPATH settings")
  set(CMAKE_SKIP_BUILD_RPATH  FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

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
if(SEG3D_BUILD_INTERFACE)
  # see http://doc.qt.io/qt-5/cmake-manual.html
  find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets Svg REQUIRED HINTS ${Qt5_PATH})
  
  get_target_property(QtCore_location_Release Qt5::Core LOCATION_Release)
  #get_target_property(QtCore_location_Debug Qt5::Core LOCATION_Debug)
  get_target_property(QtGui_location_Release Qt5::Gui LOCATION_Release)
  get_target_property(QtOpenGL_location_Release Qt5::OpenGL LOCATION_Release)
  get_target_property(QtWidgets_location_Release Qt5::Widgets LOCATION_Release)
  get_target_property(QtSvg_location_Release Qt5::Svg LOCATION_Release)

  if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 is required for building the Seg3D GUI")
  endif()

  add_definitions(-DQT_NO_KEYWORDS)

  # should be in ${Qt5_PATH}/bin
  if(APPLE)
    find_program(MACDEPLOYQT_EXE
      HINTS ${Qt5_PATH} ${Qt5_PATH}/bin
      NAMES macdeployqt
      NO_DEFAULT_PATH
    )
    if(MACDEPLOYQT_EXE MATCHES "NOTFOUND")
      message(FATAL_ERROR "macdeployqt is required to build the Seg3D app bundle. Please ensure it is installed and in your path.")
    endif()
    set(MACDEPLOYQT ${MACDEPLOYQT_EXE} CACHE INTERNAL "macdeployqt executable")
    mark_as_advanced(MACDEPLOYQT)
  else()
    if(WIN32)
      find_program(WINDEPLOYQT_EXE
        HINTS ${Qt5_PATH} ${Qt5_PATH}/bin
        NAMES windeployqt.exe
      )
      if (WINDEPLOYQT_EXE MATCHES "NOTFOUND")
        message(FATAL_ERROR "windeployqt.exe is required to build the Seg3D installer.  Please ensure that it is installed and in your path.  It should be found within the Qt5 install path.")
      endif()
      set(WINDEPLOYQT ${WINDEPLOYQT_EXE} CACHE INTERNAL "windeployqt executable")
      mark_as_advanced(WINDEPLOYQT)
    endif()
  endif()

endif()

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

add_subdirectory(ThirdParty)

###########################################
# Check for "XInitThreads" on X-window
# systems.
###########################################

if(NOT WIN32 AND NOT APPLE)
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} X11)

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
  if(${X11_THREADSAFE})
    message(STATUS "XInitThreads found")
    option(MULTITHREADED_RENDERING "Enable multi-threaded rendering." OFF)
    if(MULTITHREADED_RENDERING)
      add_definitions(-DX11_THREADSAFE)
    endif()
  endif()
endif()

###########################################
# Set Revision information for GLP
###########################################

include(${CMAKE_SOURCE_DIR}/CMake/RevisionInfo.cmake)
GENERATE_REVISION_INFO()

###########################################
# Build ApplicationConfiguration.h
# this file contains information such as
# version numbers and architecture
# on which code can depend.
###########################################

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Configuration/ApplicationConfiguration.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/ApplicationConfiguration.h
  @ONLY )

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Configuration/InterfaceConfiguration.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/InterfaceConfiguration.h
  @ONLY )

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

###########################################
# Set the include paths
###########################################

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5OpenGL_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

# TODO: make use files for thirdparty projects
include_directories(${SCI_MATLABIO_INCLUDE})
include_directories(${SCI_MRC2000IO_INCLUDE})
include_directories(${SCI_TINYXML_INCLUDE})

if(BUILD_WITH_PYTHON)
  include(CMake/GenerateActionPythonWrapper.cmake)
endif()

if(BUILD_TESTING)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/submodules/googletest/googletest/include)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/submodules/googletest/googlemock/include)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

###########################################
# Generate a variable for registering all
# the plugins
###########################################

include(CMake/Core.cmake)
include(CMake/RegisterClass.cmake)

###########################################

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")

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

message(STATUS "Configuring Seg3D Core")
add_subdirectory(Core)

message(STATUS "Configuring Seg3D Application")
add_subdirectory(Application)

if(BUILD_LARGE_VOLUME_TOOLS)
  message(STATUS "Configuring Large Volume tools")
  add_subdirectory(LargeVolumeUtils)
endif()

message(STATUS "Configuring Seg3D Interface")
if(SEG3D_BUILD_INTERFACE)
  add_subdirectory(QtUtils)
  add_subdirectory(Interface)
endif()

###########################################
# Plugins:
# Plugin libraries need be generated before
# we build the registration objects as they
# may contain as well pieces that need to
# be registered in the main object
# factories.
###########################################

set(IGNORE_RESOURCES OFF CACHE INTERNAL "Ignore default resources library.")
if(NOT SEG3D_BUILD_INTERFACE)
  set(IGNORE_RESOURCES ON CACHE INTERNAL "Ignore default resources library.")
endif()

message(STATUS "Configuring Seg3D Plugins")
file(GLOB PLUGIN_DIRS "Plugins/*" )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Plugins")

foreach(DIR ${PLUGIN_DIRS})
  if(IS_DIRECTORY ${DIR})
    add_subdirectory(${DIR})
  endif()
endforeach()

###########################################
# Resources:
# images + icons
###########################################

if(NOT IGNORE_RESOURCES)
  message(STATUS "Configuring Seg3D Resources")
  if(SEG3D_BUILD_INTERFACE)
    add_subdirectory(Resources)
  endif()
endif()


###########################################
# Main:
# Main needs to be included after custom
# plugins
###########################################

if(SEG3D_BUILD_INTERFACE)
  message(STATUS "Configuring Seg3D main")
  add_subdirectory(Main)
else()
  message(STATUS "Configuring Seg3D headless main")
  add_subdirectory(HeadlessMain)
endif()


###########################################
# Build the plugin generator files
# This code will automatically register all
# the components with the proper object
# factories
###########################################

message(STATUS "Configuring Seg3D registered functions")
GENERATE_REGISTRATION_FILES()


###########################################
# Generate the python functions registration file
###########################################

if(BUILD_WITH_PYTHON)
  message(STATUS "Configuring action python wrapper functions")
  GENERATE_ACTION_PYTHON_WRAPPER_REGISTRATION_FILE()
endif()


###########################################
# Configure LaTeX and Doxygen documentation
###########################################

if(BUILD_DOCUMENTATION)
  add_subdirectory(Documentation)
endif()

if(BUILD_DOXYGEN_DOCUMENTATION)
  add_subdirectory(Documentation/Doxygen)
endif()


###########################################
# Bootstrap
# Add code for packaging shaders and other
# objects into the executable
###########################################

add_subdirectory(Bootstrap)


###########################################
# Configure testing code
###########################################

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()


###########################################
# Packaging:
# This part sets up the packager using
# CPack.
###########################################

message(STATUS "Configuring Seg3D packaging")

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "${APPLICATION_NAME}")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME}")
set(CPACK_PACKAGE_VENDOR "${APPLICATION_VENDOR}")

set(CPACK_PACKAGE_DESCRIPTION_FILE ${PACKAGING_README} )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME} ${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}.${SEG3D_PATCH_VERSION} is a segmentation program")

set(CPACK_RESOURCE_FILE_LICENSE ${PACKAGING_LICENSE})
set(CPACK_RESOURCE_FILE_README ${PACKAGING_RELEASENOTES})
set(CPACK_RESOURCE_FILE_WELCOME ${PACKAGING_WELCOME})

set(CPACK_PACKAGE_VERSION_MAJOR "${SEG3D_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${SEG3D_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${SEG3D_PATCH_VERSION}")
set(CPACK_PACKAGE_VERSION "${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}.${SEG3D_PATCH_VERSION}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${APPLICATION_NAME}_${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}")

if(WIN32 AND NOT UNIX)
  # There is a bug in NSIS that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  set(CPACK_PACKAGE_ICON "${PACKAGING_ICON}")

  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  endif()

  if(SEG3D_BITS MATCHES 64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  else()
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
  endif()

  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${APPLICATION_NAME}")
  set(CPACK_NSIS_MUI_ICON "${APPLICATION_WINDOWS_ICON}")
  set(CPACK_NSIS_DISPLAY_NAME "${APPLICATION_NAME} ${SEG3D_MAJOR_VERSION}.${SEG3D_MINOR_VERSION}")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\${APPLICATION_URL}")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\${APPLICATION_URL}")
  set(CPACK_NSIS_CONTACT ${APPLICATION_CONTACT} )
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}" "${APPLICATION_NAME}")
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
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
  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    DeleteRegKey HKCR '.nrrd'
    DeleteRegKey HKCR '.nhdr'
    DeleteRegKey HKCR '${APPLICATION_EXTENSION}'
    DeleteRegKey HKCR '${APPLICATION_NAME}'
  ")
else()
  if(APPLE)
    set(CPACK_GENERATOR "productbuild;TGZ")
    set(CPACK_TOPLEVEL_TAG "")
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      set(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
      set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    endif()
    # productbuild packaging repair hack
    # call repair_package.sh after running package target in build
    # Tip: can inspect packages using the pkgutil --expand command
    if(CPACK_GENERATOR MATCHES "productbuild")
      # make sure there are paths to packaging tools
      find_program(PKGBUILD_EXE pkgbuild)
      find_program(PRODUCTBUILD_EXE productbuild)
      find_program(PKGUTIL_EXE pkgutil)
      set(PKG_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Darwin")
      set(PKG_BUILD_DIR "${CMAKE_BINARY_DIR}/_CPack_Packages/productbuild/${PKG_FILE_NAME}")
      set(PKG_ROOT_DIR "${PKG_BUILD_DIR}/Applications")

      configure_file(${CMAKE_SOURCE_DIR}/CMake/Modules/CPack.Distribution.dist.in
                     ${CMAKE_BINARY_DIR}/CPack.Distribution.dist)
      configure_file(${CMAKE_SOURCE_DIR}/CMake/Scripts/repair_package.sh.in
                     ${CMAKE_BINARY_DIR}/repair_package.sh)
    endif()
  else()
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_SOURCE_STRIP_FILES TRUE)
    set(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}" "${APPLICATION_NAME}")
  endif()
endif()

include(CPack)
