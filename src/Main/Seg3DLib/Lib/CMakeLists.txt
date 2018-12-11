#
#  For more information, please see: http://software.sci.utah.edu
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


############################################
# Set Seg3D main sources
###########################################

SET(SEG3DLIB_MAIN_SRCS
  Seg3D_lib.cc
  ../../Seg3DGui.cc
  ../../Seg3DBase.cc
)

###########################################
# Build the Seg3D executable
###########################################

ADD_LIBRARY(Seg3D_lib ${SEG3DLIB_MAIN_SRCS})

TARGET_LINK_LIBRARIES(Seg3D_lib
  ${SCI_BOOST_LIBRARY}
  ${SCI_GLEW_LIBRARY}
  Core_Utils
  Core_DataBlock
  Core_EventHandler
  Core_Graphics
  Core_Volume
  Core_Application
  Core_Interface
  Core_Action
  Core_Log
  Application_Tools
  Application_Filters
  QtUtils_Utils
  QtUtils_Widgets
  QtUtils_Bridge
  Interface_Application
  Interface_ToolInterface
  Core_Utils
  Core_EventHandler
  Core_Action
  Core_Interface
  Core_State
  Application_Project
  Application_ProjectManager
  Application_Renderer
  Application_Tool
  Application_ToolManager
  Application_Tools
  Application_LayerIO
  ${SCI_ZLIB_LIBRARY}
  ${SCI_PNG_LIBRARY}
  ${SCI_TEEM_LIBRARY}
  ${ImplicitFunction_LIBRARY}
  ${RESOURCES_LIBRARY}
  ${Qt5Core_QTMAIN_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
)

REGISTERED_TARGET_LINK_LIBRARIES(Seg3D_lib)

#########################################################
# Make Seg3D static lib file
#########################################################

SET(ALL_LIBS_DEST_DIR ${CMAKE_BINARY_DIR}/Main/Seg3DLib/Lib/All_Libs)
SET(DEPENDENCIES_DEST_DIR ${ALL_LIBS_DEST_DIR}/../Release)
STRING(REGEX REPLACE "/" "\\\\" ALL_LIBS_DEST_DIR ${ALL_LIBS_DEST_DIR})
STRING(REGEX REPLACE "/" "\\\\" DEPENDENCIES_DEST_DIR ${DEPENDENCIES_DEST_DIR})

STRING(REGEX REPLACE "/" "\\\\" SUPERBUILD_LIBS_SOURCE_DIR_CMD ${SUPERBUILD_LIBS_SOURCE_DIR})

SET(ZLIB_FILE ${SUPERBUILD_LIBS_SOURCE_DIR}/Externals/Build/Zlib_external/Release/z.lib)
STRING(REGEX REPLACE "/" "\\\\" ZLIB_FILE ${ZLIB_FILE})

get_filename_component(_vs_bin_path "${CMAKE_LINKER}" DIRECTORY)
SET(BATCH_FILE_PATH \"${_vs_bin_path}/../../../../../../../Common7/Tools/VsDevCmd.bat\")
SET(LIB_TOOL_PATH \"${_vs_bin_path}/lib.exe\")
STRING(REGEX REPLACE "/" "\\\\" _vs_bin_path ${_vs_bin_path})
STRING(REGEX REPLACE "/" "\\\\" LIB_TOOL_PATH ${LIB_TOOL_PATH})
STRING(REGEX REPLACE "/" "\\\\" BATCH_FILE_PATH ${BATCH_FILE_PATH})

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/Scripts/build_full_static_lib.bat.in
    ${CMAKE_BINARY_DIR}/build_full_static_lib.bat)

SET(BATCH_COMMAND ${CMAKE_BINARY_DIR}/build_full_static_lib.bat)
STRING(REGEX REPLACE "/" "\\\\" BATCH_COMMAND ${BATCH_COMMAND})

ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
POST_BUILD
COMMAND cmd /c ${BATCH_COMMAND}
)
