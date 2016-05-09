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
#include <Core/Geometry/Vector.h>

namespace Core
{

bool Vector::find_orthogonal( Vector& v1, Vector& v2 ) const
{
  Vector v0( Cross( *this, Vector( 1.0, 0.0, 0.0 ) ) );
  if ( v0.length2() == 0 )
  {
    v0 = Cross( *this, Vector( 0.0, 1.0, 0.0 ) );
  }
  v1 = Cross( *this, v0 );
  if ( v1.normalize() == 0.0 ) return ( false );
  v2 = Cross( *this, v1 );
  if ( v2.normalize() == 0.0 ) return ( false );
  return ( true );
}

bool VectorF::find_orthogonal( VectorF& v1, VectorF& v2 ) const
{
  VectorF v0( Cross( *this, VectorF( 1.0f, 0.0f, 0.0f ) ) );
  if ( v0.length2() == 0 )
  {
    v0 = Cross( *this, VectorF( 0.0f, 1.0f, 0.0f ) );
  }
  v1 = Cross( *this, v0 );
  if ( v1.normalize() == 0.0f ) return ( false );
  v2 = Cross( *this, v1 );
  if ( v2.normalize() == 0.0f ) return ( false );
  return ( true );
}

std::ostream& operator<<( std::ostream& os, const Vector& v )
{
  os << '[' << v.x() << ' ' << v.y() << ' ' << v.z() << ']';
  return os;
}

std::ostream& operator<<( std::ostream& os, const VectorF& v )
{
  os << '[' << v.x() << ' ' << v.y() << ' ' << v.z() << ']';
  return os;
}

std::string ExportToString( const Vector& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.x() ) + ',' 
    + ExportToString( value.y() ) + ','
      + ExportToString( value.z() ) + ']' );
}

std::string ExportToString( const VectorF& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.x() ) + ',' 
    + ExportToString( value.y() ) + ','
      + ExportToString( value.z() ) + ']' );
}

std::string ExportToString( const std::vector< Vector >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ExportToString( value[ j ] ) + ',';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString( const std::vector< VectorF >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ExportToString( value[ j ] ) + ',';
  result[ result.size() - 1 ] = ']';
  return result;
}

bool ImportFromString( const std::string& str, Vector& value )
{
  std::vector< double > values;
  ImportFromString( str, values );
  if ( values.size() == 3 )
  {
    value = Vector( values[ 0 ], values[ 1 ], values[ 2 ] );
    return true;
  }
  return false;
}

bool ImportFromString( const std::string& str, VectorF& value )
{
  std::vector< float > values;
  ImportFromString( str, values );
  if ( values.size() == 3 )
  {
    value = VectorF( values[ 0 ], values[ 1 ], values[ 2 ] );
    return true;
  }
  return false;
}

bool ImportFromString( const std::string& str, std::vector< Vector >& value )
{
  std::vector< double > values;
  ImportFromString( str, values );

  size_t num_values = values.size() / 3;
  if ( values.size() == num_values * 3 )
  {
    for ( size_t j = 0; j < num_values; j++ )
    {
      size_t offset = j * 3;
      value[ j ] = Vector( values[ offset + 0 ], values[ offset + 1 ], values[ offset + 2 ] );
    }
    return true;
  }
  return false;
}

bool ImportFromString( const std::string& str, std::vector< VectorF >& value )
{
  std::vector< float > values;
  ImportFromString( str, values );

  size_t num_values = values.size() / 3;
  if ( values.size() == num_values * 3 )
  {
    for ( size_t j = 0; j < num_values; j++ )
    {
      size_t offset = j * 3;
      value[ j ] = VectorF( values[ offset + 0 ], values[ offset + 1 ], values[ offset + 2 ] );
    }
    return true;
  }
  return false;
}

} // End namespace Core
