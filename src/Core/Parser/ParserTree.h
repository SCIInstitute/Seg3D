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

#ifndef CORE_PARSER_PARSERTREE_H 
#define CORE_PARSER_PARSERTREE_H 

// STL includes
#include <string>

// Core includes
#include <Core/Parser/ParserFWD.h>

namespace Core
{

// Hide header includes, private interface and implementation
class ParserTreePrivate;
typedef boost::shared_ptr< ParserTreePrivate > ParserTreePrivateHandle;

/// ParserTree : This class is the toplevel class of an expression. It binds
///              the output variable with the tree of nodes that describe
///              how a variable is computed.
class ParserTree
{
public:
  // Constructor
  ParserTree( std::string varname, ParserNodeHandle expression );

  /// Retrieve the name of the variable that needs to be assigned
  std::string get_varname();

  /// Retrieve the tree for computing the expression
  ParserNodeHandle get_expression_tree();

  /// Set expression tree
  void set_expression_tree( ParserNodeHandle& handle );

  /// Set the final output type of the expression
  void set_type( std::string type );

  /// Retrieve final output type
  std::string get_type();

  // For debugging
  void print();

private:
  ParserTreePrivateHandle private_;
};

}

#endif
