/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef CORE_VIEWER_MOUSE_H
#define CORE_VIEWER_MOUSE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// Boost includes 
#include <boost/smart_ptr.hpp>

// Core includes
#include <Core/Utils/EnumClass.h>

namespace Core
{

// CLASS MouseButton:
// Enums for mouse buttons 
// they have the same values as corresponding Qt ones
SCI_ENUM_CLASS
(
  MouseButton,
  NO_BUTTON_E = 0x00000000,
  LEFT_BUTTON_E = 0x00000001,
  RIGHT_BUTTON_E = 0x00000002,
  MID_BUTTON_E = 0x00000004
)

// CLASS KeyModifier:
// Enums for key modifiers
// they have the same values as corresponding Qt ones
SCI_ENUM_CLASS
(
  KeyModifier,
  NO_MODIFIER_E = 0x00000000,
  SHIFT_MODIFIER_E = 0x02000000,
  CONTROL_MODIFIER_E = 0x04000000,
  ALT_MODIFIER_E = 0x08000000
)

// CLASS MousePosition:
// Class for recording the location of the mouse

class MousePosition
{
public:
  MousePosition( int x_in = 0, int y_in = 0 ) :
    x_( x_in ), 
    y_( y_in )
  {
  }

  // Mouse location
  int x_;
  int y_;
};

typedef boost::shared_ptr< MousePosition > MousePositionHandle;

// CLASS MouseHistory:
// Class for recording the history of a mouse event
// This is needed to track mouse gestures

class MouseHistory
{
public:
  MousePosition left_start_;
  MousePosition right_start_;
  MousePosition mid_start_;
  MousePosition previous_;
  MousePosition current_;
};

} // end namespace Core

#endif
