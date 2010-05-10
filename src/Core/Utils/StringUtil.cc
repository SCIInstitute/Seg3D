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

#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>

#include <stdlib.h>
#include <limits>

namespace Core
{

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

  // Handle special cases: nan, inf, and -inf

  // handle nan
  if ( data.size() > 2 && ( data[ 0 ] == 'n' || data[ 0 ] == 'N' ) && ( data[ 1 ] == 'A'
      || data[ 1 ] == 'A' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) )
  {
    value = std::numeric_limits< double >::quiet_NaN();
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
  if ( data.size() > 2 && ( data[ 0 ] == 'n' || data[ 0 ] == 'N' ) && ( data[ 1 ] == 'A'
      || data[ 1 ] == 'A' ) && ( data[ 2 ] == 'n' || data[ 2 ] == 'N' ) )
  {
    value = std::numeric_limits< float >::quiet_NaN();
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

// Function to split a list of options delimited by a characher into a vector of
// strings
std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter)
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
    option_list_string = option_list_string.substr( loc + 1 );
  }

  return option_list;
}

} // End namespace Core

