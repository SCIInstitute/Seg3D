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

// Boost includes
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

// Core includes
#include <Core/Geometry/Measurement.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{
const std::string Measurement::COMMENT_DELIMITER_C = " COMMENT_END]]";
const Color Measurement::DEFAULT_COLOR_C = Color( 1.0f, 1.0f, 0.0f ); // Yellow
const std::string Measurement::REGEX_VALIDATOR_C( "[^\\[\\]]+" ); // Allowed characters

Measurement::Measurement( const std::string& name, const std::string& comment, 
  const Point& p0, const Point& p1, const Color& color, bool visible ) 
{
  this->set_name( name );
  this->set_comment( comment );
  this->set_point( 0, p0 );
  this->set_point( 1, p1 );
  this->set_color( color );
  this->set_visible( visible );
}

Measurement::Measurement() :
  name_( "" ), 
  comment_( "" ),
  color_( Measurement::DEFAULT_COLOR_C ),
  visible_( false ) 
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

std::string Measurement::get_name() const
{
  return this->name_;
}

void Measurement::set_name( std::string name )
{
  // Strip brackets 
  boost::algorithm::replace_all( name, "[", "" );
  boost::algorithm::replace_all( name, "]", "" );
  this->name_ = name;
}

double Measurement::get_length() const
{
  return ( this->points_[ 1 ] - this->points_[ 0 ] ).length();
}

void Measurement::set_length( double length )
{
  if( length > 0.0 ) // Don't want to lose the vector
  {
    Vector unit_vector = ( this->points_[ 1 ] - this->points_[ 0 ] );
    unit_vector.normalize();
    this->points_[ 1 ] = this->points_[ 0 ] + ( length * unit_vector );
  }
}

std::string Measurement::get_comment() const
{
  return this->comment_;
}

void Measurement::set_comment( std::string comment )
{
  // Strip brackets 
  boost::algorithm::replace_all( comment, "[", "" );
  boost::algorithm::replace_all( comment, "]", "" );
  this->comment_ = comment;
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

void Measurement::get_color( Color& color ) const
{
  color = this->color_;
}

void Measurement::set_color( const Color& color )
{
  this->color_ = color;
}

std::string ExportToString( const Measurement& value )
{
  Point p0;
  value.get_point( 0, p0 );
  Point p1;
  value.get_point( 1, p1 );
  Color color;
  value.get_color( color );
  return ( std::string( 1, '[' ) + 
    ExportToString( value.get_visible() ) + ' ' + 
    ExportToString( value.get_name() ) + ' ' + 
    ExportToString( p0 ) + ' ' + 
    ExportToString( p1 ) + ' ' + 
    ExportToString( color ) + ' ' + 
    ExportToString( value.get_comment() ) + 
    ']' );
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
  bool success = false;
  bool visible = false;
  std::string name;
  Point p0, p1;
  Color color;
  std::string comment;

  // Backwards compatibility -- old code used comment delimiter
  if( boost::algorithm::find_first( str, Measurement::COMMENT_DELIMITER_C ) ) 
  {
    // Example: [true M1 [0 0 0] [1 1 1] [1 0 0] [Knee COMMENT_END]]

    std::string open_bracket_reg = "(\\[)";
    std::string word_reg = "(\\w*)";
    std::string visible_reg = word_reg;
    std::string space_reg = "(\\s)";
    std::string name_reg = word_reg;
    std::string point_reg = "(\\[[^\\]]*])";
    std::string color_reg = point_reg;
    std::string comment_reg = "(.*?)";
    std::string comment_end_reg = "(\\sCOMMENT_END\\]\\])";

    std::string full_reg = open_bracket_reg + visible_reg + space_reg + name_reg + space_reg + 
      point_reg + space_reg + point_reg + space_reg + color_reg + space_reg + open_bracket_reg + 
      comment_reg + comment_end_reg;

    boost::regex reg( full_reg );
    boost::smatch m;
    if( boost::regex_match( str, m, reg ) ) 
    {
      ImportFromString( m[ 2 ].str(), visible );
      name = m[ 4 ].str();
      ImportFromString( m[ 6 ].str(), p0 );
      ImportFromString( m[ 8 ].str(), p1 );
      ImportFromString( m[ 10 ].str(), color );
      comment = m[ 13 ].str();
      success = true;
    }
  }
  else // New code
  {
    // NOTE: New code allows any characters except bracket in measurement name, including empty string
    // Example: [true M1 [0 0 0] [1 1 1] [1 0 0] Knee]
    // Example: [true [Name with spaces] [0 0 0] [1 1 1] [1 0 0] [Comment with spaces and "quotes"]]
    // Example: [true [] [0 0 0] [1 1 1] [1 0 0] []]

    std::vector< std::string > parts = SplitStringByBracketsThenSpaces( str );
    if( parts.size() == 6 )
    {
      if( !ImportFromString( parts[ 0 ], visible ) ) return false;
      name = parts[ 1 ];
      if( !ImportFromString( parts[ 2 ], p0 ) ) return false;
      if( !ImportFromString( parts[ 3 ], p1 ) ) return false;
      if( !ImportFromString( parts[ 4 ], color ) ) return false;;
      comment = parts[ 5 ];
      success = true;
    }
  }

  if( success )
  {
    value.set_visible( visible );
    value.set_name( name );
    value.set_point( 0, p0 );
    value.set_point( 1, p1 );
    value.set_color( color );
    value.set_comment( comment );
    return true;
  }
  return false;
}

bool ImportFromString( const std::string& str, std::vector< Measurement >& value )
{
  value.clear();

  // Backwards compatibility -- old code used comment delimiter
  if( boost::algorithm::find_first( str, Measurement::COMMENT_DELIMITER_C ) ) 
  {
    std::vector< std::string > parts = SplitString( str, Measurement::COMMENT_DELIMITER_C );
    // The last part is ""
    for( size_t i = 0; i < parts.size() - 1; i++ )
    {
      std::string measurement_string = parts[ i ];
      measurement_string += Measurement::COMMENT_DELIMITER_C;
      Measurement measurement;
      if( ImportFromString( measurement_string, measurement ) )
      {
        value.push_back( measurement );
      }
    }
  }
  else // New code
  {
    // Break string into individual measurement strings
    std::vector< std::string > parts = SplitStringByBracketsThenSpaces( str );
    for( size_t i = 0; i < parts.size(); i++ )
    {
      std::string measurement_string = parts[ i ];
      Measurement measurement;
      if( ImportFromString( measurement_string, measurement ) )
      {
        value.push_back( measurement );
      }
    }
  }

  return true;
}

} // end namespace Core
