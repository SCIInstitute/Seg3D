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

// STL includes
#include <sstream>

// Core includes
#include <Core/Geometry/Point.h>

namespace Core
{

PointF AffineCombination( const PointF& p1, float w1, const PointF& p2, float w2 )
{
  return PointF( p1.data_[ 0 ] * w1 + p2.data_[ 0 ] * w2,
      p1.data_[ 1 ] * w1 + p2.data_[ 1 ] * w2, p1.data_[ 2 ] * w1 + p2.data_[ 2 ] * w2 );
}

PointF AffineCombination( const PointF& p1, float w1, const PointF& p2, float w2, const PointF& p3,
    float w3 )
{
  return PointF( p1.data_[ 0 ] * w1 + p2.data_[ 0 ] * w2 + p3.data_[ 0 ] * w3, p1.data_[ 1 ] * w1
      + p2.data_[ 1 ] * w2 + p3.data_[ 1 ] * w3, p1.data_[ 2 ] * w1 + p2.data_[ 2 ] * w2
      + p3.data_[ 2 ] * w3 );
}

PointF AffineCombination( const PointF& p1, float w1, const PointF& p2, float w2, const PointF& p3,
    float w3, const PointF& p4, float w4 )
{
  return PointF( p1.data_[ 0 ] * w1 + p2.data_[ 0 ] * w2 + p3.data_[ 0 ] * w3 + p4.data_[ 0 ]
      * w4, p1.data_[ 1 ] * w1 + p2.data_[ 1 ] * w2 + p3.data_[ 1 ] * w3 + p4.data_[ 1 ] * w4,
      p1.data_[ 2 ] * w1 + p2.data_[ 2 ] * w2 + p3.data_[ 2 ] * w3 + p4.data_[ 2 ] * w4 );
}

Point AffineCombination( const Point& p1, double w1, const Point& p2, double w2 )
{
  return Point( p1.data_[ 0 ] * w1 + p2.data_[ 0 ] * w2, p1.data_[ 1 ] * w1 + p2.data_[ 1 ] * w2,
      p1.data_[ 2 ] * w1 + p2.data_[ 2 ] * w2 );
}

Point AffineCombination( const Point& p1, double w1, const Point& p2, double w2, const Point& p3,
    double w3 )
{
  return Point( p1.data_[ 0 ] * w1 + p2.data_[ 0 ] * w2 + p3.data_[ 0 ] * w3, p1.data_[ 1 ] * w1
      + p2.data_[ 1 ] * w2 + p3.data_[ 1 ] * w3, p1.data_[ 2 ] * w1 + p2.data_[ 2 ] * w2
      + p3.data_[ 2 ] * w3 );
}

Point AffineCombination( const Point& p1, double w1, const Point& p2, double w2, const Point& p3,
    double w3, const Point& p4, double w4 )
{
  return Point(
      p1.data_[ 0 ] * w1 + p2.data_[ 0 ] * w2 + p3.data_[ 0 ] * w3 + p4.data_[ 0 ] * w4,
      p1.data_[ 1 ] * w1 + p2.data_[ 1 ] * w2 + p3.data_[ 1 ] * w3 + p4.data_[ 1 ] * w4,
      p1.data_[ 2 ] * w1 + p2.data_[ 2 ] * w2 + p3.data_[ 2 ] * w3 + p4.data_[ 2 ] * w4 );
}

std::ostream& operator<<( std::ostream& os, const Point& p )
{
  os << '[' << p.x() << ' ' << p.y() << ' ' << p.z() << ']';
  return os;
}

std::ostream& operator<<( std::ostream& os, const PointF& p )
{
  os << '[' << p.x() << ' ' << p.y() << ' ' << p.z() << ']';
  return os;
}

bool Point::Overlap( double a, double b, double e )
{
  return ( ( a + e > b - e ) && ( a - e < b + e ) );
}

bool Point::InInterval( Point a, double epsilon )
{
  return ( Overlap( data_[ 0 ], a.x(), epsilon ) && Overlap( data_[ 1 ], a.y(), epsilon )
      && Overlap( data_[ 2 ], a.z(), epsilon ) );
}

bool PointF::Overlap( float a, float b, float e )
{
  return ( ( a + e > b - e ) && ( a - e < b + e ) );
}

bool PointF::InInterval( PointF a, float epsilon )
{
  return ( Overlap( data_[ 0 ], a.x(), epsilon ) && Overlap( data_[ 1 ], a.y(), epsilon )
      && Overlap( data_[ 2 ], a.z(), epsilon ) );
}


std::string ExportToString( const Point& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.x() ) + ',' 
    + ExportToString( value.y() ) + ','
      + ExportToString( value.z() ) + ']' );
}

std::string ExportToString( const PointF& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.x() ) + ',' 
    + ExportToString( value.y() ) + ','
      + ExportToString( value.z() ) + ']' );
}

std::string ExportToString( const std::vector< Point >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ExportToString( value[ j ] ) + ',';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString( const std::vector< PointF >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ExportToString( value[ j ] ) + ',';
  result[ result.size() - 1 ] = ']';
  return result;
}

bool ImportFromString( const std::string& str, Point& value )
{
  std::vector< double > values;
  ImportFromString( str, values );
  if ( values.size() == 3 )
  {
    value = Point( values[ 0 ], values[ 1 ], values[ 2 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, PointF& value )
{
  std::vector< float > values;
  ImportFromString( str, values );
  if ( values.size() == 3 )
  {
    value = PointF( values[ 0 ], values[ 1 ], values[ 2 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, std::vector< Point >& value )
{
  std::vector< double > values;
  ImportFromString( str, values );

  size_t num_values = values.size() / 3;
  if ( values.size() == num_values * 3 )
  {
    value.resize( num_values );
    for ( size_t j = 0; j < num_values; j++ )
    {
      size_t offset = j * 3;
      value[ j ] = Point( values[ offset + 0 ], values[ offset + 1 ], values[ offset + 2 ] );
    }
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, std::vector< PointF >& value )
{
  std::vector< float > values;
  ImportFromString( str, values );

  size_t num_values = values.size() / 3;
  if ( values.size() == num_values * 3 )
  {
    value.resize( num_values );
    for ( size_t j = 0; j < num_values; j++ )
    {
      size_t offset = j * 3;
      value[ j ] = PointF( values[ offset + 0 ], values[ offset + 1 ], values[ offset + 2 ] );
    }
    return ( true );
  }
  return ( false );
}

} // End namespace SCIRun
