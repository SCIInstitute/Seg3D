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

SET(INTERFACE_TOOLINTERFACE_SRCS
  CropToolInterface.cc
  PaintToolInterface.cc
  PolylineToolInterface.cc
  InvertToolInterface.cc
  ResampleToolInterface.cc
  ThresholdToolInterface.cc
  ArithmeticFilterInterface.cc
  BinaryDilateErodeFilterInterface.cc
  #BinaryIterativeDilateErodeFilterInterface.cc
  BinarySmoothDilateErodeFilterInterface.cc
  CannyEdgeDetectionFilterInterface.cc
  ConfidenceConnectedFilterInterface.cc
  ConnectedComponentFilterInterface.cc
  ConnectedComponentSizeFilterInterface.cc
  CurvatureAnisotropicDiffusionFilterInterface.cc
  DiscreteGaussianFilterInterface.cc
  DistanceFilterInterface.cc
  FillHolesFilterInterface.cc
  GradientAnisotropicDiffusionFilterInterface.cc
  GradientMagnitudeFilterInterface.cc
  HistogramEqualizationFilterInterface.cc
  IntensityCorrectionFilterInterface.cc
  MaskDataFilterInterface.cc
  MedianFilterInterface.cc
  MeanFilterInterface.cc
  NeighborhoodConnectedFilterInterface.cc
  OtsuThresholdFilterInterface.cc
  AndFilterInterface.cc
  RemoveFilterInterface.cc
  OrFilterInterface.cc
  XorFilterInterface.cc
  ThresholdSegmentationLSFilterInterface.cc
  FlipToolInterface.cc
  TransformToolInterface.cc
  ClipboardToolInterface.cc
  MeasurementToolInterface.cc
  PointSetFilterInterface.cc
  SpeedlineToolInterface.cc
  PointsSelectToolInterface.cc
)

SET(INTERFACE_TOOLINTERFACE_MOC_SRCS
  CropToolInterface.h
  InvertToolInterface.h
  PaintToolInterface.h
  PolylineToolInterface.h
  ResampleToolInterface.h
  ThresholdToolInterface.h
  ArithmeticFilterInterface.h
  BinaryDilateErodeFilterInterface.h
  #BinaryIterativeDilateErodeFilterInterface.h
  BinarySmoothDilateErodeFilterInterface.h
  CannyEdgeDetectionFilterInterface.h
  ConfidenceConnectedFilterInterface.h
  ConnectedComponentFilterInterface.h
  ConnectedComponentSizeFilterInterface.h
  CurvatureAnisotropicDiffusionFilterInterface.h
  DiscreteGaussianFilterInterface.h
  DistanceFilterInterface.h
  FillHolesFilterInterface.h
  GradientAnisotropicDiffusionFilterInterface.h
  GradientMagnitudeFilterInterface.h
  HistogramEqualizationFilterInterface.h
  IntensityCorrectionFilterInterface.h
  MaskDataFilterInterface.h
  MedianFilterInterface.h
  MeanFilterInterface.h
  NeighborhoodConnectedFilterInterface.h
  OtsuThresholdFilterInterface.h
  AndFilterInterface.h
  RemoveFilterInterface.h
  OrFilterInterface.h
  XorFilterInterface.h
  ThresholdSegmentationLSFilterInterface.h
  FlipToolInterface.h
  TransformToolInterface.h
  ClipboardToolInterface.h
  MeasurementToolInterface.h 
  detail/MeasurementTableView.h
  detail/MeasurementTableModel.h
  detail/MeasurementItemDelegate.h
  detail/MeasurementTextEdit.h
  PointSetFilterInterface.h
  SpeedlineToolInterface.h
  PointsSelectToolInterface.h
)

SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS
  detail/MeasurementTableView.cc
  detail/MeasurementTableModel.cc
  detail/MeasurementItemDelegate.cc
  detail/MeasurementTextEdit.cc
)

SET(INTERFACE_TOOLINTERFACE_UI_SRCS
  CropToolInterface.ui
  InvertToolInterface.ui
  PaintToolInterface.ui
  PolylineToolInterface.ui
  ThresholdToolInterface.ui
  ResampleToolInterface.ui  
  ArithmeticFilterInterface.ui
  BinaryDilateErodeFilterInterface.ui
  #BinaryIterativeDilateErodeFilterInterface.ui
  BinarySmoothDilateErodeFilterInterface.ui
  CannyEdgeDetectionFilterInterface.ui  
  ConfidenceConnectedFilterInterface.ui
  ConnectedComponentFilterInterface.ui
  ConnectedComponentSizeFilterInterface.ui
  CurvatureAnisotropicDiffusionFilterInterface.ui
  DiscreteGaussianFilterInterface.ui
  DistanceFilterInterface.ui
  FillHolesFilterInterface.ui
  GradientAnisotropicDiffusionFilterInterface.ui
  GradientMagnitudeFilterInterface.ui
  HistogramEqualizationFilterInterface.ui
  IntensityCorrectionFilterInterface.ui
  MaskDataFilterInterface.ui
  MedianFilterInterface.ui
  MeanFilterInterface.ui
  NeighborhoodConnectedFilterInterface.ui
  OtsuThresholdFilterInterface.ui
  AndFilterInterface.ui
  RemoveFilterInterface.ui
  OrFilterInterface.ui
  XorFilterInterface.ui
  ThresholdSegmentationLSFilterInterface.ui
  FlipToolInterface.ui
  TransformToolInterface.ui
  ClipboardToolInterface.ui
  MeasurementToolInterface.ui
  PointSetFilterInterface.ui
  SpeedlineToolInterface.ui
  PointsSelectToolInterface.ui
)

##################################################
# Get Icon files
##################################################

#QT4_ADD_RESOURCES(INTERFACE_TOOLINTERFACE_QT_RCC_SRCS Resources.qrc)

##################################################
# Generate header out of UI code
##################################################

QT4_WRAP_UI(INTERFACE_TOOLINTERFACE_QT_UI_SRCS ${INTERFACE_TOOLINTERFACE_UI_SRCS})

##################################################
# Wrap QT code to expand all the moc code
##################################################

QT4_WRAP_CPP(INTERFACE_TOOLINTERFACE_QT_MOC_SRCS ${INTERFACE_TOOLINTERFACE_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY(Interface_ToolInterface 
  ${INTERFACE_TOOLINTERFACE_SRCS} 
  ${INTERFACE_TOOLINTERFACE_QT_UI_SRCS}
  ${INTERFACE_TOOLINTERFACE_QT_MOC_SRCS}
  ${INTERFACE_TOOLINTERFACE_NO_MOC_SRCS} 
  ${INTERFACE_TOOLINTERFACE_MOC_SRCS}
)
                                        
TARGET_LINK_LIBRARIES(Interface_ToolInterface
  Core_Utils
  Core_EventHandler
  Core_Application
  Core_Interface
  Core_Action
  Application_Tool
  QtUtils_Utils
  QtUtils_Widgets
  QtUtils_Bridge
  Interface_Application
  ${QT_LIBRARIES} 
  ${SCI_BOOST_LIBRARY}
)

# Register action classes
REGISTER_LIBRARY_AND_CLASSES(Interface_ToolInterface
  ${INTERFACE_TOOLINTERFACE_SRCS}
)
