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

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Utils/EnumClass.h>

namespace Core
{

CORE_ENUM_CLASS 
( 
  MeasureSliceType,
  AXIAL_E,
  CORONAL_E,
  SAGITTAL_E,
  NOVIEW_E
)

//=============================================================================
// Class: Measurement
//=============================================================================

class Measurement 
{
public:

  Measurement( bool visible, std::string label, std::string note, Core::Point p1, Core::Point p2, 
    MeasureSliceType slice_type );
  Measurement();

  // GET_VISIBLE:
  // Get whether this measurement is visible.
  bool get_visible() const;

  // SET_VISIBLE:
  // Set whether this measurement is visible.  User-editable.
  void set_visible( bool visible ); 

  // GET_LABEL:
  // Get short label to be rendered above measurement line.
  std::string get_label() const; 

  // SET_LABEL:
  // Set short label to be rendered above measurement line.
  void set_label( std::string label );

  // GET_LENGTH:
  // Get length of measurement
  double get_length() const;

  // GET_NOTE:
  // Get user-editable description of measurement.  
  // May contain any characters including line breaks.
  std::string get_note() const;

  // SET_NOTE:
  // Set user-editable description of measurement.
  // May contain any characters including line breaks.
  void set_note( std::string note ); 

  // GET_POINT1:
  // Get 3D world coordinate of 1st point
  Core::Point get_point1() const;

  // SET_POINT1:
  // Set 3D world coordinate of 1st point
  void set_point1( Core::Point p1 );

  // GET_POINT2:
  // Get 3D world coordinate of 2nd point
  Core::Point get_point2() const;

  // SET_POINT2:
  // Set 3D world coordinate of 2nd point
  void set_point2( Core::Point p2 ); // P2 moves during measurement creation

  // GET_VIEW_AXIS:
  // Get type of slice (axial, sagittal, coronal) where measurement was made.  This is needed
  // to facilitate jumping to the measurement.
  MeasureSliceType get_slice_type() const;

  // SET_VIEW_AXIS:
  // Set type of slice (axial, sagittal, coronal) where measurement was made.
  void set_slice_type( MeasureSliceType slice_type );

  inline bool operator==( const Measurement& ) const;
  inline bool operator!=( const Measurement& ) const;

private:
  bool visible_;
  std::string label_; // Unique ID
  std::string note_;
  Core::Point p1_; // 3D world coordinate of 1st point
  Core::Point p2_; // 3D world coordinate of 2nd point
  MeasureSliceType slice_type_;

public:
  static const std::string NOTE_DELIMITER_C;
};

inline bool Measurement::operator==( const Measurement& m ) const
{
  return ( this->label_ == m.label_ );
}

inline bool Measurement::operator!=( const Measurement& m ) const
{
  return ( this->label_ != m.label_ );
}

std::string ExportToString( const Measurement& value );
std::string ExportToString( const std::vector< Measurement >& value );

bool ImportFromString( const std::string& str, Measurement& value );
bool ImportFromString( const std::string& str, std::vector< Measurement >& value );

} // end namespace Core

#endif 
