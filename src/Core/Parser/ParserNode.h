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

#ifndef CORE_PARSER_PARSERNODE_H 
#define CORE_PARSER_PARSERNODE_H 

// STL includes
#include <string>

// Core includes
#include <Core/Parser/ParserFWD.h>

namespace Core
{

// Hide header includes, private interface and implementation
class ParserNodePrivate;
typedef boost::shared_ptr< ParserNodePrivate > ParserNodePrivateHandle;

/// ParserNode : This class is a piece of an expression. The ParserNode
///              class marks: functions, variables, constants, and string
///              constants. This are currently the pieces that are supported
class ParserNode
{
public:
  /// Constructor 1
  ParserNode( int kind, std::string value );

  /// Constructor 2
  ParserNode( int kind, std::string value, std::string type );

  /// Retrieve the kind of the node (this is function, constant, string, etc)
  int get_kind();

  /// Retrieve the type of the node (this is the return type of the variable)
  std::string get_type();

  /// Retrieve the value of the value
  std::string get_value();

  /// Set the value of the node
  void set_value( std::string& value );

  /// Get a pointer to one of the arguments of a function
  ParserNodeHandle get_arg( size_t j );

  /// Set the argument to a sub tree
  void set_arg( size_t j, ParserNodeHandle& handle );

  /// Set the argument to a sub tree (using pointers)
  void set_arg( size_t j, ParserNode* ptr );

  /// Set a copy to the function pointer, this is used later by the interpreter
  void set_function( ParserFunction* func );

  /// Retrieve the function pointer
  ParserFunction* get_function();

  /// Set the type of the node (this is used by the validator)
  void set_type( std::string type );

  /// Retrieve the number of arguments a function has
  size_t num_args();

  /// For debugging
  void print( int level );

private:
  ParserNodePrivateHandle private_;
};

}

#endif
