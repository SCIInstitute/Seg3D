
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

SET(APPLICATION_LAYER_SRCS
  LayerFWD.h
  LayerGroup.h
  LayerGroup.cc
  Layer.h
  Layer.cc
  LayerAbstractFilter.h
  LayerAbstractFilter.cc
  DataLayer.h
  DataLayer.cc
  MaskLayer.h
  MaskLayer.cc
  LayerAvailabilityNotifier.h
  LayerAvailabilityNotifier.cc
  LayerManager.h
  LayerManager.cc
  LayerScene.h
  LayerCheckPoint.h
  LayerCheckPoint.cc
  LayerUndoBufferItem.h
  LayerUndoBufferItem.cc
  LayerActionParameter.h
  LayerActionParameter.cc
  LayerAction.h
  LayerAction.cc
  LayerRecreationUndoBufferItem.h
  LayerRecreationUndoBufferItem.cc
  ProvenanceScript.h
  ProvenanceScript.cc
  LargeVolumeLayer.h
  LargeVolumeLayer.cc
)

SET(APPLICATION_LAYER_ACTIONS_SRCS
  Actions/ActionActivateLayer.h
  Actions/ActionActivateLayer.cc
  Actions/ActionShiftActiveLayer.h
  Actions/ActionShiftActiveLayer.cc
  Actions/ActionCalculateMaskVolume.h
  Actions/ActionCalculateMaskVolume.cc
  Actions/ActionComputeIsosurface.h
  Actions/ActionComputeIsosurface.cc
  Actions/ActionDeleteIsosurface.h
  Actions/ActionDeleteIsosurface.cc
  Actions/ActionDeleteLayers.h
  Actions/ActionDeleteLayers.cc
  Actions/ActionMoveGroup.h
  Actions/ActionMoveGroup.cc
  Actions/ActionMoveLayer.h
  Actions/ActionMoveLayer.cc
  Actions/ActionNewMaskLayer.h
  Actions/ActionNewMaskLayer.cc
  Actions/ActionDuplicateLayer.h
  Actions/ActionDuplicateLayer.cc
  Actions/ActionMigrateSandboxLayer.h
  Actions/ActionMigrateSandboxLayer.cc
  Actions/ActionCreateSandbox.h
  Actions/ActionCreateSandbox.cc
  Actions/ActionDeleteSandbox.h
  Actions/ActionDeleteSandbox.cc
  Actions/ActionRecreateLayer.h
  Actions/ActionRecreateLayer.cc
  Actions/ActionBeginScriptStatusReport.h
  Actions/ActionBeginScriptStatusReport.cc
  Actions/ActionEndScriptStatusReport.h
  Actions/ActionEndScriptStatusReport.cc
  Actions/ActionReportScriptStatus.h
  Actions/ActionReportScriptStatus.cc
)

IF(BUILD_WITH_PYTHON)
  GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER Application_Layer ${APPLICATION_LAYER_ACTIONS_SRCS})
  SET(APPLICATION_LAYER_SRCS ${APPLICATION_LAYER_SRCS} ${PYTHON_WRAPPER})
ENDIF(BUILD_WITH_PYTHON)

CORE_ADD_LIBRARY(Application_Layer ${APPLICATION_LAYER_SRCS}
  ${APPLICATION_LAYER_ACTIONS_SRCS}
)
            
TARGET_LINK_LIBRARIES(Application_Layer
  Core_Action
  Core_Application
  Core_Interface
  Core_Isosurface
  Core_State
  Core_Utils
  Application_InterfaceManager
  Application_Provenance
  ${SCI_BOOST_LIBRARY}
)

# register actions            
REGISTER_LIBRARY_AND_CLASSES(Application_Layer
  ${APPLICATION_LAYER_ACTIONS_SRCS}
)
