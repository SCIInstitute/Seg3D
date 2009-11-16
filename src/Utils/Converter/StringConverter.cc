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

std::string export_to_string(const char value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned char value)
{
  return to_string(value);
}

std::string export_to_string(const short value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned short value)
{
  return to_string(value);
}

std::string export_to_string(const int value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned int value)
{
  return to_string(value);
}

std::string export_to_string(const long value)
{
  return to_string(value);
}

std::string export_to_string(const unsigned long value)
{
  return to_string(value);
}

std::string export_to_string(const float value)
{
  return to_string(value);
}

std::string export_to_string(const double value)
{
  return to_string(value);
}

std::string export_to_string(const std::string& value)
{
  return value;
}

bool import_from_string(const std::string& xmlstring, char& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, unsigned char& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, short& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, unsigned short& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, int& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, unsigned int& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, long& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, unsigned long& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, float& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, double& value)
{
  return (from_string(xmlstring,value));
}

bool import_from_string(const std::string& xmlstring, std::string& value)
{
  value = xmlstring;
  return (true);
}

} // end namesapce Utils
