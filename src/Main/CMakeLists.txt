#
#  For more information, please see: http://software.sci.utah.edu
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


############################################
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

IF(APPLE)
  SET(MAIN_SRCS ${MAIN_SRCS} "${APPLICATION_OSX_ICON_FULL_PATH}")
  SET_SOURCE_FILES_PROPERTIES(${APPLICATION_OSX_ICON_FULL_PATH}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
  SET(MACOSX_BUNDLE_ICON_FILE "${APPLICATION_OSX_ICON}" )
  ADD_EXECUTABLE(${APPLICATION_NAME} MACOSX_BUNDLE ${MAIN_SRCS})
ELSE()
  IF(WIN32)
    SET(MAIN_SRCS ${MAIN_SRCS} "${APPLICATION_WINDOWS_RC}")
    IF(SEG3D_SHOW_CONSOLE)
      ADD_EXECUTABLE(${APPLICATION_NAME} ${MAIN_SRCS})
    ELSE()
      ADD_EXECUTABLE(${APPLICATION_NAME} WIN32 ${MAIN_SRCS})
    ENDIF()
  ELSE()
    ADD_EXECUTABLE(${APPLICATION_NAME} ${MAIN_SRCS})
  ENDIF()
ENDIF()

TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
  ${QT_LIBRARIES}
  ${SCI_BOOST_LIBRARY}
  Core_Utils
  Core_DataBlock
  Core_EventHandler
  Core_Graphics
  Core_Volume
  Core_Application
  Core_Interface
  Core_Action
  Core_Log
  Application_Tools
  Application_Filters
  Application_Socket
  QtUtils_Utils
  QtUtils_Widgets
  QtUtils_Bridge
  Interface_Application
  Interface_ToolInterface
  ${SCI_ZLIB_LIBRARY}
  ${SCI_PNG_LIBRARY}
  ${SCI_TEEM_LIBRARY}
  ${ImplicitFunction_LIBRARY}
  ${RESOURCES_LIBRARY}
)

IF(WIN32)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} ${QT_QTMAIN_LIBRARY} )
  SET_TARGET_PROPERTIES(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "/INCREMENTAL:NO")
ENDIF()

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} Core_Python)
ENDIF()

# add libraries with plug-ins and extensions
REGISTERED_TARGET_LINK_LIBRARIES(${APPLICATION_NAME})

###########################################
# SETUP MACOSX APPLICATION BUNDLE
# There is no good way for incorporating
# external libraries into a bundle, hence
# it is hard coded here what needs to be
# done to generate an application that
# does not depend on anything except the
# operating system
###########################################

IF(APPLE)
  IF(${CMAKE_GENERATOR} MATCHES "Xcode")

    #ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
    #   POST_BUILD
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || cp -rf "${QT_QTCORE_LIBRARY_RELEASE}/Versions/4/QtCore" "${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/"
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || cp -rf "${QT_QTGUI_LIBRARY_RELEASE}/Versions/4/QtGui" "${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/"
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || cp -rf "${QT_QTOPENGL_LIBRARY_RELEASE}/Versions/4/QtOpenGL" "${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/"
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -id QtCore ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/QtCore
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -id QtGui ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/QtGui
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -id QtOpenGL ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/QtOpenGL
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/QtGui
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/QtOpenGL
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/QtOpenGL
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || install_name_tool -change QtOpenGL.framework/Versions/4/QtOpenGL @executable_path/QtOpenGL ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || cp "-rf" "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources" "${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents"
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || [ -d ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/plugins ] || mkdir ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/plugins
    #   COMMAND [ ! -d ${CMAKE_CFG_INTDIR} ] || cp -rf "${CMAKE_SOURCE_DIR}/CMake/qt.conf" ${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/Resources
    #   WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    #IF(BUILD_WITH_PYTHON)
    #  ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
    #    POST_BUILD
    #    COMMAND [ ! -d "${CMAKE_CFG_INTDIR}/lib" ] || cp -rf "${CMAKE_CFG_INTDIR}/lib" "${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents/MacOS/"
    #    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    #ENDIF()
  ELSE()
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
       POST_BUILD
       # configure Qt libs
      #COMMAND echo "${MACDEPLOYQT} ${APPLICATION_NAME}.app -verbose=3"
       COMMAND ${MACDEPLOYQT} ${APPLICATION_NAME}.app -verbose=3
       WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    IF(BUILD_WITH_PYTHON)
      SET(VERSION_PATH Versions/${SCI_PYTHON_VERSION_SHORT})
      SET(FRAMEWORK_PATH Frameworks/Python.framework)
      SET(PYTHON_PATH ${APPLICATION_NAME}.app/Contents/${FRAMEWORK_PATH}/${VERSION_PATH})
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
        POST_BUILD
        # macdeployqt puts frameworks in app/Content/Frameworks
        COMMAND ${CMAKE_COMMAND} ARGS -E chdir  ${APPLICATION_NAME}.app/Contents/Frameworks tar xvf ${SCI_PYTHON_FRAMEWORK_ARCHIVE}
        COMMAND chmod u+w  ${PYTHON_PATH}/Python
        COMMAND  install_name_tool -id Python ${PYTHON_PATH}/Python
        COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../${FRAMEWORK_PATH}/${VERSION_PATH}/Python ${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
        # TODO: other executables...
        COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../Python ${PYTHON_PATH}/bin/python${SCI_PYTHON_VERSION_SHORT}
        WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    ENDIF()
  ENDIF()
ELSE()
  IF(BUILD_WITH_PYTHON)
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  ENDIF()
ENDIF()

IF(APPLE)
  INSTALL(TARGETS ${APPLICATION_NAME}
        RUNTIME DESTINATION .
        BUNDLE DESTINATION .
  )
ELSE()
  GET_FILENAME_COMPONENT(QTCORENAME ${QT_QTCORE_LIBRARY_RELEASE} NAME_WE)
  GET_FILENAME_COMPONENT(QTGUINAME ${QT_QTGUI_LIBRARY_RELEASE} NAME_WE)
  GET_FILENAME_COMPONENT(QTOPENGLNAME ${QT_QTOPENGL_LIBRARY_RELEASE} NAME_WE)

  IF(WIN32)
    INSTALL(TARGETS ${APPLICATION_NAME}
        RUNTIME DESTINATION bin
    )

    INSTALL(PROGRAMS
      ${QT_BINARY_DIR}/${QTCORENAME}.dll
      ${QT_BINARY_DIR}/${QTGUINAME}.dll
      ${QT_BINARY_DIR}/${QTOPENGLNAME}.dll
      DESTINATION bin
    )

    INSTALL(FILES
      ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt
      DESTINATION bin
    )

    IF(BUILD_WITH_PYTHON)
      INSTALL(PROGRAMS
        ${SCI_PYTHON_LIBRARY_PATH}
        DESTINATION bin
      )

      INSTALL(DIRECTORY
        "${SCI_PYTHON_MODULE_LIBRARY_PATH}/"
        DESTINATION bin/${PYTHON_MODULE_SEARCH_PATH}
      )
    ENDIF()
  ELSE()
    INSTALL(TARGETS ${APPLICATION_NAME}
      RUNTIME DESTINATION bin
    )

    INSTALL(FILES
      ${QT_QTCORE_LIBRARY_RELEASE}.${QT_VERSION}
      ${QT_QTGUI_LIBRARY_RELEASE}.${QT_VERSION}
      ${QT_QTOPENGL_LIBRARY_RELEASE}.${QT_VERSION}
      DESTINATION lib
    )

    INSTALL(FILES
      ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt
      DESTINATION bin
    )

    IF(BUILD_WITH_PYTHON)
      INSTALL(FILES
        ${SCI_PYTHON_LIBRARY_PATH}
        DESTINATION bin
      )
      INSTALL(DIRECTORY
        ${SCI_PYTHON_MODULE_LIBRARY_PATH}
        DESTINATION bin/${PYTHON_MODULE_SEARCH_PATH}
      )
    ENDIF()
  ENDIF()
ENDIF()
