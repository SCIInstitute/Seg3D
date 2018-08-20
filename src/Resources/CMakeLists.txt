
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

SET(RESOURCES_QTRESOURCES_SRCS
  QtResources.cc
)

SET(RESOURCES_QTRESOURCES_MOC_SRCS
)

SET(RESOURCES_QTRESOURCES_NO_MOC_SRCS
  QtResources.h
)

SET(RESOURCES_QTRESOURCES_UI_SRCS
)

SET(RESOURCES_QTRESOURCES_RCC_SRCS
  QtResources.qrc
)

##################################################
# Resource files
##################################################

QT5_ADD_RESOURCES(RESOURCES_QTRESOURCES_QT_RCC_SRCS ${RESOURCES_QTRESOURCES_RCC_SRCS})

##################################################
# Generate header out of UI code
##################################################

QT5_WRAP_UI(RESOURCES_QTRESOURCES_QT_UI_SRCS ${RESOURCES_QTRESOURCES_UI_SRCS})

##################################################
# Wrap QT code to expand all the moc code
##################################################

QT5_WRAP_CPP(RESOURCES_QTRESOURCES_QT_MOC_SRCS ${RESOURCES_QTRESOURCES_MOC_SRCS})

##################################################
# Ensure that we can find the files generated
# by the moc and ui builder
##################################################

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

##################################################
# Build the Components library
##################################################

CORE_ADD_LIBRARY(Resources_QtResources
  ${RESOURCES_QTRESOURCES_SRCS}
  ${RESOURCES_QTRESOURCES_QT_UI_SRCS}
  ${RESOURCES_QTRESOURCES_QT_MOC_SRCS}
  ${RESOURCES_QTRESOURCES_MOC_SRCS}
  ${RESOURCES_QTRESOURCES_NO_MOC_SRCS}
  ${RESOURCES_QTRESOURCES_QT_RCC_SRCS}
)

TARGET_LINK_LIBRARIES(Resources_QtResources
)

target_link_libraries(Resources_QtResources Qt5::Core)

SET(RESOURCES_LIBRARY Resources_QtResources CACHE INTERNAL "Resources library." FORCE)
