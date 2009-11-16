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

#ifndef UTILS_CORE_STRINGUTIL_H
#define UTILS_CORE_STRINGUTIL_H

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
#include <sstream>
#include <fstream>

namespace Utils {

// Convert multiple values in a string into a vector with numbers

template <class T>
bool multiple_from_string(const std::string &str, std::vector<T> &values)
{
  values.clear();
  
  std::string data = str;
  for (size_t j=0; j<data.size(); j++) 
    if ((data[j] == '\t')||(data[j] == '\r')||(data[j] == '\n')||(data[j]=='"')
        ||(data[j]==',')||(data[j]=='[')||(data[j]==']')||(data[j]=='(')
        ||(data[j]==')')) data[j] = ' ';
    
  std::vector<std::string> nums;
  for (size_t p=0;p<data.size();)
  { 
    while((data[p] == ' ')&&(p<data.size())) p++;
    if (p >= data.size()) break;

    std::string::size_type next_space = data.find(' ',p);
    if (next_space == std::string::npos) next_space = data.size();
    T value;
    if(from_string(data.substr(p,next_space-p),value)) values.push_back(value);
    p = next_space;

    if (p >= data.size()) break;
  }
  if (values.size() > 0) return (true);
  return (false);
}


// Convert a value into a string

template <class T>
bool from_string(const std::string &str, T &value)
{
  std::string data = str+" ";
  for (size_t j=0; j<data.size(); j++) 
    if ((data[j] == '\t')||(data[j] == '\r')||(data[j] == '\n')||(data[j]=='"')
        ||(data[j]==',')||(data[j]=='[')||(data[j]==']')||(data[j]=='(')
        ||(data[j]==')')) data[j] = ' ';

  std::istringstream iss(data);
  iss.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    iss >> value;
    return (true);
  }
  catch (...)
  {
    return (false);
  }
}


// Export a value to a string

template <class T>
std::string to_string(T val) 
{  
  std::ostringstream oss;
  oss << val;
  return (oss.str()); 
}

// Export a value to a string with percision control

template <class T>
std::string to_string(T val,int precision) 
{  
  std::ostringstream oss;
  oss.precision(precision);
  oss << val; 
  return (oss.str()); 
}
 
// Convert string to upper or lower case 
std::string string_to_upper(std::string);
std::string string_to_lower(std::string);

// Special cases that need additional rules to deal with inf and nan
bool from_string(const std::string &str, double &value);
bool from_string(const std::string &str, float &value);

// Functions to strip out spaces at the start or at both ends of the string
void strip_spaces(std::string& str);
void strip_surrounding_spaces(std::string& str);

} // End namespace Utils

#endif
