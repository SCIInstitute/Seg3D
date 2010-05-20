
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

SET(QTINTERFACE_WIDGETS_SRCS
  QtColorBarWidget.cc
  QtColorButton.cc
  QtHistogramGraph.cc
  QtHistogramWidget.cc
  QtSliderDoubleCombo.cc
  QtSliderIntCombo.cc
)

SET(QTINTERFACE_WIDGETS_MOC_SRCS
  QtColorBarWidget.h
  QtColorButton.h
  QtHistogramGraph.h
  QtHistogramWidget.h
  QtSliderDoubleCombo.h
  QtSliderIntCombo.h
)

SET(QTINTERFACE_WIDGETS_NO_MOC_SRCS
)

SET(QTINTERFACE_WIDGETS_UI_SRCS
  QtColorBarWidget.ui
  QtHistogramWidget.ui
  QtSliderDoubleCombo.ui
  QtSliderIntCombo.ui 
)

##################################################
# Generate header out of UI code
##################################################

QT4_WRAP_UI(QTINTERFACE_WIDGETS_QT_UI_SRCS ${QTINTERFACE_WIDGETS_UI_SRCS})

##################################################
# Wrap QT code to expand all the moc code
##################################################

QT4_WRAP_CPP(QTINTERFACE_WIDGETS_QT_MOC_SRCS ${QTINTERFACE_WIDGETS_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY( QtInterface_Widgets
            ${QTINTERFACE_WIDGETS_SRCS} 
            ${QTINTERFACE_WIDGETS_QT_UI_SRCS}
            ${QTINTERFACE_WIDGETS_QT_MOC_SRCS}
            ${QTINTERFACE_WIDGETS_MOC_SRCS}
            ${QTINTERFACE_WIDGETS_NO_MOC_SRCS} )
            
TARGET_LINK_LIBRARIES( QtInterface_Widgets
                       Core_Utils
                       Core_Geometry
                       ${QT_LIBRARIES} 
                       ${SCI_GLEW_LIBRARY}
                       ${SCI_BOOST_LIBRARY} )
