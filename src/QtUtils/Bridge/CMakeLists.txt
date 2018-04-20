
#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
# 
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

SET(QTUTILS_BRIDGE_SRCS
  QtBridge.cc
  QtConnection.cc
  detail/QtGroupBoxConnector.cc
  detail/QtComboBoxConnector.cc
  detail/QtSpinBoxConnector.cc
  detail/QtLineEditConnector.cc
  detail/QtSliderIntComboConnector.cc
  detail/QtLogSliderIntComboConnector.cc
  detail/QtSliderDoubleComboConnector.cc
  detail/QtAbstractButtonConnector.cc
  detail/QtAbstractButtonVectorConnector.cc
  detail/QtActionConnector.cc
  detail/QtActionVectorConnector.cc
  detail/QtActionGroupConnector.cc
  detail/QtColorButtonConnector.cc
  detail/QtColorBarWidgetConnector.cc
  detail/QtEnableConnector.cc
  detail/QtButtonGroupConnector.cc
  detail/QtListWidgetConnector.cc
  detail/QtLabelConnector.cc
  detail/QtShowConnector.cc
  detail/QtPlainTextEditConnector.cc
  detail/QtTristateToolButtonConnector.cc
  detail/QtTransferFunctionCurveConnector.cc
  detail/QtTransferFunctionSceneConnector.cc
  detail/QtTextMatrixConnector.cc
)

SET(QTUTILS_BRIDGE_MOC_SRCS
  detail/QtConnectorBase.h
  detail/QtGroupBoxConnector.h
  detail/QtComboBoxConnector.h
  detail/QtSpinBoxConnector.h
  detail/QtLineEditConnector.h
  detail/QtSliderIntComboConnector.h
  detail/QtLogSliderIntComboConnector.h
  detail/QtSliderDoubleComboConnector.h
  detail/QtAbstractButtonConnector.h
  detail/QtAbstractButtonVectorConnector.h
  detail/QtActionConnector.h
  detail/QtActionVectorConnector.h
  detail/QtActionGroupConnector.h
  detail/QtColorButtonConnector.h
  detail/QtColorBarWidgetConnector.h
  detail/QtEnableConnector.h
  detail/QtButtonGroupConnector.h
  detail/QtListWidgetConnector.h
  detail/QtLabelConnector.h
  detail/QtShowConnector.h
  detail/QtPlainTextEditConnector.h
  detail/QtTristateToolButtonConnector.h
  detail/QtTransferFunctionCurveConnector.h
  detail/QtTransferFunctionSceneConnector.h
  detail/QtTextMatrixConnector.h
)

SET(QTUTILS_BRIDGE_NO_MOC_SRCS
  QtBridge.h
  QtConnection.h
)


##################################################
# Wrap QT code to expand all the moc code
##################################################

QT5_WRAP_CPP(QTUTILS_BRIDGE_QT_MOC_SRCS ${QTUTILS_BRIDGE_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY(QtUtils_Bridge
  ${QTUTILS_BRIDGE_SRCS}
  ${QTUTILS_BRIDGE_QT_MOC_SRCS}
  ${QTUTILS_BRIDGE_MOC_SRCS}
  ${QTUTILS_BRIDGE_NO_MOC_SRCS}
)

TARGET_LINK_LIBRARIES(QtUtils_Bridge
  Core_Utils
  Core_Interface
  QtUtils_Widgets
#  ${QT_LIBRARIES}
  ${SCI_BOOST_LIBRARY}
)
