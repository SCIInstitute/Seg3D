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

#ifndef CORE_CONVERTER_STRINGCONVERTER_H
#define CORE_CONVERTER_STRINGCONVERTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <typeinfo>
#include <string>

#include <Core/Utils/Exception.h>

#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Color.h>
#include <Core/Geometry/Plane.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/GridTransform.h>
#include <Core/Geometry/Quaternion.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/View2D.h>
#include <Core/Geometry/View3D.h>

namespace Core
{

template< class T >
std::string ExportToString( const T& value )
{
  SCI_THROW_NOTIMPLEMENTEDERROR(std::string("ExportToString(")+
    typeid(value).name()+") was not implemented");
}

std::string ExportToString( const bool& value );

std::string ExportToString( const char& value );
std::string ExportToString(const unsigned char& value);

std::string ExportToString( const short& value );
std::string ExportToString(const unsigned short& value);

std::string ExportToString( const int& value );
std::string ExportToString(const unsigned int& value);

std::string ExportToString( const long& value );
std::string ExportToString(const unsigned long& value);

std::string ExportToString( const long long& value );
std::string ExportToString( const unsigned long long& value );

std::string ExportToString( const float& value );
std::string ExportToString( const double& value );

std::string ExportToString( const std::string& value );

std::string ExportToString( const Point& value );
std::string ExportToString( const Vector& value );
std::string ExportToString( const Color& value );

std::string ExportToString( const BBox& value );
std::string ExportToString( const Transform& value );
std::string ExportToString( const GridTransform& value );

std::string ExportToString( const Plane& value );
std::string ExportToString( const Quaternion& value );

std::string ExportToString( const View2D& value );
std::string ExportToString( const View3D& value );

std::string ExportToString( const std::vector< char >& value );
std::string ExportToString(const std::vector<unsigned char>& value);

std::string ExportToString( const std::vector< short >& value );
std::string ExportToString(const std::vector<unsigned short>& value);

std::string ExportToString( const std::vector< int >& value );
std::string ExportToString(const std::vector<unsigned int>& value);

std::string ExportToString( const std::vector< long >& value );
std::string ExportToString(const std::vector<unsigned long>& value);

std::string ExportToString( const std::vector< float >& value );
std::string ExportToString( const std::vector< double >& value );

std::string ExportToString( const std::vector< Point >& value );
std::string ExportToString( const std::vector< Vector >& value );

// ImportFromString:
// Function to convert a string back into the data it is representing

template< class T >
bool ImportFromString( const std::string& str, T& value )
{
  SCI_THROW_NOTIMPLEMENTEDERROR(std::string("ImportFromString(")+
    typeid(value).name()+") was not implemented");
}

bool ImportFromString( const std::string& str, bool& value );

bool ImportFromString( const std::string& str, char& value );
bool ImportFromString(const std::string& str, unsigned char& value);

bool ImportFromString( const std::string& str, short& value );
bool ImportFromString(const std::string& str, unsigned short& value);

bool ImportFromString( const std::string& str, int& value );
bool ImportFromString(const std::string& str, unsigned int& value);

bool ImportFromString(const std::string& str, long long& value);
bool ImportFromString(const std::string& str, unsigned long long& value);

bool ImportFromString( const std::string& str, float& value );
bool ImportFromString( const std::string& str, double& value );

bool ImportFromString( const std::string& str, std::string& value );

bool ImportFromString( const std::string& str, Point& value );
bool ImportFromString( const std::string& str, Vector& value );
bool ImportFromString( const std::string& str, Color& value );
bool ImportFromString( const std::string& str, Quaternion& value );

bool ImportFromString( const std::string& str, BBox& value );
bool ImportFromString( const std::string& str, Transform& value );
bool ImportFromString( const std::string& str, GridTransform& value );
bool ImportFromString( const std::string& str, View2D& value );
bool ImportFromString( const std::string& str, View3D& value );

bool ImportFromString( const std::string& str, std::vector< char >& value );
bool ImportFromString(const std::string& str, std::vector<unsigned char>& value);

bool ImportFromString( const std::string& str, std::vector< short >& value );
bool ImportFromString(const std::string& str, std::vector<unsigned short>& value);

bool ImportFromString( const std::string& str, std::vector< int >& value );
bool ImportFromString(const std::string& str, std::vector<unsigned int>& value);

bool ImportFromString(const std::string& str, std::vector<long long>& value);
bool ImportFromString(const std::string& str, std::vector<unsigned long long>& value);

bool ImportFromString( const std::string& str, std::vector< float >& value );
bool ImportFromString( const std::string& str, std::vector< double >& value );

bool ImportFromString( const std::string& str, std::vector< Point >& value );
bool ImportFromString( const std::string& str, std::vector< Vector >& value );

} // end namespace Core

#endif
