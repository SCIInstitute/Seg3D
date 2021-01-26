
###########################################
# TinyXML project

cmake_minimum_required(VERSION 2.6)
project(TinyXML)

###########################################
# Test for some required system information.

include (CMakeBackwardCompatibilityC)

###########################################
# Source files for zlib

set(TINYXML_SRCS
  tinystr.cpp
  tinystr.h
  tinyxml.cpp
  tinyxml.h
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)

###########################################
# Set the name of the library

set(SCI_TINYXML_LIBRARY tinyxml CACHE INTERNAL "TinyXML library name." FORCE)

###########################################
# Include directory of tinyxml

set(SCI_TINYXML_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "TinyXML include directories." FORCE)
include_directories(${SCI_TINYXML_INCLUDE})

###########################################
# Ensure it uses STL

add_definitions(-DTIXML_USE_STL)

###########################################
# Add the library

add_library(${SCI_TINYXML_LIBRARY} STATIC ${TINYXML_SRCS})

###########################################
# Set Windows options (Visual Studio)
# Force Visual Studio to link against the 
# version that is in the redistributable
# Options for Visual Studio to limit 
# the number of warnings.

if(WIN32)
  add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)

  if(NOT UNIX AND NOT BORLAND AND NOT MINGW)
#    set_target_properties(${SCI_TINYXML_LIBRARY}
#                            PROPERTIES
#                            COMPILE_FLAGS "/Ob1 /Gy /GF -W3 -wd4267 -wd4244"
#                            LINK_FLAGS "/DYNAMICBASE:NO")
  endif()
endif()
