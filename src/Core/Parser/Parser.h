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

#ifndef CORE_PARSER_PARSER_H 
#define CORE_PARSER_PARSER_H 

// STL includes
#include <string>
#include <vector>

// Core includes 
#include <Core/Parser/ParserFWD.h>

namespace Core
{

// The parser builds a tree based on these classes
// ParserNode : This class is a piece of an expression. The ParserNode
//              class marks: functions, variables, constants, and string
//              constants. This are currently the pieces that are supported
// ParserTree : This class is the toplevel class of an expression. It binds
//              the output variable with the tree of nodes that describe
//              how a variable is computed.
// ParserProgram: This class simply binds a series of expressions together.
// ParserVariable: A variable for the the parser program. This class contains
//              the names of the variables that are defined at the start of the
//              program and which need to be defined at the end of the program
// ParserFunctionCatalog: This class contains a list of functions that are
//              available, and hence is used for validating the program

// Hide header includes, private interface and implementation
class ParserPrivate;
typedef boost::shared_ptr< ParserPrivate > ParserPrivateHandle;

std::string ParserFunctionID( std::string name );
std::string ParserFunctionID( std::string name, int arg1 );
std::string ParserFunctionID( std::string name, int arg1, int arg2 );
std::string ParserFunctionID( std::string name, std::vector< int > args );

//-----------------------------------------------------------------------------
// Below the code for the parser starts
// The parser takes a piece of raw code and analyzes its contents and creates
// a program tree that splits the pieces of the expression in the right
// hierarchical order.
class Parser
{
public:
  // Constructor
  Parser();

  bool parse( ParserProgramHandle& program, std::string expressions, std::string& error );

  bool add_input_variable( ParserProgramHandle& program, std::string name, std::string type,
    int flags = 0 );

  bool add_output_variable( ParserProgramHandle& program, std::string name, 
    std::string type = "U", int flags = 0 );

  bool add_output_variable( ParserProgramHandle& program, std::string name );

  bool get_input_variable_type( ParserProgramHandle& program, std::string name, 
    std::string& type );

  bool get_output_variable_type( ParserProgramHandle& program, std::string name, 
    std::string& type );

  bool get_input_variable_type( ParserProgramHandle& program, std::string name, std::string& type,
    int& flags );

  bool get_output_variable_type( ParserProgramHandle& program, std::string name, std::string& type,
    int& flags );

  bool validate( ParserProgramHandle& program, ParserFunctionCatalogHandle& catalog,
    std::string& error );

  bool optimize( ParserProgramHandle& program, std::string& error );

  //--------------------------------------------------------------------------
  // Setup of parser

  // These functions add or overload the function names of default operators
  // and even add additional operators if a new one is specified.

  // These functions are intended for customizing the parser
  // NOTE OPERATOR NAMES CURRENTLY CANNOT BE LONGER THAN TWO CHARACTERS
  // IF LONGER OPERATOR NAMES ARE NEEDED THE CODE SHOULD BE UPGRADED
  void add_binary_operator( std::string op, std::string funname, int priority );
  void add_unary_pre_operator( std::string op, std::string funname );
  void add_unary_post_operator( std::string op, std::string funname );

  // Mark special variable names as constants
  void add_numerical_constant( std::string name, float val );

private:
  ParserPrivateHandle private_;
};

}

#endif
