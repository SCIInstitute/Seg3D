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

#include <Core/Utils/StringUtil.h>
#include <Core/Converter/StringConverter.h>

namespace Core
{

std::string ExportToString( const bool& value )
{
  if ( value ) return ( std::string( "true" ) );
  else return ( std::string( "false" ) );
}

std::string ExportToString( const char& value )
{
  return ToString( value );
}

std::string ExportToString(const unsigned char& value)
{
  return ToString(value);
}

std::string ExportToString( const short& value )
{
  return ToString( value );
}

std::string ExportToString(const unsigned short& value)
{
  return ToString(value);
}

std::string ExportToString( const int& value )
{
  return ToString( value );
}

std::string ExportToString(const unsigned int& value)
{
  return ToString(value);
}

std::string ExportToString( const long& value )
{
  return ToString( value );
}

std::string ExportToString(const unsigned long& value)
{
  return ToString(value);
}

std::string ExportToString( const long long& value )
{
  return ToString( value );
}

std::string ExportToString( const unsigned long long& value )
{
  return ToString( value );
}

std::string ExportToString( const float& value )
{
  return ToString( value );
}

std::string ExportToString( const double& value )
{
  return ToString( value );
}

std::string ExportToString( const View3D& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.eyep() ) + ' ' + ExportToString(
      value.lookat() ) + ' ' + ExportToString( value.up() ) + ' ' + ExportToString(
      value.fov() ) + ']' );
}

std::string ExportToString( const View2D& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.center() ) + ' ' + ExportToString(
      value.scalex() ) + ' ' + ExportToString( value.scaley() ) + ']' );
}

std::string ExportToString( const Point& value )
{
  return ( std::string( 1, '[' ) + ToString( value.x() ) + ' ' + ToString( value.y() ) + ' '
      + ToString( value.z() ) + ']' );
}

std::string ExportToString( const Vector& value )
{
  return ( std::string( 1, '[' ) + ToString( value.x() ) + ' ' + ToString( value.y() ) + ' '
      + ToString( value.z() ) + ']' );
}

std::string ExportToString( const Color& value )
{
  return ( std::string( 1, '[' ) + ToString( value.r() ) + ' ' + ToString( value.g() ) + ' '
      + ToString( value.b() ) + ']' );
}

std::string ExportToString( const BBox& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.min() ) + ' ' + ExportToString(
      value.max() ) + ']' );
}

std::string ExportToString( const std::vector< char >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ToString( value[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString(const std::vector<unsigned char>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += ToString(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string ExportToString( const std::vector< short >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ToString( value[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString(const std::vector<unsigned short>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += ToString(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string ExportToString( const std::vector< int >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ToString( value[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString(const std::vector<unsigned int>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += ToString(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string ExportToString(const std::vector<long long>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += ToString(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string ExportToString(const std::vector<unsigned long long>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += ToString(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string ExportToString( const std::vector< float >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ToString( value[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString( const std::vector< double >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ToString( value[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString( const std::vector< Point >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ExportToString( value[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString( const std::vector< Vector >& value )
{
  std::string result( 1, '[' );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ExportToString( value[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return result;
}

std::string ExportToString( const std::string& value )
{
  bool need_quotes = false;
  for ( size_t j = 0; j < value.size(); j++)
  {
    if ( value[j] == ' ' || value[j] == '\t' || value[j] == '[' || value[j] == ']' ||
      value[j] == '(' || value[j] == ')' || value[j] == ',' ) need_quotes = true;
  }
  if ( need_quotes ) return std::string("\"") + value + std::string("\"");
  else return value;
}

std::string ExportToString( const Transform& value )
{
  std::string result( 1, '[' );
  std::vector< double > trans( 16 );
  value.get( &trans[ 0 ] );
  for ( size_t j = 0; j < 16; j++ )
    result += ToString( trans[ j ] ) + ' ';
  result[ result.size() - 1 ] = ']';
  return ( result );
}

std::string ExportToString( const GridTransform& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.get_nx() ) + ' ' + ExportToString(
      value.get_ny() ) + ' ' + ExportToString( value.get_nz() ) + ' ' + ExportToString(
      value.transform() ) + ']' );
}

std::string ExportToString( const Plane& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.normal() ) + ' ' + ExportToString(
      value.distance() ) + ']' );
}

std::string ExportToString( const Quaternion& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.w() ) + ' ' + ExportToString(
      value.x() ) + ' ' + ExportToString( value.y() ) + ' ' + ExportToString( value.z() )
      + " ]" );
}

bool ImportFromString( const std::string& str, bool& value )
{
  std::string tmpstr( str );
  StripSurroundingSpaces( tmpstr );
  tmpstr = StringToLower( tmpstr );
  if ( ( tmpstr == "0" ) || ( tmpstr == "false" ) || ( tmpstr == "off" ) )
  {
    value = false;
    return ( true );
  }
  else if ( ( tmpstr == "1" ) || ( tmpstr == "true" ) || ( tmpstr == "on" ) )
  {
    value = true;
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, char& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString(const std::string& str, unsigned char& value)
{
  return (FromString(str,value));
}

bool ImportFromString( const std::string& str, short& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString(const std::string& str, unsigned short& value)
{
  return (FromString(str,value));
}

bool ImportFromString( const std::string& str, int& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString(const std::string& str, unsigned int& value)
{
  return (FromString(str,value));
}

bool ImportFromString(const std::string& str, long long& value)
{
  return (FromString(str,value));
}

bool ImportFromString(const std::string& str, unsigned long long& value)
{
  return (FromString(str,value));
}

bool ImportFromString( const std::string& str, float& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, double& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, Point& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 3 )
  {
    value = Point( values[ 0 ], values[ 1 ], values[ 2 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, Quaternion& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 4 )
  {
    value = Quaternion( values[ 0 ], values[ 1 ], values[ 2 ], values[ 3 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, View3D& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 10 )
  {
    value
        = View3D( Point( values[ 0 ], values[ 1 ], values[ 2 ] ), Point( values[ 3 ],
            values[ 4 ], values[ 5 ] ), Vector( values[ 6 ], values[ 7 ], values[ 8 ] ),
            values[ 9 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, View2D& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 16 )
  {
    value = View2D( Point( values[ 0 ], values[ 1 ], values[ 2 ] ), values[ 3 ], values[ 4 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, Vector& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 3 )
  {
    value = Vector( values[ 0 ], values[ 1 ], values[ 2 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, Color& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 3 )
  {
    value = Color( values[ 0 ], values[ 1 ], values[ 2 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, std::vector< char >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString(const std::string& str, std::vector<unsigned char>& value)
{
  return (MultipleFromString(str,value));
}

bool ImportFromString( const std::string& str, std::vector< short >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString(const std::string& str, std::vector<unsigned short>& value)
{
  return (MultipleFromString(str,value));
}

bool ImportFromString( const std::string& str, std::vector< int >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString(const std::string& str, std::vector<unsigned int>& value)
{
  return (MultipleFromString(str,value));
}

bool ImportFromString(const std::string& str, std::vector<long long>& value)
{
  return (MultipleFromString(str,value));
}

bool ImportFromString(const std::string& str, std::vector<unsigned long long>& value)
{
  return (MultipleFromString(str,value));
}

bool ImportFromString( const std::string& str, std::vector< float >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< double >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::string& value )
{
  value = str;
  return ( true );
}

bool ImportFromString( const std::string& str, std::vector< Point >& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );

  size_t num_values = values.size() / 3;
  if ( values.size() == num_values * 3 )
  {
    for ( size_t j = 0; j < num_values; j++ )
    {
      size_t offset = j * 3;
      value[ j ] = Point( values[ offset + 0 ], values[ offset + 1 ], values[ offset + 2 ] );
    }
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, std::vector< Vector >& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );

  size_t num_values = values.size() / 3;
  if ( values.size() == num_values * 3 )
  {
    for ( size_t j = 0; j < num_values; j++ )
    {
      size_t offset = j * 3;
      value[ j ] = Vector( values[ offset + 0 ], values[ offset + 1 ], values[ offset + 2 ] );
    }
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, BBox& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 6 )
  {
    value = BBox( Point( values[ 0 ], values[ 1 ], values[ 2 ] ), Point( values[ 3 ],
        values[ 4 ], values[ 5 ] ) );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, Transform& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 16 )
  {
    value.set( &values[ 0 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, GridTransform& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 19 )
  {
    value.set_nx( static_cast< size_t > ( values[ 0 ] ) );
    value.set_ny( static_cast< size_t > ( values[ 1 ] ) );
    value.set_nz( static_cast< size_t > ( values[ 2 ] ) );
    value.transform().set( &values[ 3 ] );
    return ( true );
  }
  return ( false );
}

bool ImportFromString( const std::string& str, Plane& value )
{
  std::vector< double > values;
  MultipleFromString( str, values );
  if ( values.size() == 4 )
  {
    value = Plane( Vector( values[ 0 ], values[ 1 ], values[ 2 ] ), values[ 3 ] );
    return ( true );
  }
  return ( false );
}

} // end namesapce Core
