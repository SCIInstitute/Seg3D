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
#include <Core/Parser/ParserNode.h>
#include <Core/Parser/ParserTree.h>
#include <Core/Parser/ParserVariable.h>

namespace Core
{

class ParserTreePrivate
{
public:
  // The name of the variable that needs to be assigned
  std::string varname_;

  // The tree of functions that need to be called to compute this variable
  ParserNodeHandle expression_;

  // Return type of the expression
  std::string type_;
};

ParserTree::ParserTree( std::string varname, ParserNodeHandle expression ) :
  private_( new ParserTreePrivate )
{
  this->private_->varname_ = varname;
  this->private_->expression_ = expression;
  this->private_->type_ = "U";
}

// Print function for debugging
void ParserTree::print()
{
  std::cout << "NEW VARIABLE NAME (" << ParserVariableType( this->private_->type_ ) << "):" 
    << this->private_->varname_ << "\n";
  // Print the contents of the tree
  this->private_->expression_->print( 1 );
}

std::string ParserTree::get_varname()
{
  return this->private_->varname_;
}

ParserNodeHandle ParserTree::get_expression_tree()
{
  return this->private_->expression_;
}

void ParserTree::set_expression_tree( ParserNodeHandle& handle )
{
  this->private_->expression_ = handle;
}

void ParserTree::set_type( std::string type )
{
  this->private_->type_ = type;
}

std::string ParserTree::get_type()
{
  return this->private_->type_;
}

}

