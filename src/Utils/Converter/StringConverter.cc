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

std::string export_to_string(const std::vector<char>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<unsigned char>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<short>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<unsigned short>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<int>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<unsigned int>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<long long>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<unsigned long long>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<float>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<double>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<Point>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += export_to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}

std::string export_to_string(const std::vector<Vector>& value)
{
  std::string result(1,'[');
  for (size_t j=0;j<value.size();j++) result += export_to_string(value[j])+' ';
  result[result.size()-1] = ']';
  return result;
}


std::string export_to_string(const std::string& value)
{
  return value;
}

std::string export_to_string(const Point& value)
{
  return (std::string(1,'[')+to_string(value.x())+' '+
      to_string(value.y())+' '+to_string(value.z())+']');
}

std::string export_to_string(const Vector& value)
{
  return (std::string(1,'[')+to_string(value.x())+' '+
      to_string(value.y())+' '+to_string(value.z())+']');
}

std::string export_to_string(const BBox& value)
{
  return (std::string(1,'[')+export_to_string(value.min())+' '+export_to_string(value.max())+']');
}

std::string export_to_string(const Transform& value)
{
  std::string result(1,']');
  std::vector<double> trans(16);
  value.get(&trans[0]);
  for (size_t j=0; j<16;j++) result += to_string(trans[j])+' ';
  result[result.size()-1] = ']';
  return (result);
}

std::string export_to_string(const Plane& value)
{
  return (std::string(1,'[')+export_to_string(value.normal())+' '+export_to_string(value.distance())+']');
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

bool import_from_string(const std::string& str, long long& value)
{
  return (from_string(str,value));
}

bool import_from_string(const std::string& str, unsigned long long& value)
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

bool import_from_string(const std::string& str, std::vector<char>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<unsigned char>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<short>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<unsigned short>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<int>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<unsigned int>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<long long>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<unsigned long long>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<float>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::vector<double>& value)
{
  return (multiple_from_string(str,value));
}

bool import_from_string(const std::string& str, std::string& value)
{
  value = str;
  return (true);
}

bool import_from_string(const std::string& str, Point& value)
{
  std::vector<double> values;
  multiple_from_string(str,values);
  if (values.size() == 3) 
  { 
    value = Point(values[0],values[1],values[2]);
    return (true);
  }
  return (false);
}

bool import_from_string(const std::string& str, Vector& value)
{
  std::vector<double> values;
  multiple_from_string(str,values);
  if (values.size() == 3) 
  { 
    value = Vector(values[0],values[1],values[2]);
    return (true);
  }
  return (false);
}

bool import_from_string(const std::string& str, std::vector<Point>& value)
{
  std::vector<double> values;
  multiple_from_string(str,values);

  size_t num_values = values.size()/3;
  if (values.size() == num_values*3) 
  { 
    for (size_t j=0; j<num_values;j++)
    {
      size_t offset = j*3;
      value[j] = Point(values[offset+0],values[offset+1],values[offset+2]);
    }
    return (true);
  }
  return (false);
}

bool import_from_string(const std::string& str, std::vector<Vector>& value)
{
  std::vector<double> values;
  multiple_from_string(str,values);

  size_t num_values = values.size()/3;
  if (values.size() == num_values*3) 
  { 
    for (size_t j=0; j<num_values;j++)
    {
      size_t offset = j*3;
      value[j] = Vector(values[offset+0],values[offset+1],values[offset+2]);
    }
    return (true);
  }
  return (false);
}

bool import_from_string(const std::string& str, BBox& value)
{
  std::vector<double> values;
  multiple_from_string(str,values);
  if (values.size() == 6) 
  { 
    value = BBox(Point(values[0],values[1],values[2]),
                 Point(values[3],values[4],values[5]));
    return (true);
  }
  return (false);
}

bool import_from_string(const std::string& str, Transform& value)
{
  std::vector<double> values;
  multiple_from_string(str,values);
  if (values.size() == 16)
  {
    value.set(&values[0]);
    return (true);
  }
  return (false);
}

bool import_from_string(const std::string& str, Plane& value)
{
  std::vector<double> values;
  multiple_from_string(str,values);
  if (values.size() == 4) 
  { 
    value = Plane(Vector(values[0],values[1],values[2]),values[3]);
    return (true);
  }
  return (false);
}

} // end namesapce Utils
