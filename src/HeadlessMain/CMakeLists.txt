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
ENDIF()

ADD_EXECUTABLE(${APPLICATION_NAME} ${MAIN_SRCS})

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
  ${SCI_ZLIB_LIBRARY}
  ${SCI_PNG_LIBRARY}
  ${SCI_TEEM_LIBRARY}
  ${ImplicitFunction_LIBRARY}
)

IF(WIN32)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} )
  SET_TARGET_PROPERTIES(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "/INCREMENTAL:NO")
ENDIF(WIN32)

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} Core_Python)
ENDIF()

# add libraries with plug-ins and extensions
REGISTERED_TARGET_LINK_LIBRARIES(${APPLICATION_NAME})

IF(APPLE)
  IF(BUILD_WITH_PYTHON)
    SET(VERSION_PATH Versions/${SCI_PYTHON_VERSION_SHORT})
    SET(PYTHON_PATH Python.framework/${VERSION_PATH})
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      # macdeployqt puts frameworks in app/Content/Frameworks
      COMMAND ${CMAKE_COMMAND} ARGS -E tar xvf ${SCI_PYTHON_FRAMEWORK_ARCHIVE}
      COMMAND chmod u+w  ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -id Python ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/${PYTHON_PATH}/Python ${APPLICATION_NAME}
      # TODO: other executables...
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../Python ${PYTHON_PATH}/bin/python${SCI_PYTHON_VERSION_SHORT}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  ENDIF()
ELSEIF(WIN32)
  IF(BUILD_WITH_PYTHON)
    # TODO: change $<CONFIGURATION> to $<CONFIG> when upgrading to cmake 3
	# TODO: copy python dll
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory $<CONFIGURATION>/${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} $<CONFIGURATION>/${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  ENDIF()
ELSE()
  IF(BUILD_WITH_PYTHON)
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    IF(SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH AND EXISTS ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH})
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND echo "${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}"
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory  ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    ENDIF()
#    INSTALL(PROGRAMS
#      ${SCI_PYTHON_DLL_PATH}
#      DESTINATION bin
#    )
  ENDIF()

#  INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION bin)
#  INSTALL(FILES ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt DESTINATION bin)
ENDIF()
