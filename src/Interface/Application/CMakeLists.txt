
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

# Split into multiple projects to improve parallelization of compilation in Visual Studio.
# Visual Studio doesn't parallelize compilation within projects, but it does parallelize
# across projects.
  
SET(NUM_PROJECTS 8)

# PROJECT 1
SET(INTERFACE_APPLICATION_SRCS_1
  ApplicationInterface.cc
  ColorPickerWidget.cc
  ControllerInterface.cc
  ControllerContext.cc
  ControllerActionHistory.cc
  ToolWidget.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_1
  ApplicationInterface.h
  ColorPickerWidget.h
  ControllerInterface.h
  ControllerActionHistory.h
  ToolWidget.h
) 
SET(INTERFACE_APPLICATION_NO_MOC_SRCS_1
  ControllerContext.h
  LayerIOFunctions.h
  LayerIOFunctions.cc
  StyleSheet.h
  StyleSheet.cc
)
SET(INTERFACE_APPLICATION_UI_SRCS_1
  ColorPickerWidget.ui
  ControllerInterface.ui
)

# PROJECT 2
SET(INTERFACE_APPLICATION_SRCS_2
  ControllerStateEngine.cc
  ControllerLogHistory.cc
  ControllerUndoBuffer.cc
  ControllerRedoBuffer.cc
  DropSpaceWidget.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_2
  ControllerStateEngine.h
  ControllerLogHistory.h
  ControllerUndoBuffer.h
  ControllerRedoBuffer.h
  DropSpaceWidget.h
)  

# PROJECT 3
SET(INTERFACE_APPLICATION_SRCS_3
  GroupButtonMenu.cc
  HistoryDockWidget.cc
  LayerImporterWidget.cc
  LayerGroupWidget.cc
  LayerManagerDockWidget.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_3
  GroupButtonMenu.h
  HistoryDockWidget.h
  LayerImporterWidget.h
  LayerGroupWidget.h
  LayerManagerDockWidget.h
)
SET(INTERFACE_APPLICATION_UI_SRCS_3
  GroupButtonMenu.ui
  HistoryDockWidget.ui
  LayerImporterWidget.ui
  LayerGroupWidget.ui
  LayerManagerDockWidget.ui
)

# PROJECT 4
SET(INTERFACE_APPLICATION_SRCS_4
  LayerManagerWidget.cc
  LayerResamplerDialog.cc
  LayerWidget.cc
  Menu.cc
  MessageWindow.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_4
  LayerManagerWidget.h
  LayerResamplerDialog.h
  LayerWidget.h
  Menu.h
  MessageWindow.h
)
SET(INTERFACE_APPLICATION_UI_SRCS_4
  LayerResamplerDialog.ui
  LayerWidget.ui
  MessageWindow.ui
)

# PROJECT 5
SET(INTERFACE_APPLICATION_SRCS_5
  OverlayWidget.cc
  ProgressWidget.cc
  PreferencesInterface.cc
  ProjectDockWidget.cc
  ProjectExportWizard.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_5
  OverlayWidget.h
  PreferencesInterface.h
  ProgressWidget.h
  ProjectDockWidget.h
  ProjectExportWizard.h
)
SET(INTERFACE_APPLICATION_UI_SRCS_5
  PreferencesInterface.ui
  ProjectDockWidget.ui
  ProgressWidget.ui
)

# PROJECT 6
SET(INTERFACE_APPLICATION_SRCS_6
  ProjectWizard.cc
  ProvenanceDockWidget.cc
  PushDragButton.cc
  QtLayerListWidget.cc
  SaveProjectAsWizard.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_6
  ProjectWizard.h
  ProvenanceDockWidget.h
  PushDragButton.h
  QtLayerListWidget.h
  SaveProjectAsWizard.h
)
SET(INTERFACE_APPLICATION_UI_SRCS_6
  ProvenanceDockWidget.ui
  QtLayerListWidget.ui
)

# PROJECT 7
SET(INTERFACE_APPLICATION_SRCS_7
  SegmentationExportWizard.cc
  ShortcutsInterface.cc
  SplashScreen.cc
  StatusBarWidget.cc
  ToolBoxWidget.cc
  ToolsDockWidget.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_7
  SegmentationExportWizard.h 
  ShortcutsInterface.h
  SplashScreen.h
  StatusBarWidget.h
  ToolBoxWidget.h
  ToolsDockWidget.h
)
SET(INTERFACE_APPLICATION_UI_SRCS_7
  ShortcutsInterface.ui
  SplashScreen.ui
  StatusBarWidget.ui
  ToolBoxPageWidget.ui
)
  
# PROJECT 8
SET(INTERFACE_APPLICATION_SRCS_8
  ViewAction.cc
  ViewerInterface.cc
  ViewerWidget.cc
  RenderingDockWidget.cc
  TransferFunctionFeatureWidget.cc
)
SET(INTERFACE_APPLICATION_MOC_SRCS_8
  ViewAction.h
  ViewerInterface.h
  ViewerWidget.h
  RenderingDockWidget.h
  TransferFunctionFeatureWidget.h
)
SET(INTERFACE_APPLICATION_UI_SRCS_8
  ViewerInterface.ui
  ViewerWidget.ui
  RenderingDockWidget.ui
  TransferFunctionFeatureWidget.ui
)
IF(BUILD_WITH_PYTHON)
  SET(INTERFACE_APPLICATION_SRCS_8 ${INTERFACE_APPLICATION_SRCS_8} PythonConsoleWidget.cc)
  SET(INTERFACE_APPLICATION_MOC_SRCS_8 ${INTERFACE_APPLICATION_MOC_SRCS_8} PythonConsoleWidget.h)
ENDIF(BUILD_WITH_PYTHON)

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
  
##################################################
# Set link libraries
##################################################

IF(APPLE)
SET(INTERFACE_APPLICATION_LINK_LIBS 
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
  ${Application_Tools}
  Application_LayerIO
  QtUtils_Utils
  QtUtils_Widgets
  QtUtils_Bridge
  ${QT_LIBRARIES} 
  ${SCI_BOOST_LIBRARY}
  /System/Library/Frameworks/CoreServices.framework
)
ELSE(APPLE)
SET(INTERFACE_APPLICATION_LINK_LIBS 
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
  ${Application_Tools}
  Application_LayerIO
  QtUtils_Utils
  QtUtils_Widgets
  QtUtils_Bridge
  ${QT_LIBRARIES} 
  ${SCI_BOOST_LIBRARY}
)
ENDIF(APPLE)

IF(BUILD_WITH_PYTHON)
  SET(INTERFACE_APPLICATION_LINK_LIBS $INTERFACE_APPLICATION_LINK_LIBS Core_Python)
ENDIF(BUILD_WITH_PYTHON)

##################################################
# Create project(s)
##################################################
            
IF (SEG3D_SPLIT_PROJECTS)

  # Split large projects into smaller projects to speed up compilation with Visual Studio.
  # Visual Studio doesn't parallelize compilation within projects, but it does parallelize
  # across projects.  
  
  # Clear cached value
  SET(Interface_Application "" CACHE INTERNAL "Interface_Application libraries")
    
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    ##################################################
    # Generate header out of UI code
    ##################################################
  
    SET(INTERFACE_APPLICATION_UI_SRCS "INTERFACE_APPLICATION_UI_SRCS_${PROJ_IDX}")
    # Clear variable of values from last iteration
    SET(INTERFACE_APPLICATION_QT_UI_SRCS "")
    QT4_WRAP_UI(INTERFACE_APPLICATION_QT_UI_SRCS ${${INTERFACE_APPLICATION_UI_SRCS}})
    
    ##################################################
    # Wrap QT code to expand all the moc code
    ##################################################

    SET(INTERFACE_APPLICATION_MOC_SRCS "INTERFACE_APPLICATION_MOC_SRCS_${PROJ_IDX}")
    # Clear variable of values from last iteration
    SET(INTERFACE_APPLICATION_QT_MOC_SRCS "")
    QT4_WRAP_CPP(INTERFACE_APPLICATION_QT_MOC_SRCS ${${INTERFACE_APPLICATION_MOC_SRCS}})
    
    ##################################################
    # Build the Components library
    ##################################################
    
    SET(INTERFACE_APPLICATION_SRCS "INTERFACE_APPLICATION_SRCS_${PROJ_IDX}")
    SET(INTERFACE_APPLICATION_NO_MOC_SRCS "INTERFACE_APPLICATION_NO_MOC_SRCS_${PROJ_IDX}") 
    SET(INTERFACE_APPLICATION_MOC_SRCS "INTERFACE_APPLICATION_MOC_SRCS_${PROJ_IDX}")                
    SET(INTERFACE_APPLICATION_LIB "Interface_Application_${PROJ_IDX}")  
    CORE_ADD_LIBRARY(${INTERFACE_APPLICATION_LIB} 
      ${${INTERFACE_APPLICATION_SRCS}} 
      ${INTERFACE_APPLICATION_QT_UI_SRCS}
      ${INTERFACE_APPLICATION_QT_MOC_SRCS}
      ${${INTERFACE_APPLICATION_NO_MOC_SRCS}} 
      ${${INTERFACE_APPLICATION_MOC_SRCS}})     
                        
    TARGET_LINK_LIBRARIES(${INTERFACE_APPLICATION_LIB}
      ${INTERFACE_APPLICATION_LINK_LIBS})
      
    # Create single CMake variable that other libraries can depend on
    # Tack library onto end of list 
    # Note: Use INTERNAL so that the variable is put in the cache (global scope), but not
    # exposed to the user in the CMake interface.   
    SET(Interface_Application ${Interface_Application} ${INTERFACE_APPLICATION_LIB} CACHE INTERNAL "Interface_Application libraries")
    
  ENDFOREACH (PROJ_IDX)
  
ELSE (SEG3D_SPLIT_PROJECTS)

  # Consolidate sources back into single project
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    SET(INTERFACE_APPLICATION_PROJ_SRCS "INTERFACE_APPLICATION_SRCS_${PROJ_IDX}") 
    SET(INTERFACE_APPLICATION_SRCS "${INTERFACE_APPLICATION_SRCS};${${INTERFACE_APPLICATION_PROJ_SRCS}}")
      
    SET(INTERFACE_APPLICATION_MOC_PROJ_SRCS "INTERFACE_APPLICATION_MOC_SRCS_${PROJ_IDX}")   
    SET(INTERFACE_APPLICATION_MOC_SRCS "${INTERFACE_APPLICATION_MOC_SRCS};${${INTERFACE_APPLICATION_MOC_PROJ_SRCS}}")   
    
    SET(INTERFACE_APPLICATION_NO_MOC_PROJ_SRCS "INTERFACE_APPLICATION_NO_MOC_SRCS_${PROJ_IDX}")
    SET(INTERFACE_APPLICATION_NO_MOC_SRCS "${INTERFACE_APPLICATION_NO_MOC_SRCS};${${INTERFACE_APPLICATION_NO_MOC_PROJ_SRCS}}")  
    
    SET(INTERFACE_APPLICATION_UI_PROJ_SRCS "INTERFACE_APPLICATION_UI_SRCS_${PROJ_IDX}") 
    SET(INTERFACE_APPLICATION_UI_SRCS "${INTERFACE_APPLICATION_UI_SRCS};${${INTERFACE_APPLICATION_UI_PROJ_SRCS}}") 
    
  ENDFOREACH (PROJ_IDX)
  
  ##################################################
  # Generate header out of UI code
  ##################################################

  QT4_WRAP_UI(INTERFACE_APPLICATION_QT_UI_SRCS ${INTERFACE_APPLICATION_UI_SRCS})

  ##################################################
  # Wrap QT code to expand all the moc code
  ##################################################

  QT4_WRAP_CPP(INTERFACE_APPLICATION_QT_MOC_SRCS ${INTERFACE_APPLICATION_MOC_SRCS})

  ##################################################
  # Build the Components library
  ##################################################

  # Note: Use INTERNAL so that the variable is put in the cache (global scope), but not
  # exposed to the user in the CMake interface. 
  SET(Interface_Application "Interface_Application" CACHE INTERNAL "Interface_Application library")
  CORE_ADD_LIBRARY(${Interface_Application} 
        ${INTERFACE_APPLICATION_SRCS} 
        ${INTERFACE_APPLICATION_QT_UI_SRCS}
        ${INTERFACE_APPLICATION_QT_MOC_SRCS}
        ${INTERFACE_APPLICATION_NO_MOC_SRCS} 
        ${INTERFACE_APPLICATION_MOC_SRCS})

  TARGET_LINK_LIBRARIES(${Interface_Application}
    ${INTERFACE_APPLICATION_LINK_LIBS})

ENDIF (SEG3D_SPLIT_PROJECTS)  

