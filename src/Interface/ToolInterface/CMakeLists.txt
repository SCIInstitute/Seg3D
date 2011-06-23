
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
  
# NOTE: Tried splitting into 8 projects, but compilation actually got slower.  Maybe it has 
# something to do with the moc compiler?
SET(NUM_PROJECTS 4)

# PROJECT 1
SET(INTERFACE_TOOLINTERFACE_SRCS_1
  CropToolInterface.cc
  PaintToolInterface.cc
  PolylineToolInterface.cc
  InvertToolInterface.cc
  ResampleToolInterface.cc
  ThresholdToolInterface.cc
  ArithmeticFilterInterface.cc
  BinaryDilateErodeFilterInterface.cc
  BinarySmoothDilateErodeFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_1
  CropToolInterface.h
  InvertToolInterface.h
  PaintToolInterface.h
  PolylineToolInterface.h
  ResampleToolInterface.h
  ThresholdToolInterface.h
  ArithmeticFilterInterface.h
  BinaryDilateErodeFilterInterface.h
  BinarySmoothDilateErodeFilterInterface.h
  ResampleToolInterface.h
  ThresholdToolInterface.h  
  ArithmeticFilterInterface.h
  BinaryDilateErodeFilterInterface.h
  BinarySmoothDilateErodeFilterInterface.h
)
SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS_1 "")
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_1
  CropToolInterface.ui
  InvertToolInterface.ui
  PaintToolInterface.ui
  PolylineToolInterface.ui
  ResampleToolInterface.ui
  ThresholdToolInterface.ui  
  ArithmeticFilterInterface.ui
  BinaryDilateErodeFilterInterface.ui
  BinarySmoothDilateErodeFilterInterface.ui
)

# PROJECT 2
SET(INTERFACE_TOOLINTERFACE_SRCS_2
  CannyEdgeDetectionFilterInterface.cc
  ConfidenceConnectedFilterInterface.cc
  ConnectedComponentFilterInterface.cc
  ConnectedComponentSizeFilterInterface.cc
  CurvatureAnisotropicDiffusionFilterInterface.cc
  DiscreteGaussianFilterInterface.cc
  DistanceFilterInterface.cc
  FillHolesFilterInterface.cc
  GradientAnisotropicDiffusionFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_2
  CannyEdgeDetectionFilterInterface.h
  ConfidenceConnectedFilterInterface.h
  ConnectedComponentFilterInterface.h
  ConnectedComponentSizeFilterInterface.h
  CurvatureAnisotropicDiffusionFilterInterface.h
  DiscreteGaussianFilterInterface.h
  DistanceFilterInterface.h
  FillHolesFilterInterface.h
  GradientAnisotropicDiffusionFilterInterface.h 
)
SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS_2 "")
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_2
  CannyEdgeDetectionFilterInterface.ui  
  ConfidenceConnectedFilterInterface.ui
  ConnectedComponentFilterInterface.ui
  ConnectedComponentSizeFilterInterface.ui
  CurvatureAnisotropicDiffusionFilterInterface.ui
  DiscreteGaussianFilterInterface.ui
  DistanceFilterInterface.ui
  FillHolesFilterInterface.ui
  GradientAnisotropicDiffusionFilterInterface.ui
)

# PROJECT 3
SET(INTERFACE_TOOLINTERFACE_SRCS_3
  GradientMagnitudeFilterInterface.cc
  HistogramEqualizationFilterInterface.cc
  IntensityCorrectionFilterInterface.cc
  MaskDataFilterInterface.cc
  MedianFilterInterface.cc
  MeanFilterInterface.cc
  NeighborhoodConnectedFilterInterface.cc
  OtsuThresholdFilterInterface.cc
  AndFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_3
  GradientMagnitudeFilterInterface.h
  HistogramEqualizationFilterInterface.h
  IntensityCorrectionFilterInterface.h
  MaskDataFilterInterface.h
  MedianFilterInterface.h
  MeanFilterInterface.h
  NeighborhoodConnectedFilterInterface.h
  OtsuThresholdFilterInterface.h
  AndFilterInterface.h
)
#SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS_3 "")
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_3
  GradientMagnitudeFilterInterface.ui
  HistogramEqualizationFilterInterface.ui
  IntensityCorrectionFilterInterface.ui
  MaskDataFilterInterface.ui
  MedianFilterInterface.ui
  MeanFilterInterface.ui
  NeighborhoodConnectedFilterInterface.ui
  OtsuThresholdFilterInterface.ui
  AndFilterInterface.ui
)

# PROJECT 4
SET(INTERFACE_TOOLINTERFACE_SRCS_4
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
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_4
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
)
SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS_4 
  detail/MeasurementTableView.cc
  detail/MeasurementTableModel.cc
  detail/MeasurementItemDelegate.cc
  detail/MeasurementTextEdit.cc

)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_4
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
)

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Set link libraries
##################################################

SET(INTERFACE_TOOLINTERFACE_LINK_LIBS 
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

# NOTE: Project splitting disabled since compiling these files in parallel often resulted in moc 
# generation errors. Compiling split projects individually did not result on moc generation errors.               
IF (FALSE) #(SEG3D_SPLIT_PROJECTS)

  # Split large projects into smaller projects to speed up compilation with Visual Studio.
  # Visual Studio doesn't parallelize compilation within projects, but it does parallelize
  # across projects.  

  
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    ##################################################
    # Generate header out of UI code
    ##################################################
  
    SET(INTERFACE_TOOLINTERFACE_UI_SRCS "INTERFACE_TOOLINTERFACE_UI_SRCS_${PROJ_IDX}")
    QT4_WRAP_UI(INTERFACE_TOOLINTERFACE_QT_UI_SRCS ${${INTERFACE_TOOLINTERFACE_UI_SRCS}})
    
    ##################################################
    # Wrap QT code to expand all the moc code
    ##################################################

    SET(INTERFACE_TOOLINTERFACE_MOC_SRCS "INTERFACE_TOOLINTERFACE_MOC_SRCS_${PROJ_IDX}")
    QT4_WRAP_CPP(INTERFACE_TOOLINTERFACE_QT_MOC_SRCS ${${INTERFACE_TOOLINTERFACE_MOC_SRCS}})
    
    ##################################################
    # Build the Components library
    ##################################################
    
    SET(INTERFACE_TOOLINTERFACE_SRCS "INTERFACE_TOOLINTERFACE_SRCS_${PROJ_IDX}")
    SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS "INTERFACE_TOOLINTERFACE_NO_MOC_SRCS_${PROJ_IDX}") 
    SET(INTERFACE_TOOLINTERFACE_MOC_SRCS "INTERFACE_TOOLINTERFACE_MOC_SRCS_${PROJ_IDX}")                
    SET(INTERFACE_TOOLINTERFACE_LIB "Interface_ToolInterface_${PROJ_IDX}")  
    CORE_ADD_LIBRARY(${INTERFACE_TOOLINTERFACE_LIB} 
      ${${INTERFACE_TOOLINTERFACE_SRCS}} 
      ${INTERFACE_TOOLINTERFACE_QT_UI_SRCS}
      ${INTERFACE_TOOLINTERFACE_QT_MOC_SRCS}
      ${${INTERFACE_TOOLINTERFACE_NO_MOC_SRCS}} 
      ${${INTERFACE_TOOLINTERFACE_MOC_SRCS}})     
                        
    TARGET_LINK_LIBRARIES(${INTERFACE_TOOLINTERFACE_LIB}
      ${INTERFACE_TOOLINTERFACE_LINK_LIBS})
                                                        
    # Register action classes
    REGISTER_LIBRARY_AND_CLASSES(${INTERFACE_TOOLINTERFACE_LIB}
          ${${INTERFACE_TOOLINTERFACE_SRCS}})
    
    # Create single CMake variable that other libraries can depend on
    # Tack library onto end of list     
    SET(Interface_ToolInterface "${Interface_ToolInterface};${INTERFACE_TOOLINTERFACE_LIB}")
    
  ENDFOREACH (PROJ_IDX)

ELSE (SEG3D_SPLIT_PROJECTS)

  # Consolidate sources back into single project
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    SET(INTERFACE_TOOLINTERFACE_PROJ_SRCS "INTERFACE_TOOLINTERFACE_SRCS_${PROJ_IDX}") 
    SET(INTERFACE_TOOLINTERFACE_SRCS "${INTERFACE_TOOLINTERFACE_SRCS};${${INTERFACE_TOOLINTERFACE_PROJ_SRCS}}")
      
    SET(INTERFACE_TOOLINTERFACE_MOC_PROJ_SRCS "INTERFACE_TOOLINTERFACE_MOC_SRCS_${PROJ_IDX}")   
    SET(INTERFACE_TOOLINTERFACE_MOC_SRCS "${INTERFACE_TOOLINTERFACE_MOC_SRCS};${${INTERFACE_TOOLINTERFACE_MOC_PROJ_SRCS}}")   
    
    SET(INTERFACE_TOOLINTERFACE_NO_MOC_PROJ_SRCS "INTERFACE_TOOLINTERFACE_NO_MOC_SRCS_${PROJ_IDX}")
    SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS "${INTERFACE_TOOLINTERFACE_NO_MOC_SRCS};${${INTERFACE_TOOLINTERFACE_NO_MOC_PROJ_SRCS}}")  
    
    SET(INTERFACE_TOOLINTERFACE_UI_PROJ_SRCS "INTERFACE_TOOLINTERFACE_UI_SRCS_${PROJ_IDX}") 
    SET(INTERFACE_TOOLINTERFACE_UI_SRCS "${INTERFACE_TOOLINTERFACE_UI_SRCS};${${INTERFACE_TOOLINTERFACE_UI_PROJ_SRCS}}") 
    
  ENDFOREACH (PROJ_IDX)
    
  ##################################################
  # Generate header out of UI code
  ##################################################

  QT4_WRAP_UI(INTERFACE_TOOLINTERFACE_QT_UI_SRCS ${INTERFACE_TOOLINTERFACE_UI_SRCS})

  ##################################################
  # Wrap QT code to expand all the moc code
  ##################################################

  QT4_WRAP_CPP(INTERFACE_TOOLINTERFACE_QT_MOC_SRCS ${INTERFACE_TOOLINTERFACE_MOC_SRCS})

  ##################################################
  # Build the Components library
  ##################################################
                      
  CORE_ADD_LIBRARY(Interface_ToolInterface 
    ${INTERFACE_TOOLINTERFACE_SRCS} 
    ${INTERFACE_TOOLINTERFACE_QT_UI_SRCS}
    ${INTERFACE_TOOLINTERFACE_QT_MOC_SRCS}
    ${INTERFACE_TOOLINTERFACE_NO_MOC_SRCS} 
    ${INTERFACE_TOOLINTERFACE_MOC_SRCS})      
                        
  TARGET_LINK_LIBRARIES(Interface_ToolInterface
    ${INTERFACE_TOOLINTERFACE_LINK_LIBS})
                                                
  # Register action classes
  REGISTER_LIBRARY_AND_CLASSES(Interface_ToolInterface
    ${INTERFACE_TOOLINTERFACE_SRCS})
        
ENDIF (SEG3D_SPLIT_PROJECTS)  