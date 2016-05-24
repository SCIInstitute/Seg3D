
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

SET(QTUTILS_WIDGETS_SRCS
  QtColorBarWidget.cc
  QtColorButton.cc
  QtCustomDialog.cc
  QtCustomDockWidget.cc
  QtDoubleClickToolButton.cc
  QtDoubleClickPushButton.cc
  QtDropableComboBox.cc
  QtHistogramGraph.cc
  QtHistogramWidget.cc
  QtSignalFocusLineEdit.cc
  QtSliderDoubleCombo.cc
  QtSliderIntCombo.cc
  QtLogSliderIntCombo.cc
  QtTristateToolButton.cc
  SingleShotToolButton.cc
  QtWarningMessageWidget.cc
  QtTransferFunctionControlPoint.cc
  QtTransferFunctionEdge.cc
  QtTransferFunctionCurve.cc
  QtTransferFunctionScene.cc
  QtTransferFunctionView.cc
)

SET(QTUTILS_WIDGETS_MOC_SRCS
  QtColorBarWidget.h
  QtColorButton.h
  QtCustomDialog.h
  QtCustomDockWidget.h
  QtDoubleClickToolButton.h
  QtDoubleClickPushButton.h
  QtDropableComboBox.h
  QtHistogramGraph.h
  QtHistogramWidget.h
  QtSignalFocusLineEdit.h
  QtSliderDoubleCombo.h
  QtSliderIntCombo.h
  QtLogSliderIntCombo.h
  QtTristateToolButton.h
  SingleShotToolButton.h
  QtWarningMessageWidget.h
  QtTransferFunctionControlPoint.h
  QtTransferFunctionEdge.h
  QtTransferFunctionCurve.h
  QtTransferFunctionScene.h
  QtTransferFunctionView.h
)

SET(QTUTILS_WIDGETS_NO_MOC_SRCS
)

SET(QTUTILS_WIDGETS_UI_SRCS
  QtColorBarWidget.ui
  QtHistogramWidget.ui
  QtSliderDoubleCombo.ui
  QtSliderIntCombo.ui
  QtWarningMessageWidget.ui
)

##################################################
# Generate header out of UI code
##################################################

QT5_WRAP_UI(QTUTILS_WIDGETS_QT_UI_SRCS ${QTUTILS_WIDGETS_UI_SRCS})

##################################################
# Wrap QT code to expand all the moc code
##################################################

QT5_WRAP_CPP(QTUTILS_WIDGETS_QT_MOC_SRCS ${QTUTILS_WIDGETS_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY( QtUtils_Widgets
  ${QTUTILS_WIDGETS_SRCS}
  ${QTUTILS_WIDGETS_QT_UI_SRCS}
  ${QTUTILS_WIDGETS_QT_MOC_SRCS}
  ${QTUTILS_WIDGETS_MOC_SRCS}
  ${QTUTILS_WIDGETS_NO_MOC_SRCS}
)

TARGET_LINK_LIBRARIES( QtUtils_Widgets
  Core_Utils
  Core_Geometry
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${SCI_BOOST_LIBRARY}
)
