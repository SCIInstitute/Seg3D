
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

SET(INTERFACE_APPCONTROLLER_SRCS
  AppController.cc
  AppControllerContext.cc
  AppControllerActionHistory.cc
  AppControllerStateEngine.cc
  AppControllerLogHistory.cc
)

SET(INTERFACE_APPCONTROLLER_MOC_SRCS
  AppController.h
  AppControllerContext.h
  AppControllerActionHistory.h
  AppControllerStateEngine.h
  AppControllerLogHistory.h
)

SET(INTERFACE_APPCONTROLLER_NO_MOC_SRCS
)

SET(INTERFACE_APPCONTROLLER_UI_SRCS
  AppController.ui
)

SET(INTERFACE_APPCONTROLLER_RCC_SRCS
#  AppController.qrc
)


##################################################
# Get Icon files
##################################################

QT4_ADD_RESOURCES(INTERFACE_APPCONTROLLER_QT_RCC_SRCS ${INTERFACE_APPCONTROLLER_RCC_SRCS})

##################################################
# Generate header out of UI code
##################################################

QT4_WRAP_UI(INTERFACE_APPCONTROLLER_QT_UI_SRCS ${INTERFACE_APPCONTROLLER_UI_SRCS})

##################################################
# Wrap QT code to expand all the moc code
##################################################

QT4_WRAP_CPP(INTERFACE_APPCONTROLLER_QT_MOC_SRCS ${INTERFACE_APPCONTROLLER_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

ADD_LIBRARY(Interface_AppController STATIC 
            ${INTERFACE_APPCONTROLLER_SRCS} 
            ${INTERFACE_APPCONTROLLER_QT_UI_SRCS}
            ${INTERFACE_APPCONTROLLER_QT_RCC_SRCS}
            ${INTERFACE_APPCONTROLLER_QT_MOC_SRCS}
            ${INTERFACE_APPCONTROLLER_NO_MOC_SRCS} 
            ${INTERFACE_APPCONTROLLER_MOC_SRCS})

            
TARGET_LINK_LIBRARIES(Interface_AppController
                      Core_Utils
                      Core_EventHandler
                      Core_Action
                      Core_Interface
                      Core_ActionManager
                      Application_Tool
                      Application_Tools
                      Interface_QtInterface
                      ${QT_LIBRARIES} 
                      ${SCI_BOOST_LIBRARY})

