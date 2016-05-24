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

#ifndef CORE_GEOMETRY_MEASUREMENT_H
#define CORE_GEOMETRY_MEASUREMENT_H

// System includes
#include <string>
#include <vector>

// Core includes
#include <Core/Geometry/Color.h>
#include <Core/Geometry/Point.h>

namespace Core
{

//=============================================================================
// Class: Measurement
//=============================================================================

class Measurement 
{
public:

  Measurement( const std::string& name, const std::string& comment, 
    const Point& p0, const Point& p1, const Color& color, bool visible );
  Measurement();

  // GET_VISIBLE:
  /// Get whether this measurement is visible.
  bool get_visible() const;

  // SET_VISIBLE:
  /// Set whether this measurement is visible.  User-editable.
  void set_visible( bool visible ); 

  // GET_LABEL:
  /// Get short name to be rendered above measurement line.
  std::string get_name() const; 

  // SET_LABEL:
  /// Set short name to be rendered above measurement line.
  void set_name( std::string name );

  // GET_LENGTH:
  /// Get length of measurement in world units.
  double get_length() const;

  // SET_LENGTH:
  /// Set length of measurement in world units.  Moves second point to specified distance along 
  /// vector between points.  
  void set_length( double length );

  // GET_NOTE:
  // Get user-editable description of measurement.  
  // May contain any characters including line breaks.
  std::string get_comment() const;

  // SET_NOTE:
  /// Set user-editable description of measurement.
  /// May contain any characters including line breaks.
  void set_comment( std::string comment ); 

  // GET_POINT:
  /// Get 3D world coordinate of point at given index (0 or 1)
  bool get_point( int index, Point& pt ) const;

  // SET_POINT:
  /// Set 3D world coordinate of point at given index (0 or 1)
  bool set_point( int index, const Point& pt );

  // GET_COLOR:
  void get_color( Color& color ) const;

  // SET_COLOR:
  void set_color( const Color& color );

  inline bool operator==( const Measurement& ) const;
  inline bool operator!=( const Measurement& ) const;

private:
  std::string name_; // User-defined name
  std::string comment_; // User-defined comment
  Point points_[ 2 ]; // 3D world coordinates of end points
  Color color_;
  bool visible_;

public:
  // Backwards compatibility -- old code used comment delimiter
  static const std::string COMMENT_DELIMITER_C;
  static const Color DEFAULT_COLOR_C;
  // The regular expression for validating a name or comment string
  const static std::string REGEX_VALIDATOR_C;
};

inline bool Measurement::operator==( const Measurement& m ) const
{
  return ( this->name_ == m.name_ && this->comment_ == m.comment_ &&
    this->points_[ 0 ] == m.points_[ 0 ] && this->points_[ 1 ] == m.points_[ 1 ] &&
    this->color_ == m.color_  && this->visible_ == m.visible_ );
}

inline bool Measurement::operator!=( const Measurement& m ) const
{
  return ( this->name_ != m.name_ || this->comment_ != m.comment_ ||
    this->points_[ 0 ] != m.points_[ 0 ] || this->points_[ 1 ] != m.points_[ 1 ] || 
    this->color_ != m.color_ || this->visible_ != m.visible_ );
}

std::string ExportToString( const Measurement& value );
std::string ExportToString( const std::vector< Measurement >& value );

bool ImportFromString( const std::string& str, Measurement& value );
bool ImportFromString( const std::string& str, std::vector< Measurement >& value );

} // end namespace Core

#endif 
