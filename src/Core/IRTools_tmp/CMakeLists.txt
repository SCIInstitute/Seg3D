#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

# code that needs to be reworked and/or replaced

SET(CORE_IRTOOLS_TMP_SRCS
  optimizer_observer.hxx
  the_boost_mutex.cxx
  the_boost_mutex.hxx
  the_boost_thread.cxx
  the_boost_thread.hxx
  the_boost_thread_storage.hxx
  the_dynamic_array.hxx
  the_exception.hxx
  the_log.cxx
  the_log.hxx
  the_mutex_interface.cxx
  the_mutex_interface.hxx
  the_terminator.cxx
  the_terminator.hxx
  the_text.cxx
  the_text.hxx
  the_thread_interface.cxx
  the_thread_interface.hxx
  the_thread_pool.cxx
  the_thread_pool.hxx
  the_thread_storage.cxx
  the_thread_storage.hxx
  the_transaction.cxx
  the_transaction.hxx
  the_utils.hxx
)

CORE_ADD_LIBRARY(Core_IRTools_tmp
  ${CORE_IRTOOLS_TMP_SRCS}
)

TARGET_LINK_LIBRARIES(Core_IRTools_tmp
  ${SCI_BOOST_LIBRARY}
)
