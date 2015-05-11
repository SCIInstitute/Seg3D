#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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


###########################################
# Set Seg3D main sources
###########################################

SET(MAIN_SRCS
  main.cc
)

###########################################
# Build the Seg3D executable
###########################################

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEG3D_BINARY_DIR})
ADD_DEFINITIONS(-DAPPLICATION_NAME=${APPLICATION_NAME})

IF(WIN32)
  SET(MAIN_SRCS ${MAIN_SRCS} "${APPLICATION_WINDOWS_RC}")
  IF (SEG3D_SHOW_CONSOLE)
    ADD_EXECUTABLE(${APPLICATION_NAME} ${MAIN_SRCS})
  ELSE (SEG3D_SHOW_CONSOLE)
    ADD_EXECUTABLE(${APPLICATION_NAME} WIN32 ${MAIN_SRCS})
  ENDIF (SEG3D_SHOW_CONSOLE)
ELSE(WIN32)
  ADD_EXECUTABLE(${APPLICATION_NAME} ${MAIN_SRCS})
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(${APPLICATION_NAME} 
  ${SCI_BOOST_LIBRARY}
  Core_Utils
  Core_DataBlock
  Core_EventHandler
  Core_Graphics
  Core_Volume
  Core_VolumeRenderer
  Core_Application
  Core_Interface
  Core_Action
  Core_Log
  Application_Tools
  Application_Filters
  Application_Socket
)

IF(WIN32)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} )
  SET_TARGET_PROPERTIES(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "/INCREMENTAL:NO")
ENDIF(WIN32)

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} Core_Python)
ENDIF(BUILD_WITH_PYTHON)

# add libraries with plug-ins and extensions
REGISTERED_TARGET_LINK_LIBRARIES(${APPLICATION_NAME})

IF(WIN32)
  INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION bin)
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt DESTINATION bin)

  IF(BUILD_WITH_PYTHON)
    INSTALL(FILES ${PYTHON_SOURCE_DIR}/pythonlib.zip DESTINATION bin/lib)
  ENDIF(BUILD_WITH_PYTHON)

ELSE(WIN32)
  INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION bin)
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt DESTINATION bin)
    
ENDIF(WIN32)
