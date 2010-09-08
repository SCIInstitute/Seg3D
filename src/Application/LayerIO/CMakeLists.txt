
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

SET(APPLICATION_LAYERIO_SRCS
  LayerImporter.h
  LayerImporter.cc
  LayerImporterInfo.h
  LayerImporterInfo.cc
  LayerIO.h
  LayerIO.cc
)

SET(APPLICATION_LAYERIO_ACTIONS_SRCS
)

SET(APPLICATION_LAYERIO_IMPORTERS_SRCS
  ITKLayerImporter.h
  ITKLayerImporter.cc
  MatlabLayerImporter.h
  MatlabLayerImporter.cc
  NrrdLayerImporter.h
  NrrdLayerImporter.cc
  VFFLayerImporter.h
  VFFLayerImporter.cc
)

CORE_ADD_LIBRARY(Application_LayerIO ${APPLICATION_LAYERIO_SRCS}
      ${APPLICATION_LAYERIO_IMPORTERS_SRCS}
            ${APPLICATION_LAYERIO_ACTIONS_SRCS} )
            
TARGET_LINK_LIBRARIES(Application_LayerIO
          Core_Utils
          Core_Geometry
          Core_DataBlock
          Core_Volume
          Core_Application
          Core_Interface
          Core_Action
          Core_State
          Application_InterfaceManager
          Application_Layer
          ${SCI_BOOST_LIBRARY})

# Register actions and importers
REGISTER_LIBRARY_AND_CLASSES(Application_LayerIO 
  ${APPLICATION_LAYERIO_ACTIONS_SRCS}
  ${APPLICATION_LAYERIO_IMPORTERS_SRCS})

