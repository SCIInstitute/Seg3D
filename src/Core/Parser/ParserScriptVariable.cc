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
#include <Core/Parser/ParserEnums.h>
#include <Core/Parser/ParserFunction.h>
#include <Core/Parser/ParserScriptFunction.h>
#include <Core/Parser/ParserScriptVariable.h>

namespace Core
{

class ParserScriptVariablePrivate
{
public:
  // The function that created this variable
  ParserScriptFunctionHandle parent_;

  // The kind of variable
  int kind_;

  // The type of the variable
  std::string type_;

  // The flags for sequential/single/constant
  int flags_;

  // Name of the variable for input and output variables
  std::string name_;

  // Unique name of the variable
  std::string uname_;

  std::string dependence_;

  // For const scalar/string variables
  float scalar_value_;
  std::string string_value_;

  int var_number_;
};

ParserScriptVariable::ParserScriptVariable( std::string name, std::string uname, std::string type, 
  int flags ) :
  private_( new ParserScriptVariablePrivate )
{
  this->private_->kind_ = SCRIPT_INPUT_E;
  this->private_->type_ = type;
  this->private_->flags_ = flags;
  this->private_->name_ = name;
  this->private_->uname_ = uname;
  this->private_->scalar_value_ = 0.0;
  this->private_->var_number_ = 0;
}

ParserScriptVariable::ParserScriptVariable( std::string uname, std::string type, int flags ) :
  private_( new ParserScriptVariablePrivate )
{
  this->private_->kind_ = SCRIPT_VARIABLE_E;
  this->private_->type_ = type;
  this->private_->flags_ = flags;
  this->private_->uname_ = uname;
  this->private_->scalar_value_ = 0.0;
  this->private_->var_number_ = 0;
}

ParserScriptVariable::ParserScriptVariable( std::string uname, float value ) :
  private_( new ParserScriptVariablePrivate )
{
  this->private_->kind_ = SCRIPT_CONSTANT_SCALAR_E;
  this->private_->type_ = "S";
  this->private_->flags_ = SCRIPT_CONST_VAR_E;
  this->private_->uname_ = uname;
  this->private_->scalar_value_ = value;
  this->private_->var_number_ = 0;
}

ParserScriptVariable::ParserScriptVariable( std::string uname, std::string value ) :  
  private_( new ParserScriptVariablePrivate )
{
  this->private_->kind_ = SCRIPT_CONSTANT_STRING_E;
  this->private_->type_ = "A";
  this->private_->flags_ = SCRIPT_CONST_VAR_E;
  this->private_->uname_ = uname;
  this->private_->scalar_value_ = 0.0;
  this->private_->string_value_ = value;
  this->private_->var_number_ = 0;
}

void ParserScriptVariable::print()
{
  std::cout << "  " << this->private_->uname_ << "(" << this->private_->name_ << ")" << " type=" << this->private_->type_ 
    << ", flags=" << this->private_->flags_;
  if ( this->private_->kind_ == SCRIPT_CONSTANT_SCALAR_E ) 
  {
    std::cout << ", value=" << this->private_->scalar_value_;
  }
  else if ( this->private_->kind_ == SCRIPT_CONSTANT_STRING_E ) 
  {
    std::cout << ", value=" << this->private_->string_value_;
  }
  std::cout << "\n";
}

void ParserScriptVariable::compute_dependence()
{
  if ( this->private_->parent_.get() )
  {
    this->private_->dependence_ = this->private_->parent_->get_name() + "(";
    size_t num_input_vars = this->private_->parent_->num_input_vars();
    if ( ( num_input_vars == 2 ) && ( this->private_->parent_->get_function()->get_flags()
      & PARSER_SYMMETRIC_FUNCTION_E ) )
    {
      std::string uname1 = this->private_->parent_->get_input_var( 0 )->get_uname();
      std::string uname2 = this->private_->parent_->get_input_var( 1 )->get_uname();
      if ( uname1.compare( uname2 ) < 0 ) 
      {
        this->private_->dependence_ += uname1 + "," + uname2 + ")";
      }
      else 
      {
        this->private_->dependence_ += uname2 + "," + uname1 + ")";
      }
    }
    else
    {
      for ( size_t j = 0; j < num_input_vars; j++ )
      {
        this->private_->dependence_ += this->private_->parent_->get_input_var( j )->get_uname();
        if ( j < ( num_input_vars - 1 ) ) 
        {
          this->private_->dependence_ += ",";
        }
      }
      this->private_->dependence_ += ")";
    }
  }
  else
  {
    this->private_->dependence_ = this->private_->uname_;
  }
}

void ParserScriptVariable::set_parent( ParserScriptFunctionHandle& handle )
{
  this->private_->parent_ = handle;
}

Core::ParserScriptFunctionHandle ParserScriptVariable::get_parent()
{
  return this->private_->parent_;
}

int ParserScriptVariable::get_kind()
{
  return this->private_->kind_;
}

void ParserScriptVariable::set_kind( int kind )
{
  this->private_->kind_ = kind;
}

std::string ParserScriptVariable::get_type()
{
  return this->private_->type_;
}

void ParserScriptVariable::set_type( std::string type )
{
  this->private_->type_ = type;
}

int ParserScriptVariable::get_flags()
{
  return this->private_->flags_;
}

void ParserScriptVariable::set_flags( int flags )
{
  this->private_->flags_ |= flags;
}

void ParserScriptVariable::clear_flags()
{
  this->private_->flags_ = 0;
}

void ParserScriptVariable::set_const_var()
{
  this->private_->flags_ |= SCRIPT_CONST_VAR_E;
}

void ParserScriptVariable::set_single_var()
{
  this->private_->flags_ |= SCRIPT_SINGLE_VAR_E;
}

void ParserScriptVariable::set_sequential_var()
{
  this->private_->flags_ |= SCRIPT_SEQUENTIAL_VAR_E;
}

bool ParserScriptVariable::is_const_var()
{
  return ( this->private_->flags_ & SCRIPT_CONST_VAR_E ) != 0;
}

bool ParserScriptVariable::is_single_var()
{
  return ( this->private_->flags_ & SCRIPT_SINGLE_VAR_E ) != 0;
}

bool ParserScriptVariable::is_sequential_var()
{
  return ( this->private_->flags_ & SCRIPT_SEQUENTIAL_VAR_E ) != 0;
}

std::string ParserScriptVariable::get_name()
{
  return this->private_->name_;
}

void ParserScriptVariable::set_name( std::string name )
{
  this->private_->name_ = name;
}

std::string ParserScriptVariable::get_uname()
{
  return this->private_->uname_;
}

void ParserScriptVariable::set_uname( std::string uname )
{
  this->private_->uname_ = uname;
}

float ParserScriptVariable::get_scalar_value()
{
  return this->private_->scalar_value_;
}

std::string ParserScriptVariable::get_string_value()
{
  return this->private_->string_value_;
}

std::string ParserScriptVariable::get_dependence()
{
  return this->private_->dependence_;
}

void ParserScriptVariable::clear_dependence()
{
  this->private_->dependence_.clear();
}

int ParserScriptVariable::get_var_number()
{
  return this->private_->var_number_;
}

void ParserScriptVariable::set_var_number( int var_number )
{
  this->private_->var_number_ = var_number;
}

}

