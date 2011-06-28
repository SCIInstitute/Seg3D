
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

SET(APPLICATION_VIEWMANAGER_SRCS
  ViewerManager.h
  ViewerManager.cc
  )

SET(APPLICATION_VIEWMANAGER_ACTIONS_SRCS
  Actions/ActionPickPoint.h
  Actions/ActionPickPoint.cc  
  Actions/ActionNewFeature.h
  Actions/ActionNewFeature.cc
  Actions/ActionDeleteFeature.h
  Actions/ActionDeleteFeature.cc
  )

IF(BUILD_WITH_PYTHON)
  GENERATE_ACTION_PYTHON_WRAPPER(PYTHON_WRAPPER Application_ViewerManager ${APPLICATION_VIEWMANAGER_ACTIONS_SRCS})
  SET(APPLICATION_VIEWMANAGER_SRCS ${APPLICATION_VIEWMANAGER_SRCS} ${PYTHON_WRAPPER})
ENDIF(BUILD_WITH_PYTHON)

CORE_ADD_LIBRARY(Application_ViewerManager ${APPLICATION_VIEWMANAGER_SRCS}
            ${APPLICATION_VIEWMANAGER_ACTIONS_SRCS} )
            
TARGET_LINK_LIBRARIES(Application_ViewerManager
                      Core_Utils
                      Core_Application
                      Core_Interface
                      Core_Action
                      Core_State
                      Application_Viewer
                      ${SCI_BOOST_LIBRARY})

# Register action classes
REGISTER_LIBRARY_AND_CLASSES(Application_ViewerManager
                    ${APPLICATION_VIEWMANAGER_ACTIONS_SRCS})
                    