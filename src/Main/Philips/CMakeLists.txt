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

SET(PHILIPS_MAIN_SRCS
  philips_main.cc
  ../Seg3DGui.cc
)

###########################################
# Build the Seg3D executable
###########################################

#SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEG3D_BINARY_DIR})
#ADD_DEFINITIONS(-DAPPLICATION_NAME=${APPLICATION_NAME})

ADD_EXECUTABLE(philips ${PHILIPS_MAIN_SRCS})

TARGET_LINK_LIBRARIES(philips
  philips_lib
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
  Application_Socket
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

IF(WIN32)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} ${Qt5Core_QTMAIN_LIBRARIES} )
  SET_TARGET_PROPERTIES(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "/INCREMENTAL:NO")
ENDIF()

IF(APPLE)
  SET_TARGET_PROPERTIES(philips
    PROPERTIES
    LINK_FLAGS "-framework CoreGraphics"
  )
ENDIF()

REGISTERED_TARGET_LINK_LIBRARIES(philips)

ADD_SUBDIRECTORY(Lib)
