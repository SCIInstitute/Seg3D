
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

SET(APPLICATION_LAYERMANAGER_SRCS
    LayerManager.h
    LayerManager.cc
  LayerScene.h
  LayerScene.cc
  )

SET(APPLICATION_LAYERMANAGER_ACTIONS_SRCS
  Actions/ActionInsertLayer.h
  Actions/ActionInsertLayer.cc
  Actions/ActionInsertLayerAbove.h
  Actions/ActionInsertLayerAbove.cc
  Actions/ActionCloneLayer.h
  Actions/ActionCloneLayer.cc
  Actions/ActionCrop.h
  Actions/ActionCrop.cc
  Actions/ActionFlipLayer.h
  Actions/ActionFlipLayer.cc
  Actions/ActionImportLayer.h
  Actions/ActionImportLayer.cc
  Actions/ActionLayer.h
  Actions/ActionLayer.cc
  Actions/ActionLayerFromFile.h
  Actions/ActionLayerFromFile.cc
  Actions/ActionNewMaskLayer.h
  Actions/ActionNewMaskLayer.cc
  Actions/ActionRemoveLayer.h
  Actions/ActionRemoveLayer.cc
  Actions/ActionResampleLayer.h
  Actions/ActionResampleLayer.cc
  Actions/ActionTransformLayer.h
  Actions/ActionTransformLayer.cc
  Actions/ActionActivateLayer.h
  Actions/ActionActivateLayer.cc
  Actions/ActionDeleteLayers.h
  Actions/ActionDeleteLayers.cc
  )


ADD_LIBRARY(Application_LayerManager STATIC 
            ${APPLICATION_LAYERMANAGER_SRCS}
            ${APPLICATION_LAYERMANAGER_ACTIONS_SRCS} )
            
TARGET_LINK_LIBRARIES(Application_LayerManager
                      Utils_Converter
                      Utils_Core
                      Utils_EventHandler
                      Application_Application
                      Application_Interface
                      Application_Action
                      Application_State
                      Application_StateManager
                      Application_Layer
                      ${SCI_BOOST_LIBRARY})

# Register actions
ADD_LIBRARY_ACTIONS(Application_LayerManager
                    ${APPLICATION_LAYERMANAGER_ACTIONS_SRCS})

