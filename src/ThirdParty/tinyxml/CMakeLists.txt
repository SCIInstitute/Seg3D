
###########################################
# TinyXML project

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(TinyXML)

###########################################
# Test for some required system information.

INCLUDE (CMakeBackwardCompatibilityC)

###########################################
# Source files for zlib

SET(TINYXML_SRCS
  tinystr.cpp
  tinystr.h
  tinyxml.cpp
  tinyxml.h
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)

###########################################
# Set the name of the library

SET(SCI_TINYXML_LIBRARY tinyxml CACHE INTERNAL "TinyXML library name." FORCE)

###########################################
# Include directory of tinyxml

SET(SCI_TINYXML_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR} 
    CACHE INTERNAL "TinyXML include directories." FORCE)
INCLUDE_DIRECTORIES(${SCI_TINYXML_INCLUDE})

###########################################
# Ensure it uses STL

ADD_DEFINITIONS(-DTIXML_USE_STL)

###########################################
# Add the library

ADD_LIBRARY(${SCI_TINYXML_LIBRARY} STATIC ${TINYXML_SRCS})

###########################################
# Set Windows options (Visual Studio)
# Force Visual Studio to link against the 
# version that is in the redistributable
# Options for Visual Studio to limit 
# the number of warnings.

IF(WIN32)
  ADD_DEFINITIONS(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)

  IF(NOT UNIX AND NOT BORLAND AND NOT MINGW)
#    SET_TARGET_PROPERTIES(${SCI_TINYXML_LIBRARY}
#                            PROPERTIES
#                            COMPILE_FLAGS "/Ob1 /Gy /GF -W3 -wd4267 -wd4244"
#                            LINK_FLAGS "/DYNAMICBASE:NO")
  ENDIF(NOT UNIX AND NOT BORLAND AND NOT MINGW)
ENDIF(WIN32)
