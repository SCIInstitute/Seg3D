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

IF(NOT EXISTS ${ALL_LIBS_DEST_DIR})
  ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ALL_LIBS_DEST_DIR}
  )
  FILE(GLOB_RECURSE LIB_FILES RELATIVE "${SUPERBUILD_LIBS_SOURCE_DIR}"
      "${SUPERBUILD_LIBS_SOURCE_DIR}/*.lib")

  FOREACH(CURRENT_LIB_FILE ${LIB_FILES})

    SET(LIB_SOURCE_FILE ${SUPERBUILD_LIBS_SOURCE_DIR}/${CURRENT_LIB_FILE})

    ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LIB_SOURCE_FILE} ${ALL_LIBS_DEST_DIR}/
    )

  ENDFOREACH(CURRENT_LIB_FILE)

ENDIF()

###########################################
# Make Seg3D and External .lib file
###########################################

get_filename_component(_vs_bin_path "${CMAKE_LINKER}" DIRECTORY)
MESSAGE("vs bin path: " ${_vs_bin_path} )
STRING(REGEX REPLACE "/" "\\\\" _vs_bin_path ${_vs_bin_path})

SET(LIB_COMMAND " lib.exe ")
STRING(REGEX REPLACE "/" "\\\\" ALL_LIBS_DEST_DIR_CMD ${ALL_LIBS_DEST_DIR})
SET(ARGUMENTS "/OUT:composite.lib ${ALL_LIBS_DEST_DIR_CMD}\\*.lib" )
MESSAGE("Command line: " ${LIB_COMMAND} ${ARGUMENTS})

SET(VSINSTALL_PATH "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\")
SET(ENVIRONMENT "VCINSTALL = ${VSINSTALL_PATH};$ENV{VSINSTALL}")
MESSAGE("Environment: ${ENVIRONMENT}")

ADD_CUSTOM_COMMAND(TARGET Seg3D_lib
COMMAND ${CMAKE_COMMAND} -E env ${ENVIRONMENT} chdir ${_vs_bin_path} ${LIB_COMMAND} ${ARGUMENTS}
)
