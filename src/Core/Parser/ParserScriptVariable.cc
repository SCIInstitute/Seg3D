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

// STL includes
#include <iostream>

// Core includes
#include <Core/Parser/ParserEnums.h>
#include <Core/Parser/ParserFunction.h>
#include <Core/Parser/ParserScriptFunction.h>
#include <Core/Parser/ParserScriptVariable.h>

namespace Core
{

void ParserScriptVariable::print()
{
  std::cout << "  " << this->uname_ << "(" << this->name_ << ")" << " type=" << this->type_ 
    << ", flags=" << this->flags_;
  if ( this->kind_ == SCRIPT_CONSTANT_SCALAR_E ) 
  {
    std::cout << ", value=" << this->scalar_value_;
  }
  else if ( this->kind_ == SCRIPT_CONSTANT_STRING_E ) 
  {
    std::cout << ", value=" << this->string_value_;
  }
  std::cout << "\n";
}

void ParserScriptVariable::compute_dependence()
{
  if ( this->parent_.get() )
  {
    this->dependence_ = this->parent_->get_name() + "(";
    size_t num_input_vars = this->parent_->num_input_vars();
    if ( ( num_input_vars == 2 ) && ( this->parent_->get_function()->get_flags()
      & PARSER_SYMMETRIC_FUNCTION_E ) )
    {
      std::string uname1 = this->parent_->get_input_var( 0 )->get_uname();
      std::string uname2 = this->parent_->get_input_var( 1 )->get_uname();
      if ( uname1.compare( uname2 ) < 0 ) 
      {
        this->dependence_ += uname1 + "," + uname2 + ")";
      }
      else 
      {
        this->dependence_ += uname2 + "," + uname1 + ")";
      }
    }
    else
    {
      for ( size_t j = 0; j < num_input_vars; j++ )
      {
        this->dependence_ += this->parent_->get_input_var( j )->get_uname();
        if ( j < ( num_input_vars - 1 ) ) 
        {
          this->dependence_ += ",";
        }
      }
      this->dependence_ += ")";
    }
  }
  else
  {
    this->dependence_ = this->uname_;
  }
}

ParserScriptVariable::ParserScriptVariable( std::string name, std::string uname, std::string type, 
  int flags ) :
kind_( SCRIPT_INPUT_E ), 
  type_( type ), 
  flags_( flags ), 
  name_( name ),
  uname_( uname ), 
  scalar_value_( 0.0 ), 
  var_number_( 0 )
{
}

ParserScriptVariable::ParserScriptVariable( std::string uname, std::string type, int flags ) :
kind_( SCRIPT_VARIABLE_E ), 
  type_( type ), 
  flags_( flags ), 
  uname_( uname ),
  scalar_value_( 0.0 ), 
  var_number_( 0 )
{
}

ParserScriptVariable::ParserScriptVariable( std::string uname, double value ) :
kind_( SCRIPT_CONSTANT_SCALAR_E ), 
  type_( "S" ),
  flags_( SCRIPT_CONST_VAR_E ), 
  uname_( uname ), 
  scalar_value_( value ), 
  var_number_( 0 )
{
}

ParserScriptVariable::ParserScriptVariable( std::string uname, std::string value ) :
kind_( SCRIPT_CONSTANT_STRING_E ), 
  type_( "A" ),
  flags_( SCRIPT_CONST_VAR_E ), 
  uname_( uname ), 
  scalar_value_( 0.0 ), 
  string_value_( value ), 
  var_number_( 0 )
{
}

void ParserScriptVariable::set_parent( ParserScriptFunctionHandle& handle )
{
  this->parent_ = handle;
}

Core::ParserScriptFunctionHandle ParserScriptVariable::get_parent()
{
  return this->parent_;
}

int ParserScriptVariable::get_kind()
{
  return this->kind_;
}

void ParserScriptVariable::set_kind( int kind )
{
  this->kind_ = kind;
}

std::string ParserScriptVariable::get_type()
{
  return this->type_;
}

void ParserScriptVariable::set_type( std::string type )
{
  this->type_ = type;
}

int ParserScriptVariable::get_flags()
{
  return this->flags_;
}

void ParserScriptVariable::set_flags( int flags )
{
  this->flags_ |= flags;
}

void ParserScriptVariable::clear_flags()
{
  this->flags_ = 0;
}

void ParserScriptVariable::set_const_var()
{
  this->flags_ |= SCRIPT_CONST_VAR_E;
}

void ParserScriptVariable::set_single_var()
{
  this->flags_ |= SCRIPT_SINGLE_VAR_E;
}

void ParserScriptVariable::set_sequential_var()
{
  this->flags_ |= SCRIPT_SEQUENTIAL_VAR_E;
}

bool ParserScriptVariable::is_const_var()
{
  return ( this->flags_ & SCRIPT_CONST_VAR_E ) != 0;
}

bool ParserScriptVariable::is_single_var()
{
  return ( this->flags_ & SCRIPT_SINGLE_VAR_E ) != 0;
}

bool ParserScriptVariable::is_sequential_var()
{
  return ( this->flags_ & SCRIPT_SEQUENTIAL_VAR_E ) != 0;
}

std::string ParserScriptVariable::get_name()
{
  return this->name_;
}

void ParserScriptVariable::set_name( std::string name )
{
  this->name_ = name;
}

std::string ParserScriptVariable::get_uname()
{
  return this->uname_;
}

void ParserScriptVariable::set_uname( std::string uname )
{
  this->uname_ = uname;
}

double ParserScriptVariable::get_scalar_value()
{
  return this->scalar_value_;
}

std::string ParserScriptVariable::get_string_value()
{
  return this->string_value_;
}

std::string ParserScriptVariable::get_dependence()
{
  return this->dependence_;
}

void ParserScriptVariable::clear_dependence()
{
  this->dependence_.clear();
}

int ParserScriptVariable::get_var_number()
{
  return this->var_number_;
}

void ParserScriptVariable::set_var_number( int var_number )
{
  this->var_number_ = var_number;
}

}

