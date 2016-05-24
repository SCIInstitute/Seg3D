/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

#ifndef CORE_VIEWER_CURSOR_H
#define CORE_VIEWER_CURSOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Boost includes 
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/Utils/EnumClass.h>

namespace Core
{

// CLASS CursorShape:
/// Enums for various cursors.
/// They have the same values as corresponding Qt ones
CORE_ENUM_CLASS
(
  CursorShape,
  ARROW_E = 0,
  UP_ARROW_E = 1,
  CROSS_E = 2,
  WAIT_E = 3,
  IBEAM_E = 4,
  SIZE_VER_E = 5,
  SIZE_HOR_E = 6,
  SIZE_BDIAG_E = 7,
  SIZE_FDIAG_E = 8,
  SIZE_ALL_E = 9,
  BLANK_E = 10,
  SPLIT_V_E = 11,
  SPLIT_H_E = 12,
  POINTING_HAND_E = 13,
  FORBIDDEN_E = 14,
  WHATS_THIS_E = 15,
  BUSY_E = 16,
  OPEN_HAND_E = 17,
  CLOSED_HAND_E = 18,
  BITMAP_E = 24
)

} // end namespace Core

#endif
