#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

SET(APPLICATION_IRTOOLS_SRCS
  AssembleFilter.cc
  AssembleFilter.h
  BlobFeatureEnhancementFilter.cc
  BlobFeatureEnhancementFilter.h
  CLAHEFilter.cc
  CLAHEFilter.h
  FFTFilter.cc
  FFTFilter.h
  RefineGridFilter.cc
  RefineGridFilter.h
  RefineTranslateFilter.cc
  RefineTranslateFilter.h
  SliceToSliceBruteFilter.cc
  SliceToSliceBruteFilter.h
  SliceToSliceFilter.cc
  SliceToSliceFilter.h
  SliceToSliceGridFilter.cc
  SliceToSliceGridFilter.h
  SliceToVolumeFilter.cc
  SliceToVolumeFilter.h
)

SET(APPLICATION_IRTOOLS_ACTIONS_SRCS
  Actions/ActionAddTransformsFilter.cc
  Actions/ActionAddTransformsFilter.h
  Actions/ActionAssembleFilter.cc
  Actions/ActionAssembleFilter.h
  Actions/ActionBlobFeatureEnhancementFilter.cc
  Actions/ActionBlobFeatureEnhancementFilter.h
  Actions/ActionCLAHEFilter.cc
  Actions/ActionCLAHEFilter.h
  Actions/ActionFFTFilter.cc
  Actions/ActionFFTFilter.h
  Actions/ActionRefineGridFilter.cc
  Actions/ActionRefineGridFilter.h
  Actions/ActionRefineTranslateFilter.cc
  Actions/ActionRefineTranslateFilter.h
  Actions/ActionSliceToSliceBruteFilter.cc
  Actions/ActionSliceToSliceBruteFilter.h
  Actions/ActionSliceToSliceFilter.cc
  Actions/ActionSliceToSliceFilter.h
  Actions/ActionSliceToSliceGridFilter.cc
  Actions/ActionSliceToSliceGridFilter.h
  Actions/ActionSliceToVolumeFilter.cc
  Actions/ActionSliceToVolumeFilter.h
)

IF(BUILD_WITH_PYTHON)
 GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER Application_ImageRegistrationTools ${APPLICATION_IRTOOLS_ACTIONS_SRCS})
 SET(APPLICATION_IRTOOLS_NOREGISTER_SRCS ${PYTHON_WRAPPER})
ENDIF(BUILD_WITH_PYTHON)

CORE_ADD_LIBRARY(Application_ImageRegistrationTools
  ${APPLICATION_IRTOOLS_NOREGISTER_SRCS}
  ${APPLICATION_IRTOOLS_SRCS}
  ${APPLICATION_IRTOOLS_ACTIONS_SRCS}
)
            
TARGET_LINK_LIBRARIES(Application_ImageRegistrationTools
  Core_Utils
  Core_Application
  Core_Interface
  Core_Action
  Core_State
  Core_Geometry
  Core_ITKCommon
  Application_Filters
  Application_Tool
  Application_ToolManager
  ${SCI_BOOST_LIBRARY}
  ${SCI_ITK_LIBRARIES}
)

# Register tool classes
REGISTER_LIBRARY_AND_CLASSES(Application_ImageRegistrationTools
  ${APPLICATION_IRTOOLS_SRCS}
  ${APPLICATION_IRTOOLS_ACTIONS_SRCS}
)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(
    Actions/Tests
  )
ENDIF(BUILD_TESTING)
