
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

SET(INTERFACE_APPPROJECTEXPORTWIZARD_SRCS
  AppProjectExportWizard.cc
)

SET(INTERFACE_APPPROJECTEXPORTWIZARD_MOC_SRCS
  AppProjectExportWizard.h
)

SET(INTERFACE_APPPROJECTEXPORTWIZARD_NO_MOC_SRCS
)

SET(INTERFACE_APPPROJECTEXPORTWIZARD_UI_SRCS
)

SET(INTERFACE_APPPROJECTEXPORTWIZARD_RCC_SRCS
)


##################################################
# Get Icon files
##################################################

QT4_ADD_RESOURCES(INTERFACE_APPPROJECTEXPORTWIZARD_QT_RCC_SRCS ${INTERFACE_APPPROJECTEXPORTWIZARD_RCC_SRCS})

##################################################
# Wrap QT code to expand all the moc code
##################################################

QT4_WRAP_CPP(INTERFACE_APPPROJECTEXPORTWIZARD_QT_MOC_SRCS ${INTERFACE_APPPROJECTEXPORTWIZARD_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY(Interface_AppProjectExportWizard 
            ${INTERFACE_APPPROJECTEXPORTWIZARD_SRCS} 
            ${INTERFACE_APPPROJECTEXPORTWIZARD_QT_RCC_SRCS}
            ${INTERFACE_APPPROJECTEXPORTWIZARD_QT_MOC_SRCS}
            ${INTERFACE_APPPROJECTEXPORTWIZARD_NO_MOC_SRCS} 
            ${INTERFACE_APPPROJECTEXPORTWIZARD_MOC_SRCS})

            
TARGET_LINK_LIBRARIES(Interface_AppProjectExportWizard
                      Interface_AppInterface
                      Core_Utils
                      Core_Application
                      Core_EventHandler
                      Core_Action
                      Core_Interface
            Application_ProjectManager
                      ${QT_LIBRARIES} 
                      ${SCI_BOOST_LIBRARY})

