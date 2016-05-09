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

// Core includes
#include <Core/Parser/ParserFunction.h>

namespace Core
{

class ParserFunctionPrivate
{
public:
  // The ID of the function, which is the name plus arguments in one string
  std::string function_id_;

  // The return type of the function
  std::string function_type_;

  // Flags define properties of this function
  int function_flags_;
};

ParserFunction::ParserFunction( std::string function_id, std::string type ) :
  private_( new ParserFunctionPrivate )
{
  this->private_->function_id_ = function_id;
  this->private_->function_type_ = type;
}

ParserFunction::ParserFunction( std::string function_id, std::string type, int fflags ) :
  private_( new ParserFunctionPrivate )
{
  this->private_->function_id_ = function_id;
  this->private_->function_type_ = type;
  this->private_->function_flags_ = fflags;
}

std::string ParserFunction::get_function_id()
{
  return this->private_->function_id_;
}

std::string ParserFunction::get_return_type()
{
  return this->private_->function_type_;
}

int ParserFunction::get_flags()
{
  return this->private_->function_flags_;
}

}

