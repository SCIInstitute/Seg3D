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

#ifndef CORE_GEOMETRY_MEASUREMENT_H
#define CORE_GEOMETRY_MEASUREMENT_H

// System includes
#include <string>
#include <vector>

// Boost includes
#include <boost/thread.hpp>
#include <boost/utility.hpp> // Needed for noncopyable

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Utils/Singleton.h>

namespace Core
{

enum ViewAxis {
  AXIAL_E,
  CORONAL_E,
  SAGITTAL_E,
  VOLUME_E,
  MIP_E,
  OBLIQUE_E,
  NOVIEW_E
};

//=============================================================================
// Class: Measurement
//=============================================================================

// This class may need to be modified when we complete the state design across layers.  For now
// this class stores test data for display in the Measurements tool panel.
class Measurement 
{
public:

  Measurement( bool visible, std::string label, std::string note, Core::Point p1, Core::Point p2, 
    ViewAxis view_axis, int slice_num, int slice_thickness, std::string oblique_name = "" ) : 
    visible_( visible ), 
    label_( label ), 
    note_( note ), 
    p1_( p1 ), 
    p2_( p2 ), 
    view_axis_( view_axis )
  {}
  Measurement() :
    visible_( false ), 
    label_( "" ), 
    note_( "" ), 
    view_axis_( NOVIEW_E ) {}

  // DESIGN NOTE: Originally I made a virtual destructor out of habit.  But since this is not
  // designed to be a base class, should not make destructor virtual in order to avoid cost of
  // vtable.  Because this class contains no dynamically allocated memory, can use default 
  // destructor.

  // DESIGN NOTE: Using const for member functions that don't modify member data
  bool      get_visible() const { return visible_; }
  void      set_visible( bool visible ) { visible_ = visible; } // User-editable
  std::string   get_label() const { return label_; } 
  void      set_label( std::string label ) { label_ = label; }
  double      get_length() const { return (p2_ - p1_).length(); }
  std::string   get_note() const { return note_; }
  void      set_note( std::string note ) { note_ = note; } // User-editable
  Core::Point   get_point1() const { return p1_; }
  void      set_point1( Core::Point p1 ) { p1_ = p1; }
  Core::Point   get_point2() const { return p2_; }
  void      set_point2( Core::Point p2 ) { p2_ = p2; } // P2 moves during measurement creation
  ViewAxis    get_view_axis() const { return view_axis_; }
  void      set_view_axis( ViewAxis view_axis ) { view_axis_ = view_axis; }

  inline bool operator==( const Measurement& ) const;
  inline bool operator!=( const Measurement& ) const;

private:
  bool      visible_;
  std::string   label_; // Unique ID
  std::string   note_;
  Core::Point   p1_; // 3D world coordinate of 1st point
  Core::Point   p2_; // 3D world coordinate of 2nd point
  ViewAxis    view_axis_;

public:
  static const std::string NOTE_DELIMITER_C;
};

inline bool Measurement::operator==( const Measurement& m ) const
{
  return ( this->p1_ == m.p1_ && this->p2_ == m.p2_ && this->view_axis_ == m.view_axis_ );
}

inline bool Measurement::operator!=( const Measurement& m ) const
{
  return ( this->p1_ != m.p1_ || this->p2_ != m.p2_ || this->view_axis_ != m.view_axis_ );
}

std::string ExportToString( const Measurement& value );
std::string ExportToString( const std::vector< Measurement >& value );

bool ImportFromString( const std::string& str, Measurement& value );
bool ImportFromString( const std::string& str, std::vector< Measurement >& value );

} // end namespace Core

#endif 
