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

// Boost includes
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

// Core includes
#include <Core/Geometry/Measurement.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{

const std::string Measurement::NOTE_DELIMITER_C = " NOTE_END]]";

Measurement::Measurement( std::string id, bool visible, std::string note, 
  Core::Point p0, Core::Point p1 ) : 
  id_( id ),
  visible_( visible ),  
  note_( note )
{
  this->points_[ 0 ] = p0;
  this->points_[ 1 ] = p1;
}

Measurement::Measurement() :
  id_( "" ), 
  visible_( false ), 
  note_( "" )
{
}

bool Measurement::get_visible() const
{
  return this->visible_;
}

void Measurement::set_visible( bool visible )
{
  this->visible_ = visible;
}

std::string Measurement::get_id() const
{
  return this->id_;
}

void Measurement::set_id( std::string id )
{
  this->id_ = id;
}

double Measurement::get_length() const
{
  return ( this->points_[ 1 ] - this->points_[ 0 ] ).length();
}

std::string Measurement::get_note() const
{
  return this->note_;
}

void Measurement::set_note( std::string note )
{
  this->note_ = note;
}

bool Measurement::get_point( int index, Point& pt ) const
{
  if( !( index == 0 || index == 1 ) ) return false;

  pt = this->points_[ index ];
  return true;
}

bool Measurement::set_point( int index, const Point& pt )
{
  if( !( index == 0 || index == 1 ) ) return false;

  this->points_[ index ] = pt;
  return true;
}

std::string ExportToString( const Measurement& value )
{
  // Need to use special delimiter for note since any characters are allowed in a note,
  // including ']'.  Put note at end so that it functions as both a note delimiter and 
  // measurement delimiter.
  Point p0;
  value.get_point( 0, p0 );
  Point p1;
  value.get_point( 1, p1 );
  return ( std::string( 1, '[' ) + ExportToString( value.get_visible() ) + ' ' + value.get_id() 
    + ' ' + ExportToString( p0 ) + ' ' + ExportToString( p1 ) + 
    ' ' + '[' + value.get_note() + Measurement::NOTE_DELIMITER_C );
}

std::string ExportToString( const std::vector< Measurement >& value )
{
  std::string export_string = "";
  BOOST_FOREACH( Measurement measurement, value )
  {
    export_string += ExportToString( measurement );
  }
  return export_string;
}

bool ImportFromString( const std::string& str, Measurement& value )
{
  // Example string: [true M1 [0 0 0] [1 1 1] [Knee NOTE_END]]
  std::string open_bracket_reg = "(\\[)";
  std::string word_reg = "(\\w*)";
  std::string visible_reg = word_reg;
  std::string space_reg = "(\\s)";
  std::string label_reg = word_reg;
  std::string point_reg = "(\\[[^\\]]*])";
  std::string note_reg = "(.*?)";
  std::string note_end_reg = "(\\sNOTE_END\\]\\])";
  std::string full_reg = open_bracket_reg + visible_reg + space_reg + label_reg + space_reg + 
    point_reg + space_reg + point_reg + space_reg + open_bracket_reg + note_reg + 
    note_end_reg;
  boost::regex reg( full_reg );
  boost::smatch m;
  if( boost::regex_match( str, m, reg ) ) 
  {
    bool visible = false;
    ImportFromString( m[ 2 ].str(), visible );
    value.set_visible( visible );
    value.set_id( m[ 4 ].str() );
    Point p0;
    ImportFromString( m[ 6 ].str(), p0 );
    value.set_point( 0, p0 );
    Point p1;
    ImportFromString( m[ 8 ].str(), p1 );
    value.set_point( 1, p1 );
    value.set_note( m[ 11 ].str() );
    return true;
  } 
  return false;
}

bool ImportFromString( const std::string& str, std::vector< Measurement >& value )
{
  value.clear();
  std::vector< std::string > parts = Core::SplitString( str, Measurement::NOTE_DELIMITER_C );
  // The last part is ""
  for( size_t i = 0; i < parts.size() - 1; i++ )
  {
    std::string measurement_string = parts[ i ];
    measurement_string += Measurement::NOTE_DELIMITER_C;
    Measurement measurement;
    if( ImportFromString( measurement_string, measurement ) )
    {
      value.push_back( measurement );
    }
  }
  return true;
}


} // end namespace Core