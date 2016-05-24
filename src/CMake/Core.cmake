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

OPTION(USE_PRECOMPILED_HEADERS "Use precompiled headers to speed up compilation" OFF)

MACRO(CORE_ADD_LIBRARY name)

  ADD_LIBRARY( ${name} STATIC ${ARGN})

  IF(USE_PRECOMPILED_HEADERS)
    IF(${CMAKE_GENERATOR} MATCHES "Xcode")
    FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Precompiled )
  
      SET_TARGET_PROPERTIES( ${name} PROPERTIES 
      XCODE_ATTRIBUTE_SHARED_PRECOMPS_DIR ${CMAKE_BINARY_DIR}/Precompiled
      XCODE_ATTRIBUTE_GCC_PREFIX_HEADER ${CMAKE_SOURCE_DIR}/Configuration/PrefixHeader.h
      XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER YES
      XCODE_ATTRIBUTE_PRECOMPS_INCLUDE_HEADERS_FROM_BUILT_PRODUCTS_DIR NO)
    ENDIF(${CMAKE_GENERATOR} MATCHES "Xcode")
  ENDIF(USE_PRECOMPILED_HEADERS)

ENDMACRO(CORE_ADD_LIBRARY)

MACRO (CORE_WRAP_XML outfiles )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

FOREACH (it ${ARGN})
  GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
  GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)

  SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.xml.h)
  ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
  COMMAND XmlConverter
  ARGS ${it} ${outfile}
  DEPENDS ${it} XmlConverter)
  SET(${outfiles} ${${outfiles}} ${outfile})
ENDFOREACH(it)
ENDMACRO (CORE_WRAP_XML)

# A macro for importing GLSL shaders
MACRO (CORE_IMPORT_SHADER outfiles )
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

  FOREACH (it ${ARGN})
    GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(ext ${it} EXT)
    
    STRING(LENGTH ${ext} ext_len)
    MATH(EXPR ext_len ${ext_len}-1)
    STRING(SUBSTRING ${ext} 1 ${ext_len} ext)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}_${ext})
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
    COMMAND ShaderImporter
    ARGS ${it} ${outfile}
    DEPENDS ${it} ShaderImporter)
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH(it)
ENDMACRO (CORE_IMPORT_SHADER)
