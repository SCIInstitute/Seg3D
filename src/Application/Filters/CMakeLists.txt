
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

SET(NUM_PROJECTS 8)

SET(APPLICATION_FILTERS_SRCS_1
  LayerFilter.h
  LayerFilter.cc
)
SET(APPLICATION_FILTERS_SRCS_2
  LayerFilterLock.h
  LayerFilterLock.cc
)
SET(APPLICATION_FILTERS_SRCS_3
  ITKFilter.h
  ITKFilter.cc
)
SET(APPLICATION_FILTERS_SRCS_4
  NrrdFilter.h
  NrrdFilter.cc
)
SET(APPLICATION_FILTERS_SRCS_5
  LayerResampler.h
  LayerResampler.cc
)

SET(APPLICATION_FILTERS_ACTIONS_SRCS_1
  Actions/ActionResample.h
  Actions/ActionResample.cc
  Actions/ActionCannyEdgeDetectionFilter.h
  Actions/ActionCannyEdgeDetectionFilter.cc
  Actions/ActionConfidenceConnectedFilter.h
  Actions/ActionConfidenceConnectedFilter.cc
  Actions/ActionConnectedComponentFilter.h
  Actions/ActionConnectedComponentFilter.cc
  Actions/ActionConnectedComponentSizeFilter.h
  Actions/ActionConnectedComponentSizeFilter.cc
)
SET(APPLICATION_FILTERS_ACTIONS_SRCS_2
  Actions/ActionCurvatureAnisotropicDiffusionFilter.h
  Actions/ActionCurvatureAnisotropicDiffusionFilter.cc
  Actions/ActionDilateFilter.h
  Actions/ActionDilateFilter.cc
  Actions/ActionIterativeDilateFilter.h
  Actions/ActionIterativeDilateFilter.cc
  Actions/ActionSmoothDilateFilter.h
  Actions/ActionSmoothDilateFilter.cc
  Actions/ActionDilateErodeFilter.h
  Actions/ActionDilateErodeFilter.cc
)
SET(APPLICATION_FILTERS_ACTIONS_SRCS_3
  Actions/ActionIterativeDilateErodeFilter.h
  Actions/ActionIterativeDilateErodeFilter.cc
  Actions/ActionSmoothDilateErodeFilter.h
  Actions/ActionSmoothDilateErodeFilter.cc
  Actions/ActionDiscreteGaussianFilter.h
  Actions/ActionDiscreteGaussianFilter.cc
  Actions/ActionDistanceFilter.h
  Actions/ActionDistanceFilter.cc
  Actions/ActionErodeFilter.h
  Actions/ActionErodeFilter.cc
)
SET(APPLICATION_FILTERS_ACTIONS_SRCS_4
  Actions/ActionIterativeErodeFilter.h
  Actions/ActionIterativeErodeFilter.cc
  Actions/ActionSmoothErodeFilter.h
  Actions/ActionSmoothErodeFilter.cc
  Actions/ActionGradientAnisotropicDiffusionFilter.h
  Actions/ActionGradientAnisotropicDiffusionFilter.cc
  Actions/ActionGradientMagnitudeFilter.h
  Actions/ActionGradientMagnitudeFilter.cc
  Actions/ActionFillHolesFilter.h
  Actions/ActionFillHolesFilter.cc
)
SET(APPLICATION_FILTERS_ACTIONS_SRCS_5
  Actions/ActionHistogramEqualizationFilter.h
  Actions/ActionHistogramEqualizationFilter.cc
  Actions/ActionIntensityCorrectionFilter.h
  Actions/ActionIntensityCorrectionFilter.cc
  Actions/ActionMaskDataFilter.h
  Actions/ActionMaskDataFilter.cc
  Actions/ActionMedianFilter.h
  Actions/ActionMedianFilter.cc
  Actions/ActionMeanFilter.h
  Actions/ActionMeanFilter.cc
)
SET(APPLICATION_FILTERS_ACTIONS_SRCS_6
  Actions/ActionNeighborhoodConnectedFilter.h
  Actions/ActionNeighborhoodConnectedFilter.cc
  Actions/ActionOtsuThresholdFilter.h
  Actions/ActionOtsuThresholdFilter.cc
  Actions/ActionAndFilter.h
  Actions/ActionAndFilter.cc
  Actions/ActionRemoveFilter.h
  Actions/ActionRemoveFilter.cc
  Actions/ActionOrFilter.h
  Actions/ActionOrFilter.cc
)
SET(APPLICATION_FILTERS_ACTIONS_SRCS_7
  Actions/ActionXorFilter.h
  Actions/ActionXorFilter.cc
  Actions/ActionThresholdSegmentationLSFilter.h
  Actions/ActionThresholdSegmentationLSFilter.cc
  Actions/ActionThreshold.h
  Actions/ActionThreshold.cc
  Actions/ActionInvert.h
  Actions/ActionInvert.cc
  Actions/ActionCrop.h
  Actions/ActionCrop.cc
)
SET(APPLICATION_FILTERS_ACTIONS_SRCS_8
  Actions/ActionArithmeticFilter.h
  Actions/ActionArithmeticFilter.cc
  Actions/ActionPermute.h
  Actions/ActionPermute.cc
  Actions/ActionTransform.h
  Actions/ActionTransform.cc
  Actions/ActionPointSetRegisterFilter.h
  Actions/ActionPointSetRegisterFilter.cc
  Actions/ActionPointSetTransformFilter.h
  Actions/ActionPointSetTransformFilter.cc
  Actions/ActionSpeedlineImageFilter.h
  Actions/ActionSpeedlineImageFilter.cc
  )

##################################################
# Set link libraries
##################################################

SET(APPLICATION_FILTERS_LINK_LIBS
  Core_Utils
  Core_Action
  Core_State
  Core_Parser
  Application_Layer
  Application_LayerManager
  Application_Project
  Application_ProjectManager
  ${SCI_BOOST_LIBRARY}
)

##################################################
# Create projects
##################################################
  
IF (SEG3D_SPLIT_PROJECTS)
  
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    SET(APPLICATION_FILTERS_ACTIONS_SRCS "APPLICATION_FILTERS_ACTIONS_SRCS_${PROJ_IDX}")
    SET(APPLICATION_FILTERS_SRCS "APPLICATION_FILTERS_SRCS_${PROJ_IDX}")
    
    IF(BUILD_WITH_PYTHON)
      GENERATE_ACTION_PYTHON_WRAPPER(${${APPLICATION_FILTERS_ACTIONS_SRCS}})
      SET(APPLICATION_FILTERS_SRCS ${${APPLICATION_FILTERS_SRCS}} ${CMAKE_CURRENT_BINARY_DIR}/ActionPythonWrapper.cc)
    ENDIF(BUILD_WITH_PYTHON)
    
    SET(APPLICATION_FILTERS_LIB "Application_Filters_${PROJ_IDX}")  
    CORE_ADD_LIBRARY(${APPLICATION_FILTERS_LIB} ${${APPLICATION_FILTERS_SRCS}}
          ${${APPLICATION_FILTERS_ACTIONS_SRCS}})
                
    TARGET_LINK_LIBRARIES(${APPLICATION_FILTERS_LIB}
      ${APPLICATION_FILTERS_LINK_LIBS})

    # Register action classes
    REGISTER_LIBRARY_AND_CLASSES(${APPLICATION_FILTERS_LIB}
      ${${APPLICATION_FILTERS_ACTIONS_SRCS}})
      
    # Create single CMake variable that other libraries can depend on
    # Tack library onto end of list     
    SET(Application_Filters "${Application_Filters};${APPLICATION_FILTERS_LIB}")
                 
  ENDFOREACH (PROJ_IDX)

ELSE (SEG3D_SPLIT_PROJECTS)

  # Consolidate sources back into single project
  FOREACH (PROJ_IDX RANGE 1 ${NUM_PROJECTS})
  
    SET(APPLICATION_FILTERS_ACTIONS_PROJ_SRCS "APPLICATION_FILTERS_ACTIONS_SRCS_${PROJ_IDX}") 
    SET(APPLICATION_FILTERS_ACTIONS_SRCS "${APPLICATION_FILTERS_ACTIONS_SRCS};${${APPLICATION_FILTERS_ACTIONS_PROJ_SRCS}}")
      
    SET(APPLICATION_FILTERS_PROJ_SRCS "APPLICATION_FILTERS_SRCS_${PROJ_IDX}")   
    SET(APPLICATION_FILTERS_SRCS "${APPLICATION_FILTERS_SRCS};${${APPLICATION_FILTERS_PROJ_SRCS}}")   
    
  ENDFOREACH (PROJ_IDX)
  
  IF(BUILD_WITH_PYTHON)
    GENERATE_ACTION_PYTHON_WRAPPER(${APPLICATION_FILTERS_ACTIONS_SRCS})
    SET(APPLICATION_FILTERS_SRCS ${APPLICATION_FILTERS_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/ActionPythonWrapper.cc)
  ENDIF(BUILD_WITH_PYTHON)
    
  CORE_ADD_LIBRARY(Application_Filters ${APPLICATION_FILTERS_SRCS}
    ${APPLICATION_FILTERS_ACTIONS_SRCS} )
              
  TARGET_LINK_LIBRARIES(Application_Filters
    ${APPLICATION_FILTERS_LINK_LIBS})

  # Register action classes
  REGISTER_LIBRARY_AND_CLASSES(Application_Filters
    ${APPLICATION_FILTERS_ACTIONS_SRCS})
                 
ENDIF (SEG3D_SPLIT_PROJECTS)