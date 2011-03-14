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

//=============================================================================
// Class: Measurement
//=============================================================================

class Measurement 
{
public:

  Measurement( bool visible, std::string id, std::string note, Core::Point p0, Core::Point p1 );
  Measurement();

  // GET_VISIBLE:
  // Get whether this measurement is visible.
  bool get_visible() const;

  // SET_VISIBLE:
  // Set whether this measurement is visible.  User-editable.
  void set_visible( bool visible ); 

  // GET_LABEL:
  // Get short label to be rendered above measurement line.
  std::string get_id() const; 

  // SET_LABEL:
  // Set short label to be rendered above measurement line.
  void set_id( std::string id );

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

  // GET_POINT:
  // Get 3D world coordinate of point at given index (0 or 1)
  bool get_point( int index, Point& pt ) const;

  // SET_POINT1:
  // Set 3D world coordinate of point at given index (0 or 1)
  bool set_point( int index, const Point& pt );

  inline bool operator==( const Measurement& ) const;
  inline bool operator!=( const Measurement& ) const;

private:
  bool visible_;
  std::string id_; // Unique ID
  std::string note_;
  // TODO: Convert to 2D array of points
  Core::Point p0_; // 3D world coordinate of 1st point
  Core::Point p1_; // 3D world coordinate of 2nd point

public:
  static const std::string NOTE_DELIMITER_C;
};

inline bool Measurement::operator==( const Measurement& m ) const
{
  return ( this->id_ == m.id_ );
}

inline bool Measurement::operator!=( const Measurement& m ) const
{
  return ( this->id_ != m.id_ );
}

std::string ExportToString( const Measurement& value );
std::string ExportToString( const std::vector< Measurement >& value );

bool ImportFromString( const std::string& str, Measurement& value );
bool ImportFromString( const std::string& str, std::vector< Measurement >& value );

} // end namespace Core

#endif 
