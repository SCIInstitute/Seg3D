#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
# Set Utils main sources
###########################################

SET(LV_UTILS_SRCS
  CreateLargeVolume
)

SET(UTILS_LIBS 
  ${SCI_BOOST_LIBRARY}
  ${SCI_ZLIB_LIBRARY}
  Core_Utils
  Core_DataBlock
  Core_EventHandler
  Core_Graphics
  Core_Volume
  Core_Application
  Core_Interface
  Core_Action
  Core_Log
  Core_LargeVolume
  Application_Tools
  Application_Filters
  Application_Socket
)


###########################################
# Build the Seg3D utilities
###########################################

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEG3D_BINARY_DIR})

SET(LV_UTILS_TARGETS "" CACHE INTERNAL "Large volume utilities." FORCE)

FOREACH(UTIL ${LV_UTILS_SRCS})
  ADD_DEFINITIONS(-DAPPLICATION_NAME=${UTIL})
  ADD_EXECUTABLE(${UTIL} ${UTIL}.cc)
  TARGET_LINK_LIBRARIES(${UTIL} ${UTILS_LIBS})
  LIST(APPEND LV_UTILS_TARGETS ${UTIL})

  # add libraries with plug-ins and extensions
  REGISTERED_TARGET_LINK_LIBRARIES(${UTIL})

  IF(APPLE)
    INSTALL(TARGETS ${UTIL} RUNTIME DESTINATION ${OSX_UTILS_DIR} BUNDLE DESTINATION ${OSX_UTILS_DIR})
    IF(BUILD_WITH_PYTHON)
      SET(VERSION_PATH Versions/${SCI_PYTHON_VERSION_SHORT})
      SET(PYTHON_PATH .libs/Python.framework/${VERSION_PATH})

      ADD_CUSTOM_COMMAND(TARGET ${UTIL}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PYTHON_PATH}
        # only need python library
        COMMAND ${CMAKE_COMMAND} ARGS -E copy ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python ${PYTHON_PATH}
        COMMAND chmod u+w  ${PYTHON_PATH}/Python
        COMMAND  install_name_tool -id Python ${PYTHON_PATH}/Python
        COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/${PYTHON_PATH}/Python ${UTIL}
       WORKING_DIRECTORY ${SEG3D_BINARY_DIR})

      INSTALL(DIRECTORY ${SEG3D_BINARY_DIR}/.libs
        DESTINATION Seg3DUtils
        PATTERN *
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_READ WORLD_EXECUTE)
    ENDIF()
  ELSEIF(WIN32)
    INSTALL(TARGETS ${UTIL} RUNTIME DESTINATION bin)
  ENDIF()

ENDFOREACH()
