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

include_directories(${SCI_BOOST_INCLUDE})
link_directories(${SCI_BOOST_LIBRARY_DIR})
add_definitions(-DBOOST_ALL_NO_LIB)
if(BUILD_WITH_PYTHON)
  add_definitions(-DBOOST_PYTHON_STATIC_LIB -DBOOST_PYTHON_STATIC_MODULE)
endif()

# TODO: if static runtime link is supported, then ABI tag postfix must include s
# see:
# http://www.boost.org/doc/libs/1_58_0/more/getting_started/windows.html
# http://www.boost.org/doc/libs/1_58_0/more/getting_started/unix-variants.html
if(WIN32)
  set(DEBUG_POSTFIX "-gyd")
  set(boost_LIB_PREFIX "lib")
else()
  set(DEBUG_POSTFIX "-yd")
  set(boost_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
endif()

foreach(lib ${SCI_BOOST_LIBRARY})
  add_library(${lib} STATIC IMPORTED GLOBAL)

  if(CMAKE_GENERATOR MATCHES "Makefiles" AND CMAKE_BUILD_TYPE MATCHES "Debug")
    set(FULL_LIB_NAME "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  elseif(CMAKE_GENERATOR MATCHES "Makefiles")
    set(FULL_LIB_NAME "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
  
  if(${lib} MATCHES "boost_python-mt")
    if(WIN32)
	  set(FULL_LIB_NAME_RELEASE "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python34-mt${CMAKE_STATIC_LIBRARY_SUFFIX}")
	  set(FULL_LIB_NAME_DEBUG "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python34-mt${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    else()
	  set(FULL_LIB_NAME_RELEASE "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
	  set(FULL_LIB_NAME_DEBUG "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif()
  else()
    set(FULL_LIB_NAME_RELEASE "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(FULL_LIB_NAME_DEBUG "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()
  endif()

  message(STATUS "Configure Boost library ${lib}")

  if(CMAKE_GENERATOR MATCHES "Makefiles")
    set_target_properties(${lib}
      PROPERTIES
        IMPORTED_LOCATION ${FULL_LIB_NAME}
    )
  else() # IDEs: Xcode, VS, others...
    set_target_properties(${lib}
      PROPERTIES
        IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
        IMPORTED_LOCATION_RELEASE ${FULL_LIB_NAME_RELEASE}
        IMPORTED_LOCATION_DEBUG ${FULL_LIB_NAME_DEBUG}
    )
  endif()
endforeach()
