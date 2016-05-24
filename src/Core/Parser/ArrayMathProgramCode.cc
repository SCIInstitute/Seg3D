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

// STL includes
#include <iostream>

// Core includes
#include <Core/Parser/ArrayMathProgramCode.h> 

namespace Core
{

void ArrayMathProgramCode::print()
{
  std::cout << "function_ = " << this->function_ << "\n";
  for ( size_t j = 0; j < this->variables_.size(); j++ )
  {
    std::cout << "variable[" << j << "]_ = " << this->variables_[ j ] << "\n";
  }
}

ArrayMathProgramCode::ArrayMathProgramCode( ArrayMathFunctionObject function ) :
  function_(function), 
  index_( 0 ), 
  size_( 1 )
{
}

ArrayMathProgramCode::ArrayMathProgramCode() :
  function_( 0 ), 
  index_( 0 ), 
  size_( 1 )
{
}

} // end namespace
