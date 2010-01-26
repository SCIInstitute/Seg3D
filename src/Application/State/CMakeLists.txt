
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

SET(APPLICATION_STATE_SRCS
  StateBase.h
  State.h
  StateOption.cc
  StateOption.h
  StateValue.h
  StateVector.h
  StateRangedValue.h
  StateManager.h
  StateManager.cc
  StateHandler.h
  StateHandler.cc
  )

SET(APPLICATION_STATE_ACTIONS_SRCS
  Actions/ActionSet.h
  Actions/ActionSet.cc
  Actions/ActionGet.h
  Actions/ActionGet.cc
  )

ADD_LIBRARY(Application_State STATIC 
            ${APPLICATION_STATE_SRCS}
            ${APPLICATION_STATE_ACTIONS_SRCS} )
            
TARGET_LINK_LIBRARIES(Application_State
                      Utils_Converter
                      Utils_Core
                      Application_Application
                      Application_Interface
                      Application_Action
                      ${SCI_BOOST_LIBRARY})

# Register actions
ADD_LIBRARY_ACTIONS(Application_State
                    ${APPLICATION_STATE_ACTIONS_SRCS})
                    