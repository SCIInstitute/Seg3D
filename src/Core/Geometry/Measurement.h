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

namespace Core
{

//=============================================================================
// Class: Measurement
//=============================================================================

class Measurement 
{
public:

  Measurement( std::string id, bool visible, std::string note, Core::Point p0, Core::Point p1 );
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
  // Get length of measurement in world units.
  double get_length() const;

  // SET_LENGTH:
  // Set length of measurement in world units.  Moves second point to specified distance along 
  // vector between points.  
  void set_length( double length );

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

  // SET_POINT:
  // Set 3D world coordinate of point at given index (0 or 1)
  bool set_point( int index, const Point& pt );

  inline bool operator==( const Measurement& ) const;
  inline bool operator!=( const Measurement& ) const;

private:
  std::string id_; // Unique ID
  bool visible_;
  std::string note_; // User-defined note
  Core::Point points_[ 2 ]; // 3D world coordinates of end points

public:
  // User-defined notes can contain any character including line breaks, so we have to use our
  // own delimiter to signify the end of a note for parsing.
  static const std::string NOTE_DELIMITER_C;
};

inline bool Measurement::operator==( const Measurement& m ) const
{
  return ( this->id_ == m.id_ && this->visible_ == m.visible_ && this->note_ == m.note_ &&
    this->points_[ 0 ] == m.points_[ 0 ] && this->points_[ 1 ] == m.points_[ 1 ] );
}

inline bool Measurement::operator!=( const Measurement& m ) const
{
  return ( this->id_ != m.id_ || this->visible_ != m.visible_ || this->note_ != m.note_ ||
    this->points_[ 0 ] != m.points_[ 0 ] || this->points_[ 1 ] != m.points_[ 1 ] );
}

std::string ExportToString( const Measurement& value );
std::string ExportToString( const std::vector< Measurement >& value );

bool ImportFromString( const std::string& str, Measurement& value );
bool ImportFromString( const std::string& str, std::vector< Measurement >& value );

} // end namespace Core

#endif 
