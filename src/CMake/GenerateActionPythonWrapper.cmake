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


SET(ACTION_PYTHON_WRAPPER_REGISTRATION_LIST "" CACHE INTERNAL "list of actions that have python wrappers" FORCE)
SET(ACTION_PYTHON_WRAPPER_TEMPLATE "" CACHE INTERNAL "String template for generating action wrapper functions" FORCE)

FILE(STRINGS ${CMAKE_SOURCE_DIR}/Configuration/ActionPythonWrapperFunctionSource.in ACTION_PYTHON_TEMPLATE_STRINGS)
FOREACH(LINE ${ACTION_PYTHON_TEMPLATE_STRINGS})
  SET(ACTION_PYTHON_WRAPPER_TEMPLATE "${ACTION_PYTHON_WRAPPER_TEMPLATE}${LINE}\n")
ENDFOREACH(LINE ${ACTION_PYTHON_TEMPLATE_STRINGS})

MACRO(GENERATE_ACTION_PYTHON_WRAPPER wrapper_file proj)
  SET(ACTION_LIST "")
  FOREACH(ACTION ${ARGN})
    GET_FILENAME_COMPONENT(ACTIONFILE ${ACTION} NAME_WE)
  STRING(LENGTH ${ACTIONFILE} ACTION_NAME_LENGTH)
  MATH(EXPR ACTION_NAME_LENGTH "${ACTION_NAME_LENGTH}-6")
  STRING(SUBSTRING ${ACTIONFILE} 6 ${ACTION_NAME_LENGTH} ACTION_NAME)
  STRING(TOLOWER ${ACTION_NAME} ACTION_NAME)
    SET(ACTION_LIST ${ACTION_LIST} ${ACTION_NAME})
  ENDFOREACH(ACTION ${ARGN})

  LIST(REMOVE_DUPLICATES ACTION_LIST)
  SET(ACTION_PYTHON_WRAPPER_REGISTRATION_LIST ${ACTION_PYTHON_WRAPPER_REGISTRATION_LIST} ${ACTION_LIST} CACHE INTERNAL "")
  
  SET(WRAPPER_FUNCTIONS "")
  SET(WRAPPER_REGISTRATIONS "")
  
  FOREACH(ACTION ${ACTION_LIST})
  STRING(CONFIGURE "${ACTION_PYTHON_WRAPPER_TEMPLATE}" WRAPPER_FUNCTION @ONLY)
  SET(WRAPPER_FUNCTIONS "${WRAPPER_FUNCTIONS}${WRAPPER_FUNCTION}")
  
  STRING(CONFIGURE "void register_action_@ACTION@_python_wrapper()\n{\n\tboost::python::def(\"@ACTION@\", boost::python::raw_function(action_@ACTION@_python_wrapper));\n}\n" 
       WRAPPER_REGISTRATION @ONLY)
  SET(WRAPPER_REGISTRATIONS "${WRAPPER_REGISTRATIONS}${WRAPPER_REGISTRATION}")
  ENDFOREACH(ACTION ${ACTION_LIST})
  
  SET(${wrapper_file} ${CMAKE_CURRENT_BINARY_DIR}/${proj}_ActionPythonWrapper.cc)
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/Configuration/ActionPythonWrapper.cc.in
                 ${${wrapper_file}}
                 @ONLY)
ENDMACRO(GENERATE_ACTION_PYTHON_WRAPPER)

MACRO(GENERATE_ACTION_PYTHON_WRAPPER_REGISTRATION_FILE)
  SET(DECLARATIONS "")
  SET(IMPLEMENTATIONS "")
  
  FOREACH(ACTION ${ACTION_PYTHON_WRAPPER_REGISTRATION_LIST})
    STRING(CONFIGURE "extern void register_action_@ACTION@_python_wrapper();\n" DECLARATION)
  SET(DECLARATIONS "${DECLARATIONS}${DECLARATION}")
  
  STRING(CONFIGURE "\tregister_action_@ACTION@_python_wrapper();\n" IMPLEMENTATION)
  SET(IMPLEMENTATIONS "${IMPLEMENTATIONS}${IMPLEMENTATION}")
  ENDFOREACH(ACTION ${ACTION_PYTHON_WRAPPER_REGISTRATION_LIST})
  
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/Configuration/ActionPythonWrapperRegistration.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/ActionPythonWrapperRegistration.h
                 @ONLY)
ENDMACRO(GENERATE_ACTION_PYTHON_WRAPPER_REGISTRATION_FILE)
