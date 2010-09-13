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

#ifndef CORE_PARSER_PARSER_H 
#define CORE_PARSER_PARSER_H 

// STL includes
#include <list>
#include <map>
#include <string>
#include <vector>

// Boost includes
#include <boost/smart_ptr.hpp> // Needed for shared_ptr
#include <boost/thread.hpp>

// Local includes -- these could be moved to a ParserFWD.h if needed 
#include <Core/Parser/ParserFunction.h>
#include <Core/Parser/ParserNode.h>
#include <Core/Parser/ParserProgram.h>
#include <Core/Parser/ParserTree.h>
#include <Core/Parser/ParserVariable.h>

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

  // Classes for storing the operator information
  class BinaryOperator
  {
  public:
    std::string operator_;
    std::string funname_;
    int priority_;
  };

  class UnaryOperator
  {
  public:
    std::string operator_;
    std::string funname_;
  };

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
  void add_numerical_constant( std::string name, double val );

private:
  // Get an expression from the program code. Expressions are read sequentially
  // and this function will modify the input and remove the first expression.
  bool scan_expression( std::string& expressions, std::string& expression );

  // This function breaks down an expression into two components
  // The part to the left of the equal sign
  // The part to the right of the equal sign
  // The variable name is checked to see whether it is valid
  // The expression part is not parsed by this function that is done separately
  bool split_expression( std::string expression, std::string& varname, std::string& vartree );

  // Parse the parts on the right hand side of the equal sign of the expression
  bool parse_expression_tree( std::string expression, ParserNodeHandle& ehandle,
    std::string& error );

  // Scan whether the string starts with a sub expression, i.e. '( ..... )'
  bool scan_sub_expression( std::string& expression, std::string& subexpression );

  // Scan whether the string starts with a subs assignment expression, i.e. '[ ..... ]'
  bool scan_subs_expression( std::string& expression, std::string& subexpression );

  // Scan for a numeric constant at the start of the string
  // A numeric constant must start with a number
  // Numbers like 0x12 077 1.23E-8 etc should all be detected by this
  // function. It strips out the number and returns that piece of the string
  bool scan_constant_value( std::string& expression, std::string& value );

  // Scan for a variable name at the start of the string
  // A variable name should start with a-zA-Z or _ and can have
  // numbers for the second character and further.
  // It should not contain any spaces.
  // This function checks as well if the name is followed by spaces and '('
  // If so it is not a variable name, but a function name
  // This function strips out the variable name out of expression and
  // returns in var_name
  bool scan_variable_name( std::string& expression, std::string& var_name );

  // Scan for a string. A srting is a piece of code between quotes e.g. "foo"
  // This function should recognize most escape characters in the string and
  // will translate them as well
  bool scan_constant_string( std::string& expression, std::string& str );

  // Scan for a function. A function starts with a variable name, but is
  // followed by optional spaces and then (...,....,...), which is the 
  // argument list. This function should scan for those argument lists
  bool scan_function( std::string& expression, std::string& function );

  // Scan for the equal sign, this is used to parse expressions
  // Each expression should contain one
  bool scan_equal_sign( std::string& expression );

  // Split a string with code for a function into the function name and into
  // its arguments
  void split_function( std::string& expression, std::string& fun_name,
    std::vector< std::string >& fun_args );

  // Split a string with code for a subs function into its arguments
  void split_subs( std::string& expression, std::vector< std::string >& start_args,
    std::vector< std::string >& step_args, std::vector< std::string >& end_args,
    std::string& varname );

  // Check for syntax errors in the code
  bool check_syntax( std::string& expression, std::string& error );

  // Scan whether the expression starts with a binary operator
  bool scan_binary_operator( std::string& expression, std::string& binary_operator );

  // Scan whether the expression starts with an unary operator
  bool scan_pre_unary_operator( std::string& expression, std::string& unary_operator );

  // Scan whether the expression starts with an unary operator
  bool scan_post_unary_operator( std::string& expression, std::string& unary_operator );

  // Get the functionname of the unary operator
  // The name that should be in the FunctionCatalog
  bool get_unary_function_name( std::string& unary_operator, std::string& fun_name );

  // Get the functionname of the binary operator
  // The name that should be in the FunctionCatalog
  bool get_binary_function_name( std::string& binary_operator, std::string& fun_name );

  // Binary operators need a priority in which they are evaluated
  // This one returns the priority of each operator
  bool get_binary_priority( std::string& binary_operator, int& priority );

  //------------------------------------------------------------
  // General functions for parsing text    
  // Remove a parentices pair at the start and end of the string
  bool remove_global_parentices( std::string& expression );

  // Remove // and /* comments from the expression
  void remove_comments( std::string& expression );

  //------------------------------------------------------------
  // Sub functions for validation
  bool recursive_validate( ParserNodeHandle& handle, ParserFunctionCatalogHandle& fhandle,
    ParserVariableList& var_list, std::string& error, std::string& expression );

  //------------------------------------------------------------
  // Sub functions for optimization
  void optimize_mark_used( ParserScriptFunctionHandle& fhandle );

  bool optimize_process_node( ParserNodeHandle& nhandle,
    std::list< ParserScriptVariableHandle >& variables,
    std::map< std::string, ParserScriptVariableHandle >& named_variables,
    std::list< ParserScriptFunctionHandle >& functions,
    ParserScriptVariableHandle& ohandle, int& cnt, std::string& error );

private:
  // List of supported binary operators e.g. + - * /
  std::map< std::string, BinaryOperator > binary_operators_;
  // List of pre unary opertors e.g. + - ! ~
  std::map< std::string, UnaryOperator > unary_pre_operators_;
  // List of post unary opertors e.g. '
  std::map< std::string, UnaryOperator > unary_post_operators_;

  // List of numerical constants, e.g. nan, inf, false, true etc.
  std::map< std::string, double > numerical_constants_;

};

}

#endif
