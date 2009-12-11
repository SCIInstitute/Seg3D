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

###########################################
# Set Seg3D main sources
###########################################

SET(MAIN_SRCS
  main.cc
)

###########################################
# Build the Seg3D executable
###########################################

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SEG3D_OUTPUT_DIRECTORY})

IF(APPLE)
SET(MACOSX_BUNDLE_ICON_FILE "${CMAKE_SOURCE_DIR}/Resources/Seg3D2.icns")
ADD_EXECUTABLE(Seg3D MACOSX_BUNDLE ${MAIN_SRCS})
ELSE(APPLE)
ADD_EXECUTABLE(Seg3D WIN32 ${MAIN_SRCS})
ENDIF(APPLE)

TARGET_LINK_LIBRARIES(Seg3D 
  ${QT_LIBRARIES}
  ${SCI_BOOST_LIBRARY}
  Utils_Core
  Utils_Converter
  Utils_EventHandler
  Application_Application
  Application_Interface
  Application_Action
  Interface_AppInterface
  Interface_QtInterface
  Init_Init
)

IF(WIN32)
  TARGET_LINK_LIBRARIES(Seg3D ${QT_QTMAIN_LIBRARY} )
ENDIF(WIN32)

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

    ADD_CUSTOM_COMMAND(TARGET Seg3D 
       POST_BUILD
       COMMAND [ ! -d Debug ] || cp -rf "${QT_QTCORE_LIBRARY_RELEASE}/Versions/4/QtCore" "Debug/Seg3D.app/Contents/MacOS/" 
       COMMAND [ ! -d Debug ] || cp -rf "${QT_QTGUI_LIBRARY_RELEASE}/Versions/4/QtGui" "Debug/Seg3D.app/Contents/MacOS/"
       COMMAND [ ! -d Debug ] || install_name_tool -id QtCore Debug/Seg3D.app/Contents/MacOS/QtCore
       COMMAND [ ! -d Debug ] || install_name_tool -id QtGui Debug/Seg3D.app/Contents/MacOS/QtGui
       COMMAND [ ! -d Debug ] || install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore Debug/Seg3D.app/Contents/MacOS/QtGui
       COMMAND [ ! -d Debug ] || install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui Debug/Seg3D.app/Contents/MacOS/Seg3D
       COMMAND [ ! -d Debug ] || install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore Debug/Seg3D.app/Contents/MacOS/Seg3D
       COMMAND [ ! -d Debug ] || cp "-rf" "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources" "Debug/Seg3D.app/Contents"
       COMMAND [ ! -d Debug ] || [ -d Debug/Seg3D.app/Contents/MacOS/plugins ] || mkdir Debug/Seg3D.app/Contents/MacOS/plugins
       COMMAND [ ! -d Debug ] || cp -rf "${CMAKE_SOURCE_DIR}/CMake/qt.conf" Debug/Seg3D.app/Contents/Resources

       COMMAND [ ! -d Release ] || cp -rf "${QT_QTCORE_LIBRARY_RELEASE}/Versions/4/QtCore" "Release/Seg3D.app/Contents/MacOS/" 
       COMMAND [ ! -d Release ] || cp -rf "${QT_QTGUI_LIBRARY_RELEASE}/Versions/4/QtGui" "Release/Seg3D.app/Contents/MacOS/"
       COMMAND [ ! -d Release ] || install_name_tool -id QtCore Release/Seg3D.app/Contents/MacOS/QtCore
       COMMAND [ ! -d Release ] || install_name_tool -id QtGui Release/Seg3D.app/Contents/MacOS/QtGui
       COMMAND [ ! -d Release ] || install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore Release/Seg3D.app/Contents/MacOS/QtGui
       COMMAND [ ! -d Release ] || install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui Release/Seg3D.app/Contents/MacOS/Seg3D
       COMMAND [ ! -d Release ] || install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore Release/Seg3D.app/Contents/MacOS/Seg3D
       COMMAND [ ! -d Release ] || cp "-rf" "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources" "Release/Seg3D.app/Contents"
       COMMAND [ ! -d Release ] || [ -d Release/Seg3D.app/Contents/MacOS/plugins ] || mkdir Release/Seg3D.app/Contents/MacOS/plugins
       COMMAND [ ! -d Release ] || cp -rf "${CMAKE_SOURCE_DIR}/CMake/qt.conf" Release/Seg3D.app/Contents/Resources

       WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  ELSE(${CMAKE_GENERATOR} MATCHES "Xcode")
    ADD_CUSTOM_COMMAND(TARGET Seg3D 
       POST_BUILD
       COMMAND cp -rf "${QT_QTCORE_LIBRARY_RELEASE}/Versions/4/QtCore" "Seg3D.app/Contents/MacOS/"
       COMMAND cp -rf "${QT_QTGUI_LIBRARY_RELEASE}/Versions/4/QtGui" "Seg3D.app/Contents/MacOS/"
       COMMAND install_name_tool -id QtCore Seg3D.app/Contents/MacOS/QtCore
       COMMAND install_name_tool -id QtGui Seg3D.app/Contents/MacOS/QtGui
       COMMAND install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore Seg3D.app/Contents/MacOS/QtGui
       COMMAND install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui Seg3D.app/Contents/MacOS/Seg3D
       COMMAND install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore Seg3D.app/Contents/MacOS/Seg3D
       COMMAND cp "-rf" "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources" "Seg3D.app/Contents"
       COMMAND [ -d Seg3D.app/Contents/MacOS/plugins ] || mkdir Seg3D.app/Contents/MacOS/plugins
       COMMAND cp -rf "${CMAKE_SOURCE_DIR}/CMake/qt.conf" Seg3D.app/Contents/Resources

       WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  ENDIF(${CMAKE_GENERATOR} MATCHES "Xcode")
ENDIF(APPLE)

IF(APPLE)
  INSTALL(TARGETS Seg3D 
        RUNTIME DESTINATION .
        BUNDLE DESTINATION . )
ELSE(APPLE)
  IF(WIN32)
    INSTALL(TARGETS Seg3D 
        RUNTIME DESTINATION bin )
    GET_FILENAME_COMPONENT(QTLIBDIR ${QT_QTCORE_LIBRARY_RELEASE} PATH)      
    GET_FILENAME_COMPONENT(QTLIBDIR ${QTLIBDIR} PATH)      

    INSTALL(PROGRAMS ${QTLIBDIR}/bin/QtCore4.dll ${QTLIBDIR}/bin/QTGui4.dll
      DESTINATION bin )
  ELSE(WIN32)
    INSTALL(TARGETS Seg3D 
        RUNTIME DESTINATION bin )

  ENDIF(WIN32)
ENDIF(APPLE)
