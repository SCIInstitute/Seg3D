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

# TODO: build from archive - Git not used
SET(compress_type "GIT" CACHE INTERNAL "")
SET(ep_base "${CMAKE_BINARY_DIR}/Externals" CACHE INTERNAL "")

FIND_PACKAGE(Git)

IF(NOT GIT_FOUND)
  MESSAGE(ERROR "Cannot find Git. Git is required for Seg3D's Superbuild")
ENDIF()

#OPTION( USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)
#SET(git_protocol "https")

INCLUDE( ExternalProject )

# Compute -G arg for configuring external projects with the same CMake generator:
#if(CMAKE_EXTRA_GENERATOR)
#  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
#else()
#  set(gen "${CMAKE_GENERATOR}" )
#endif()

SET( Seg3D_DEPENDENCIES )

#set(ep_common_args
#    "-DCMAKE_BUILD_TYPE:STRING=Release"
#)
#
#include( ${CMAKE_SOURCE_DIR}/External-ITK.cmake )
#list( APPEND ITKWikiExamples_DEPENDENCIES ITK )

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/ZlibExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES Zlib_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/GlewExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES Glew_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/FreetypeExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES Freetype_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/SQLiteExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES SQLite_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/LibPNGExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES LibPNG_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/PythonExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES Python_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/ITKExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES ITK_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/BoostExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES Boost_external)

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/Superbuild/TeemExternal.cmake )
LIST(APPEND Seg3D_DEPENDENCIES Teem_external)

ExternalProject_Add( Seg3D
  DEPENDS ${Seg3D_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}
  #CMAKE_ARGS
  #  ${ep_common_args}
  #  -DBUILD_SHARED_LIBS:BOOL=FALSE
  #  -DBUILD_TESTING:BOOL=FALSE
  #   # ITK
  #  -DITK_DIR:PATH=${ITK_DIR}
  #  # VTK
  #  -DUSE_VTK:BOOL=ON
  #  -DVTK_DIR:PATH=${VTK_DIR}
  INSTALL_COMMAND ""
)
