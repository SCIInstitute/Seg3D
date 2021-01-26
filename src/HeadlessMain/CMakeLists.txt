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


###########################################
# Set Seg3D main sources
###########################################

set(MAIN_SRCS
  main.cc
)

###########################################
# Build the Seg3D executable
###########################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEG3D_BINARY_DIR})
add_definitions(-DAPPLICATION_NAME=${APPLICATION_NAME})

if(WIN32)
  set(MAIN_SRCS ${MAIN_SRCS} "${APPLICATION_WINDOWS_RC}")
endif()

add_executable(${APPLICATION_NAME} ${MAIN_SRCS})

target_link_libraries(${APPLICATION_NAME}
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
  ${SCI_ZLIB_LIBRARY}
  ${SCI_PNG_LIBRARY}
  ${SCI_TEEM_LIBRARY}
  ${ImplicitFunction_LIBRARY}
)

if(WIN32)
  target_link_libraries(${APPLICATION_NAME} )
  set_target_properties(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "/INCREMENTAL:NO")
endif(WIN32)

if(APPLE)
  set_target_properties(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "-framework CoreGraphics")
endif()

if(BUILD_WITH_PYTHON)
  target_link_libraries(${APPLICATION_NAME}
    Core_Python
    Application_Socket
  )
endif()

# add libraries with plug-ins and extensions
REGISTERED_TARGET_LINK_LIBRARIES(${APPLICATION_NAME})

if(APPLE)
  if(BUILD_WITH_PYTHON)
    set(VERSION_PATH Versions/${SCI_PYTHON_VERSION_SHORT})
    set(PYTHON_PATH Python.framework/${VERSION_PATH})
    add_custom_command(TARGET ${APPLICATION_NAME}
      POST_BUILD
      # macdeployqt puts frameworks in app/Content/Frameworks
      COMMAND ${CMAKE_COMMAND} ARGS -E tar xf ${SCI_PYTHON_FRAMEWORK_ARCHIVE}
      COMMAND chmod u+w  ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -id Python ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/${PYTHON_PATH}/Python ${APPLICATION_NAME}
      # TODO: other executables...
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../Python ${PYTHON_PATH}/bin/python${SCI_PYTHON_VERSION_SHORT}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  endif()
elseif(WIN32)
  if(BUILD_WITH_PYTHON)
    # TODO: change $<CONFIGURATION> to $<CONFIG> when upgrading to cmake 3
	# TODO: copy python dll
    add_custom_command(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory $<CONFIGURATION>/${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} $<CONFIGURATION>/${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  endif()
else()
  if(BUILD_WITH_PYTHON)
    add_custom_command(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    if(SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH AND EXISTS ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH})
      add_custom_command(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND echo "${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}"
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory  ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    endif()
#    install(PROGRAMS
#      ${SCI_PYTHON_DLL_PATH}
#      DESTINATION bin
#    )
  endif()

#  install(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION bin)
#  install(FILES ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt DESTINATION bin)
endif()
