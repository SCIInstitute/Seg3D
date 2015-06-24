
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

SET(CORE_ACTION_SRCS
  Action.h
  Action.cc
  ActionContext.h
  ActionContext.cc
  ActionContextContainer.h
  ActionContextContainer.cc
  ActionParameter.h
  ActionParameter.cc
  ActionResult.h
  ActionProgress.h
  ActionProgress.cc
  ActionDispatcher.h
  ActionDispatcher.cc
  ActionFactory.h
  ActionFactory.cc
  ActionHistory.h
  ActionHistory.cc
  ActionInfo.h
  ActionInfo.cc
)

CORE_ADD_LIBRARY(Core_Action ${CORE_ACTION_SRCS} )
            
TARGET_LINK_LIBRARIES(Core_Action
  Core_Utils
  Core_EventHandler
  Core_Application
  ${SCI_BOOST_LIBRARY})
