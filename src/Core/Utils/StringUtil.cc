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
#include <stdlib.h>
#include <limits>
#include <sstream>
#include <fstream>
#include <iomanip>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Math/MathFunctions.h>

namespace Core
{

// Convert multiple values in a string into a vector with numbers

// Convert a value into a string

template< class T >
bool FromString( const std::string &str, T &value )
{
    std::string data = str + " ";
    for ( size_t j = 0; j < data.size(); j++ )
        if ( ( data[ j ] == '\t' ) || ( data[ j ] == '\r' ) || ( data[ j ] == '\n' ) || ( data[ j ]
                                                                                         == '"' ) || ( data[ j ] == ',' ) || ( data[ j ] == '[' ) || ( data[ j ] == ']' )
            || ( data[ j ] == '(' ) || ( data[ j ] == ')' ) ) data[ j ] = ' ';
    
    std::istringstream iss( data );
    iss.exceptions( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
    try
    {
        iss >> value;
        return ( true );
    }
    catch ( ... )
    {
        return ( false );
    }
}    
    
template< class T >
bool MultipleFromString( const std::string &str, std::vector< T > &values )
{
  values.clear();

  // Clear out any markup of the numbers that make it easier to read and
  // replace it all with spaces.
  std::string data = str;
  for ( size_t j = 0; j < data.size(); j++ )
    if ( ( data[ j ] == '\t' ) || ( data[ j ] == '\r' ) || ( data[ j ] == '\n' ) || ( data[ j ]
        == '"' ) || ( data[ j ] == ',' ) || ( data[ j ] == '[' ) || ( data[ j ] == ']' )
        || ( data[ j ] == '(' ) || ( data[ j ] == ')' ) ) data[ j ] = ' ';

  // Loop over the data and extract all numbers from it.
  for ( size_t p = 0; p < data.size(); )
  {
    // find where the number starts
    while ( ( p < data.size() ) && ( data[ p ] == ' ' ) )
      p++;
    // Exit if we are at the end of the series
    if ( p >= data.size() ) break;

    // strip of the next number
    std::string::size_type next_space = data.find( ' ', p );
    if ( next_space == std::string::npos ) next_space = data.size();

    // Extract the number
    T value;
    if ( FromString<T>( data.substr( p, next_space - p ), value ) ) values.push_back( value );
    p = next_space;

    if ( p >= data.size() ) break;
  }

  // If no numbers were extracted return false
  if ( values.size() > 0 ) return ( true );
  return ( false );
}



// Export a value to a string
template< class T >
std::string ToString( T val )
{
  std::ostringstream oss;
  oss << val;
  return ( oss.str() );
}

inline std::string ToString( float val )
{
  if ( IsNan( val ) ) return "NaN";
  if ( IsInfinite( val ) )
  {
    if ( val > 0 ) return "INF";
    return "-INF";
  }
  
  std::ostringstream oss;
  oss << val;
  return oss.str();
}

inline std::string ToString( double val )
{
  if ( IsNan( val ) ) return "NaN";
  if ( IsInfinite( val ) )
  {
    if ( val > 0 ) return "INF";
    return "-INF";
  }

  std::ostringstream oss;
  oss << val;
  return oss.str();
}

inline std::string ToString( const std::string& value )
{
  return ExportToString( value );
}

// Export a value to a string with precision control

template< class T >
std::string ToString( T val, int precision )
{
  std::ostringstream oss;
  oss.precision( precision );
  oss << val;
  return ( oss.str() );
}


inline std::string ToString( unsigned char val, int precision )
{
  std::ostringstream oss;
  
  oss.precision( precision );
  oss << std::right << std::setfill( '0' ) << std::setw( precision ) << val;
  return ( oss.str() );
}


inline std::string ToString( unsigned short val, int precision )
{
  std::ostringstream oss;
  
  oss.precision( precision );
  oss << std::right << std::setfill( '0' ) << std::setw( precision ) << val;
  return ( oss.str() );
}


inline std::string ToString( unsigned int val, int precision )
{
  std::ostringstream oss;
  
  oss.precision( precision );
  oss << std::right << std::setfill( '0' ) << std::setw( precision ) << val;
  return ( oss.str() );
}


inline std::string ToString( unsigned long long val, int precision )
{
  std::ostringstream oss;
  
  oss.precision( precision );
  oss << std::right << std::setfill( '0' ) << std::setw( precision ) << val;
  return ( oss.str() );
}



inline std::string ToString( float val, int precision )
{
  if ( IsNan( val ) ) return "NaN";
  if ( IsInfinite( val ) )
  {
    if ( val > 0 ) return "INF";
    return "-INF";
  }

  std::ostringstream oss;
  oss.precision( precision );
  oss << val;
  return oss.str();
}

inline std::string ToString( double val, int precision )
{
  if ( IsNan( val ) ) return "NaN";
  if ( IsInfinite( val ) )
  {
    if ( val > 0 ) return "INF";
    return "-INF";
  }

  std::ostringstream oss;
  oss.precision( precision );
  oss << val;
  return oss.str();
}

inline std::string ToString( double val, size_t digits )
{
  if ( IsNan( val ) ) return "NaN";
  if ( IsInfinite( val ) )
  {
    if ( val > 0 ) return "INF";
    return "-INF";
  }
  
  std::ostringstream oss;
  
  oss << std::fixed << std::setprecision( digits ) << val;
  return oss.str();
}

template< class CONTAINER_TYPE >
std::string MultipleToString( const CONTAINER_TYPE& values )
{
  typename CONTAINER_TYPE::const_iterator it = values.begin();
  typename CONTAINER_TYPE::const_iterator it_end = values.end();
  std::string result( "[" );
  while ( it != it_end )
  {
    result += ToString( *it ) + ',';
    ++it;
  }
  if ( result.size() > 1 )
  {
    result[ result.size() - 1 ] = ']';
  }
  else
  {
    result += "]";
  }
  return result;
}

std::string StringToUpper( std::string str )
{
  std::string::iterator iter = str.begin();
  std::string::iterator iend = str.end();
  for ( ; iter != iend; ++iter )
    *iter = toupper( *iter );
  return str;
}

std::string StringToLower( std::string str )
{
  std::string::iterator iter = str.begin();
  std::string::iterator iend = str.end();
  for ( ; iter != iend; ++iter )
    *iter = tolower( *iter );
  return str;
}

bool FromString( const std::string &str, unsigned int &value )
{
  // Clear out any markup of the numbers that make it easier to read and
  // replace it all with spaces.
  std::string data = str + " ";
  for ( size_t j = 0; j < data.size(); j++ )
    if ( ( data[ j ] == '\t' ) || ( data[ j ] == '\r' ) || ( data[ j ] == '\n' ) || ( data[ j ]
          == '"' ) || ( data[ j ] == ',' ) || ( data[ j ] == '[' ) || ( data[ j ] == ']' )
        || ( data[ j ] == '(' ) || ( data[ j ] == ')' ) ) data[ j ] = ' ';
  
  // if empty just return
  if ( data.size() == 0 ) return ( false );
  
  // Handle special cases: max, min

  // handle max
  if ( data.size() > 2 && ( data[ 0 ] == 'm' || data[ 0 ] == 'M' ) && ( data[ 1 ] == 'a'
    || data[ 1 ] == 'A' ) && ( data[ 2 ] == 'x' || data[ 2 ] == 'X' ) )
  {
    value = std::numeric_limits< double >::max();
    return ( true );
  }
  // handle min
  else if ( data.size() > 2 && ( data[ 0 ] == 'm' || data[ 0 ] == 'M' ) && ( data[ 1 ] == 'i'
    || data[ 1 ] == 'I' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) )
  {
    value = std::numeric_limits< double >::min();
    return ( true );
  }

  std::istringstream iss( data );
  iss.exceptions( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
  try
  {
    iss >> value;
    return ( true );
  }
  catch ( std::istringstream::failure e )
  {
    CORE_LOG_DEBUG(e.what());
    return ( false );
  }
}
  
bool FromString( const std::string &str, double &value )
{
  // Clear out any markup of the numbers that make it easier to read and
  // replace it all with spaces.
  std::string data = str + " ";
  for ( size_t j = 0; j < data.size(); j++ )
    if ( ( data[ j ] == '\t' ) || ( data[ j ] == '\r' ) || ( data[ j ] == '\n' ) || ( data[ j ]
        == '"' ) || ( data[ j ] == ',' ) || ( data[ j ] == '[' ) || ( data[ j ] == ']' )
        || ( data[ j ] == '(' ) || ( data[ j ] == ')' ) ) data[ j ] = ' ';

  // if empty just return
  if ( data.size() == 0 ) return ( false );

  // Handle special cases: nan, max, min, inf, and -inf

  // handle nan
  if ( data.size() > 2 && ( data[ 0 ] == 'n' || data[ 0 ] == 'N' ) && ( data[ 1 ] == 'a'
      || data[ 1 ] == 'A' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) )
  {
    value = std::numeric_limits< double >::quiet_NaN();
    return ( true );
  }
  // handle max
  else if ( data.size() > 2 && ( data[ 0 ] == 'm' || data[ 0 ] == 'M' ) && ( data[ 1 ] == 'a'
    || data[ 1 ] == 'A' ) && ( data[ 2 ] == 'x' || data[ 2 ] == 'X' ) )
  {
    value = std::numeric_limits< double >::max();
    return ( true );
  }
  // handle min
  else if ( data.size() > 2 && ( data[ 0 ] == 'm' || data[ 0 ] == 'M' ) && ( data[ 1 ] == 'i'
    || data[ 1 ] == 'I' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) )
  {
    value = std::numeric_limits< double >::min();
    return ( true );
  }
  // handle inf
  else if ( data.size() > 2 && ( data[ 0 ] == 'i' || data[ 0 ] == 'I' ) && ( data[ 1 ] == 'n'
      || data[ 1 ] == 'N' ) && ( data[ 2 ] == 'f' || data[ 2 ] == 'F' ) )
  {
    value = std::numeric_limits< double >::infinity();
    return ( true );
  }
  // handle +inf and -inf
  else if ( data.size() > 3 && ( data[ 0 ] == '-' || data[ 0 ] == '+' ) && ( data[ 1 ] == 'i'
      || data[ 1 ] == 'I' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) && ( data[ 3 ] == 'f'
      || data[ 3 ] == 'F' ) )
  {
    if ( data[ 0 ] == '-' )
    {
      value = -std::numeric_limits< double >::infinity();
    }
    else
    {
      value = std::numeric_limits< double >::infinity();
    }

    return ( true );
  }

  // NOTE: On MAC this value does not convert properly even though it can write it out.
  // Hence fix this by special casing this

  if ( data.size() > 11 && data.substr( 0, 12 ) == "2.22507e-308" )
  {
    value = 2.22507e-308;
    return true;
  }
  

  std::istringstream iss( data );
  iss.exceptions( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
  try
  {
    iss >> value;
    return ( true );
  }
  catch ( std::istringstream::failure e )
  {
    CORE_LOG_DEBUG(e.what());
    return ( false );
  }
}

bool FromString( const std::string &str, float &value )
{
  // Clear out any markup of the numbers that make it easier to read and
  // replace it all with spaces.
  std::string data = str + " ";
  for ( size_t j = 0; j < data.size(); j++ )
    if ( ( data[ j ] == '\t' ) || ( data[ j ] == '\r' ) || ( data[ j ] == '\n' ) || ( data[ j ]
        == '"' ) || ( data[ j ] == ',' ) || ( data[ j ] == '[' ) || ( data[ j ] == ']' )
        || ( data[ j ] == '(' ) || ( data[ j ] == ')' ) ) data[ j ] = ' ';

  // if empty just return
  if ( data.size() == 0 ) return ( false );

  // Handle special cases: nan, inf, and -inf
  // Handle special cases: nan, inf, and -inf

  // handle nan
  if ( data.size() > 2 && ( data[ 0 ] == 'n' || data[ 0 ] == 'N' ) && ( data[ 1 ] == 'a'
      || data[ 1 ] == 'A' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) )
  {
    value = std::numeric_limits< float >::quiet_NaN();
    return ( true );
  }
  // handle max
  else if ( data.size() > 2 && ( data[ 0 ] == 'm' || data[ 0 ] == 'M' ) && ( data[ 1 ] == 'a'
                                                                            || data[ 1 ] == 'A' ) && ( data[ 2 ] == 'x' || data[ 2 ] == 'X' ) )
  {
    value = std::numeric_limits< float >::max();
    return ( true );
  }
  // handle min
  else if ( data.size() > 2 && ( data[ 0 ] == 'm' || data[ 0 ] == 'M' ) && ( data[ 1 ] == 'i'
                                                                            || data[ 1 ] == 'I' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) )
  {
    value = std::numeric_limits< float >::min();
    return ( true );
  }
  // handle inf
  else if ( data.size() > 2 && ( data[ 0 ] == 'i' || data[ 0 ] == 'I' ) && ( data[ 1 ] == 'n'
      || data[ 1 ] == 'N' ) && ( data[ 2 ] == 'f' || data[ 2 ] == 'F' ) )
  {
    value = std::numeric_limits< float >::infinity();
    return ( true );
  }
  // handle +inf and -inf
  else if ( data.size() > 3 && ( data[ 0 ] == '-' || data[ 0 ] == '+' ) && ( data[ 1 ] == 'i'
      || data[ 1 ] == 'I' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) && ( data[ 3 ] == 'f'
      || data[ 3 ] == 'F' ) )
  {
    if ( data[ 0 ] == '-' )
    {
      value = -std::numeric_limits< float >::infinity();
    }
    else
    {
      value = std::numeric_limits< float >::infinity();
    }

    return ( true );
  }

  std::istringstream iss( data );
  iss.exceptions( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
  try
  {
    iss >> value;
    return ( true );
  }
  catch ( ... )
  {
    return ( false );
  }
}

// Strip out space at the start and at the end of the string
void StripSpaces( std::string& str )
{
  size_t esize = str.size();
  size_t idx = 0;

  // Strip out spaces at the start of the str
  while ( ( idx < esize ) && ( ( str[ idx ] == ' ' ) || ( str[ idx ] == '\t' ) || ( str[ idx ]
      == '\n' ) || ( str[ idx ] == '\r' ) || ( str[ idx ] == '\f' ) || ( str[ idx ] == '\v' ) ) )
    idx++;

  // Get the substring without spaces at the start or at the end
  str = str.substr( idx, ( str.size() - idx ) );
}

// Strip out space at the start and at the end of the string
void StripSurroundingSpaces( std::string& str )
{
  size_t esize = str.size();
  size_t idx = 0;

  // Strip out spaces at the start of the str
  while ( ( idx < esize ) && ( ( str[ idx ] == ' ' ) || ( str[ idx ] == '\t' ) || ( str[ idx ]
      == '\n' ) || ( str[ idx ] == '\r' ) || ( str[ idx ] == '\f' ) || ( str[ idx ] == '\v' ) ) )
    idx++;

  size_t ridx = 0;
  if ( str.size() ) ridx = str.size() - 1;

  // Strip out spaces at the end of the str
  while ( ( ridx > 0 ) && ( ( str[ ridx ] == ' ' ) || ( str[ ridx ] == '\t' ) || ( str[ ridx ]
      == '\n' ) || ( str[ ridx ] == '\r' ) || ( str[ ridx ] == '\f' ) || ( str[ ridx ] == '\v' ) ) )
    ridx--;

  // Get the substring without spaces at the start or at the end
  str = str.substr( idx, ( ridx - idx + 1 ) );
}

// Function to split a list of options delimited by a character into a vector of
// strings
std::vector<std::string> SplitString( const std::string& str, const std::string& delimiter )
{
  std::string option_list_string = str;
  std::vector<std::string> option_list;
  while ( 1 )
  {
    size_t loc = option_list_string.find( delimiter );
    if ( loc >= option_list_string.size() )
    {
      option_list.push_back( option_list_string );
      break;
    }
    option_list.push_back( option_list_string.substr( 0, loc ) );
    option_list_string = option_list_string.substr( loc + delimiter.size() );
  }

  return option_list;
}

void AppendSpaceSplitSubstring( std::string str, size_t string_start, size_t string_end, 
                 std::vector< std::string >& value )
{
  std::string sub_str = 
    str.substr( string_start, string_end - string_start + 1 );
  std::vector< std::string > space_strings = SplitString( sub_str, " " );
  for( size_t str_idx = 0; str_idx < space_strings.size(); str_idx++ ) 
  {
    if( space_strings[ str_idx ] != "" )
    {
      value.push_back( space_strings[ str_idx ] );
    }
  }
}

std::vector<std::string> SplitStringByBracketsThenSpaces( const std::string& str )
{
  std::string data = str;
  std::vector<std::string> value;

  size_t j = 0;
  size_t space_string_start = 0; // Inclusive
  // Loop over all characters in string
  while ( j < data.size() )
  {
    // If a start bracket is found, go until matching end bracket is found
    if ( data[ j ] == '[' )
    {
      // Split the preceding string by spaces and add it to the vector
      if( j > space_string_start )
      {
        size_t space_string_end = j - 1; // Inclusive
        AppendSpaceSplitSubstring( data, space_string_start, space_string_end, value );
      }

      j++;
      size_t start = j;
      size_t paren_count = 0;

      while ( j < data.size() && ( data[ j ] != ']' || paren_count > 0 ) )
      {
        if ( data[ j ] == '[' ) paren_count++;
        if ( data[ j ] == ']' ) paren_count--;
        j++;
      }

      // if there is no end bracket
      if ( j == data.size() ) return value;

      value.push_back( data.substr( start, j-start ) );
      space_string_start = j + 1;
    }
    else if( j == data.size() - 1 ) // Reached end of string
    {
      // Split the preceding string by spaces and add it to the vector
      size_t space_string_end = j; // Inclusive
      AppendSpaceSplitSubstring( data, space_string_start, space_string_end, value );
    }
    j++;
  }

  return value;
}

std::string ExportToString( bool value )
{
  if ( value ) return ( std::string( "true" ) );
  else return ( std::string( "false" ) );
}

std::string ExportToString( char value )
{
  return ToString( value );
}

std::string ExportToString( unsigned char value )
{
  return ToString(value);
}

std::string ExportToString( short value )
{
  return ToString( value );
}

std::string ExportToString( unsigned short value )
{
  return ToString(value);
}

std::string ExportToString( int value )
{
  return ToString( value );
}

std::string ExportToString( unsigned int value )
{
  return ToString(value);
}

std::string ExportToString( long value )
{
  return ToString( value );
}

std::string ExportToString( unsigned long value )
{
  return ToString( value );
}

std::string ExportToString( long long value )
{
  return ToString( value );
}

std::string ExportToString( unsigned long long value )
{
  return ToString( value );
}

std::string ExportToString( float value )
{
  return ToString( value );
}

std::string ExportToString( double value )
{
  return ToString( value );
}

std::string ExportToString( unsigned char value, int precision )
{
  return ToString( value, precision );
}

std::string ExportToString( unsigned short value, int precision )
{
  return ToString( value, precision );
}

std::string ExportToString( unsigned int value, int precision )
{
  return ToString( value, precision );
}

std::string ExportToString( unsigned long long value, int precision )
{
  return ToString( value, precision );
}

std::string ExportToString( float value, int precision )
{
  return ToString( value, precision );
}

std::string ExportToString( double value, int precision )
{
  return ToString( value, precision );
}

std::string ExportToString( const double& value, size_t digits )
{
  return ToString( value, digits );
}

std::string ExportToString( const std::string& value )
{
  bool need_quotes = false;
  for ( size_t j = 0; j < value.size(); j++)
  {
    if ( value[j] == ' ' || value[j] == '\t' || value[j] == '[' || value[j] == ']' ||
      value[j] == '(' || value[j] == ')' || value[j] == ',' || value[j] == '"' ) 
    {
      need_quotes = true;
    }
  }
  if ( value.size() == 0) need_quotes = true;
  
  if ( need_quotes ) return std::string(1,'[') + value + std::string(1,']');
  else return value;
}

std::string ExportToString( const std::vector< char >& value )
{
  return MultipleToString( value );
}
  
std::string ExportToString( const std::vector< std::string >& value )
{

  std::vector< std::string >::const_iterator it = value.begin();
  std::vector< std::string >::const_iterator it_end = value.end();
  
  std::string result( "[" );
  while ( it != it_end )
  {
    result += '[' + *it + ']' + ',';
    ++it;
  }
  if ( result.size() > 1 )
  {
    result[ result.size() - 1 ] = ']';
  }
  else
  {
    result += "]";
  }
  return result;
  
}

std::string ExportToString(const std::vector< unsigned char >& value)
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< short >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< unsigned short >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< int >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< unsigned int >& value)
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< long >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< unsigned long >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< long long >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< unsigned long long >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< float >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< double >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::set< int >& value )
{
  return MultipleToString( value );
}

std::string ExportToString( const std::vector< float >& value, int precision )
{
  std::string result( "[ " );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ToString( value[ j ], precision ) + ' ';
  result += "]";
  return result;
}

std::string ExportToString( const std::vector< double >& value, int precision )
{
  std::string result( "[ " );
  for ( size_t j = 0; j < value.size(); j++ )
    result += ToString( value[ j ], precision ) + ' ';
  result += "]";
  return result;
}

bool ImportFromString( const std::string& str, bool& value )
{
  std::string tmpstr( str );
  StripSurroundingSpaces( tmpstr );
  tmpstr = StringToLower( tmpstr );
  
  if ( ( tmpstr == "0" ) || ( tmpstr == "false" ) || ( tmpstr == "off" ) || ( tmpstr == "no" ) )
  {
    value = false;
    return ( true );
  }
  else if ( ( tmpstr == "1" ) || ( tmpstr == "true" ) || ( tmpstr == "on" ) || ( tmpstr == "yes" ) )
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

bool ImportFromString( const std::string& str, unsigned char& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, short& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString(const std::string& str, unsigned short& value)
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, int& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, unsigned int& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, long& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, unsigned long& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, long long& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, unsigned long long& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, float& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, double& value )
{
  return ( FromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< char >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< unsigned char >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< short >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< unsigned short >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< int >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector<unsigned int>& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector<long>& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector<unsigned long>& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector<long long>& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector<unsigned long long>& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< float >& value )
{
  return ( MultipleFromString( str, value ) );
}

bool ImportFromString( const std::string& str, std::vector< double >& value )
{
  return ( MultipleFromString( str, value ) );
}
  
bool ImportFromString( const std::string& str, std::vector< std::string >& value )
{
  std::string data = str;
  if ( data.size() > 1 )
  {
    if ( ( data[ 0 ] == '[' ) && ( data[ data.size() - 1 ] == ']' ) )
    {
      data = data.substr( 1, data.size() - 2 );
    }
  }
  
  value.clear();
  
  size_t j = 0;
  while ( j < data.size() )
  {
    while ( j < data.size() && ( ( data[ j ] == ' ') || ( data[ j ] == '\t' ) || 
      ( data[ j ] == '\r' ) || ( data[ j ] == '\n' ) || ( data[ j ] == ',' ) ) ) j++;

    if ( j == data.size() ) return true;
    if ( data[ j ] == '[' )
    {
      j++;
      size_t start = j;
      size_t paren_count = 0;

      while ( j < data.size() && ( data[ j ] != ']' || paren_count > 0 ) )
      {
        if ( data[ j ] == '[' ) paren_count++;
        if ( data[ j ] == ']' ) paren_count--;
        j++;
      }

      // if there is no end quotation mark
      if ( j == data.size() ) return false;
      value.push_back( data.substr( start, j-start ) );
      j++;
    }
    else if ( data[ j ] == '\'' )
    {
      ++j;
      size_t start = j;
      while ( j < data.size() && data[ j ] != '\'' ) ++j;

      if ( j == data.size() ) return false; // No matching quote

      value.push_back( data.substr( start, j - start ) );
      ++j;
    }
    else if ( data[ j ] == '"' )
    {
      ++j;
      size_t start = j;
      while ( j < data.size() && data[ j ] != '"' ) ++j;

      if ( j == data.size() ) return false; // No matching double quote

      value.push_back( data.substr( start, j - start ) );
      ++j;
    }
    else
    {
      size_t start = j;
      while ( j < data.size() && ( ( data[ j ] != ' ' )  && ( data[ j ] != '\t' ) &&
        ( data[ j ] != '\r' ) && ( data[ j ] != '\n' ) && ( data[ j ] != ',' ) ) ) j++;
    
      value.push_back( data.substr( start, j-start ) );
    }
  }

  return true;
}
  
bool ImportFromString( const std::string& str, std::string& value )
{
  value = str;
  // Remove quotes if needed
  if ( str.size() > 2 )
  {
    if ( ( str[0] == '"' ) && ( str[ str.size() - 1 ] == '"' ) )
    {
      value = str.substr( 1, str.size() - 2 );
    }
    else if ( ( str[0] == '[' ) && ( str[ str.size() - 1 ] == ']' ) )
    {
      value = str.substr( 1, str.size() - 2 );
    }
  }
  
  return true;
}

bool ImportFromString( const std::string& str, std::set< int >& value )
{
  value.clear();
  std::vector< int > tmp;
  if ( MultipleFromString( str, tmp ) )
  {
    value.insert( tmp.begin(), tmp.end() );
    return true;
  }
  return false;
}


} // End namespace Core

