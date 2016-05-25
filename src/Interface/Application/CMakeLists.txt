
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

##################################################
# Set sources
##################################################

SET(INTERFACE_APPLICATION_SRCS
  ApplicationInterface.cc
  ColorPickerWidget.cc
  ControllerInterface.cc
  ControllerContext.cc
  ControllerActionHistory.cc
  ControllerStateEngine.cc
  ControllerLogHistory.cc
  ControllerUndoBuffer.cc
  ControllerRedoBuffer.cc
  DropSpaceWidget.cc
  GroupButtonMenu.cc
  HistoryDockWidget.cc
  LayerImporterWidget.cc
  LayerGroupWidget.cc
  LayerManagerDockWidget.cc
  LayerManagerWidget.cc
  LayerResamplerDialog.cc
  LayerWidget.cc
  Menu.cc
  MessageWindow.cc
  OverlayWidget.cc
  ProgressWidget.cc
  PreferencesInterface.cc
  ProjectDockWidget.cc
  ProjectExportWizard.cc
  ProjectWizard.cc
  ProvenanceDockWidget.cc
  PushDragButton.cc
  QtLayerListWidget.cc
  SaveProjectAsWizard.cc
  SegmentationExportWizard.cc
  ShortcutsInterface.cc
  SplashScreen.cc
  StatusBarWidget.cc
  ToolBoxWidget.cc
  ToolsDockWidget.cc
  ToolWidget.cc
  ViewAction.cc
  ViewerInterface.cc
  ViewerWidget.cc
  RenderingDockWidget.cc
  TransferFunctionFeatureWidget.cc
  ProvenanceTreeModel.cc
  ProvenanceTreeView.cc
  SliceRangeDialog.cc
)

SET(INTERFACE_APPLICATION_MOC_SRCS
  ApplicationInterface.h
  ColorPickerWidget.h
  ControllerInterface.h
  ControllerActionHistory.h
  ControllerStateEngine.h
  ControllerLogHistory.h
  ControllerUndoBuffer.h
  ControllerRedoBuffer.h
  DropSpaceWidget.h
  GroupButtonMenu.h
  HistoryDockWidget.h
  LayerImporterWidget.h
  LayerGroupWidget.h
  LayerManagerDockWidget.h
  LayerManagerWidget.h
  LayerResamplerDialog.h
  LayerWidget.h
  Menu.h
  MessageWindow.h
  OverlayWidget.h
  PreferencesInterface.h
  ProgressWidget.h
  ProjectDockWidget.h
  ProjectExportWizard.h
  ProjectWizard.h
  ProvenanceDockWidget.h
  PushDragButton.h
  QtLayerListWidget.h
  SaveProjectAsWizard.h
  SegmentationExportWizard.h
  ShortcutsInterface.h
  SplashScreen.h
  StatusBarWidget.h
  ToolBoxWidget.h
  ToolsDockWidget.h
  ToolWidget.h
  ViewAction.h
  ViewerInterface.h
  ViewerWidget.h
  RenderingDockWidget.h
  TransferFunctionFeatureWidget.h
  ProvenanceTreeModel.h
  ProvenanceTreeView.h
  SliceRangeDialog.h
)

IF(BUILD_WITH_PYTHON)
  SET(INTERFACE_APPLICATION_SRCS ${INTERFACE_APPLICATION_SRCS} PythonConsoleWidget.cc)
  SET(INTERFACE_APPLICATION_MOC_SRCS ${INTERFACE_APPLICATION_MOC_SRCS} PythonConsoleWidget.h)
ENDIF()

SET(INTERFACE_APPLICATION_NO_MOC_SRCS
  ControllerContext.h
  LayerIOFunctions.h
  LayerIOFunctions.cc
  StyleSheet.h
  StyleSheet.cc
)

SET(INTERFACE_APPLICATION_UI_SRCS
  ColorPickerWidget.ui
  ControllerInterface.ui
  GroupButtonMenu.ui
  HistoryDockWidget.ui
  LayerGroupWidget.ui
  LayerImporterWidget.ui
  LayerManagerWidget.ui
  LayerResamplerDialog.ui
  LayerWidget.ui
  MessageWindow.ui
  PreferencesInterface.ui
  ProjectDockWidget.ui
  ProgressWidget.ui
  ProvenanceDockWidget.ui
  QtLayerListWidget.ui
  StatusBarWidget.ui
  ShortcutsInterface.ui
  SplashScreen.ui
  ToolBoxPageWidget.ui
  ViewerInterface.ui
  ViewerWidget.ui
  RenderingDockWidget.ui
  TransferFunctionFeatureWidget.ui
  SliceRangeDialog.ui
)

SET(INTERFACE_APPLICATION_RCC_SRCS
)


##################################################
# Resource files
##################################################

QT5_ADD_RESOURCES(INTERFACE_APPLICATION_QT_RCC_SRCS ${INTERFACE_APPLICATION_RCC_SRCS})

##################################################
# Generate header out of UI code
##################################################

QT5_WRAP_UI(INTERFACE_APPLICATION_QT_UI_SRCS ${INTERFACE_APPLICATION_UI_SRCS})


##################################################
# Wrap QT code to expand all the moc code
##################################################

QT5_WRAP_CPP(INTERFACE_APPLICATION_QT_MOC_SRCS ${INTERFACE_APPLICATION_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY(Interface_Application
  ${INTERFACE_APPLICATION_SRCS}
  ${INTERFACE_APPLICATION_QT_UI_SRCS}
  ${INTERFACE_APPLICATION_QT_MOC_SRCS}
  ${INTERFACE_APPLICATION_NO_MOC_SRCS}
  ${INTERFACE_APPLICATION_MOC_SRCS}
  ${INTERFACE_APPLICATION_QT_RCC_SRCS})

IF(APPLE)
  TARGET_LINK_LIBRARIES(Interface_Application
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
    QtUtils_Utils
    QtUtils_Widgets
    QtUtils_Bridge
#    ${QT_LIBRARIES}
    ${SCI_BOOST_LIBRARY}
    /System/Library/Frameworks/CoreServices.framework)
ELSE()
  TARGET_LINK_LIBRARIES(Interface_Application
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
    QtUtils_Utils
    QtUtils_Widgets
    QtUtils_Bridge
#    ${QT_LIBRARIES}
    ${SCI_BOOST_LIBRARY})
ENDIF()

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(Interface_Application Core_Python)
ENDIF()
