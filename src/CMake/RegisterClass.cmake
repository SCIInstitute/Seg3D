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

SET(CLASS_REGISTRATION_LIBRARY_LIST "" CACHE INTERNAL "list of libraries needed for registration" FORCE)
SET(CLASS_REGISTRATION_LIST "" CACHE INTERNAL "list of classes that need to be registered" FORCE)

MACRO(REGISTER_LIBRARY_AND_CLASSES library)

  SET(CLASS_REGISTRATION_LIBRARY_LIST_TEMP ${CLASS_REGISTRATION_LIBRARY_LIST} ${library})
  SET(CLASS_REGISTRATION_LIBRARY_LIST ${CLASS_REGISTRATION_LIBRARY_LIST_TEMP} CACHE INTERNAL "list of libraries needed for registration")

  SET(CLASS_REGISTRATION_LIST_TEMP ${CLASS_REGISTRATION_LIST} ${ARGN})
  SET(CLASS_REGISTRATION_LIST ${CLASS_REGISTRATION_LIST_TEMP} CACHE INTERNAL "list of classes that need to be registered")
  
ENDMACRO(REGISTER_LIBRARY_AND_CLASSES)


MACRO(GENERATE_REGISTRATION_FILES)

  SET(CLASSLIST "")
  FOREACH(CLASS ${CLASS_REGISTRATION_LIST})
    GET_FILENAME_COMPONENT(CLASSFILE ${CLASS} NAME_WE)
    SET(CLASSLIST ${CLASSLIST} ${CLASSFILE})
  ENDFOREACH(CLASS ${CLASS_REGISTRATION_LIST})

  LIST(REMOVE_DUPLICATES CLASSLIST)

  SET(DECLARATIONS "")
  SET(IMPLEMENTATIONS "")

  FOREACH(CLASS ${CLASSLIST})
    STRING(CONFIGURE "extern void register_@CLASS@();\n" DECLARATION)
    SET(DECLARATIONS "${DECLARATIONS} ${DECLARATION}")

    STRING(CONFIGURE "  register_@CLASS@();\n" IMPLEMENTATION)
    SET(IMPLEMENTATIONS "${IMPLEMENTATIONS} ${IMPLEMENTATION}")
  ENDFOREACH(CLASS ${CLASSLIST})
  
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Configuration/ClassRegistration.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/ClassRegistration.h
                 @ONLY)
                 
ENDMACRO(GENERATE_REGISTRATION_FILES)

MACRO(REGISTERED_TARGET_LINK_LIBRARIES library)
TARGET_LINK_LIBRARIES(${library} ${CLASS_REGISTRATION_LIBRARY_LIST})

ENDMACRO(REGISTERED_TARGET_LINK_LIBRARIES)
