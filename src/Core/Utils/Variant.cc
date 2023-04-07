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

#include <Core/Utils/Variant.h>

namespace Core
{

VariantBase::~VariantBase()
{
}

Variant::Variant()
{
}

Variant::~Variant()
{
}

std::string Variant::export_to_string() const
{
  // Export a value that is still typed or has been converted to a string
  // if typed_value exist, we need to convert it
  if ( this->typed_value_.get() )
  {
    return this->typed_value_->export_to_string();
  }
  else
  {
    // in case typed_value does not exist it must be recorded as a string
    return this->string_value_;
  }
}

bool Variant::import_from_string( const std::string& str )
{
  // As we do not know the implied type. It can only be recorded as a string
  this->typed_value_.reset();
  this->string_value_ = str;

  return true;
}

std::string ExportToString( const Variant& variant )
{
  return variant.export_to_string();
}


bool ImportFromString( const std::string& str, Variant& variant )
{
  return variant.import_from_string( str );
}

} // namespace Core
