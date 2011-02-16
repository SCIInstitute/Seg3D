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

Measurement::Measurement( bool visible, std::string label, std::string note, 
  Core::Point p1, Core::Point p2, MeasureSliceType view_axis ) : 
  visible_( visible ), 
  label_( label ), 
  note_( note ), 
  p1_( p1 ), 
  p2_( p2 ), 
  slice_type_( view_axis )
{
}

Measurement::Measurement() :
  visible_( false ), 
  label_( "" ), 
  note_( "" ), 
  slice_type_( MeasureSliceType::NOVIEW_E )
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

std::string Measurement::get_label() const
{
  return this->label_;
}

void Measurement::set_label( std::string label )
{
  this->label_ = label;
}

double Measurement::get_length() const
{
  return ( this->p2_ - this->p1_ ).length();
}

std::string Measurement::get_note() const
{
  return this->note_;
}

void Measurement::set_note( std::string note )
{
  this->note_ = note;
}

Core::Point Measurement::get_point1() const
{
  return this->p1_;
}

void Measurement::set_point1( Core::Point p1 )
{
  this->p1_ = p1;
}

Core::Point Measurement::get_point2() const
{
  return this->p2_;
}

void Measurement::set_point2( Core::Point p2 )
{
  this->p2_ = p2;
}

MeasureSliceType Measurement::get_slice_type() const
{
  return this->slice_type_;
}

void Measurement::set_slice_type( MeasureSliceType view_axis )
{
  this->slice_type_ = view_axis;
}

std::string ExportToString( const Measurement& value )
{
  // Need to use special delimiter for note since any characters are allowed in a note,
  // including ']'.  Put note at end so that it functions as both a note delimiter and 
  // measurement delimiter.
  return ( std::string( 1, '[' ) + ExportToString( value.get_visible() ) + ' ' + value.get_label() 
    + ' ' + ExportToString( value.get_point1() ) + ' ' + ExportToString( value.get_point2() ) + 
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
  std::string matched_string = "";
  if( boost::regex_match( str, m, reg ) ) 
  {
    bool visible = false;
    ImportFromString( m[ 2 ].str(), visible );
    value.set_visible( visible );
    value.set_label( m[ 4 ].str() );
    Point p1;
    ImportFromString( m[ 6 ].str(), p1 );
    value.set_point1( p1 );
    Point p2;
    ImportFromString( m[ 8 ].str(), p2 );
    value.set_point2( p2 );
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