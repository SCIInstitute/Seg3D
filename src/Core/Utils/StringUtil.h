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

#ifndef CORE_UTILS_STRINGUTIL_H
#define CORE_UTILS_STRINGUTIL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// ConvertString
// This utility contains templated functions for converting strings from
// number and into numbers. The converters handle as well cases missing
// in the standard converter utilities to deal with NaN, inf, and -inf on
// on the input string.


// STL includes
#include <string>
#include <vector>

namespace Core
{

// Export functions to strings
std::string ExportToString( const bool& value );

std::string ExportToString( const char& value );
std::string ExportToString( const unsigned char& value);

std::string ExportToString( const short& value );
std::string ExportToString( const unsigned short& value);

std::string ExportToString( const int& value );
std::string ExportToString( const unsigned int& value);

std::string ExportToString( const long& value );
std::string ExportToString( const unsigned long& value );

std::string ExportToString( const long long& value );
std::string ExportToString( const unsigned long long& value );

std::string ExportToString( const float& value );
std::string ExportToString( const double& value );

std::string ExportToString( const float& value, int percision );
std::string ExportToString( const double& value, int percision );

std::string ExportToString( const std::string& value );

std::string ExportToString( const std::vector< char >& value );
std::string ExportToString( const std::vector< unsigned char>& value);

std::string ExportToString( const std::vector< short >& value );
std::string ExportToString( const std::vector< unsigned short>& value);

std::string ExportToString( const std::vector< int >& value );
std::string ExportToString( const std::vector< unsigned int>& value);

std::string ExportToString( const std::vector< long long >& value );
std::string ExportToString( const std::vector< unsigned long long >& value);

std::string ExportToString( const std::vector< long >& value );
std::string ExportToString( const std::vector< unsigned long >& value);

std::string ExportToString( const std::vector< float >& value );
std::string ExportToString( const std::vector< double >& value );

std::string ExportToString( const std::vector< float >& value, int percision );
std::string ExportToString( const std::vector< double >& value, int percision );

// Import functions from strings
bool ImportFromString( const std::string& str, bool& value );

bool ImportFromString( const std::string& str, char& value );
bool ImportFromString(const std::string& str, unsigned char& value);

bool ImportFromString( const std::string& str, short& value );
bool ImportFromString(const std::string& str, unsigned short& value);

bool ImportFromString( const std::string& str, int& value );
bool ImportFromString(const std::string& str, unsigned int& value);

bool ImportFromString(const std::string& str, long& value);
bool ImportFromString(const std::string& str, unsigned long& value);

bool ImportFromString(const std::string& str, long long& value);
bool ImportFromString(const std::string& str, unsigned long long& value);

bool ImportFromString( const std::string& str, float& value );
bool ImportFromString( const std::string& str, double& value );

bool ImportFromString( const std::string& str, std::string& value );

bool ImportFromString( const std::string& str, std::vector< char >& value );
bool ImportFromString( const std::string& str, std::vector< unsigned char>& value);

bool ImportFromString( const std::string& str, std::vector< short >& value );
bool ImportFromString( const std::string& str, std::vector< unsigned short >& value);

bool ImportFromString( const std::string& str, std::vector< int >& value );
bool ImportFromString( const std::string& str, std::vector< unsigned int >& value);

bool ImportFromString( const std::string& str, std::vector< long >& value);
bool ImportFromString( const std::string& str, std::vector< unsigned long >& value);

bool ImportFromString( const std::string& str, std::vector< long long >& value);
bool ImportFromString( const std::string& str, std::vector< unsigned long long>& value);

bool ImportFromString( const std::string& str, std::vector< float >& value );
bool ImportFromString( const std::string& str, std::vector< double >& value );

// Convert string to upper or lower case 
std::string StringToUpper( std::string );
std::string StringToLower( std::string );

// Functions to strip out spaces at the start or at both ends of the string
void StripSpaces( std::string& str );
void StripSurroundingSpaces( std::string& str );

// Function to split a list of options delimited by a characher into a vector of
// strings
std::vector<std::string> SplitString( const std::string& str, const std::string& delimiter );

} // End namespace Core

#endif
