
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

SET(CORE_PYTHON_SRCS
  Util.h
  Util.cc
  PythonInterpreter.h
  PythonInterpreter.cc
  PythonActionContext.h
  PythonActionContext.cc
  ToPythonConverters.h
  ToPythonConverters.cc
  PythonCLI.h
  PythonCLI.cc
)

IF(WIN32)
  ADD_DEFINITIONS(-DPYTHONPATH=L"${SCI_PYTHON_MODULE_PARENT_PATH}"
                  -DPYTHONNAME=L"${SCI_PYTHON_NAME}")
ELSEIF(APPLE)
  IF(BUILD_TESTING)
    ADD_DEFINITIONS(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}:${APPLICATION_NAME}.app/Contents/Frameworks/${PYTHON_MODULE_SEARCH_ROOT}")
  ELSE()
    ADD_DEFINITIONS(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}")
  ENDIF()
  ADD_DEFINITIONS(-DPYTHONLIBDIR=L"${SCI_PYTHON_LIBRARY_DIR}"
                  -DPYTHONLIB=L"${SCI_PYTHON_LIBRARY}")
ELSE()
  ADD_DEFINITIONS(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}")
ENDIF()

CORE_ADD_LIBRARY(Core_Python ${CORE_PYTHON_SRCS})

TARGET_LINK_LIBRARIES(Core_Python
  Core_Utils
  Core_Action
  ${SCI_PYTHON_LIBRARY}
  ${SCI_BOOST_LIBRARY}
)

SET_TARGET_PROPERTIES(Core_Python
  PROPERTIES
  COMPILE_DEFINITIONS_DEBUG "BOOST_DEBUG_PYTHON"
)

ADD_TEST_DIR(Tests)
