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
#include <Core/Parser/ArrayMathProgramSource.h> 

namespace Core
{

void ArrayMathProgramSource::set_bool_array( std::vector< bool >* array )
{
  this->bool_array_ = array;
}

std::vector< bool >* ArrayMathProgramSource::get_bool_array()
{
  return this->bool_array_;
}

bool ArrayMathProgramSource::is_bool_array()
{
  return this->bool_array_ != 0;
}

void ArrayMathProgramSource::set_double_array( std::vector< double >* array )
{
  this->double_array_ = array;
}

std::vector< double >* ArrayMathProgramSource::get_double_array()
{
  return this->double_array_;
}

bool ArrayMathProgramSource::is_double_array()
{
  return this->double_array_ != 0;
}

void ArrayMathProgramSource::set_data_block( DataBlock* data_block )
{
  this->data_block_ = data_block;
}

DataBlock* ArrayMathProgramSource::get_data_block()
{
  return this->data_block_;
}

bool ArrayMathProgramSource::is_data_block()
{
  return this->data_block_ != 0;
}

void ArrayMathProgramSource::set_mask_data_block( MaskDataBlock* mask_data_block )
{
  this->mask_data_block_ = mask_data_block;
}

MaskDataBlock* ArrayMathProgramSource::get_mask_data_block()
{
  return this->mask_data_block_;
}

bool ArrayMathProgramSource::is_mask_data_block()
{
  return this->mask_data_block_ != 0;
}




} // end namespace
