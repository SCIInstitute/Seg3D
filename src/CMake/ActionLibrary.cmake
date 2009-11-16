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

SET(ACTION_LIBRARY_LIST "" CACHE INTERNAL "list of libraries" FORCE)
SET(ACTION_LIST "" CACHE INTERNAL "list of actions" FORCE)

MACRO(ADD_LIBRARY_ACTIONS library)

  SET(ACTION_LIBRARY_LIST_TEMP ${ACTION_LIBRARY_LIST} ${library})
  SET(ACTION_LIBRARY_LIST ${ACTION_LIBRARY_LIST_TEMP} CACHE INTERNAL "list of libraries")

  SET(ACTION_LIST_TEMP ${ACTION_LIST} ${ARGN})
  SET(ACTION_LIST ${ACTION_LIST_TEMP} CACHE INTERNAL "list of actions")
  
ENDMACRO(ADD_LIBRARY_ACTIONS)


MACRO(BUILD_REGISTER_ACTIONS_LIBRARY)

  SET(ACTIONLIST "")
  FOREACH(ACTION ${ACTION_LIST})
    STRING(REPLACE ".cc" "" ACTIONTEMP ${ACTION})
    STRING(REPLACE ".h" "" ACTIONFILE ${ACTIONTEMP})
    SET(ACTIONLIST ${ACTIONLIST} ${ACTIONFILE})
  ENDFOREACH(ACTION ${ACTION_LIST})

  LIST(REMOVE_DUPLICATES ACTIONLIST)

  SET(DECLARATIONS "")
  SET(IMPLEMENTATIONS "")

  FOREACH(ACTION ${ACTIONLIST})
    STRING(CONFIGURE "extern void register_@ACTION@();\n" DECLARATION)
    SET(DECLARATIONS "${DECLARATIONS} ${DECLARATION}")

    STRING(CONFIGURE "  register_@ACTION@();\n" IMPLEMENTATION)
    SET(IMPLEMENTATIONS "${IMPLEMENTATIONS} ${IMPLEMENTATION}")
  ENDFOREACH(ACTION ${ACTIONLIST})
  
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Configuration/ActionLibraryInit.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/Init/ActionLibraryInit.h
                 @ONLY)
                 
ENDMACRO(BUILD_REGISTER_ACTIONS_LIBRARY)
