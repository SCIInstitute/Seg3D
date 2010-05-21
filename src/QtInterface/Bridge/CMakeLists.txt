
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

SET(QTINTERFACE_BRIDGE_SRCS
  QtBridge.cc
  detail/QtComboBoxConnector.cc
  detail/QtCheckBoxConnector.cc
  detail/QtSpinBoxConnector.cc
  detail/QtLineEditConnector.cc
  detail/QtSliderIntComboConnector.cc
  detail/QtSliderDoubleComboConnector.cc
  detail/QtAbstractButtonConnector.cc
  detail/QtActionConnector.cc
  detail/QtActionGroupConnector.cc
  detail/QtColorButtonConnector.cc
  detail/QtColorBarWidgetConnector.cc
)

SET(QTINTERFACE_BRIDGE_MOC_SRCS
  detail/QtConnectorBase.h
  detail/QtComboBoxConnector.h
  detail/QtCheckBoxConnector.h
  detail/QtSpinBoxConnector.h
  detail/QtLineEditConnector.h
  detail/QtSliderIntComboConnector.h
  detail/QtSliderDoubleComboConnector.h
  detail/QtAbstractButtonConnector.h
  detail/QtActionConnector.h
  detail/QtActionGroupConnector.h
  detail/QtColorButtonConnector.h
  detail/QtColorBarWidgetConnector.h
)

SET(QTINTERFACE_BRIDGE_NO_MOC_SRCS
  QtBridge.h
)


##################################################
# Wrap QT code to expand all the moc code
##################################################

QT4_WRAP_CPP(QTINTERFACE_BRIDGE_QT_MOC_SRCS ${QTINTERFACE_BRIDGE_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY(QtInterface_Bridge
         ${QTINTERFACE_BRIDGE_SRCS} 
         ${QTINTERFACE_BRIDGE_QT_MOC_SRCS}
         ${QTINTERFACE_BRIDGE_MOC_SRCS}
         ${QTINTERFACE_BRIDGE_NO_MOC_SRCS} )
            
TARGET_LINK_LIBRARIES(QtInterface_Bridge 
                      Core_Utils
                      Core_EventHandler
            Core_Graphics
            Core_RendererBase
            Core_Viewer
            QtInterface_Widgets         
                      ${QT_LIBRARIES} 
                      ${SCI_GLEW_LIBRARY}
                      ${SCI_BOOST_LIBRARY})
