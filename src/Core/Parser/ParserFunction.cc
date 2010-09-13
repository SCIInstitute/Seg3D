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

// Core includes
#include <Core/Parser/ParserEnums.h>
#include <Core/Parser/ParserFunction.h>
#include <Core/Parser/ParserVariable.h> 

namespace Core
{

ParserFunction::ParserFunction( std::string function_id, std::string type ) :
function_id_(function_id),
function_type_(type)
{
}

ParserFunction::ParserFunction( std::string function_id,std::string type, int fflags ) :
function_id_(function_id),
function_type_(type),
function_flags_(fflags)
{
}

std::string ParserFunction::get_function_id()
{
  return this->function_id_;
}

std::string ParserFunction::get_return_type()
{
  return this->function_type_;
}

int ParserFunction::get_flags()
{
  return this->function_flags_;
}

}

