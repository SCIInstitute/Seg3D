
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

SET(APPLICATION_PROJECT_SRCS
  DataManager.h
  DataManager.cc
  InputFilesImporter.h
  InputFilesImporter.cc
  Project.h
  Project.cc
  SessionInfo.h
  SessionInfo.cc
  ProjectNote.h
)

SET(APPLICATION_PROJECT_ACTIONS_SRCS
)

CORE_ADD_LIBRARY(Application_Project ${APPLICATION_PROJECT_SRCS}
            ${APPLICATION_PROJECT_ACTIONS_SRCS} )
            
IF(APPLE)
TARGET_LINK_LIBRARIES(Application_Project
                      Core_Action
                      Core_State
                      Core_Utils
                      Core_Application
                      Core_Interface
                      Application_InterfaceManager
                  Application_Layer
                  Application_Provenance
                      ${SCI_BOOST_LIBRARY}
                      ${SCI_SQLITE_LIBRARY}
            /System/Library/Frameworks/CoreServices.framework)
ELSE(APPLE)                    
TARGET_LINK_LIBRARIES(Application_Project
                      Core_Action
                      Core_State
                      Core_Utils
                      Core_Application
                      Core_Interface
                      Application_InterfaceManager
                  Application_Layer
                  Application_Provenance
                      ${SCI_BOOST_LIBRARY}
                      ${SCI_SQLITE_LIBRARY})

ENDIF(APPLE)          
          
