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


set(ACTION_PYTHON_WRAPPER_REGISTRATION_LIST "" CACHE INTERNAL "list of actions that have python wrappers" FORCE)
set(ACTION_PYTHON_WRAPPER_TEMPLATE "" CACHE INTERNAL "String template for generating action wrapper functions" FORCE)

file(STRINGS ${CMAKE_SOURCE_DIR}/Configuration/ActionPythonWrapperFunctionSource.in ACTION_PYTHON_TEMPLATE_STRINGS)
foreach(LINE ${ACTION_PYTHON_TEMPLATE_STRINGS})
  set(ACTION_PYTHON_WRAPPER_TEMPLATE "${ACTION_PYTHON_WRAPPER_TEMPLATE}${LINE}\n")
endforeach(LINE ${ACTION_PYTHON_TEMPLATE_STRINGS})

macro(GENERATE_ACTION_PYTHON_WRAPPER wrapper_file proj)
  set(ACTION_LIST "")
  foreach(ACTION ${ARGN})
    get_filename_component(ACTIONFILE ${ACTION} NAME_WE)
  string(LENGTH ${ACTIONFILE} ACTION_NAME_LENGTH)
  math(EXPR ACTION_NAME_LENGTH "${ACTION_NAME_LENGTH}-6")
  string(SUBSTRING ${ACTIONFILE} 6 ${ACTION_NAME_LENGTH} ACTION_NAME)
  string(TOLOWER ${ACTION_NAME} ACTION_NAME)
    set(ACTION_LIST ${ACTION_LIST} ${ACTION_NAME})
  endforeach(ACTION ${ARGN})

  list(REMOVE_DUPLICATES ACTION_LIST)
  set(ACTION_PYTHON_WRAPPER_REGISTRATION_LIST ${ACTION_PYTHON_WRAPPER_REGISTRATION_LIST} ${ACTION_LIST} CACHE INTERNAL "")
  
  set(WRAPPER_FUNCTIONS "")
  set(WRAPPER_REGISTRATIONS "")
  
  foreach(ACTION ${ACTION_LIST})
  string(CONFIGURE "${ACTION_PYTHON_WRAPPER_TEMPLATE}" WRAPPER_FUNCTION @ONLY)
  set(WRAPPER_FUNCTIONS "${WRAPPER_FUNCTIONS}${WRAPPER_FUNCTION}")
  
  string(CONFIGURE "void register_action_@ACTION@_python_wrapper()\n{\n\tboost::python::def(\"@ACTION@\", boost::python::raw_function(action_@ACTION@_python_wrapper));\n}\n" 
       WRAPPER_REGISTRATION @ONLY)
  set(WRAPPER_REGISTRATIONS "${WRAPPER_REGISTRATIONS}${WRAPPER_REGISTRATION}")
  endforeach(ACTION ${ACTION_LIST})
  
  set(${wrapper_file} ${CMAKE_CURRENT_BINARY_DIR}/${proj}_ActionPythonWrapper.cc)
  configure_file(${CMAKE_SOURCE_DIR}/Configuration/ActionPythonWrapper.cc.in
                 ${${wrapper_file}}
                 @ONLY)
endmacro(GENERATE_ACTION_PYTHON_WRAPPER)

macro(GENERATE_ACTION_PYTHON_WRAPPER_REGISTRATION_FILE)
  set(DECLARATIONS "")
  set(IMPLEMENTATIONS "")
  
  foreach(ACTION ${ACTION_PYTHON_WRAPPER_REGISTRATION_LIST})
    string(CONFIGURE "extern void register_action_@ACTION@_python_wrapper();\n" DECLARATION)
  set(DECLARATIONS "${DECLARATIONS}${DECLARATION}")
  
  string(CONFIGURE "\tregister_action_@ACTION@_python_wrapper();\n" IMPLEMENTATION)
  set(IMPLEMENTATIONS "${IMPLEMENTATIONS}${IMPLEMENTATION}")
  endforeach(ACTION ${ACTION_PYTHON_WRAPPER_REGISTRATION_LIST})
  
  configure_file(${CMAKE_SOURCE_DIR}/Configuration/ActionPythonWrapperRegistration.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/ActionPythonWrapperRegistration.h
                 @ONLY)
endmacro(GENERATE_ACTION_PYTHON_WRAPPER_REGISTRATION_FILE)
