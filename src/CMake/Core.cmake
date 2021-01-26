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

option(USE_PRECOMPILED_HEADERS "Use precompiled headers to speed up compilation" OFF)

macro(CORE_ADD_LIBRARY name)

  add_library( ${name} STATIC ${ARGN})

  if(USE_PRECOMPILED_HEADERS)
    if(${CMAKE_GENERATOR} MATCHES "Xcode")
    file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Precompiled )
  
      set_target_properties( ${name} PROPERTIES 
      XCODE_ATTRIBUTE_SHARED_PRECOMPS_DIR ${CMAKE_BINARY_DIR}/Precompiled
      XCODE_ATTRIBUTE_GCC_PREFIX_HEADER ${CMAKE_SOURCE_DIR}/Configuration/PrefixHeader.h
      XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER YES
      XCODE_ATTRIBUTE_PRECOMPS_INCLUDE_HEADERS_FROM_BUILT_PRODUCTS_DIR NO)
    endif(${CMAKE_GENERATOR} MATCHES "Xcode")
  endif(USE_PRECOMPILED_HEADERS)

endmacro(CORE_ADD_LIBRARY)

macro (CORE_WRAP_XML outfiles )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

foreach (it ${ARGN})
  get_filename_component(it ${it} ABSOLUTE)
  get_filename_component(outfile ${it} NAME_WE)

  set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.xml.h)
  add_custom_command(OUTPUT ${outfile}
  COMMAND XmlConverter
  ARGS ${it} ${outfile}
  DEPENDS ${it} XmlConverter)
  set(${outfiles} ${${outfiles}} ${outfile})
endforeach(it)
endmacro (CORE_WRAP_XML)

# A macro for importing GLSL shaders
macro (CORE_IMPORT_SHADER outfiles )
  include_directories(${CMAKE_CURRENT_BINARY_DIR})

  foreach (it ${ARGN})
    get_filename_component(it ${it} ABSOLUTE)
    get_filename_component(outfile ${it} NAME_WE)
    get_filename_component(ext ${it} EXT)
    
    string(LENGTH ${ext} ext_len)
    math(EXPR ext_len ${ext_len}-1)
    string(SUBSTRING ${ext} 1 ${ext_len} ext)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}_${ext})
    add_custom_command(OUTPUT ${outfile}
    COMMAND ShaderImporter
    ARGS ${it} ${outfile}
    DEPENDS ${it} ShaderImporter)
    set(${outfiles} ${${outfiles}} ${outfile})
  endforeach(it)
endmacro (CORE_IMPORT_SHADER)
