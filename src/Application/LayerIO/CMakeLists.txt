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
  LayerExporter.h
  LayerExporter.cc
  LayerImporter.h
  LayerImporter.cc
  LayerSingleFileImporter.h
  LayerSingleFileImporter.cc
  LayerFileSeriesImporter.h
  LayerFileSeriesImporter.cc
  LayerImporterInfo.h
  LayerImporterInfo.cc
  LayerImporterFileInfo.h
  LayerImporterFileInfo.cc
  LayerImporterFileData.h
  LayerImporterFileData.cc
  LayerExporterInfo.h
  LayerExporterInfo.cc
  LayerIO.h
  LayerIO.cc
)

SET(APPLICATION_LAYERIO_ACTIONS_SRCS
  Actions/ActionExportLayer.h
  Actions/ActionExportLayer.cc
  Actions/ActionExportSegmentation.h
  Actions/ActionExportSegmentation.cc
  Actions/ActionExportIsosurface.h
  Actions/ActionExportIsosurface.cc
  Actions/ActionImportLayer.h
  Actions/ActionImportLayer.cc
  Actions/ActionImportSeries.h
  Actions/ActionImportSeries.cc
  Actions/ActionExportPoints.h
  Actions/ActionExportPoints.cc
  Actions/ActionImportLargeVolumeLayer.h
  Actions/ActionImportLargeVolumeLayer.cc
)

SET(APPLICATION_LAYERIO_IMPORTERS_SRCS
  ITKLayerImporter.h
  ITKLayerImporter.cc
  ITKSeriesLayerImporter.h
  ITKSeriesLayerImporter.cc
  Matlab73LayerImporter.h
  Matlab73LayerImporter.cc
  MatlabLayerImporter.h
  MatlabLayerImporter.cc
  MRCLayerImporter.h
  MRCLayerImporter.cc
  NrrdLayerImporter.h
  NrrdLayerImporter.cc
  VFFLayerImporter.h
  VFFLayerImporter.cc
  GDCMLayerImporter.h
  GDCMLayerImporter.cc
)

SET(APPLICATION_LAYERIO_EXPORTERS_SRCS
  ITKDataLayerExporter.h
  ITKDataLayerExporter.cc
  ITKMaskLayerExporter.h
  ITKMaskLayerExporter.cc
  MRCLayerExporter.h
  MRCLayerExporter.cc
  NrrdLayerExporter.h
  NrrdLayerExporter.cc
  MatlabLayerExporter.h
  MatlabLayerExporter.cc
)

IF(BUILD_WITH_PYTHON)
  GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER Application_LayerIO ${APPLICATION_LAYERIO_ACTIONS_SRCS})
  SET(APPLICATION_LAYERIO_SRCS ${APPLICATION_LAYERIO_SRCS} ${PYTHON_WRAPPER})
ENDIF(BUILD_WITH_PYTHON)

CORE_ADD_LIBRARY(Application_LayerIO ${APPLICATION_LAYERIO_SRCS}
  ${APPLICATION_LAYERIO_IMPORTERS_SRCS}
  ${APPLICATION_LAYERIO_EXPORTERS_SRCS}
  ${APPLICATION_LAYERIO_ACTIONS_SRCS}
)

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
  ${SCI_BOOST_LIBRARY}
  ${ITKCommon_LIBRARIES}
  ${ITKIOImageBase_LIBRARIES}
  ${ITKIOGDCM_LIBRARIES}
  ${ITKDeprecated_LIBRARIES}
  ${ITKIOBMP_LIBRARIES}
  ${ITKIOTIFF_LIBRARIES}
  ${ITKIONIFTI_LIBRARIES}
  ${ITKIOPNG_LIBRARIES}
  ${ITKReview_LIBRARIES}
  ${ITKIOHDF5_LIBRARIES}
  ${ITKHDF5_LIBRARIES}
  ${ITKIOMRC_LIBRARIES}
  ${SCI_MATLABIO_LIBRARY}
  ${SCI_MRC2000IO_LIBRARY}
)

# Register actions and importers
REGISTER_LIBRARY_AND_CLASSES(Application_LayerIO 
  ${APPLICATION_LAYERIO_ACTIONS_SRCS}
  ${APPLICATION_LAYERIO_IMPORTERS_SRCS}
  ${APPLICATION_LAYERIO_EXPORTERS_SRCS}
)
