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
# Copy External and Seg3D library files to Lib directory
#########################################################

SET(ALL_LIBS_DEST_DIR ${CMAKE_BINARY_DIR}/Main/Seg3DLib/Lib/All_Libs)
SET(LIB_DIR_REMOVED OFF)
MESSAGE("Lib dir state: " ${LIB_DIR_REMOVED})

IF(EXISTS ${ALL_LIBS_DEST_DIR})
  ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${ALL_LIBS_DEST_DIR}
  )
  SET(LIB_DIR_REMOVED ON)
  MESSAGE("Lib dir state: " ${LIB_DIR_REMOVED})
ELSE()
  SET(LIB_DIR_REMOVED ON)
  MESSAGE("Lib dir state: " ${LIB_DIR_REMOVED})
ENDIF()

IF(${LIB_DIR_REMOVED})
 ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
 COMMAND ${CMAKE_COMMAND} -E make_directory ${ALL_LIBS_DEST_DIR}
 )

FILE(GLOB_RECURSE LIB_FILES RELATIVE "${SUPERBUILD_LIBS_SOURCE_DIR}"
    "${SUPERBUILD_LIBS_SOURCE_DIR}/*.lib")

FOREACH(CURRENT_LIB_FILE ${LIB_FILES})

SET(LIB_SOURCE_FILE ${SUPERBUILD_LIBS_SOURCE_DIR}/${CURRENT_LIB_FILE})

STRING(FIND ${LIB_SOURCE_FILE} "Seg3D_lib" CONTAINS_SEG3D_LIB)

IF(${CONTAINS_SEG3D_LIB} EQUAL "-1")
  ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${LIB_SOURCE_FILE} ${ALL_LIBS_DEST_DIR}/
  )
ENDIF()

ENDFOREACH(CURRENT_LIB_FILE)

SET(ZLIB_FILE ${SUPERBUILD_LIBS_SOURCE_DIR}/Externals/Build/Zlib_external/Release/z.lib)

ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB_FILE} ${ALL_LIBS_DEST_DIR}/
)

ENDIF()

###########################################
# Make Seg3D and External .lib file
###########################################

#get_filename_component(_vs_bin_path "${CMAKE_LINKER}" DIRECTORY)
#STRING(REGEX REPLACE "/" "\\\\" _vs_bin_path ${_vs_bin_path})

#SET(LIB_TOOL_PATH ${_vs_bin_path}\\lib.exe)

#SET(BATCH_FILE_PATH ${_vs_bin_path}\\..\\..\\..\\..\\..\\..\\..\\Common7\\Tools\\VsDevCmd.bat)

#STRING(REGEX REPLACE "/" "\\\\" ALL_LIBS_DEST_DIR_CMD ${ALL_LIBS_DEST_DIR})

#SET(DEPENDENCIES_DEST_DIR ${ALL_LIBS_DEST_DIR_CMD}\\..\\Release)

#SET(ARGUMENTS " /OUT:${DEPENDENCIES_DEST_DIR}\\Seg_lib_dependencies.lib ${ALL_LIBS_DEST_DIR_CMD}\\*.lib" )

#MESSAGE("Command line: " \"${BATCH_FILE_PATH}\" " & " \"${LIB_TOOL_PATH}\" ${ARGUMENTS})

#ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
#POST_BUILD
#COMMAND \"${BATCH_FILE_PATH}\" " & " \"${LIB_TOOL_PATH}\" ${ARGUMENTS}
#)

#"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat" & "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\bin\Hostx86\x64\lib.exe"
#/OUT:C:\Users\Ally\Documents\composite.lib C:\Users\Ally\Desktop\Seg3DLibDependencies2017\*.lib
