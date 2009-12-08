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

#include <Utils/Core/StringUtil.h>
#include <Utils/Converter/StringConverter.h>

namespace Utils {

std::string export_to_string(const bool& value)
{
  if (value) return (std::string("true")); else return (std::string("false"));
}

std::string export_to_string(const char& value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned char& value)
{
  return to_string(value);
}

std::string export_to_string(const short& value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned short& value)
{
  return to_string(value);
}

std::string export_to_string(const int& value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned int& value)
{
  return to_string(value);
}

std::string export_to_string(const long& value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned long& value)
{
  return to_string(value);
}

std::string export_to_string(const float& value)
{
  return to_string(value);
}

std::string export_to_string(const double& value)
{
  return to_string(value);
}

std::string export_to_string(const std::string& value)
{
  return value;
}

bool import_from_string(const std::string& str, bool& value)
{
  std::string tmpstr(str);
  strip_surrounding_spaces(tmpstr);
  if ((tmpstr == "0")||(tmpstr == "false")||(tmpstr == "FALSE")) return (false);
  return (true);
}


bool import_from_string(const std::string& str, char& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, unsigned char& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, short& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, unsigned short& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, int& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, unsigned int& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, long& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, unsigned long& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, float& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, double& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, std::string& value)
{
  value = str;
  return (true);
}

} // end namesapce Utils
