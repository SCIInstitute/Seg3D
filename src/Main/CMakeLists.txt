#
#  For more information, please see: http://software.sci.utah.edu
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


############################################
# Set Seg3D main sources
###########################################

set(MAIN_SRCS
  main.cc
  Seg3DBase.h
  Seg3DBase.cc
  Seg3DGui.h
  Seg3DGui.cc
  Seg3DHeadless.h
  Seg3DHeadless.cc
)

###########################################
# Build the Seg3D executable
###########################################

if(NOT BUILD_STANDALONE_LIBRARY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEG3D_BINARY_DIR})
  add_definitions(-DAPPLICATION_NAME=${APPLICATION_NAME})
  if(APPLE)
    list(APPEND MAIN_SRCS ${APPLICATION_OSX_ICON_FULL_PATH})
    set_source_files_properties(${APPLICATION_OSX_ICON_FULL_PATH}
      PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
    add_executable(${APPLICATION_NAME} MACOSX_BUNDLE ${MAIN_SRCS})
    set_target_properties(${APPLICATION_NAME}
      PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${SEG3D_SOURCE_DIR}/Resources/info.plist
      MACOSX_BUNDLE_ICON_FILE ${APPLICATION_OSX_ICON}
    )
  else()
    if(WIN32)
      list(APPEND MAIN_SRCS ${APPLICATION_WINDOWS_RC})
      if(SEG3D_SHOW_CONSOLE)
        add_executable(${APPLICATION_NAME} ${MAIN_SRCS})
      else()
        add_executable(${APPLICATION_NAME} WIN32 ${MAIN_SRCS})
      endif()
    else()
      add_executable(${APPLICATION_NAME} ${MAIN_SRCS})
    endif()
  endif()

target_link_libraries(${APPLICATION_NAME}
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

if(WIN32)
  target_link_libraries(${APPLICATION_NAME} ${Qt5Core_QTMAIN_LIBRARIES} )
  set_target_properties(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "/INCREMENTAL:NO")
endif()

if(APPLE)
  set_target_properties(${APPLICATION_NAME}
    PROPERTIES
    LINK_FLAGS "-framework CoreGraphics"
  )
endif()

if(BUILD_WITH_PYTHON)
  target_link_libraries(${APPLICATION_NAME}
    Core_Python
    Application_Socket
  )
endif()

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

if(APPLE)
  # set up app bundle
  add_custom_command(TARGET ${APPLICATION_NAME}
    POST_BUILD
    # configure Qt libs
    COMMAND ${MACDEPLOYQT} ${APPLICATION_NAME}.app -verbose=${MACDEPLOYQT_OUTPUT_LEVEL}
    WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  if(BUILD_WITH_PYTHON)
    set(VERSION_PATH Versions/${SCI_PYTHON_VERSION_SHORT})
    set(FRAMEWORK_PATH Frameworks/Python.framework)
    set(PYTHON_PATH ${APPLICATION_NAME}.app/Contents/${FRAMEWORK_PATH}/${VERSION_PATH})
      add_custom_command(TARGET ${APPLICATION_NAME}
      POST_BUILD
      # macdeployqt puts frameworks in app/Content/Frameworks
      COMMAND ${CMAKE_COMMAND} ARGS -E chdir ${APPLICATION_NAME}.app/Contents/Frameworks tar xf ${SCI_PYTHON_FRAMEWORK_ARCHIVE}
      COMMAND chmod u+w  ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -id Python ${PYTHON_PATH}/Python
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../${FRAMEWORK_PATH}/${VERSION_PATH}/Python ${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
      # TODO: other executables...
      COMMAND  install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python @executable_path/../Python ${PYTHON_PATH}/bin/python${SCI_PYTHON_VERSION_SHORT}
      WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
  endif()
elseif(WIN32)
  message(STATUS "Setting ${WINDEPLOYQT} Command for path ${SEG3D_BINARY_DIR}/$<CONFIG>")
  add_custom_command(TARGET ${APPLICATION_NAME}
  POST_BUILD
  # copy Qt libs
  COMMAND ${WINDEPLOYQT} ARGS --verbose 5 -core -widgets -gui -opengl --libdir ${SEG3D_BINARY_DIR}/$<CONFIG> ${SEG3D_BINARY_DIR}/$<CONFIG>/${APPLICATION_NAME}.exe
  )

  if(BUILD_WITH_PYTHON)
    # TODO: change $<CONFIGURATION> to $<CONFIG> when upgrading to cmake 3
    # TODO: copy python dll
    add_custom_command(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS -E make_directory $<CONFIG>/${PYTHON_MODULE_SEARCH_PATH}
      COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} $<CONFIG>/${PYTHON_MODULE_SEARCH_PATH}
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
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory  ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
        WORKING_DIRECTORY ${SEG3D_BINARY_DIR})
    endif()
  endif()
endif()

if(APPLE)
  install(TARGETS ${APPLICATION_NAME}
        RUNTIME DESTINATION .
        BUNDLE DESTINATION .
  )
else()
  get_filename_component(QTCORENAME ${QtCore_location_Release} NAME_WE)
  get_filename_component(QTGUINAME ${QtGui_location_Release} NAME_WE)
  get_filename_component(QTOPENGLNAME ${QtOpenGL_location_Release} NAME_WE)
  get_filename_component(QTWIDGETSNAME ${QtWidgets_location_Release} NAME_WE)
  get_filename_component(QTSVGNAME ${QtSvg_location_Release} NAME_WE)

  if(WIN32)
    install(TARGETS ${APPLICATION_NAME}
      RUNTIME DESTINATION bin
    )

    install(PROGRAMS
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTCORENAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTGUINAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTOPENGLNAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTWIDGETSNAME}.dll
      ${SEG3D_BINARY_DIR}/$<CONFIG>/${QTSVGNAME}.dll
        DESTINATION bin
    )

    install(DIRECTORY
      ${SEG3D_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/platforms/
        DESTINATION bin/platforms
    )

    install(DIRECTORY
      ${SEG3D_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/translations/
        DESTINATION bin/translations
    )

    install(DIRECTORY
      ${SEG3D_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/imageformats/
        DESTINATION bin/imageformats
    )

    install(FILES
      ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt
        DESTINATION bin
    )

    if(BUILD_WITH_PYTHON)
      install(PROGRAMS
        ${SCI_PYTHON_DLL_PATH}
          DESTINATION bin
      )

      install(DIRECTORY
        "${SCI_PYTHON_MODULE_LIBRARY_PATH}/"
          DESTINATION bin/${PYTHON_MODULE_SEARCH_PATH}
      )
    endif()
#  else()
#    install(TARGETS ${APPLICATION_NAME}
#      RUNTIME DESTINATION bin
#    )
#
#    install(FILES
#      ${QT_QTCORE_LIBRARY_RELEASE}.${QT_VERSION}
#      ${QT_QTGUI_LIBRARY_RELEASE}.${QT_VERSION}
#      ${QT_QTOPENGL_LIBRARY_RELEASE}.${QT_VERSION}
#      DESTINATION lib
#    )
#
#    install(FILES
#      ${CMAKE_SOURCE_DIR}/ReleaseNotes.txt
#      DESTINATION bin
#    )
#
#    if(BUILD_WITH_PYTHON)
#      install(FILES
#        ${SCI_PYTHON_DLL_PATH}
#        DESTINATION bin
#      )
#      install(DIRECTORY
#        ${SCI_PYTHON_MODULE_LIBRARY_PATH}
#        DESTINATION bin/${PYTHON_MODULE_SEARCH_PATH}
#      )
#    endif()
  endif()
endif()
endif()

if(BUILD_STANDALONE_LIBRARY)
  add_subdirectory(Seg3DLib)
endif()
