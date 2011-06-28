
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
  detail/MaskShader.frag
  )
  
CORE_IMPORT_SHADER(APPLICATION_TOOLS_SHADER_STRING_SRCS
          ${APPLICATION_TOOLS_SHADER_SRCS})

##################################################
# Set sources
##################################################

# Split into multiple projects to improve parallelization of compilation in Visual Studio.
# Visual Studio doesn't parallelize compilation within projects, but it does parallelize
# across projects.

SET(NUM_PROJECTS 8)

SET(APPLICATION_TOOLS_SRCS_1
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
)
SET(APPLICATION_TOOLS_SRCS_2
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
)
SET(APPLICATION_TOOLS_SRCS_3
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
)
SET(APPLICATION_TOOLS_SRCS_4
  FillHolesFilter.h
  FillHolesFilter.cc
  GradientAnisotropicDiffusionFilter.h
  GradientAnisotropicDiffusionFilter.cc
  GradientMagnitudeFilter.h
  GradientMagnitudeFilter.cc
  HistogramEqualizationFilter.h
  HistogramEqualizationFilter.cc
  IntensityCorrectionFilter.h
  IntensityCorrectionFilter.cc
)
SET(APPLICATION_TOOLS_SRCS_5
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
)
SET(APPLICATION_TOOLS_SRCS_6
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
)
SET(APPLICATION_TOOLS_SRCS_7
  XorFilter.h
  XorFilter.cc
  ResampleTool.h
  ResampleTool.cc
  CropTool.h
  CropTool.cc
  FlipTool.h
  FlipTool.cc
)
SET(APPLICATION_TOOLS_SRCS_8
  TransformTool.h
  TransformTool.cc
  ClipboardTool.h
  ClipboardTool.cc
  PointSetFilter.h
  PointSetFilter.cc
  SpeedlineTool.h
  SpeedlineTool.cc
)

SET(APPLICATION_TOOLS_NOREGISTER_SRCS_1
  detail/MaskShader.h
  detail/MaskShader.cc
)

SET(APPLICATION_TOOLS_NOREGISTER_SRCS_2
)
 
SET(APPLICATION_TOOLS_NOREGISTER_SRCS_3
)

SET(APPLICATION_TOOLS_NOREGISTER_SRCS_4
)

SET(APPLICATION_TOOLS_NOREGISTER_SRCS_5
)

SET(APPLICATION_TOOLS_NOREGISTER_SRCS_6
)

SET(APPLICATION_TOOLS_NOREGISTER_SRCS_7
)

SET(APPLICATION_TOOLS_NOREGISTER_SRCS_8
)
  
SET(APPLICATION_TOOLS_ACTIONS_SRCS_1
  Actions/ActionPaint.h
  Actions/ActionPaint.cc
)
SET(APPLICATION_TOOLS_ACTIONS_SRCS_2
  Actions/ActionPolyline.h
  Actions/ActionPolyline.cc
)
SET(APPLICATION_TOOLS_ACTIONS_SRCS_3
  Actions/ActionCopy.h
  Actions/ActionCopy.cc
)
SET(APPLICATION_TOOLS_ACTIONS_SRCS_4
  Actions/ActionCopyPaste.h
  Actions/ActionCopyPaste.cc
)
SET(APPLICATION_TOOLS_ACTIONS_SRCS_5
  Actions/ActionPaste.h
  Actions/ActionPaste.cc
)
SET(APPLICATION_TOOLS_ACTIONS_SRCS_6
  Actions/ActionFloodFill.h
  Actions/ActionFloodFill.cc
)
SET(APPLICATION_TOOLS_ACTIONS_SRCS_7
  Actions/ActionSpeedline.h
  Actions/ActionSpeedline.cc
)
 
##################################################
# Set link libraries
##################################################
 
SET(APPLICATION_TOOLS_LINK_LIBS
  Core_Utils
  Core_Application
  Core_Interface
  Core_Action
  Core_State
  Core_Geometry
  Application_Clipboard
  ${Application_Filters}
  Application_Tool
  Application_ToolManager
  ${SCI_BOOST_LIBRARY}
  ${SCI_ITK_LIBRARIES}
)
  
##################################################
# Create project(s)
##################################################
            
IF (SEG3D_SPLIT_PROJECTS)
  
  # Split large projects into smaller projects to speed up compilation with Visual Studio.
  # Visual Studio doesn't parallelize compilation within projects, but it does parallelize
  # across projects.  

  # Clear cached value
  SET(Application_Tools "" CACHE INTERNAL "Application_Tools libraries")
  
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    SET(APPLICATION_TOOLS_ACTIONS_SRCS "APPLICATION_TOOLS_ACTIONS_SRCS_${PROJ_IDX}")
    SET(APPLICATION_TOOLS_SRCS "APPLICATION_TOOLS_SRCS_${PROJ_IDX}")
    SET(APPLICATION_TOOLS_NOREGISTER_SRCS "APPLICATION_TOOLS_NOREGISTER_SRCS_${PROJ_IDX}")
    SET(APPLICATION_TOOLS_LIB "Application_Tools_${PROJ_IDX}")  
    
    IF(BUILD_WITH_PYTHON)
      GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER ${APPLICATION_TOOLS_LIB} ${${APPLICATION_TOOLS_ACTIONS_SRCS}})
      SET(${APPLICATION_TOOLS_NOREGISTER_SRCS} ${${APPLICATION_TOOLS_NOREGISTER_SRCS}} ${PYTHON_WRAPPER})
    ENDIF(BUILD_WITH_PYTHON)

    CORE_ADD_LIBRARY(${APPLICATION_TOOLS_LIB}
             ${${APPLICATION_TOOLS_SRCS}}
             ${${APPLICATION_TOOLS_NOREGISTER_SRCS}}
             ${${APPLICATION_TOOLS_ACTIONS_SRCS}}
             ${APPLICATION_TOOLS_SHADER_SRCS}
             ${APPLICATION_TOOLS_SHADER_STRING_SRCS})
                
    TARGET_LINK_LIBRARIES(${APPLICATION_TOOLS_LIB}
      ${APPLICATION_TOOLS_LINK_LIBS})

    # Register tool classes
    REGISTER_LIBRARY_AND_CLASSES(${APPLICATION_TOOLS_LIB}
                   ${${APPLICATION_TOOLS_SRCS}}
                   ${${APPLICATION_TOOLS_ACTIONS_SRCS}})
                  
      
    # Create single CMake variable that other libraries can depend on
    # Tack library onto end of list     
    # Note: Use INTERNAL so that the variable is put in the cache (global scope), but not
    # exposed to the user in the CMake interface.   
    SET(Application_Tools ${Application_Tools} ${APPLICATION_TOOLS_LIB} CACHE INTERNAL "Application_Tools libraries")
                 
  ENDFOREACH (PROJ_IDX)
    
ELSE (SEG3D_SPLIT_PROJECTS)

  # Consolidate sources back into single project
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    SET(APPLICATION_TOOLS_ACTIONS_PROJ_SRCS "APPLICATION_TOOLS_ACTIONS_SRCS_${PROJ_IDX}") 
    SET(APPLICATION_TOOLS_ACTIONS_SRCS "${APPLICATION_TOOLS_ACTIONS_SRCS};${${APPLICATION_TOOLS_ACTIONS_PROJ_SRCS}}")
      
    SET(APPLICATION_TOOLS_PROJ_SRCS "APPLICATION_TOOLS_SRCS_${PROJ_IDX}")   
    SET(APPLICATION_TOOLS_SRCS "${APPLICATION_TOOLS_SRCS};${${APPLICATION_TOOLS_PROJ_SRCS}}")
    
    SET(APPLICATION_TOOLS_NOREGISTER_PROJ_SRCS "APPLICATION_TOOLS_NOREGISTER_SRCS_${PROJ_IDX}")
    SET(APPLICATION_TOOLS_NOREGISTER_SRCS "${APPLICATION_TOOLS_NOREGISTER_SRCS};${${APPLICATION_TOOLS_NOREGISTER_PROJ_SRCS}}")
    
  ENDFOREACH (PROJ_IDX)
  
  IF(BUILD_WITH_PYTHON)
    GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER Application_Tools ${APPLICATION_TOOLS_ACTIONS_SRCS})
    SET(APPLICATION_TOOLS_NOREGISTER_SRCS ${APPLICATION_TOOLS_NOREGISTER_SRCS} ${PYTHON_WRAPPER})
  ENDIF(BUILD_WITH_PYTHON)

  # Note: Use INTERNAL so that the variable is put in the cache (global scope), but not
  # exposed to the user in the CMake interface. 
  SET(Application_Tools "Application_Tools" CACHE INTERNAL "Application_Tools library")   
  CORE_ADD_LIBRARY(${Application_Tools} 
           ${APPLICATION_TOOLS_SRCS}
           ${APPLICATION_TOOLS_NOREGISTER_SRCS}
           ${APPLICATION_TOOLS_ACTIONS_SRCS}
           ${APPLICATION_TOOLS_SHADER_SRCS}
           ${APPLICATION_TOOLS_SHADER_STRING_SRCS})
              
  TARGET_LINK_LIBRARIES(${Application_Tools}
    ${APPLICATION_TOOLS_LINK_LIBS})

  # Register tool classes
  REGISTER_LIBRARY_AND_CLASSES(${Application_Tools}
                 ${APPLICATION_TOOLS_SRCS}
                 ${APPLICATION_TOOLS_ACTIONS_SRCS})
               
ENDIF (SEG3D_SPLIT_PROJECTS)

                    