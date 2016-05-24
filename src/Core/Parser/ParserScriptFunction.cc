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
#include <vector>

// Core includes
#include <Core/Parser/ParserScriptFunction.h>
#include <Core/Parser/ParserScriptVariable.h>

namespace Core
{

class ParserScriptFunctionPrivate
{
public:
  // The name of the function
  std::string name_;

  // Flags that describe whether this is a single or sequential call
  int flags_;

  // Pointer to the function information block
  ParserFunction* function_;

  // Input variables in the function depends on
  std::vector< ParserScriptVariableHandle > input_variables_;

  // Output variable
  ParserScriptVariableHandle output_variable_;
};

ParserScriptFunction::ParserScriptFunction( std::string name, ParserFunction* function ) :
  private_( new ParserScriptFunctionPrivate )
{
  this->private_->name_ = name;
  this->private_->flags_ = 0;
  this->private_->function_ = function;
}

void ParserScriptFunction::print()
{
  std::cout << "  " << this->private_->output_variable_->get_uname() << " = " << this->private_->name_ << "(";
  for ( size_t j = 0; j < this->private_->input_variables_.size(); j++ )
  {
    std::cout << this->private_->input_variables_[ j ]->get_uname();
    if ( j < ( this->private_->input_variables_.size() - 1 ) )
    {
      std::cout << ",";
    }
  }
  std::cout << ")  flags=" << this->private_->flags_ << " \n";
}

std::string ParserScriptFunction::get_name()
{
  return this->private_->name_;
}

size_t ParserScriptFunction::num_input_vars()
{
  return this->private_->input_variables_.size();
}

Core::ParserScriptVariableHandle ParserScriptFunction::get_input_var( size_t j )
{
  if ( j < this->private_->input_variables_.size() )
  {
    return this->private_->input_variables_[ j ];
  }
  return ParserScriptVariableHandle();
}

void ParserScriptFunction::set_input_var( size_t j, ParserScriptVariableHandle& handle )
{
  if ( j >= this->private_->input_variables_.size() ) 
  {
    this->private_->input_variables_.resize( j + 1 );
  }
  this->private_->input_variables_[ j ] = handle;
}

Core::ParserScriptVariableHandle ParserScriptFunction::get_output_var()
{
  return this->private_->output_variable_;
}

void ParserScriptFunction::set_output_var( ParserScriptVariableHandle& handle )
{
  this->private_->output_variable_ = handle;
}

ParserFunction* ParserScriptFunction::get_function()
{
  return this->private_->function_;
}

int ParserScriptFunction::get_flags()
{
  return this->private_->flags_;
}

void ParserScriptFunction::set_flags( int flags )
{
  this->private_->flags_ |= flags;
}

void ParserScriptFunction::clear_flags()
{
  this->private_->flags_ = 0;
}

}

