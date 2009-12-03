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

namespace Utils {


template<class T>
std::string export_to_string(const T& value)
{
  SCI_THROW_NOTIMPLEMENTEDERROR(std::string("export_to_string(")+
                            typeid(value).name()+") was not implemented");
}

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

// import_from_string:
// Function to convert a string back into the data it is representing

template<class T>
bool import_from_string(const std::string& xmlstring, T& value)
{
  SCI_THROW_NOTIMPLEMENTEDERROR(std::string("import_from_string(")+
                            typeid(value).name()+") was not implemented");
}

bool import_from_string(const std::string& xmlstring, char& value);
bool import_from_string(const std::string& xmlstring, unsigned char& value);

bool import_from_string(const std::string& xmlstring, short& value);
bool import_from_string(const std::string& xmlstring, unsigned short& value);

bool import_from_string(const std::string& xmlstring, int& value);
bool import_from_string(const std::string& xmlstring, unsigned int& value);

bool import_from_string(const std::string& xmlstring, long& value);
bool import_from_string(const std::string& xmlstring, unsigned long& value);

bool import_from_string(const std::string& xmlstring, float& value);
bool import_from_string(const std::string& xmlstring, double& value);

bool import_from_string(const std::string& xmlstring, std::string& value);

} // end namespace Utils

#endif
