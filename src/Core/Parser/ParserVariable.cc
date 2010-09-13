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
#include <Core/Parser/ParserVariable.h> 

namespace Core
{

std::string ParserVariableType( std::string type )
{
  if ( type == "U" ) return std::string( "Unknown" );
  if ( type == "S" ) return std::string( "Scalar" );
  if ( type == "V" ) return std::string( "Vector" );
  if ( type == "T" ) return std::string( "Tensor" );
  if ( type == "M" ) return std::string( "Matrix" );
  if ( type == "A" ) return std::string( "String" );
  if ( type == "L" ) return std::string( "List" );
  if ( type == "FD" ) return std::string( "FieldData" );
  if ( type == "FM" ) return std::string( "FieldMesh" );
  if ( type == "FN" ) return std::string( "FieldNode" );
  if ( type == "FE" ) return std::string( "FieldElement" );
  if ( type == "AB" ) return std::string( "Boolean Vector" );
  if ( type == "AD" ) return std::string( "Double Vector" );
  if ( type == "AI" ) return std::string( "Integer Vector" );
  return std::string( "Unknown" );
}

ParserVariable::ParserVariable( std::string name, std::string type ) :
  name_( name ), 
  type_( type ), 
  flags_( SCRIPT_SINGLE_VAR_E )
{
}

ParserVariable::ParserVariable( std::string name, std::string type, int flags ) :
  name_( name ), 
  type_( type ), 
  flags_( flags )
{
  // Default is a single variable
  // Currently one can choose between a sequence or a single
  if ( flags == 0 )
  {
    this->flags_ = SCRIPT_SINGLE_VAR_E;
  }
}

std::string ParserVariable::get_type()
{
  return this->type_;
}

void ParserVariable::set_type( std::string type )
{
  this->type_ = type;
}

int ParserVariable::get_flags()
{
  return this->flags_;
}

std::string ParserVariable::get_name()
{
  return this->name_;
}

}

