
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
SET(NUM_PROJECTS 8)

# PROJECT 1
SET(INTERFACE_TOOLINTERFACE_SRCS_1
  CropToolInterface.cc
  PaintToolInterface.cc
  InvertToolInterface.cc
  PolylineToolInterface.cc
  ResampleToolInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_1
  CropToolInterface.h
  PaintToolInterface.h
  InvertToolInterface.h
  PolylineToolInterface.h
  ResampleToolInterface.h
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_1
  CropToolInterface.ui
  PaintToolInterface.ui
  InvertToolInterface.ui
  PolylineToolInterface.ui
  ResampleToolInterface.ui
)

# PROJECT 2
SET(INTERFACE_TOOLINTERFACE_SRCS_2
  ThresholdToolInterface.cc
  ArithmeticFilterInterface.cc
  BinaryDilateErodeFilterInterface.cc
  BinarySmoothDilateErodeFilterInterface.cc
  CannyEdgeDetectionFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_2
  ThresholdToolInterface.h
  ArithmeticFilterInterface.h
  BinaryDilateErodeFilterInterface.h
  BinarySmoothDilateErodeFilterInterface.h
  CannyEdgeDetectionFilterInterface.h  
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_2
  ThresholdToolInterface.ui  
  ArithmeticFilterInterface.ui
  BinaryDilateErodeFilterInterface.ui
  BinarySmoothDilateErodeFilterInterface.ui
  CannyEdgeDetectionFilterInterface.ui  
)

# PROJECT 3
SET(INTERFACE_TOOLINTERFACE_SRCS_3
  ConfidenceConnectedFilterInterface.cc
  ConnectedComponentFilterInterface.cc
  ConnectedComponentSizeFilterInterface.cc
  CurvatureAnisotropicDiffusionFilterInterface.cc
  DiscreteGaussianFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_3
  ConfidenceConnectedFilterInterface.h
  ConnectedComponentFilterInterface.h
  ConnectedComponentSizeFilterInterface.h
  CurvatureAnisotropicDiffusionFilterInterface.h
  DiscreteGaussianFilterInterface.h
  
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_3
  ConfidenceConnectedFilterInterface.ui
  ConnectedComponentFilterInterface.ui
  ConnectedComponentSizeFilterInterface.ui
  CurvatureAnisotropicDiffusionFilterInterface.ui
  DiscreteGaussianFilterInterface.ui
)

# PROJECT 4
SET(INTERFACE_TOOLINTERFACE_SRCS_4
  DistanceFilterInterface.cc
  FillHolesFilterInterface.cc
  GradientAnisotropicDiffusionFilterInterface.cc
  GradientMagnitudeFilterInterface.cc
  HistogramEqualizationFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_4
  DistanceFilterInterface.h
  FillHolesFilterInterface.h
  GradientAnisotropicDiffusionFilterInterface.h 
  GradientMagnitudeFilterInterface.h
  HistogramEqualizationFilterInterface.h
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_4
  DistanceFilterInterface.ui
  FillHolesFilterInterface.ui
  GradientAnisotropicDiffusionFilterInterface.ui
  GradientMagnitudeFilterInterface.ui
  HistogramEqualizationFilterInterface.ui
)  

# PROJECT 5
SET(INTERFACE_TOOLINTERFACE_SRCS_5
  IntensityCorrectionFilterInterface.cc
  MaskDataFilterInterface.cc
  MedianFilterInterface.cc
  MeanFilterInterface.cc
  NeighborhoodConnectedFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_5
  IntensityCorrectionFilterInterface.h
  MaskDataFilterInterface.h
  MedianFilterInterface.h
  MeanFilterInterface.h
  NeighborhoodConnectedFilterInterface.h
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_5
  IntensityCorrectionFilterInterface.ui
  MaskDataFilterInterface.ui
  MedianFilterInterface.ui
  MeanFilterInterface.ui
  NeighborhoodConnectedFilterInterface.ui
)

# PROJECT 6
SET(INTERFACE_TOOLINTERFACE_SRCS_6
  OtsuThresholdFilterInterface.cc
  AndFilterInterface.cc
  RemoveFilterInterface.cc
  OrFilterInterface.cc
  XorFilterInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_6
  OtsuThresholdFilterInterface.h
  AndFilterInterface.h
  RemoveFilterInterface.h
  OrFilterInterface.h
  XorFilterInterface.h
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_6
  OtsuThresholdFilterInterface.ui
  AndFilterInterface.ui
  RemoveFilterInterface.ui
  OrFilterInterface.ui
  XorFilterInterface.ui
)

# PROJECT 7
SET(INTERFACE_TOOLINTERFACE_SRCS_7
  ThresholdSegmentationLSFilterInterface.cc
  FlipToolInterface.cc
  TransformToolInterface.cc
  ClipboardToolInterface.cc
  PointSetFilterInterface.cc
  SpeedlineToolInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_7
  ThresholdSegmentationLSFilterInterface.h
  FlipToolInterface.h
  TransformToolInterface.h
  ClipboardToolInterface.h
  PointSetFilterInterface.h
  SpeedlineToolInterface.h
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_7
  ThresholdSegmentationLSFilterInterface.ui
  FlipToolInterface.ui
  TransformToolInterface.ui
  ClipboardToolInterface.ui
  PointSetFilterInterface.ui
  SpeedlineToolInterface.ui  
)

# PROJECT 8
SET(INTERFACE_TOOLINTERFACE_SRCS_8
  MeasurementToolInterface.cc
)
SET(INTERFACE_TOOLINTERFACE_MOC_SRCS_8
  MeasurementToolInterface.h
  detail/MeasurementTableView.h
  detail/MeasurementTableModel.h
  detail/MeasurementItemDelegate.h
  detail/MeasurementTextEdit.h
)
SET(INTERFACE_TOOLINTERFACE_NO_MOC_SRCS_8 
  detail/MeasurementTableView.cc
  detail/MeasurementTableModel.cc
  detail/MeasurementItemDelegate.cc
  detail/MeasurementTextEdit.cc
)
SET(INTERFACE_TOOLINTERFACE_UI_SRCS_8
  MeasurementToolInterface.ui
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

##################################################
# Create projects
##################################################
            
IF (SEG3D_SPLIT_PROJECTS)

  # Split large projects into smaller projects to speed up compilation with Visual Studio.
  # Visual Studio doesn't parallelize compilation within projects, but it does parallelize
  # across projects.  

  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    ##################################################
    # Generate header out of UI code
    ##################################################
  
    SET(INTERFACE_TOOLINTERFACE_UI_SRCS "INTERFACE_TOOLINTERFACE_UI_SRCS_${PROJ_IDX}")
    # Clear variable of values from last iteration
    SET(INTERFACE_TOOLINTERFACE_QT_UI_SRCS "")
    QT4_WRAP_UI(INTERFACE_TOOLINTERFACE_QT_UI_SRCS ${${INTERFACE_TOOLINTERFACE_UI_SRCS}})
    
    ##################################################
    # Wrap QT code to expand all the moc code
    ##################################################

    SET(INTERFACE_TOOLINTERFACE_MOC_SRCS "INTERFACE_TOOLINTERFACE_MOC_SRCS_${PROJ_IDX}")
    # Clear variable of values from last iteration
    SET(INTERFACE_TOOLINTERFACE_QT_MOC_SRCS "")
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