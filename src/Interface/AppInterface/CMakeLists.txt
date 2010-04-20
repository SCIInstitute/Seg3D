
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

##################################################
# Set sources
##################################################

SET(INTERFACE_APPINTERFACE_SRCS
  AppInterface.cc
  AppMenu.cc
  AppLayerIO.cc
  AppStatusBar.cc
  HistoryDockWidget.cc
  MessageHistoryWidget.cc
  LayerManagerWidget.cc
  LayerImporterWidget.cc
  LayerWidget.cc
  LayerGroupWidget.cc
  LayerManagerDockWidget.cc
  MeasurementDockWidget.cc
  ProgressWidget.cc
  ProjectDockWidget.cc
  PushDragButton.cc
  SplashStart.cc
  ToolBoxWidget.cc
  ToolsDockWidget.cc
  ToolWidget.cc
  ViewAction.cc
  ViewerInterface.cc
  ViewerWidget.cc
  DropSpaceWidget.cc
  OverlayWidget.cc
  SingleShotToolButton.cc
)

SET(INTERFACE_APPINTERFACE_MOC_SRCS
  AppInterface.h
  AppMenu.h
  AppStatusBar.h
  HistoryDockWidget.h
  LayerWidget.h
  LayerImporterWidget.h
  LayerGroupWidget.h
  LayerManagerWidget.h
  LayerManagerDockWidget.h
  MessageHistoryWidget.h
  MeasurementDockWidget.h
  ProgressWidget.h
  ProjectDockWidget.h
  PushDragButton.h
  ToolBoxWidget.h
  ToolsDockWidget.h
  ToolWidget.h
  SplashStart.h
  ViewAction.h
  ViewerInterface.h
  ViewerWidget.h
  DropSpaceWidget.h
  OverlayWidget.h
  SingleShotToolButton.h
)


SET(INTERFACE_APPINTERFACE_NO_MOC_SRCS
  StyleSheet.h
  StyleSheet.cc
)

SET(INTERFACE_APPINTERFACE_UI_SRCS
  HistoryDockWidget.ui
  LayerGroupWidget.ui
  LayerImporterWidget.ui
  LayerManagerDockWidget.ui
  LayerWidget.ui
  MeasurementDockWidget.ui
  MessageHistoryWidget.ui
  ProjectDockWidget.ui
  ProgressWidget.ui
  Splash.ui
  ToolBoxPageWidget.ui
  ViewerInterface.ui
  StatusBar.ui
)

SET(INTERFACE_APPINTERFACE_RCC_SRCS
  Resources/AppInterface.qrc
)


##################################################
# Get Icon files
##################################################

QT4_ADD_RESOURCES(INTERFACE_APPINTERFACE_QT_RCC_SRCS ${INTERFACE_APPINTERFACE_RCC_SRCS})

##################################################
# Generate header out of UI code
##################################################

QT4_WRAP_UI(INTERFACE_APPINTERFACE_QT_UI_SRCS ${INTERFACE_APPINTERFACE_UI_SRCS})


##################################################
# Wrap QT code to expand all the moc code
##################################################

QT4_WRAP_CPP(INTERFACE_APPINTERFACE_QT_MOC_SRCS ${INTERFACE_APPINTERFACE_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

ADD_LIBRARY(Interface_AppInterface STATIC 
            ${INTERFACE_APPINTERFACE_SRCS} 
            ${INTERFACE_APPINTERFACE_QT_UI_SRCS}
            ${INTERFACE_APPINTERFACE_QT_MOC_SRCS}
            ${INTERFACE_APPINTERFACE_NO_MOC_SRCS} 
            ${INTERFACE_APPINTERFACE_MOC_SRCS}
            ${INTERFACE_APPINTERFACE_QT_RCC_SRCS})
                     
            
TARGET_LINK_LIBRARIES(Interface_AppInterface
                      Utils_Core
                      Utils_EventHandler
                      Application_Action
                      Application_Interface
                      Application_ActionManager
                      Application_State
                      Application_StateManager
                      Application_Tool
                      Application_ToolManager
                      Application_Tools
                      Interface_QtInterface
                      Interface_AppController
                      ${QT_LIBRARIES} 
                      ${SCI_BOOST_LIBRARY})

ADD_LIBRARY_QTRESOURCES(AppInterface)


FILE(GLOB RESOURCE_IMAGE_FILES "${CMAKE_CURRENT_SOURCE_DIR}Resources/Images/*.png")

IF(NOT WIN32)
  ADD_DEPENDENCIES(Interface_AppInterface
    DEPENDS ${RESOURCE_IMAGE_FILES}
  )
ENDIF(NOT WIN32)
