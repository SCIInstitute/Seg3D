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
  boost::regex reg( "(\\[)(\\w*)(\\s)(\\w*)(\\s)(\\[[^\\]]*])(\\s)(\\[[^\\]]*])(\\s)(\\[)(.*?)(\\sNOTE_END\\]\\])" );
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