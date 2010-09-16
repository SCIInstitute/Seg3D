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
#include <Core/Parser/ParserNode.h> 
#include <Core/Parser/ParserVariable.h>

namespace Core
{

// Print function for debugging
void ParserNode::print( int level )
{
  // Depending on the level alter the indentation
  for ( int lev = 1; lev < level; lev++ )
  {
    std::cout << "  ";
  }

  // The four currently support constructs
  switch( this->kind_ )
  {
  case PARSER_CONSTANT_SCALAR_E:
    std::cout << "SCALAR_CONSTANT (" << ParserVariableType( this->type_ ) << "): " 
      << this->value_ << "\n";
    break;
  case PARSER_CONSTANT_STRING_E:
    std::cout << "STRING_CONSTANT (" << ParserVariableType( this->type_ ) << "): " 
      << this->value_ << "\n";
    break;
  case PARSER_VARIABLE_E:
    std::cout << "VARIABLE (" << ParserVariableType( this->type_ ) << "): " 
      << this->value_ << "\n";
    break;
  case PARSER_FUNCTION_E:
    std::cout << "FUNCTION (" << ParserVariableType( this->type_ ) << "): " 
      << this->value_ << "\n";
    for ( size_t j = 0; j < this->args_.size(); j++ )
    {
      this->args_[ j ]->print( level + 1 );
    }
    break;
  }
}

ParserNode::ParserNode( int kind, std::string value ) :
kind_( kind ), 
  type_( "U" ), 
  value_( value ), 
  function_( 0 )
{
}

ParserNode::ParserNode( int kind, std::string value, std::string type ) :
kind_( kind ), 
  type_( type ), 
  value_( value ), 
  function_( 0 )
{
}

int ParserNode::get_kind()
{
  return this->kind_;
}

std::string ParserNode::get_type()
{
  return this->type_;
}

std::string ParserNode::get_value()
{
  return this->value_;
}

void ParserNode::set_value( std::string& value )
{
  this->value_ = value;
}

Core::ParserNodeHandle ParserNode::get_arg( size_t j )
{
  return this->args_[ j ];
}

void ParserNode::set_arg( size_t j, ParserNodeHandle& handle )
{
  if ( j >= this->args_.size() )
  {
    this->args_.resize( j + 1 );
  }
  this->args_[ j ] = handle;
}

void ParserNode::set_arg( size_t j, ParserNode* ptr )
{
  if ( j >= this->args_.size() ) 
  {
    this->args_.resize( j + 1 );
  }
  this->args_[ j ] = ParserNodeHandle( ptr );
}

void ParserNode::set_function( ParserFunction* func )
{
  this->function_ = func;
}

ParserFunction* ParserNode::get_function()
{
  return this->function_;
}

void ParserNode::set_type( std::string type )
{
  this->type_ = type;
}

size_t ParserNode::num_args()
{
  return this->args_.size();
}

}

