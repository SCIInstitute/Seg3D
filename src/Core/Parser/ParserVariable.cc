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
#include <Core/Parser/ParserEnums.h>
#include <Core/Parser/ParserVariable.h> 

namespace Core
{

class ParserVariablePrivate
{
public:
  // The name of the variable
  std::string name_;

  // The type of the variable
  std::string type_;

  // Flags that guide the parser
  int flags_;
};

std::string ParserVariableType( std::string type )
{
  if ( type == "U" ) return std::string( "Unknown" );
  if ( type == "S" ) return std::string( "Scalar" );
  if ( type == "DATA" ) return std::string( "Data Block" );
  if ( type == "MASK" ) return std::string( "Mask Data Block" );
  return std::string( "Unknown" );
}

ParserVariable::ParserVariable( std::string name, std::string type ) :
  private_( new ParserVariablePrivate )
{
  this->private_->name_ = name;
  this->private_->type_ = type;
  this->private_->flags_ = SCRIPT_SINGLE_VAR_E;
}

ParserVariable::ParserVariable( std::string name, std::string type, int flags ) :
  private_( new ParserVariablePrivate )
{
  this->private_->name_ = name;
  this->private_->type_ = type;
  this->private_->flags_ = flags;

  // Default is a single variable
  // Currently one can choose between a sequence or a single
  if ( flags == 0 )
  {
    this->private_->flags_ = SCRIPT_SINGLE_VAR_E;
  }
}

std::string ParserVariable::get_type()
{
  return this->private_->type_;
}

void ParserVariable::set_type( std::string type )
{
  this->private_->type_ = type;
}

int ParserVariable::get_flags()
{
  return this->private_->flags_;
}

std::string ParserVariable::get_name()
{
  return this->private_->name_;
}

}

