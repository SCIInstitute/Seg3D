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

#ifndef UTILS_CONVERTER_STRINGCONVERTER_H
#define UTILS_CONVERTER_STRINGCONVERTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <typeinfo>
#include <string>

#include <Utils/Core/Exception.h>

#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Transform.h>
#include <Utils/Geometry/BBox.h>
#include <Utils/Geometry/Plane.h>
#include <Utils/Geometry/View2D.h>
#include <Utils/Geometry/View3D.h>

namespace Utils {


template<class T>
std::string export_to_string(const T& value)
{
  SCI_THROW_NOTIMPLEMENTEDERROR(std::string("export_to_string(")+
                            typeid(value).name()+") was not implemented");
}

std::string export_to_string(const bool& value);

std::string export_to_string(const char& value);
std::string export_to_string(const unsigned char& value);

std::string export_to_string(const short& value);
std::string export_to_string(const unsigned short& value);

std::string export_to_string(const int& value);
std::string export_to_string(const unsigned int& value);

std::string export_to_string(const long& value);
std::string export_to_string(const unsigned long& value);

std::string export_to_string(const float& value);
std::string export_to_string(const double& value);

std::string export_to_string(const std::string& value);

std::string export_to_string(const Point& value);
std::string export_to_string(const Vector& value);

std::string export_to_string(const std::vector<char>& value);
std::string export_to_string(const std::vector<unsigned char>& value);

std::string export_to_string(const std::vector<short>& value);
std::string export_to_string(const std::vector<unsigned short>& value);

std::string export_to_string(const std::vector<int>& value);
std::string export_to_string(const std::vector<unsigned int>& value);

std::string export_to_string(const std::vector<long>& value);
std::string export_to_string(const std::vector<unsigned long>& value);

std::string export_to_string(const std::vector<float>& value);
std::string export_to_string(const std::vector<double>& value);

std::string export_to_string(const std::vector<Point>& value);
std::string export_to_string(const std::vector<Vector>& value);

std::string export_to_string(const BBox& value);
std::string export_to_string(const Transform& value);
std::string export_to_string(const Plane& value);

std::string export_to_string(const View2D& value);
std::string export_to_string(const View3D& value);

// import_from_string:
// Function to convert a string back into the data it is representing

template<class T>
bool import_from_string(const std::string& str, T& value)
{
  SCI_THROW_NOTIMPLEMENTEDERROR(std::string("import_from_string(")+
                            typeid(value).name()+") was not implemented");
}

bool import_from_string(const std::string& str, bool& value);

bool import_from_string(const std::string& str, char& value);
bool import_from_string(const std::string& str, unsigned char& value);

bool import_from_string(const std::string& str, short& value);
bool import_from_string(const std::string& str, unsigned short& value);

bool import_from_string(const std::string& str, int& value);
bool import_from_string(const std::string& str, unsigned int& value);

bool import_from_string(const std::string& str, long long& value);
bool import_from_string(const std::string& str, unsigned long long& value);

bool import_from_string(const std::string& str, float& value);
bool import_from_string(const std::string& str, double& value);

bool import_from_string(const std::string& str, std::string& value);

bool import_from_string(const std::string& str, Point& value);
bool import_from_string(const std::string& str, Vector& value);

bool import_from_string(const std::string& str, std::vector<char>& value);
bool import_from_string(const std::string& str, std::vector<unsigned char>& value);

bool import_from_string(const std::string& str, std::vector<short>& value);
bool import_from_string(const std::string& str, std::vector<unsigned short>& value);

bool import_from_string(const std::string& str, std::vector<int>& value);
bool import_from_string(const std::string& str, std::vector<unsigned int>& value);

bool import_from_string(const std::string& str, std::vector<long long>& value);
bool import_from_string(const std::string& str, std::vector<unsigned long long>& value);


bool import_from_string(const std::string& str, std::vector<float>& value);
bool import_from_string(const std::string& str, std::vector<double>& value);

bool import_from_string(const std::string& str, std::vector<Point>& value);
bool import_from_string(const std::string& str, std::vector<Vector>& value);

bool import_from_string(const std::string& str, BBox& value);
bool import_from_string(const std::string& str, Transform& value);

bool import_from_string(const std::string& str, View2D& value);
bool import_from_string(const std::string& str, View3D& value);


} // end namespace Utils

#endif
