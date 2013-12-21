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
  AssembleFilter.h
  AssembleFilter.cc
  FFTFilter.h
  FFTFilter.cc
  RefineTranslateFilter.h
  RefineTranslateFilter.cc
)

SET(APPLICATION_IRTOOLS_ACTIONS_SRCS
  Actions/ActionAssembleFilter.h
  Actions/ActionAssembleFilter.cc
  Actions/ActionFFTFilter.h
  Actions/ActionFFTFilter.cc
  Actions/ActionRefineTranslateFilter.h
  Actions/ActionRefineTranslateFilter.cc
)

#SET(APPLICATION_IRTOOLS_UTILS_SRCS
#  Utils/MosaicIO.h
#  Utils/MosaicIO.cc
#  Utils/MosaicLayout.h
#  Utils/MosaicLayout.cc
#  Utils/LocalMapping.h
#  Utils/LocalMapping.cc
#)

# code that needs to be reworked and/or replaced
#SET(APPLICATION_IRTOOLS_UTILS_TMP_SRCS
#  Utils/tmp/the_text.hxx
#  Utils/tmp/the_text.cxx
#)

  
IF(BUILD_WITH_PYTHON)
 GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER Application_ImageRegistrationTools ${APPLICATION_IRTOOLS_ACTIONS_SRCS})
 SET(APPLICATION_IRTOOLS_NOREGISTER_SRCS ${PYTHON_WRAPPER})
ENDIF(BUILD_WITH_PYTHON)

CORE_ADD_LIBRARY(Application_ImageRegistrationTools
  ${APPLICATION_IRTOOLS_NOREGISTER_SRCS}
  ${APPLICATION_IRTOOLS_SRCS}
  ${APPLICATION_IRTOOLS_ACTIONS_SRCS}
#  ${APPLICATION_IRTOOLS_UTILS_SRCS}
#  ${APPLICATION_IRTOOLS_UTILS_TMP_SRCS}
)
            
TARGET_LINK_LIBRARIES(Application_ImageRegistrationTools
  Core_Utils
  Core_Application
  Core_Interface
  Core_Action
  Core_State
  Core_Geometry
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
    #Tests
    Actions/Tests
  )
ENDIF(BUILD_TESTING)
