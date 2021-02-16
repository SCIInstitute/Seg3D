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

set(CLASS_REGISTRATION_LIBRARY_LIST "" CACHE INTERNAL "list of libraries needed for registration" FORCE)
set(CLASS_REGISTRATION_LIST "" CACHE INTERNAL "list of classes that need to be registered" FORCE)

macro(REGISTER_LIBRARY_AND_CLASSES library)

  set(CLASS_REGISTRATION_LIBRARY_LIST_TEMP ${CLASS_REGISTRATION_LIBRARY_LIST} ${library})
  set(CLASS_REGISTRATION_LIBRARY_LIST ${CLASS_REGISTRATION_LIBRARY_LIST_TEMP} CACHE INTERNAL "list of libraries needed for registration")

  set(CLASS_REGISTRATION_LIST_TEMP ${CLASS_REGISTRATION_LIST} ${ARGN})
  set(CLASS_REGISTRATION_LIST ${CLASS_REGISTRATION_LIST_TEMP} CACHE INTERNAL "list of classes that need to be registered")
  
endmacro()


macro(GENERATE_REGISTRATION_FILES)

  set(CLASSLIST "")
  foreach(CLASS ${CLASS_REGISTRATION_LIST})
    get_filename_component(CLASSFILE ${CLASS} NAME_WE)
    set(CLASSLIST ${CLASSLIST} ${CLASSFILE})
  endforeach()

  list(REMOVE_DUPLICATES CLASSLIST)

  set(DECLARATIONS "")
  set(IMPLEMENTATIONS "")

  foreach(CLASS ${CLASSLIST})
    string(CONFIGURE "extern void register_@CLASS@();\n" DECLARATION)
    set(DECLARATIONS "${DECLARATIONS} ${DECLARATION}")

    string(CONFIGURE "  register_@CLASS@();\n" IMPLEMENTATION)
    set(IMPLEMENTATIONS "${IMPLEMENTATIONS} ${IMPLEMENTATION}")
  endforeach()
  
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Configuration/ClassRegistration.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/ClassRegistration.h
                 @ONLY)
                 
endmacro()

macro(REGISTERED_TARGET_LINK_LIBRARIES library)
target_link_libraries(${library} ${CLASS_REGISTRATION_LIBRARY_LIST})

endmacro()
