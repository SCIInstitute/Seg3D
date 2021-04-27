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
# Import shaders into source files
##################################################

set(APPLICATION_TOOLS_SHADER_SRCS
  detail/MaskShader.frag
)

CORE_IMPORT_SHADER(APPLICATION_TOOLS_SHADER_STRING_SRCS
  ${APPLICATION_TOOLS_SHADER_SRCS}
)

##################################################
# Set sources
##################################################

set(APPLICATION_TOOLS_SRCS
  PaintTool.h
  PaintTool.cc
  InvertTool.h
  InvertTool.cc
  PolylineTool.h
  PolylineTool.cc
  ThresholdTool.h
  ThresholdTool.cc
  ArithmeticFilter.h
  ArithmeticFilter.cc
  BinaryDilateErodeFilter.h
  BinaryDilateErodeFilter.cc
  BinaryIterativeDilateErodeFilter.h
  BinaryIterativeDilateErodeFilter.cc
  BinarySmoothDilateErodeFilter.h
  BinarySmoothDilateErodeFilter.cc
  CannyEdgeDetectionFilter.h
  CannyEdgeDetectionFilter.cc
  ConfidenceConnectedFilter.h
  ConfidenceConnectedFilter.cc
  ConnectedComponentFilter.h
  ConnectedComponentFilter.cc
  ConnectedComponentSizeFilter.h
  ConnectedComponentSizeFilter.cc
  CurvatureAnisotropicDiffusionFilter.h
  CurvatureAnisotropicDiffusionFilter.cc
  DiscreteGaussianFilter.h
  DiscreteGaussianFilter.cc
  DistanceFilter.h
  DistanceFilter.cc
  ExtractDataLayer.h
  ExtractDataLayer.cc
  FillHolesFilter.h
  FillHolesFilter.cc
  GradientAnisotropicDiffusionFilter.h
  GradientAnisotropicDiffusionFilter.cc
  GradientMagnitudeFilter.h
  GradientMagnitudeFilter.cc
  GrowCutTool.h
  GrowCutTool.cc
  HistogramEqualizationFilter.h
  HistogramEqualizationFilter.cc
  IntensityCorrectionFilter.h
  IntensityCorrectionFilter.cc
  MaskDataFilter.h
  MaskDataFilter.cc
  MedianFilter.h
  MedianFilter.cc
  MeanFilter.h
  MeanFilter.cc
  MeasurementTool.h
  MeasurementTool.cc
  NeighborhoodConnectedFilter.h
  NeighborhoodConnectedFilter.cc
  OtsuThresholdFilter.h
  OtsuThresholdFilter.cc
  ThresholdSegmentationLSFilter.h
  ThresholdSegmentationLSFilter.cc
  AndFilter.h
  AndFilter.cc
  RemoveFilter.h
  RemoveFilter.cc
  OrFilter.h
  OrFilter.cc
  XorFilter.h
  XorFilter.cc
  ResampleTool.h
  ResampleTool.cc
  CropTool.h
  CropTool.cc
  FlipTool.h
  FlipTool.cc
  TransformTool.h
  TransformTool.cc
  ClipboardTool.h
  ClipboardTool.cc
  PointSetFilter.h
  PointSetFilter.cc
  SpeedlineTool.h
  SpeedlineTool.cc
  PointsSelectTool.h
  PointsSelectTool.cc
  PadTool.h
  PadTool.cc
  ImplicitModelTool.h
  ImplicitModelTool.cc
  WatershedFilter.h
  WatershedFilter.cc
)

set(APPLICATION_TOOLS_NOREGISTER_SRCS
  SliceRange.h
  SliceRange.cc
  detail/MaskShader.h
  detail/MaskShader.cc
)

SET(APPLICATION_TOOLS_ACTIONS_SRCS
  Actions/ActionPaint.h
  Actions/ActionPaint.cc
  Actions/ActionPolyline.h
  Actions/ActionPolyline.cc
  Actions/ActionCopy.h
  Actions/ActionCopy.cc
  Actions/ActionPaste.h
  Actions/ActionPaste.cc
  Actions/ActionFloodFill.h
  Actions/ActionFloodFill.cc
  Actions/ActionGrowCut.h
  Actions/ActionGrowCut.cc
  Actions/ActionGrowCutInitialize.h
  Actions/ActionGrowCutInitialize.cc
  Actions/ActionGrowCutPostProcess.h
  Actions/ActionGrowCutPostProcess.cc
  Actions/ActionSpeedline.h
  Actions/ActionSpeedline.cc
  Actions/ActionExtractDataLayer.h
  Actions/ActionExtractDataLayer.cc
)

SET(APPLICATION_TOOLS_ALGORITHM_SRCS
  Algorithm/GrowCutter.h
  Algorithm/GrowCutter.cc
  Algorithm/IslandRemoval.h
  )

IF(BUILD_WITH_PYTHON)
  GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER Application_Tools ${APPLICATION_TOOLS_ACTIONS_SRCS})
  set(APPLICATION_TOOLS_NOREGISTER_SRCS ${APPLICATION_TOOLS_NOREGISTER_SRCS} ${PYTHON_WRAPPER})
endif()

ITK_MODULE_LOAD(ITKCommon)
ITK_MODULE_LOAD(GrowCut)

CORE_ADD_LIBRARY(Application_Tools
  ${APPLICATION_TOOLS_SRCS}
  ${APPLICATION_TOOLS_NOREGISTER_SRCS}
  ${APPLICATION_TOOLS_ACTIONS_SRCS}
  ${APPLICATION_TOOLS_ALGORITHM_SRCS}
  ${APPLICATION_TOOLS_SHADER_SRCS}
  ${APPLICATION_TOOLS_SHADER_STRING_SRCS}
)

target_link_libraries(Application_Tools
  Core_Utils
  Core_Application
  Core_Interface
  Core_Action
  Core_State
  Core_Geometry
  Core_Renderer
  Application_Clipboard
  Application_Filters
  Application_Tool
  Application_ToolManager
  ${SCI_BOOST_LIBRARY}
  ${ITKCommon_LIBRARIES}
  ${GrowCut_LIBRARIES}
)

# Register tool classes
REGISTER_LIBRARY_AND_CLASSES(Application_Tools
  ${APPLICATION_TOOLS_SRCS}
  ${APPLICATION_TOOLS_ACTIONS_SRCS}
)
