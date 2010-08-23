
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
# Import shaders into source files
##################################################

SET(APPLICATION_TOOLS_SHADER_SRCS
  detail/PaintBrushShader.frag
  )
  
CORE_IMPORT_SHADER(APPLICATION_TOOLS_SHADER_STRING_SRCS
          ${APPLICATION_TOOLS_SHADER_SRCS})

##################################################
# Set sources
##################################################

SET(APPLICATION_TOOLS_SRCS
  PaintTool.h
  PaintTool.cc
  InvertTool.h
  InvertTool.cc
  PolylineTool.h
  PolylineTool.cc
  ThresholdTool.h
  ThresholdTool.cc
  MedianFilter.h
  MedianFilter.cc
  ArithmeticFilter.h
  ArithmeticFilter.cc
  BinaryDilateErodeFilter.h
  BinaryDilateErodeFilter.cc
  BooleanFilter.h
  BooleanFilter.cc
  CannyEdgeDetectionFilter.h
  CannyEdgeDetectionFilter.cc
  ConfidenceConnectedFilter.h
  ConfidenceConnectedFilter.cc
  ConnectedComponentFilter.h
  ConnectedComponentFilter.cc
  CurvatureAnisotropicDiffusionFilter.h
  CurvatureAnisotropicDiffusionFilter.cc
  DiscreteGaussianFilter.h
  DiscreteGaussianFilter.cc  
  FillHolesFilter.h
  FillHolesFilter.cc
  GradientMagnitudeFilter.h
  GradientMagnitudeFilter.cc
  HistogramEqualizationFilter.h
  HistogramEqualizationFilter.cc
  IntensityCorrectionFilter.h
  IntensityCorrectionFilter.cc
  MaskDataFilter.h
  MaskDataFilter.cc
  NeighborhoodConnectedFilter.h
  NeighborhoodConnectedFilter.cc
  OtsuThresholdFilter.h
  OtsuThresholdFilter.cc
  ThresholdSegmentationLSFilter.h
  ThresholdSegmentationLSFilter.cc  
  )

SET(APPLICATION_TOOLS_NOREGISTER_SRCS
  SeedPointsTool.h
  SeedPointsTool.cc
  detail/PaintBrushShader.h
  detail/PaintBrushShader.cc
  )
  
SET(APPLICATION_TOOLS_ACTIONS_SRCS
  Actions/ActionPaint.h
  Actions/ActionPaint.cc
  Actions/ActionCannyEdgeDetectionFilter.h
  Actions/ActionCannyEdgeDetectionFilter.cc
  Actions/ActionCurvatureAnisotropicDiffusionFilter.h
  Actions/ActionCurvatureAnisotropicDiffusionFilter.cc
  Actions/ActionDiscreteGaussianFilter.h
  Actions/ActionDiscreteGaussianFilter.cc
  Actions/ActionGradientMagnitudeFilter.h
  Actions/ActionGradientMagnitudeFilter.cc
  Actions/ActionMedianFilter.h
  Actions/ActionMedianFilter.cc
  )

CORE_ADD_LIBRARY(Application_Tools 
                 ${APPLICATION_TOOLS_SRCS}
                 ${APPLICATION_TOOLS_NOREGISTER_SRCS}
         ${APPLICATION_TOOLS_ACTIONS_SRCS}
         ${APPLICATION_TOOLS_SHADER_SRCS}
         ${APPLICATION_TOOLS_SHADER_STRING_SRCS})
            
TARGET_LINK_LIBRARIES(Application_Tools
                      Core_Utils
                      Core_Application
                      Core_Interface
                      Core_Action
                      Core_State
                      Application_Tool
                      Application_ToolManager
                      ${SCI_BOOST_LIBRARY}
            ${SCI_ITK_LIBRARIES})

# Register tool classes
REGISTER_LIBRARY_AND_CLASSES(Application_Tools
               ${APPLICATION_TOOLS_SRCS}
               ${APPLICATION_TOOLS_ACTIONS_SRCS})
                    