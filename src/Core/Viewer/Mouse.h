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
/// Enums for mouse buttons 
/// they have the same values as corresponding Qt ones
CORE_ENUM_CLASS
(
  MouseButton,
  NO_BUTTON_E = 0x00000000,
  LEFT_BUTTON_E = 0x00000001,
  RIGHT_BUTTON_E = 0x00000002,
  MID_BUTTON_E = 0x00000004
)

// CLASS KeyModifier:
/// Enums for key modifiers
/// they have the same values as corresponding Qt ones
CORE_ENUM_CLASS
(
  KeyModifier,
  NO_MODIFIER_E = 0x00000000,
  SHIFT_MODIFIER_E = 0x02000000,
  CONTROL_MODIFIER_E = 0x04000000,
  ALT_MODIFIER_E = 0x08000000,
  META_MODIFIER_E = 0x10000000,
  KEYPAD_MODIFIER_E = 0x20000000,
  GROUPSWITCH_MODIFIER_E = 0x40000000
)

CORE_ENUM_CLASS
(
  Key,
  KEY_LEFT_E = 0x01000012,   
  KEY_UP_E = 0x01000013,   
  KEY_RIGHT_E = 0x01000014,  
  KEY_DOWN_E = 0x01000015,
  KEY_SHIFT_E = 0x01000020,
  KEY_CONTROL_E = 0x01000021,
  KEY_META_E = 0x01000022,
  
  KEY_SPACE_E = 0x20,
  KEY_EXCLAM_E = 0x21, 
  KEY_QUOTE_E = 0x22, 
  KEY_NUMBER_E = 0x23, 
  KEY_DOLLAR_E = 0x24,
  KEY_PERCENT_E = 0x25,
  KEY_AMPERSAND_E = 0x26,
  KEY_APOSTRPHE_E = 0x27,
  KEY_PARENLEFT_E = 0x28,
  KEY_PARENRIGHT_E = 0x29,
  KEY_ASTERISK_E = 0x2a,
  KEY_PLUS_E = 0x2b,
  KEY_COMMA_E = 0x2c,
  KEY_MINUS_E = 0x2d,
  KEY_PERIOD_E = 0x2e,
  KEY_SLASH_E = 0x2f,
  KEY_0_E = 0x30,  
  KEY_1_E = 0x31,  
  KEY_2_E = 0x32,  
  KEY_3_E = 0x33,  
  KEY_4_E = 0x34,  
  KEY_5_E = 0x35,  
  KEY_6_E = 0x36,  
  KEY_7_E = 0x37,  
  KEY_8_E = 0x38,  
  KEY_9_E = 0x39,  
  KEY_COLON_E = 0x3a,
  KEY_SEMICOLON_E = 0x3b,
  KEY_LESS_E = 0x3c,
  KEY_EQUAL_E = 0x3d,
  KEY_GREATER_E = 0x3e,
  KEY_QUESTION_E = 0x3f,
  KEY_AT_E = 0x40,
  KEY_A_E = 0x41,
  KEY_B_E = 0x42,
  KEY_C_E = 0x43,
  KEY_D_E = 0x44,
  KEY_E_E = 0x45,
  KEY_F_E = 0x46,
  KEY_G_E = 0x47,
  KEY_H_E = 0x48,
  KEY_I_E = 0x49,
  KEY_J_E = 0x4a,
  KEY_K_E = 0x4b,
  KEY_L_E = 0x4c,
  KEY_M_E = 0x4d,
  KEY_N_E = 0x4e,
  KEY_O_E = 0x4f,
  KEY_P_E = 0x50,
  KEY_Q_E = 0x51,
  KEY_R_E = 0x52,
  KEY_S_E = 0x53,
  KEY_T_E = 0x54,
  KEY_U_E = 0x55,
  KEY_V_E = 0x56,
  KEY_W_E = 0x57,
  KEY_X_E = 0x58,
  KEY_Y_E = 0x59,
  KEY_Z_E = 0x5a
)

// CLASS HitPosition:
/// Classifies which part of a rectangle region the mouse cursor is on.
CORE_ENUM_CLASS
(
   HitPosition,
   NONE_E = 0x00,
   LEFT_E = 0x01,
   RIGHT_E = 0x02,
   BOTTOM_E = 0x04,
   TOP_E = 0x08,
   INSIDE_E = 0x10,
   BOTTOM_LEFT_E = LEFT_E | BOTTOM_E,
   BOTTOM_RIGHT_E = RIGHT_E | BOTTOM_E,
   TOP_LEFT_E = LEFT_E | TOP_E,
   TOP_RIGHT_E = RIGHT_E | TOP_E
 )

// CLASS MousePosition:
/// Class for recording the location of the mouse

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
/// Class for recording the history of a mouse event
/// This is needed to track mouse gestures

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
