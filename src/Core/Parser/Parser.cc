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
#include <list>
#include <map>
#include <math.h>

// Core includes
#include <Core/Math/MathFunctions.h>
#include <Core/Parser/Parser.h> 
#include <Core/Parser/ParserEnums.h>
#include <Core/Parser/ParserFunction.h>
#include <Core/Parser/ParserFunctionCatalog.h>
#include <Core/Parser/ParserNode.h>
#include <Core/Parser/ParserProgram.h>
#include <Core/Parser/ParserScriptFunction.h>
#include <Core/Parser/ParserScriptVariable.h>
#include <Core/Parser/ParserTree.h>
#include <Core/Parser/ParserVariable.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{

std::string ParserFunctionID( std::string name )
{
  std::string fid = name + "$";
  return fid;
}

std::string ParserFunctionID( std::string name, std::string arg1 )
{
  std::string fid = name + "$" + arg1;
  return fid;
}

std::string ParserFunctionID( std::string name, std::string arg1, std::string arg2 )
{
  std::string fid = name + "$" + arg1 + ":" + arg2;
  return fid;
}

std::string ParserFunctionID( std::string name, std::vector< std::string > args )
{
  std::string fid = name + "$";
  for ( size_t j = 0; j < args.size(); j++ )
  {
    fid += args[ j ];
    if ( j < ( args.size() - 1 ) ) 
    {
      fid += ":";
    }
  }
  return fid;
}

class ParserPrivate
{
public:

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
  // Remove a parentheses pair at the start and end of the string
  bool remove_global_parentheses( std::string& expression );

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

  // List of supported binary operators e.g. + - * /
  std::map< std::string, BinaryOperator > binary_operators_;
  // List of pre unary opertors e.g. + - ! ~
  std::map< std::string, UnaryOperator > unary_pre_operators_;
  // List of post unary opertors e.g. '
  std::map< std::string, UnaryOperator > unary_post_operators_;

  // List of numerical constants, e.g. nan, inf, false, true etc.
  std::map< std::string, float > numerical_constants_;
};

// Strip of an expression. An expressions is a string ending with a semi-colon.
// The code makes sure that semi colons in strings are ignored.
// Every other semi-colon is view as an end of an expression
bool ParserPrivate::scan_expression( std::string& expressions, std::string& expression )
{
  size_t esize = expressions.size();
  size_t idx = 0;

  while ( idx < esize )
  {
    if ( expressions[ idx ] == '"' )
    {
      idx++;
      while ( idx < esize )
      {
        if ( expression[ idx ] == '\\' ) 
        {
          idx += 2;
        }
        else if ( expressions[ idx ] == '"' ) 
        {
          break;
        } 
        else 
        {
          idx++;
        }
      }
    }
    else if ( expressions[ idx ] == ';' )
    {
      expression = expressions.substr( 0, idx );

      // Strip out any remaining space at the end of an expression
      idx++;
      while ( ( idx < esize ) && ( ( expressions[ idx ] == ' ' ) || ( expressions[ idx ]
      == '\t' ) || ( expressions[ idx ] == '\n' ) || ( expressions[ idx ] == '\r' )
        || ( expressions[ idx ] == '\v' ) || ( expressions[ idx ] == '\f' ) ) )
        idx++;
      if ( idx < esize ) expressions = expressions.substr( idx );
      else expressions = "";

      // If expression is empty we have reached the end of the program
      return true;
    }
    idx++;
  }

  // Currently we do not demand a semi colon at the of the last expression
  expression = expressions.substr( 0, idx );
  expressions = "";

  // If expression is empty we have reached the end of the program
  return true;
}

bool ParserPrivate::split_expression( std::string expression, std::string& varname, std::string& vartree )
{
  // Strip out space at the start and at the end of the string
  StripSurroundingSpaces( expression );

  // Scan to see if the first text is a variable name
  if ( !( this->scan_variable_name( expression, varname ) ) ) return false;

  // There can be spaces between the variable name and the equal sign
  StripSurroundingSpaces( expression );

  // if we have a subexpression store it in here
  std::string subs;

  // Check whether we are using subscripts on lefthand side
  if ( this->scan_subs_expression( expression, subs ) )
  {
    StripSurroundingSpaces( expression );
  }

  // Scan for the equal sign
  if ( !( this->scan_equal_sign( expression ) ) ) return false;

  // Strip spaces again
  StripSurroundingSpaces( expression );

  // The remainder is the code for building the variable
  vartree = expression;

  if ( subs.size() )
  {
    std::vector < std::string > indices_start;
    std::vector < std::string > indices_step;
    std::vector < std::string > indices_end;
    this->split_subs( subs, indices_start, indices_step, indices_end, varname );

    // push it of the rest of the interpreter
    if ( indices_end.size() == 0 )
    {
      vartree = "assign(" + varname + "," + vartree;
      for ( size_t j = 0; j < indices_start.size(); j++ )
        vartree += std::string( "," ) + indices_start[ j ];
      vartree += ")";
    }
    else if ( indices_step.size() == 0 )
    {
      vartree = "assign_range(" + varname + "," + vartree;
      for ( size_t j = 0; j < indices_start.size(); j++ )
      {
        vartree += std::string( "," ) + indices_start[ j ];
        vartree += std::string( "," ) + indices_end[ j ];
      }
      vartree += ")";
    }
    else
    {
      vartree = "assign_steprange(" + varname + "," + vartree;
      for ( size_t j = 0; j < indices_start.size(); j++ )
      {
        vartree += std::string( "," ) + indices_start[ j ];
        vartree += std::string( "," ) + indices_step[ j ];
        vartree += std::string( "," ) + indices_end[ j ];
      }
      vartree += ")";
    }
  }

  // DEBUG CALL
#ifdef DEBUG
  //  std::cerr << "vartree="<< vartree << "\n";  
#endif

  // Success
  return true;
}

// The main function for disecting code into a tree
bool ParserPrivate::parse_expression_tree( std::string expression, ParserNodeHandle& handle,
  std::string& error )
{
  // Clear handle so what was in it is cleared
  handle.reset();

  // Extra check to make sure parentheses levels match and
  // to check whether strings are properly marked
  // This function will return a syntax error if there is one
  if ( !( this->check_syntax( expression, error ) ) )
  {
    return false;
  }

  // Strip any spaces at the start or and of the end of the expression
  StripSurroundingSpaces( expression );

  // Remove any parentheses that open at the start and close at the end
  // These are not strictly needed and hence should be removed
  while ( this->remove_global_parentheses( expression ) )
  {
    // remove all spaces at the start of the expression
    StripSurroundingSpaces( expression );
  }

  // Remainder is the remainder of the text that needs to be scanned
  std::string remainder = expression;

  // After removing the parentheses, spaces can be left in the front
  StripSurroundingSpaces( remainder );

  // Component is an expression, a variable name, a function or a constant
  std::string component;

  // Each component can have an unary operator to the end or the front of
  // that block
  std::string unary_operator;
  std::string post_unary_operator;
  std::string subs;

  // List of all components we have scanned
  std::vector < std::string > components;

  // Between components binary operators are needed to link them together
  std::string binary_operator;
  std::vector < std::string > binary_operators;

  // Record the binary operator priorities
  std::vector< int > binary_priority;

  // SCAN THE MAIN STRUCTURE OF THE EXPRESSION
  while ( remainder.size() )
  {
    // Scan whether the expression starts with an unary operator
    unary_operator = "";
    this->scan_pre_unary_operator( remainder, unary_operator );
    StripSurroundingSpaces( remainder );

    if ( this->scan_variable_name( remainder, component ) )
    {
      StripSurroundingSpaces( remainder );
      this->scan_subs_expression( remainder, subs );
      StripSurroundingSpaces( remainder );

      this->scan_post_unary_operator( remainder, post_unary_operator );
      StripSurroundingSpaces( remainder );
      components.push_back( unary_operator + component + subs + post_unary_operator );
    }
    else if ( this->scan_constant_value( remainder, component ) )
    {
      StripSurroundingSpaces( remainder );
      this->scan_post_unary_operator( remainder, post_unary_operator );
      StripSurroundingSpaces( remainder );
      components.push_back( unary_operator + component + post_unary_operator );
    }
    else if ( this->scan_constant_string( remainder, component ) )
    {
      StripSurroundingSpaces( remainder );
      this->scan_post_unary_operator( remainder, post_unary_operator );
      StripSurroundingSpaces( remainder );
      components.push_back( unary_operator + component + post_unary_operator );
    }
    else if ( this->scan_function( remainder, component ) )
    {
      StripSurroundingSpaces( remainder );
      this->scan_post_unary_operator( remainder, post_unary_operator );
      StripSurroundingSpaces( remainder );

      this->scan_subs_expression( remainder, subs );
      StripSurroundingSpaces( remainder );
      components.push_back( unary_operator + component + subs + post_unary_operator );
    }
    else if ( this->scan_sub_expression( remainder, component ) )
    {
      StripSurroundingSpaces( remainder );
      this->scan_post_unary_operator( remainder, post_unary_operator );
      StripSurroundingSpaces( remainder );

      this->scan_subs_expression( remainder, subs );
      StripSurroundingSpaces( remainder );
      components.push_back( unary_operator + component + subs + post_unary_operator );
    }
    else
    {
      error = "SYNTAX ERROR - invalid syntax detected at the start of '" + remainder + "'";
      return false;
    }

    StripSurroundingSpaces( remainder );
    if ( remainder.size() == 0 ) break;
    binary_operator = "";

    if ( !( this->scan_binary_operator( remainder, binary_operator ) ) )
    {
      error = "SYNTAX ERROR - invalid syntax detected at the start of '" + remainder + "'";
      return false;
    }

    binary_operators.push_back( binary_operator );

    int priority;
    this->get_binary_priority( binary_operator, priority );
    binary_priority.push_back( priority );

    StripSurroundingSpaces( remainder );
    if ( remainder.size() == 0 )
    {
      error = "SYNTAX ERROR - invalid syntax detected at the end of '" + expression + "'";
      return false;
    }
  }

  std::vector < ParserNodeHandle > component_handles( components.size() );

  for ( size_t k = 0; k < component_handles.size(); k++ )
  {
    component = components[ k ];
    ParserNodeHandle pre_function_handle;
    ParserNodeHandle post_function_handle;
    ParserNodeHandle subs_function_handle;

    std::string str;

    if ( this->scan_pre_unary_operator( component, unary_operator ) )
    {
      std::string fun_name;
      this->get_unary_function_name( unary_operator, fun_name );
      if ( fun_name != "pos" )
      {
        pre_function_handle = 
          ParserNodeHandle( new ParserNode( PARSER_FUNCTION_E, fun_name ) );
      }
    }

    if ( this->scan_variable_name( component, str ) )
    {
      std::map< std::string, float >::iterator cit, cit_end;
      cit = this->numerical_constants_.begin();
      cit_end = this->numerical_constants_.end();

      while ( cit != cit_end )
      {
        if ( str == ( *cit ).first )
        {
          component_handles[ k ] = 
            ParserNodeHandle( new ParserNode( PARSER_CONSTANT_SCALAR_E, str ) );
          component_handles[ k ]->set_type( "S" ); // Scalar type
          break;
        }
        ++cit;
      }
      if ( cit == cit_end )
      {
        component_handles[ k ] = 
          ParserNodeHandle( new ParserNode( PARSER_VARIABLE_E, str ) );
      }
    }
    else if ( this->scan_constant_value( component, str ) )
    {
      component_handles[ k ] = 
        ParserNodeHandle( new ParserNode( PARSER_CONSTANT_SCALAR_E, str ) );
      component_handles[ k ]->set_type( "S" ); // Scalar type
    }
    else if ( this->scan_constant_string( component, str ) )
    {
      component_handles[ k ] = 
        ParserNodeHandle( new ParserNode( PARSER_CONSTANT_STRING_E, str ) );
      component_handles[ k ]->set_type( "A" ); // String type
    }
    else if ( this->scan_function( component, str ) )
    {
      std::string fun_name;
      std::vector < std::string > fun_args;
      this->split_function( str, fun_name, fun_args );

      component_handles[ k ] = 
        ParserNodeHandle( new ParserNode( PARSER_FUNCTION_E, fun_name ) );
      for ( size_t j = 0; j < fun_args.size(); j++ )
      {
        ParserNodeHandle subhandle;
        if ( !( this->parse_expression_tree( fun_args[ j ], subhandle, error ) ) )
        {
          return false;
        }
        component_handles[ k ]->set_arg( j, subhandle );
      }
    }
    else if ( this->scan_sub_expression( component, str ) )
    {
      if ( !( this->parse_expression_tree( str, component_handles[ k ], error ) ) )
      {
        return false;
      }
    }

    if ( this->scan_subs_expression( component, subs ) )
    {
      std::vector < std::string > start_args;
      std::vector < std::string > step_args;
      std::vector < std::string > end_args;

      this->split_subs( subs, start_args, step_args, end_args, str );

      std::string subs_expression;
      if ( end_args.size() == 0 )
      {
        subs_function_handle = 
          ParserNodeHandle( new ParserNode( PARSER_FUNCTION_E, "subs" ) );
        for ( size_t j = 0; j < start_args.size(); j++ )
        {
          ParserNodeHandle subhandle;
          if ( !( this->parse_expression_tree( start_args[ j ], subhandle, error ) ) )
          {
            return false;
          }
          subs_function_handle->set_arg( j + 1, subhandle );
        }
      }
      else if ( step_args.size() == 0 )
      {
        subs_function_handle = 
          ParserNodeHandle( new ParserNode( PARSER_FUNCTION_E, "subs_range" ) );
        for ( size_t j = 0; j < start_args.size(); j++ )
        {
          ParserNodeHandle subhandle;
          if ( !( this->parse_expression_tree( start_args[ j ], subhandle, error ) ) )
          {
            return false;
          }
          subs_function_handle->set_arg( 2 * j + 1, subhandle );
          if ( !( this->parse_expression_tree( end_args[ j ], subhandle, error ) ) )
          {
            return false;
          }
          subs_function_handle->set_arg( 2 * j + 2, subhandle );
        }
      }
      else
      {
        subs_function_handle = ParserNodeHandle( 
          new ParserNode( PARSER_FUNCTION_E, "subs_steprange" ) );
        for ( size_t j = 0; j < start_args.size(); j++ )
        {
          ParserNodeHandle subhandle;
          if ( !( this->parse_expression_tree( start_args[ j ], subhandle, error ) ) )
          {
            return false;
          }
          subs_function_handle->set_arg( 3 * j + 1, subhandle );
          if ( !( this->parse_expression_tree( step_args[ j ], subhandle, error ) ) )
          {
            return false;
          }
          subs_function_handle->set_arg( 3 * j + 2, subhandle );
          if ( !( this->parse_expression_tree( end_args[ j ], subhandle, error ) ) )
          {
            return false;
          }
          subs_function_handle->set_arg( 3 * j + 3, subhandle );
        }
      }
    }

    if ( this->scan_post_unary_operator( component, unary_operator ) )
    {
      std::string fun_name;
      this->get_unary_function_name( unary_operator, fun_name );
      post_function_handle = ParserNodeHandle( new ParserNode( PARSER_FUNCTION_E, fun_name ) );
    }

    if ( subs_function_handle.get() )
    {
      ParserNodeHandle subhandle = component_handles[ k ];
      component_handles[ k ] = subs_function_handle;
      component_handles[ k ]->set_arg( 0, subhandle );
    }

    if ( post_function_handle.get() )
    {
      ParserNodeHandle subhandle = component_handles[ k ];
      component_handles[ k ] = post_function_handle;
      component_handles[ k ]->set_arg( 0, subhandle );
    }

    if ( pre_function_handle.get() )
    {
      ParserNodeHandle subhandle = component_handles[ k ];
      component_handles[ k ] = pre_function_handle;
      component_handles[ k ]->set_arg( 0, subhandle );
    }
  }

  if ( components.size() == 1 )
  {
    handle = component_handles[ 0 ];
    return true;
  }
  else if ( components.size() > 1 )
  {
    while ( 1 )
    {
      int priority = -1;
      for ( size_t j = 0; j < binary_priority.size(); j++ )
      {
        if ( binary_priority[ j ] > priority ) priority = binary_priority[ j ];
      }

      if ( priority == -1 ) break;

      std::string fun_name;

      for ( size_t j = 0; j < binary_priority.size(); j++ )
      {
        if ( priority == binary_priority[ j ] )
        {
          this->get_binary_function_name( binary_operators[ j ], fun_name );
          ParserNodeHandle nhandle( new ParserNode( PARSER_FUNCTION_E, fun_name ) );
          size_t k1 = j;
          while ( component_handles[ k1 ].get() == 0 )
            k1--;
          size_t k2 = j + 1;
          while ( component_handles[ k2 ].get() == 0 )
            k2++;

          nhandle->set_arg( 0, component_handles[ k1 ] );
          nhandle->set_arg( 1, component_handles[ k2 ] );

          component_handles[ k1 ] = nhandle;
          component_handles[ k2 ].reset();
          binary_priority[ j ] = -1;
          break;
        }
      }
    }
    handle = component_handles[ 0 ];
    return true;
  }
  else
  {
    error = "SYNTAX ERROR - error in expression '" + expression + "': expression is incomplete";
    return false;
  }
}

bool ParserPrivate::scan_binary_operator( std::string& expression, std::string& binary_operator )
{
  if ( expression.size() > 1 )
  {
    binary_operator = expression.substr( 0, 2 );
    std::map< std::string, BinaryOperator >::iterator it = this->binary_operators_.find(
      binary_operator );
    if ( it != this->binary_operators_.end() )
    {
      expression = expression.substr( 2 );
      return true;
    }
  }

  if ( expression.size() > 0 )
  {
    binary_operator = expression.substr( 0, 1 );
    std::map< std::string, BinaryOperator >::iterator it = this->binary_operators_.find(
      binary_operator );
    if ( it != this->binary_operators_.end() )
    {
      expression = expression.substr( 1 );
      return true;
    }
  }

  binary_operator = "";
  return false;
}

bool ParserPrivate::scan_pre_unary_operator( std::string& expression, std::string& unary_operator )
{
  if ( expression.size() > 1 )
  {
    unary_operator = expression.substr( 0, 2 );
    std::map< std::string, UnaryOperator >::iterator it = this->unary_pre_operators_.find(
      unary_operator );
    if ( it != this->unary_pre_operators_.end() )
    {
      expression = expression.substr( 2 );
      return true;
    }
  }

  if ( expression.size() > 0 )
  {
    unary_operator = expression.substr( 0, 1 );
    std::map< std::string, UnaryOperator >::iterator it = this->unary_pre_operators_.find(
      unary_operator );
    if ( it != this->unary_pre_operators_.end() )
    {
      expression = expression.substr( 1 );
      return true;
    }
  }

  unary_operator = "";
  return false;
}

bool ParserPrivate::scan_post_unary_operator( std::string& expression, std::string& unary_operator )
{
  if ( expression.size() > 1 )
  {
    unary_operator = expression.substr( 0, 2 );
    std::map< std::string, UnaryOperator >::iterator it = this->unary_post_operators_.find(
      unary_operator );
    if ( it != this->unary_post_operators_.end() )
    {
      expression = expression.substr( 2 );
      return true;
    }
  }

  if ( expression.size() > 0 )
  {
    unary_operator = expression.substr( 0, 1 );
    std::map< std::string, UnaryOperator >::iterator it = this->unary_post_operators_.find(
      unary_operator );
    if ( it != this->unary_post_operators_.end() )
    {
      expression = expression.substr( 1 );
      return true;
    }

  }

  unary_operator = "";
  return false;
}

bool ParserPrivate::get_unary_function_name( std::string& unary_operator, std::string& fun_name )
{
  std::map< std::string, UnaryOperator >::iterator it =
    this->unary_pre_operators_.find( unary_operator );
  if ( it != this->unary_pre_operators_.end() )
  {
    fun_name = ( *it ).second.funname_;
    return true;
  }

  it = this->unary_post_operators_.find( unary_operator );
  if ( it != this->unary_post_operators_.end() )
  {
    fun_name = ( *it ).second.funname_;
    return true;
  }
  return false;
}

bool ParserPrivate::get_binary_function_name( std::string& binary_operator, std::string& fun_name )
{
  std::map< std::string, BinaryOperator >::iterator it = 
    this->binary_operators_.find( binary_operator );
  if ( it != this->binary_operators_.end() )
  {
    fun_name = ( *it ).second.funname_;
    return true;
  }
  return false;
}

bool ParserPrivate::get_binary_priority( std::string& binary_operator, int& priority )
{
  std::map< std::string, BinaryOperator >::iterator it = 
    this->binary_operators_.find( binary_operator );
  if ( it != this->binary_operators_.end() )
  {
    priority = ( *it ).second.priority_;
    return true;
  }
  return false;
}

bool ParserPrivate::scan_variable_name( std::string& expression, std::string& var_name )
{
  size_t esize = expression.size();
  if ( ( esize > 0 ) && ( ( expression[ 0 ] == '_' ) || ( expression[ 0 ] >= 'a'
    && expression[ 0 ] <= 'z' ) || ( expression[ 0 ] >= 'A' && expression[ 0 ] <= 'Z' ) ) )
  {
    size_t idx = 1;
    while ( ( idx < esize ) && ( ( expression[ idx ] == '_' ) || ( expression[ idx ] >= 'a'
      && expression[ idx ] <= 'z' )
      || ( expression[ idx ] >= 'A' && expression[ idx ] <= 'Z' ) || ( expression[ idx ]
      >= '0' && expression[ idx ] <= '9' ) ) )
    {
      idx++;
    }


    var_name = expression.substr( 0, idx );

    size_t vidx = idx;
    while ( ( idx < esize ) && ( ( expression[ idx ] == ' ' ) || ( expression[ idx ] == '\t' )
      || ( expression[ idx ] == '\n' ) || ( expression[ idx ] == '\r' ) ) )
    {
      idx++;
    }


    // Check whether it is a function name
    if ( expression[ idx ] == '(' ) 
    {
      return false;
    }
    else
    {
      expression = expression.substr( vidx );
      return true;
    }
  }
  return false;
}

bool ParserPrivate::scan_constant_string( std::string& expression, std::string& str )
{
  size_t esize = expression.size();
  if ( esize )
  {
    if ( expression[ 0 ] == '"' )
    {
      size_t idx = 1;
      str.clear();

      while ( idx < esize )
      {
        if ( expression[ idx ] == '\\' )
        {
          idx++;
          if ( idx >= esize )
          {
            return false;
          }
          if ( expression[ idx ] == 'n' ) str += "\n";
          else if ( expression[ idx ] == 'a' ) str += "\a";
          else if ( expression[ idx ] == 'v' ) str += "\v";
          else if ( expression[ idx ] == '?' ) str += "\?";
          else if ( expression[ idx ] == 'f' ) str += "\f";
          else if ( expression[ idx ] == 't' ) str += "\t";
          else if ( expression[ idx ] == 'r' ) str += "\r";
          else if ( expression[ idx ] == 'b' ) str += "\b";
          else if ( expression[ idx ] == '"' ) str += "\"";
          else if ( expression[ idx ] == '\'' ) str += "\'";
          else if ( expression[ idx ] == '\\' ) str += "\\";
          else if ( expression[ idx ] == '0' ) str += "\0";
          else return false;
        }
        else if ( expression[ idx ] == '"' ) 
        {
          break;
        }
        else
        {
          str += expression[ idx ];
        }
      }

      if ( idx < esize ) 
      {
        return true;
      }
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
  return false;
}

// Scan whether the string starts with a function call

bool ParserPrivate::scan_function( std::string& expression, std::string& function )
{
  // Get the size of the string
  size_t esize = expression.size();
  if ( ( esize > 0 ) && ( ( expression[ 0 ] == '_' ) || ( expression[ 0 ] >= 'a'
    && expression[ 0 ] <= 'z' ) || ( expression[ 0 ] >= 'A' && expression[ 0 ] <= 'Z' ) ) )
  {
    size_t idx = 1;
    while ( ( idx < esize ) && ( ( expression[ idx ] == '_' ) || ( expression[ idx ] >= 'a'
      && expression[ idx ] <= 'z' )
      || ( expression[ idx ] >= 'A' && expression[ idx ] <= 'Z' ) || ( expression[ idx ]
      >= '0' && expression[ idx ] <= '9' ) ) )
    {
      idx++;
    }


    std::string var_name = expression.substr( 0, idx );

    std::map< std::string, float >::iterator cit, cit_end;
    cit = this->numerical_constants_.begin();
    cit_end = this->numerical_constants_.end();

    for ( size_t j = 0; j < this->numerical_constants_.size(); j++ )
    {
      while ( cit != cit_end )
      {
        if ( var_name == ( *cit ).first ) return false;
        ++cit;
      }
    }

    while ( ( idx < esize ) && ( ( expression[ idx ] == ' ' ) || ( expression[ idx ] == '\t' )
      || ( expression[ idx ] == '\n' ) || ( expression[ idx ] == '\r' ) ) )
    {
      idx++;
    }


    // Check whether it is a function name
    if ( expression[ idx ] == '(' )
    {
      int paren_cnt = 1;
      idx++;
      while ( idx < esize )
      {
        if ( expression[ idx ] == '"' )
        {
          idx++;
          while ( idx < esize )
          {
            if ( expression[ idx ] == '\\' ) 
            {
              idx += 2;
            }
            else if ( expression[ idx ] == '"' )
            {
              break;
            }
            else
            {
              idx++;
            }
          }
        }
        else if ( expression[ idx ] == '(' || expression[ idx ] == '[' ) 
        {
          paren_cnt++;
        }
        else if ( expression[ idx ] == ')' || expression[ idx ] == ']' )
        {
          paren_cnt--;
          if ( paren_cnt == 0 )
          {
            idx++;
            break;
          }
        }
        idx++;
      }
      if ( paren_cnt == 0 )
      {
        function = expression.substr( 0, idx );
        expression = expression.substr( idx );
        return true;
      }
    }
    else
    {
      return false;
    }
  }
  return false;
}

// Scan for any sub expression between parentheses
// This code will strip that part out of the string

bool ParserPrivate::scan_sub_expression( std::string& expression, std::string& subexpression )
{
  // Get the size of the string
  size_t esize = expression.size();

  if ( esize )
  {
    // First character of a subexpression needs to be a parenticis
    if ( expression[ 0 ] == '(' )
    {
      int paren_cnt = 1;
      size_t idx = 1;
      // Scan through the reaminder of the string to see where the sub expression
      // ends
      while ( idx < esize )
      {
        // Ignore anything between quotes
        if ( expression[ idx ] == '"' )
        {
          idx++;
          while ( idx < esize )
          {
            if ( expression[ idx ] == '\\' ) 
            {
              idx += 2;
            }
            else if ( expression[ idx ] == '"' ) 
            {
              break;
            }
            else 
            {
              idx++;
            }
          }
        }
        // Do parentheses counting
        else if ( expression[ idx ] == '(' ) 
        {
          paren_cnt++;
        }
        else if ( expression[ idx ] == ')' )
        {
          paren_cnt--;
          // Test whether we hit the end of the sub expression
          if ( paren_cnt == 0 ) 
          {
            break;
          }
        }
        idx++;
      }
      // if we found the end, return the expression
      // The sub expression is analysed later in the code
      // This code only detects the structure and where it should end
      if ( paren_cnt == 0 )
      {
        idx++;
        subexpression = expression.substr( 0, idx );
        expression = expression.substr( idx );
        return true;
      }
    }
  }

  return false;
}

bool ParserPrivate::scan_subs_expression( std::string& expression, std::string& subexpression )
{
  // Get the size of the string
  size_t esize = expression.size();

  if ( esize )
  {
    // First character of a subexpression needs to be a parenticis
    if ( expression[ 0 ] == '[' )
    {
      int brac_cnt = 1;
      size_t idx = 1;
      // Scan through the reaminder of the string to see where the sub expression
      // ends
      while ( idx < esize )
      {
        // Ignore anything between quotes
        if ( expression[ idx ] == '"' )
        {
          idx++;
          while ( idx < esize )
          {
            if ( expression[ idx ] == '\\' ) 
            {
              idx += 2;
            }
            else if ( expression[ idx ] == '"' ) break;
            else idx++;
          }
        }
        // Do parentheses counting
        else if ( expression[ idx ] == '[' ) brac_cnt++;
        else if ( expression[ idx ] == ']' )
        {
          brac_cnt--;
          // Test whether we hit the end of the sub expression
          if ( brac_cnt == 0 ) break;
        }
        idx++;
      }
      // if we found the end, return the expression
      // The sub expression is analysed later in the code
      // This code only detects the structure and where it should end
      if ( brac_cnt == 0 )
      {
        idx++;
        subexpression = expression.substr( 0, idx );
        expression = expression.substr( idx );
        return true;
      }
    }
  }

  subexpression = "";
  return false;
}

// Additional loop to help detect errors and help to generate better
// feedback to the user. This function should contain a couple of simple
// tests to check the syntax of the code
bool ParserPrivate::check_syntax( std::string& expression, std::string& error )
{
  int paren_cnt = 0;

  size_t esize = expression.size();
  size_t idx = 0;
  while ( idx < esize )
  {
    if ( expression[ idx ] == '"' )
    {
      idx++;
      while ( idx < esize )
      {
        if ( expression[ idx ] == '\\' ) idx += 2;
        else if ( expression[ idx ] == '"' ) break;
        else idx++;
      }

      if ( idx == esize )
      {
        error = "SYNTAX ERROR - Incomplete string in expression '" + expression + "'";
        return false;
      }
    }
    else if ( expression[ idx ] == '(' ) paren_cnt++;
    else if ( expression[ idx ] == ')' ) paren_cnt--;
    else if ( expression[ idx ] == ';' )
    {
      error
        = "SYNTAX ERROR - Incomplete expression, found semicolon before end of expression in expression '"
          + expression + "'";
      return false;
    }
    idx++;
  }

  if ( paren_cnt != 0 )
  {
    error = "SYNTAX ERROR - Missing parenthesis in expression '" + expression + "'";
    return false;
  }

  return true;
}

// Scan from a constant number
// This code should recognize whether the string has a number at the start
// of the string

bool ParserPrivate::scan_constant_value( std::string& expression, std::string& value )
{
  size_t esize = expression.size();
  if ( esize )
  {
    if ( expression[ 0 ] == '.' )
    {
      size_t idx = 1;
      while ( ( idx < esize ) && ( ( expression[ idx ] >= '0' && expression[ idx ] <= '9' ) ) )
        idx++;
      if ( idx < esize )
      {
        if ( ( expression[ idx ] == 'e' ) || ( expression[ idx ] == 'E' ) )
        {
          idx++;
          if ( idx < esize )
          {
            if ( expression[ idx ] == '-' ) idx++;
            while ( ( idx < esize ) && ( ( ( expression[ idx ] >= '0' )
              && ( expression[ idx ] <= '9' ) ) ) )
              idx++;
            if ( idx < esize ) if ( ( expression[ idx ] == 'f' ) || ( expression[ idx ]
              == 'F' ) ) idx++;
          }
        }
      }
      value = expression.substr( 0, idx );
      expression = expression.substr( idx );
      return true;
    }

    // Cases that start with a '0'
    if ( expression[ 0 ] == '0' )
    {
      if ( esize > 1 )
      {
        // Detect hexadecimal numbers
        if ( expression[ 1 ] == 'x' || expression[ 1 ] == 'X' )
        {
          size_t idx = 2;
          while ( ( idx < esize ) && ( ( expression[ idx ] >= '0' && expression[ idx ]
            <= '9' ) || ( expression[ idx ] >= 'a' && expression[ idx ] <= 'f' )
            || ( expression[ idx ] >= 'A' && expression[ idx ] <= 'F' ) ) )
          {
            idx++;
          }

          if ( idx < esize )
          {
            if ( ( expression[ idx ] == 'u' ) || ( expression[ idx ] == 'U' ) || 
              ( expression[ idx ] == 'l' ) || ( expression[ idx ] == 'L' ) ) 
            {
              idx++;
            }
          }

          if ( idx < esize )
          {
            if ( ( expression[ idx ] == 'u' ) || ( expression[ idx ] == 'U' ) || 
              ( expression[ idx ] == 'l' ) || ( expression[ idx ] == 'L' ) )
            {
              idx++;
            }
          }

          value = expression.substr( 0, idx );
          expression = expression.substr( idx );
          return true;
        }
        // Detect oct numbers
        else if ( expression[ 1 ] >= '0' && expression[ 1 ] <= '7' )
        {
          size_t idx = 2;
          while ( ( idx < esize ) && ( ( expression[ idx ] >= '0' && expression[ idx ]
            <= '7' ) ) )
          {
            idx++;
          }

          if ( idx < esize )
          {
            if ( ( expression[ idx ] == 'u' ) || ( expression[ idx ] == 'U' ) || 
              ( expression[ idx ] == 'l' ) || ( expression[ idx ] == 'L' ) )
            {
              idx++;
            }
          }

          if ( idx < esize ) 
          {
            if ( ( expression[ idx ] == 'u' ) || ( expression[ idx ] == 'U' ) || 
              ( expression[ idx ] == 'l' ) || ( expression[ idx ] == 'L' ) )
            {
              idx++;
            }
          }

          value = expression.substr( 0, idx );
          expression = expression.substr( idx );
          return true;
        }
        // Detect fractional numbers and exponential numbers
        else if ( expression[ 1 ] == '.' )
        {
          size_t idx = 2;
          while ( ( idx < esize ) && ( ( expression[ idx ] >= '0' && expression[ idx ]
            <= '9' ) ) )
            idx++;
          if ( idx < esize )
          {
            if ( ( expression[ idx ] == 'e' ) || ( expression[ idx ] == 'E' ) )
            {
              idx++;
              if ( idx < esize )
              {
                if ( expression[ idx ] == '-' ) idx++;
                while ( ( idx < esize ) && ( ( ( expression[ idx ] >= '0' )
                  && ( expression[ idx ] <= '9' ) ) ) )
                  idx++;
                if ( idx < esize ) if ( ( expression[ idx ] == 'f' )
                  || ( expression[ idx ] == 'F' ) ) idx++;
              }
            }
          }
          value = expression.substr( 0, idx );
          expression = expression.substr( idx );
          return true;
        }
        else if ( ( expression[ 1 ] == 'e' ) || ( expression[ 1 ] == 'E' ) )
        {
          size_t idx = 2;
          if ( idx < esize )
          {
            if ( expression[ idx ] == '-' ) idx++;
            while ( ( idx < esize ) && ( ( ( expression[ idx ] >= '0' )
              && ( expression[ idx ] <= '9' ) ) ) )
              idx++;
            if ( idx < esize ) if ( ( expression[ idx ] == 'f' ) || ( expression[ idx ]
              == 'F' ) ) idx++;
          }
          value = expression.substr( 0, idx );
          expression = expression.substr( idx );
          return true;
        }
        else
        {
          value = expression.substr( 0, 1 );
          expression = expression.substr( 1 );
          return true;
        }
      }
      else
      {
        value = expression.substr( 0, 1 );
        expression = expression.substr( 1 );
        return true;
      }
    }

    // Cases that start with a '1' to '9'
    if ( ( expression[ 0 ] >= '1' ) && ( expression[ 0 ] <= '9' ) )
    {
      if ( esize > 1 )
      {
        size_t idx = 1;
        while ( ( idx < esize )
          && ( ( expression[ idx ] >= '0' && expression[ idx ] <= '9' ) ) )
          idx++;
        if ( idx < esize )
        {
          if ( expression[ idx ] == '.' )
          {
            idx++;
            while ( ( idx < esize ) && ( ( expression[ idx ] >= '0' && expression[ idx ]
              <= '9' ) ) )
              idx++;
            if ( idx < esize )
            {
              if ( ( expression[ idx ] == 'e' ) || ( expression[ idx ] == 'E' ) )
              {
                idx++;
                if ( idx < esize )
                {
                  if ( expression[ idx ] == '-' ) idx++;
                  while ( ( idx < esize ) && ( ( ( expression[ idx ] >= '0' )
                    && ( expression[ idx ] <= '9' ) ) ) )
                    idx++;
                  if ( idx < esize ) if ( ( expression[ idx ] == 'f' )
                    || ( expression[ idx ] == 'F' ) ) idx++;
                }
              }
            }
            value = expression.substr( 0, idx );
            expression = expression.substr( idx );
            return true;
          }
          else if ( ( expression[ idx ] == 'e' ) || ( expression[ idx ] == 'E' ) )
          {
            idx++;
            if ( idx < esize )
            {
              if ( expression[ idx ] == '-' ) idx++;
              while ( ( idx < esize ) && ( ( ( expression[ idx ] >= '0' )
                && ( expression[ idx ] <= '9' ) ) ) )
                idx++;
              if ( idx < esize ) if ( ( expression[ idx ] == 'f' )
                || ( expression[ idx ] == 'F' ) ) idx++;
            }
            value = expression.substr( 0, idx );
            expression = expression.substr( idx );
            return true;
          }
          else
          {
            // Remove any code that tellls the number is long or unsigned
            if ( idx < esize )
            {
              if ( ( expression[ idx ] == 'u' ) || ( expression[ idx ] == 'U' ) || 
                ( expression[ idx ] == 'l' ) || ( expression[ idx ] == 'L' ) )
              {
                idx++;
              }
            }
            if ( idx < esize ) if ( ( expression[ idx ] == 'u' ) || 
              ( expression[ idx ] == 'U' ) || ( expression[ idx ] == 'l' ) || 
              ( expression[ idx ] == 'L' ) ) 
            {
              idx++;
            }

            value = expression.substr( 0, idx );
            expression = expression.substr( idx );
            return true;
          }
        }
        else
        {
          value = expression.substr( 0, idx );
          expression = expression.substr( idx );
          return true;
        }
      }
      else
      {
        // Case of a single number 
        value = expression.substr( 0, 1 );
        expression = expression.substr( 1 );
        return true;
      }
    }
  }
  return false;
}

// Finding an equal sign that denotes an expression

bool ParserPrivate::scan_equal_sign( std::string& expression )
{
  size_t esize = expression.size();
  if ( esize == 0 ) return false;

  if ( expression[ 0 ] == '=' )
  {
    if ( esize > 1 )
    {
      // These indicate different operators adn should be
      // ignored, most likely this means there is a syntax error
      // in the code
      if ( expression[ 1 ] == '=' ) return false;
      if ( expression[ 1 ] == '<' ) return false;
      if ( expression[ 1 ] == '>' ) return false;
    }
    expression = expression.substr( 1 );
    return true;
  }
  return false;
}

// Split function in different parts
// Assume a function is denoted as funname(arg1,arg2,arg3,...)

void ParserPrivate::split_function( std::string& expression, std::string& fun_name, std::vector<
                   std::string >& fun_args )
{
  // Get the size of the srting we need to scan
  size_t esize = expression.size();
  // First detect the function name
  // A function name starts with _A-Za-z
  // Any other characters can have numbers in them as well
  if ( ( esize > 0 ) && ( ( expression[ 0 ] == '_' ) || ( expression[ 0 ] >= 'a'
    && expression[ 0 ] <= 'z' ) || ( expression[ 0 ] >= 'A' && expression[ 0 ] <= 'Z' ) ) )
  {
    // Scan the rest of the function name 
    size_t idx = 1;
    while ( ( idx < esize ) && ( ( expression[ idx ] == '_' ) || ( expression[ idx ] >= 'a'
      && expression[ idx ] <= 'z' )
      || ( expression[ idx ] >= 'A' && expression[ idx ] <= 'Z' ) || ( expression[ idx ]
      >= '0' && expression[ idx ] <= '9' ) ) )
      idx++;

    fun_name = expression.substr( 0, idx );

    // scan through any white space
    while ( ( idx < esize ) && ( ( expression[ idx ] == ' ' ) || ( expression[ idx ] == '\t' )
      || ( expression[ idx ] == '\n' ) || ( expression[ idx ] == '\r' ) || ( expression[ idx ]
      == '\f' ) || ( expression[ idx ] == '\v' ) ) )
      idx++;

    // Variables for finding start and end of arguments
    size_t start, end;
    // Check whether it is a function, it should have a parenthesis open
    if ( expression[ idx ] == '(' )
    {
      int paren_cnt = 1;
      idx++;
      start = idx;
      // scan through the remainder of the string
      while ( idx < esize )
      {
        // Ignore anything between quotes
        if ( expression[ idx ] == '"' )
        {
          idx++;
          while ( idx < esize )
          {
            // Any escape character is ignored
            if ( expression[ idx ] == '\\' ) idx += 2;
            // If we encounter the end of the quote go back to main loop again
            else if ( expression[ idx ] == '"' ) break;
            else idx++;
          }
        }
        // Do parentheses counting
        else if ( expression[ idx ] == '(' || expression[ idx ] == '[' ) paren_cnt++;
        else if ( expression[ idx ] == ')' || expression[ idx ] == ']' )
        {
          paren_cnt--;
          // if we encounterd end paren_cnt should be zero 
          if ( paren_cnt == 0 )
          {
            // Get the last argument
            end = idx;
            std::string arg = expression.substr( start, end - start );
            StripSurroundingSpaces( arg );
            if ( arg.size() ) fun_args.push_back( arg );
            break;
          }
        }
        else if ( expression[ idx ] == ',' )
        {
          // If we are in the main function call and we encounter a comma
          // it separates arguments
          if ( paren_cnt == 1 )
          {
            end = idx;
            std::string arg = expression.substr( start, end - start );
            StripSurroundingSpaces( arg );
            fun_args.push_back( arg );
            start = idx + 1;
          }
        }
        idx++;
      }
    }
  }
}

// Split function in different parts
// Assume a function is denoted as funname(arg1,arg2,arg3,...)

void ParserPrivate::split_subs( std::string& expression, std::vector< std::string >& start_args,
                 std::vector< std::string >& step_args, std::vector< std::string >& end_args,
                 std::string& varname )
{
  // Get the size of the srting we need to scan
  size_t esize = expression.size();

  int col_count = 0;
  bool has_end = false;
  bool has_step = false;

  // First detect the function name
  // A function name starts with _A-Za-z
  // Any other characters can have numbers in them as well
  if ( ( esize > 0 ) && expression[ 0 ] == '[' )
  {
    // Scan the rest of the function name 
    size_t idx = 1;

    // scan through any white space
    while ( ( idx < esize ) && ( ( expression[ idx ] == ' ' ) || ( expression[ idx ] == '\t' )
      || ( expression[ idx ] == '\n' ) || ( expression[ idx ] == '\r' ) || ( expression[ idx ]
      == '\f' ) || ( expression[ idx ] == '\v' ) ) )
    {
      idx++;
    }

    // Variables for finding start and end of arguments
    size_t start, end;

    int brac_cnt = 1;
    start = idx;

    // scan through the remainder of the string
    while ( idx < esize )
    {
      // Ignore anything between quotes
      if ( expression[ idx ] == '"' )
      {
        idx++;
        while ( idx < esize )
        {
          // Any escape character is ignored
          if ( expression[ idx ] == '\\' ) idx += 2;
          // If we encounter the end of the quote go back to main loop again
          else if ( expression[ idx ] == '"' ) break;
          else idx++;
        }
      }
      // Do parentheses counting
      else if ( expression[ idx ] == '[' ) brac_cnt++;
      else if ( expression[ idx ] == ']' )
      {
        brac_cnt--;
        // if we encounterd end paren_cnt should be zero 
        if ( brac_cnt == 0 )
        {
          // Get the last argument
          end = idx;
          std::string arg = expression.substr( start, end - start );
          StripSurroundingSpaces( arg );

          if ( arg.size() )
          {
            if ( col_count == 0 )
            {
              start_args.push_back( arg );
              step_args.push_back( "1" );
              end_args.push_back( arg );
            }
            else if ( col_count == 1 )
            {
              size_t end = start_args.size() - 1;
              end_args[ end ] = arg;
            }
            else if ( col_count == 2 )
            {
              size_t end = start_args.size() - 1;
              step_args[ end ] = arg;
            }
          }

          break;
        }
      }
      else if ( expression[ idx ] == ',' )
      {
        // If we are in the main function call and we encounter a comma
        // it separates arguments
        if ( brac_cnt == 1 )
        {
          end = idx;
          std::string arg = expression.substr( start, end - start );
          StripSurroundingSpaces( arg );

          if ( col_count == 0 )
          {
            start_args.push_back( arg );
            step_args.push_back( "1" );
            end_args.push_back( arg );
          }
          else if ( col_count == 1 )
          {
            size_t end = start_args.size() - 1;
            end_args[ end ] = arg;
          }
          else if ( col_count == 2 )
          {
            size_t end = start_args.size() - 1;
            step_args[ end ] = arg;
          }
          start = idx + 1;
          col_count = 0;
        }
      }
      else if ( expression[ idx ] == ':' )
      {
        // If we are in the main function call and we encounter a comma
        // it separates arguments
        if ( brac_cnt == 1 )
        {
          end = idx;
          std::string arg = expression.substr( start, end - start );
          StripSurroundingSpaces( arg );

          if ( col_count == 0 )
          {
            start_args.push_back( arg );
            step_args.push_back( "1" );
            end_args.push_back( arg );
            has_end = true;
          }
          else if ( col_count == 1 )
          {
            size_t end = start_args.size() - 1;
            step_args[ end ] = arg;
            has_step = true;
          }
          col_count++;

          start = idx + 1;
        }
      }

      idx++;
    }
  }

  if ( !has_end )
  {
    end_args.clear();
  }

  if ( !has_step )
  {
    step_args.clear();
  }

}

// Remove a pair of parentheses and return whether there were parentheses
bool ParserPrivate::remove_global_parentheses( std::string& expression )
{
  // Make sure the expression contains a string
  if ( expression.size() > 0 )
  {
    // Check whether we have a pair
    if ( ( expression[ 0 ] == '(' ) && ( expression[ expression.size() - 1 ] == ')' ) )
    {
      int paren_cnt = 1;
      size_t esize = expression.size() - 1;
      size_t idx = 1;
      while ( idx < esize )
      {
        if ( expression[ idx ] == '"' )
        {
          idx++;
          while ( idx < esize )
          {
            if ( expression[ idx ] == '\\' ) idx += 2;
            else if ( expression[ idx ] == '"' ) break;
            else idx++;
          }
        }
        else if ( expression[ idx ] == '(' ) paren_cnt++;
        else if ( expression[ idx ] == ')' )
        {
          paren_cnt--;
          if ( paren_cnt == 0 )
          {
            idx++;
            break;
          }
        }
        idx++;
      }
      if ( paren_cnt == 0 ) return false;
      // Remove parentheses and return true
      expression = expression.substr( 1, expression.size() - 2 );
      return true;
    }
  }
  return false;
}

// Remove comments from the program. Strip everything between // and \n
// and everything between /* and */

void ParserPrivate::remove_comments( std::string& expression )
{
  size_t esize = expression.size();
  size_t idx = 0;

  std::string newexpression;

  // Scan the expression
  while ( idx < esize )
  {
    // If we have a / we may have a comment
    if ( expression[ idx ] == '/' )
    {
      // A comment always needs a second character for delimitation
      if ( idx + 1 < esize )
      {
        // if it is a //, we have comment. Scan for a newline to mark the end
        if ( expression[ idx + 1 ] == '/' )
        {
          // Skip over comment marker
          idx += 2;
          while ( ( idx < esize ) && ( expression[ idx ] != '\n' ) )
            idx++;
        }
        // if it is a *, we have a /* */ clause
        else if ( expression[ idx + 1 ] == '*' )
        {
          // Skip over comment marker
          idx += 2;
          while ( idx < esize )
          {
            // Find a */ That denotes the end of the comment
            if ( expression[ idx ] == '*' )
            {
              if ( idx + 1 < esize ) if ( expression[ idx + 1 ] == '/' )
              {
                idx += 2;
                break;
              }
            }
            idx++;
          }
        }
      }
    }
    // Add this non comment character back into the string
    newexpression += expression[ idx ];
    idx++;

  }

  expression = newexpression;
}

bool ParserPrivate::recursive_validate( ParserNodeHandle& handle, ParserFunctionCatalogHandle& fhandle,
                     ParserVariableList& var_list, std::string& error, std::string& expression )
{
  int kind = handle->get_kind();
  switch( kind )
  {
  case PARSER_CONSTANT_SCALAR_E:
    {
      // Currently no validation
      return true;
    }
  case PARSER_CONSTANT_STRING_E:
    {
      // Currently no validation
      return true;
    }
  case PARSER_VARIABLE_E:
    {
      std::string val = handle->get_value();
      ParserVariableList::iterator it = var_list.find( val );
      if ( it == var_list.end() )
      {
        error = "VARIABLE ERROR: Unknown variable '" + val + "' in expression '"
          + expression + "'.";
        return false;
      }

      std::string vartype = ( *it ).second->get_type();
      if ( vartype == "U" )
      {
        error = "VARIABLE ERROR: Variable '" + val + "' is of an unknown type.";
        return false;
      }
      handle->set_type( vartype );

      return true;
    }
  case PARSER_FUNCTION_E:
    {
      std::string funname = handle->get_value();

      size_t num_args = handle->num_args();
      std::vector < std::string > arg_types( num_args );
      for ( size_t j = 0; j < num_args; j++ )
      {
        ParserNodeHandle chandle = handle->get_arg( j );
        // This one should return the error to the user
        if ( !( this->recursive_validate( chandle, fhandle, var_list, error, expression ) ) ) 
        {
          return false;
        }

        arg_types[ j ] = chandle->get_type();
      }

      ParserFunction* function = 0;
      std::string fid = ParserFunctionID( funname, arg_types );

      if ( !( fhandle->find_function( fid, function ) ) )
      {
        bool found_it = false;
        if ( arg_types.size() == 2 )
        {
          // Try swapping the arguments;
          std::string swap = arg_types[ 0 ];
          arg_types[ 0 ] = arg_types[ 1 ];
          arg_types[ 1 ] = swap;
          std::string fid = ParserFunctionID( funname, arg_types );
          if ( fhandle->find_function( fid, function ) )
          {
            if ( function->get_flags() & PARSER_SYMMETRIC_FUNCTION_E )
            {
              ParserNodeHandle temp1 = handle->get_arg( 0 );
              ParserNodeHandle temp2 = handle->get_arg( 1 );
              handle->set_arg( 0, temp2 );
              handle->set_arg( 1, temp1 );
              found_it = true;
            }
          }
        }

        if ( !found_it )
        {
          error = "FUNCTION ERROR: Unknown function " + funname + "(";
          for ( size_t j = 0; j < num_args; j++ )
          {
            error += ParserVariableType( arg_types[ j ] );
            if ( j < num_args - 1 ) error += ",";
          }
          error += ") in expression '" + expression + "'.";
          return false;
        }
      }

      std::string return_type = function->get_return_type();

      handle->set_type( return_type );
      handle->set_function( function );
      return true;
    }
  }

  return false;
}

// Recursive mark which variables are actually used

void ParserPrivate::optimize_mark_used( ParserScriptFunctionHandle& fhandle )
{
  size_t num_input_vars = fhandle->num_input_vars();
  for ( size_t j = 0; j < num_input_vars; j++ )
  {
    ParserScriptVariableHandle vhandle = fhandle->get_input_var( j );
    vhandle->set_flags( SCRIPT_USED_VAR_E );
    ParserScriptFunctionHandle phandle = vhandle->get_parent();
    if ( phandle.get() ) 
    {
      this->optimize_mark_used( phandle );
    }
  }
  fhandle->set_flags( SCRIPT_USED_VAR_E );
}

bool ParserPrivate::optimize_process_node( ParserNodeHandle& nhandle, std::list<
                      ParserScriptVariableHandle >& variables,
                      std::map< std::string, ParserScriptVariableHandle >& named_variables, std::list<
                      ParserScriptFunctionHandle >& functions, ParserScriptVariableHandle& ohandle, int& cnt,
                      std::string& error )
{
  int kind = nhandle->get_kind();
  // This function should only be called if the children are functions
  if ( kind != PARSER_FUNCTION_E )
  {
    error = "INTERNAL ERROR -  Kind of parser node is not function.";
    return false;
  }
  std::string funname = nhandle->get_value();
  ParserFunction* function = nhandle->get_function();

  int fflags = function->get_flags();
  ParserScriptFunctionHandle fhandle( new ParserScriptFunction( funname, function ) );

  if ( fflags & PARSER_SEQUENTIAL_FUNCTION_E )
  {
    // Mark function as sequential
    fhandle->set_flags( SCRIPT_SEQUENTIAL_VAR_E );
  }

  if ( fflags & PARSER_SINGLE_FUNCTION_E )
  {
    // Mark function as sequential
    fhandle->set_flags( SCRIPT_SINGLE_VAR_E );
  }

  if ( fflags & PARSER_CONST_FUNCTION_E )
  {
    // Mark function as sequential
    fhandle->set_flags( SCRIPT_CONST_VAR_E );
  }

  fhandle->set_output_var( ohandle );
  ohandle->set_parent( fhandle );

  std::string uname;
  std::string uname_num;

  size_t num_args = nhandle->num_args();
  for ( size_t j = 0; j < num_args; j++ )
  {
    ParserScriptVariableHandle iohandle;
    ParserNodeHandle ihandle = nhandle->get_arg( j );
    int ikind = ihandle->get_kind();

    switch( ikind )
    {
    case PARSER_FUNCTION_E:
      {
        std::string type = ihandle->get_type();
        uname_num = ExportToString( cnt );
        cnt++;
        uname = "$T" + uname_num;

        iohandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, type, 0 ) );
        variables.push_back( iohandle );

        if ( !( this->optimize_process_node( ihandle, variables, named_variables, functions,
          iohandle, cnt, error ) ) ) return false;
      }
      break;
    case PARSER_VARIABLE_E:
      {
        std::string varname = ihandle->get_value();
        std::map< std::string, ParserScriptVariableHandle >::iterator it =
          named_variables.find( varname );
        if ( it == named_variables.end() )
        {
          error
            = "INTERNAL ERROR - Variable name is not found in list, whereas it should be.";
          return false;
        }
        iohandle = ( *it ).second;
      }
      break;
    case PARSER_CONSTANT_SCALAR_E:
      {
        // Get the value of the constant
        std::string value = ihandle->get_value();
        std::map< std::string, float >::iterator cit, cit_end;
        cit = this->numerical_constants_.begin();
        cit_end = this->numerical_constants_.end();
        float val;

        while ( cit != cit_end )
        {
          if ( value == ( *cit ).first )
          {
            val = ( *cit ).second;
            break;
          }
          ++cit;
        }

        if ( cit == cit_end )
        {
          ImportFromString( value, val );
        }

        std::list< ParserScriptVariableHandle >::iterator it, it_end;
        it = variables.begin();
        it_end = variables.end();
        while ( it != it_end )
        {
          if ( ( *it )->get_kind() == SCRIPT_CONSTANT_SCALAR_E )
          {
            if ( ( *it )->get_scalar_value() == val )
            {
              iohandle = *it;
              break;
            }
          }
          ++it;
        }

        if ( iohandle.get() == 0 )
        {
          uname_num = ExportToString( cnt );
          cnt++;
          uname = "$D" + uname_num;
          iohandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, val ) );
          variables.push_back( iohandle );
        }
      }
      break;
    case PARSER_CONSTANT_STRING_E:
      {
        // Get the value of the constant
        std::string value = ihandle->get_value();

        std::list< ParserScriptVariableHandle >::iterator it, it_end;
        it = variables.begin();
        it_end = variables.end();
        while ( it != it_end )
        {
          if ( ( *it )->get_kind() == SCRIPT_CONSTANT_SCALAR_E )
          {
            if ( ( *it )->get_string_value() == value )
            {
              iohandle = *it;
              break;
            }
          }
          ++it;
        }

        if ( iohandle.get() == 0 )
        {
          uname_num = ExportToString( cnt );
          cnt++;
          uname = "$S" + uname_num;
          iohandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, value ) );

          variables.push_back( iohandle );
        }
      }
      break;
    }

    // Link the variable to the function descriptor
    fhandle->set_input_var( j, iohandle );
  }
  functions.push_back( fhandle );
  return true;
}

typedef union
{
  unsigned long long i;
  float f;
} ullong_float_type;

const ullong_float_type nan_value_f =
{ 0x7fffffffffffffffull };
const ullong_float_type inf_value_f =
{ 0x7ff0000000000000ull };

// Constructor
Parser::Parser() :
  private_( new ParserPrivate )
{
  // Initialize operators we need to scan for
  add_binary_operator( "+", "add", 5 );
  add_binary_operator( "-", "sub", 5 );
  add_binary_operator( "*", "mult", 6 );
  add_binary_operator( "/", "div", 6 );
  add_binary_operator( ".*", "mmult", 6 );
  add_binary_operator( "./", "mdiv", 6 );
  add_binary_operator( "%", "rem", 7 );
  add_binary_operator( "^", "pow", 7 );
  add_binary_operator( ".%", "mrem", 7 );
  add_binary_operator( ".^", "mpow", 7 );
  add_binary_operator( "&", "bitand", 4 );
  add_binary_operator( "|", "bitor", 4 );
  add_binary_operator( "&&", "and", 1 );
  add_binary_operator( "||", "or", 1 );
  add_binary_operator( "==", "eq", 2 );
  add_binary_operator( "!=", "neq", 2 );
  add_binary_operator( "<=", "le", 2 );
  add_binary_operator( ">=", "ge", 2 );
  add_binary_operator( "<", "ls", 2 );
  add_binary_operator( ">", "gt", 2 );

  add_unary_pre_operator( "!", "not" );
  add_unary_pre_operator( "~", "bitnot" );
  add_unary_pre_operator( "-", "neg" );
  add_unary_pre_operator( "+", "pos" ); // Note pos is a function that should be ignored

  add_unary_post_operator( "'", "transpose" );

  add_numerical_constant( "true", 1.0f );
  add_numerical_constant( "false", 0.0f );
  add_numerical_constant( "True", 1.0f );
  add_numerical_constant( "False", 0.0f );
  add_numerical_constant( "TRUE", 1.0f );
  add_numerical_constant( "FALSE", 0.0f );

  // C++ does not have a default symbol for NaN
  add_numerical_constant( "nan", nan_value_f.f );
  add_numerical_constant( "NaN", nan_value_f.f );
  add_numerical_constant( "Nan", nan_value_f.f );
  add_numerical_constant( "NAN", nan_value_f.f );

  // C++ does not have a default symbol for Inf
  add_numerical_constant( "inf", inf_value_f.f );
  add_numerical_constant( "Inf", inf_value_f.f );
  add_numerical_constant( "INF", inf_value_f.f );

  add_numerical_constant( "pi", Pif() );
  add_numerical_constant( "Pi", Pif() );
  add_numerical_constant( "PI", Pif() );
  add_numerical_constant( "M_PI", Pif() );
}

// The main function for parsing strings into code
bool Parser::parse( ParserProgramHandle& program, std::string expressions, std::string& error )
{
  // Clean error string
  error = "";

  // Generate a new program if we need one
  if ( program.get() == 0 )
  {
    program = ParserProgramHandle( new ParserProgram() );
  }

  // Remove comments from the program, so we only have expressions
  this->private_->remove_comments( expressions );

  while ( expressions.size() )
  {
    // Read an expression each iteration and store it in a string
    std::string expression;

    // Get the next expression from the front of the string.
    // Currently this function will always pass
    if ( !( this->private_->scan_expression( expressions, expression ) ) )
    {
      return false;
    }

    if ( expression == "" ) continue;
    // The variable name and the variable computation tree
    std::string varname;
    std::string vartree;

    // Get both sides of the equal sign
    if ( !( this->private_->split_expression( expression, varname, vartree ) ) )
    {
      error = "SYNTAX ERROR: Expression '" + expression
          + "' is not of the type 'varname = expression;'\n";
      return false;
    }

    // Get the tree
    ParserNodeHandle node_handle;

    // This the main function that breaks down the equation in several pieces
    if ( !( this->private_->parse_expression_tree( vartree, node_handle, error ) ) )
    {
      // If it cannot be broken down, the function will return a syntax error
      // Hence here no error handling is needed
      return false;
    }

    // Add a copy operation for everything that is not a function
    //if (node_handle->get_kind() != PARSER_FUNCTION_E )
    //{
    //  ParserNodeHandle thandle = node_handle;
    //  node_handle = new ParserNode(PARSER_FUNCTION_E,"copy");
    //  node_handle->set_arg(0,thandle);
    //}

    // A new tree which binds the variable name of the output together
    // with the top node of the parsing tree
    ParserTreeHandle tree_handle( new ParserTree( varname, node_handle ) );

    // Add the code to the program.
    program->add_expression( expression, tree_handle );

    // Note we store both the raw code as well as the parsed code,
    // to improve error reporting where we can list the faulty raw expression
    // and the error together. That way the user should be able to recongize
    // the faulty line more easily
  }

  // Success
  return true;
}

void Parser::add_binary_operator( std::string op, std::string funname, int priority )
{
  // Define a new operator, by setting the function name and priority
  // A function name can be for instance 'add' or 'sub' to represent '+' or '-'
  ParserPrivate::BinaryOperator binop;
  binop.operator_ = op;
  binop.funname_ = funname;
  binop.priority_ = priority;
  this->private_->binary_operators_[ op ] = binop;
}

void Parser::add_unary_pre_operator( std::string op, std::string funname )
{
  ParserPrivate::UnaryOperator unop;
  unop.operator_ = op;
  unop.funname_ = funname;
  this->private_->unary_pre_operators_[ op ] = unop;
}

void Parser::add_unary_post_operator( std::string op, std::string funname )
{
  ParserPrivate::UnaryOperator unop;
  unop.operator_ = op;
  unop.funname_ = funname;
  this->private_->unary_post_operators_[ op ] = unop;
}

void Parser::add_numerical_constant( std::string name, float val )
{
  this->private_->numerical_constants_[ name ] = val;
}

bool Parser::add_input_variable( ParserProgramHandle& program, std::string name, std::string type,
    int flags )
{
  if ( program.get() == 0 ) 
  {
    program = ParserProgramHandle( new ParserProgram() );
  }
  program->add_input_variable( name, type, flags );
  return true;
}

bool Parser::add_output_variable( ParserProgramHandle& program, std::string name, std::string type,
    int flags )
{
  if ( program.get() == 0 ) 
  {
    program = ParserProgramHandle( new ParserProgram() );
  }
  program->add_output_variable( name, type, flags );
  return true;
}

bool Parser::get_input_variable_type( ParserProgramHandle& program, std::string name,
    std::string& type )
{
  ParserVariableList varlist;
  program->get_input_variables( varlist );
  ParserVariableList::iterator it = varlist.find( name );

  if ( it == varlist.end() ) return false;
  type = ( *it ).second->get_type();
  return true;
}

bool Parser::get_input_variable_type( ParserProgramHandle& program, std::string name,
    std::string& type, int& flags )
{
  ParserVariableList varlist;
  program->get_input_variables( varlist );
  ParserVariableList::iterator it = varlist.find( name );

  if ( it == varlist.end() ) return false;
  type = ( *it ).second->get_type();
  flags = ( *it ).second->get_flags();
  return true;
}

bool Parser::get_output_variable_type( ParserProgramHandle& program, std::string name,
    std::string& type )
{
  ParserVariableList varlist;
  program->get_output_variables( varlist );
  ParserVariableList::iterator it = varlist.find( name );

  if ( it == varlist.end() ) return false;
  type = ( *it ).second->get_type();
  return true;
}

bool Parser::get_output_variable_type( ParserProgramHandle& program, std::string name,
    std::string& type, int& flags )
{
  ParserVariableList varlist;
  program->get_output_variables( varlist );
  ParserVariableList::iterator it = varlist.find( name );

  if ( it == varlist.end() ) return false;
  type = ( *it ).second->get_type();
  flags = ( *it ).second->get_flags();
  return true;
}

// Validate the program in terms of existence of functions
// and that types match

bool Parser::validate( ParserProgramHandle& program, ParserFunctionCatalogHandle& catalog,
    std::string& error )
{
  if ( program.get() == 0 )
  {
    error = "INTERNAL ERROR - Program was empty.";
    return false;
  }

  if ( catalog.get() == 0 )
  {
    error = "INTERNAL ERROR - Function Catalog was empty.";
    return false;
  }

  program->set_catalog( catalog );

  size_t num_expressions = program->num_expressions();

  ParserVariableList var_list;
  program->get_input_variables( var_list );

  // Evaluate whether functions exist and whether output types match
  for ( size_t j = 0; j < num_expressions; j++ )
  {
    std::string expression;
    ParserTreeHandle handle;
    program->get_expression( static_cast< int >( j ), expression, handle );

    if ( handle.get() == 0 )
    {
      error = "INTERNAL ERROR: Encountered incomplete expression '" + expression + "'.";
      return false;
    }

    ParserNodeHandle nhandle = handle->get_expression_tree();
    if ( nhandle.get() == 0 )
    {
      error = "INTERNAL ERROR: Encountered incomplete expression '" + expression + "'.";
      return false;
    }

    if ( !( this->private_->recursive_validate( nhandle, catalog, var_list, error, expression ) ) )
    {
      // error should already have been filled out
      return false;
    }

    // The output type is the type of the top node of the tree
    std::string vartype = nhandle->get_type();
    handle->set_type( vartype );

    // Add the output variable name to the list so the next function can call it
    std::string varname = handle->get_varname();
    var_list[ varname ] = ParserVariableHandle( new ParserVariable( varname, vartype ) );
  }

  // Validate output types

  ParserVariableList o_var_list;
  program->get_output_variables( o_var_list );
  ParserVariableList::iterator it, it_end, var_it;
  it = o_var_list.begin();
  it_end = o_var_list.end();

  // Check whether an output variable is assigned a value
  // Check whether the type of the value matches the required value
  while ( it != it_end )
  {
    std::string varname = ( *it ).first;
    std::string vartype = ( *it ).second->get_type();

    // Check whether the output type exists
    var_it = var_list.find( varname );
    if ( var_it == var_list.end() )
    {
      error = "OUTPUT VARIABLE '" + varname + "' has not been set.";
      return false;
    }

    std::string new_var_type = ( *var_it ).second->get_type();

    // If the variable is set to unknown its output type is set here
    if ( vartype == "U" )
    {
      ( *it ).second->set_type( new_var_type );
    }
    // If its output type is known, validate whether we have the right type
    else if ( new_var_type != vartype )
    {
      error = "OUTPUT VARIABLE '" + varname
          + "' is not of the required type. It needs to be a '"
          + ParserVariableType( vartype ) + "' variable, but is a '" + ParserVariableType(
          new_var_type ) + "' variable.";
    }
    ++it;
  }

  return true;
}

bool Parser::optimize( ParserProgramHandle& program, std::string& error )
{
  // Generate a new script, a script is a list of single functions that need
  // to be evaluated. The optimizer throws all the expressions together and splits
  // them in atomical pieces. This way we can sort out duplicate pieces and make
  // sure we can easily allocate temporarily spaces for intermediate results
  // This is especially needed for vectorized expressions and matrix algebra

  std::list < ParserScriptVariableHandle > variables;
  std::map < std::string, ParserScriptVariableHandle > named_variables;
  std::map< std::string, int > named_order;
  int order = 0;

  std::list < ParserScriptFunctionHandle > functions;

  // Phase 1: Add all the input variables, we later delete the ones we do not 
  // use later

  ParserVariableList input_variables;
  program->get_input_variables( input_variables );

  ParserVariableList::iterator it, it_end;
  it = input_variables.begin();
  it_end = input_variables.end();

  int cnt = 0;

  ParserScriptVariableHandle vhandle;
  ParserScriptVariableHandle ohandle;
  ParserScriptVariableHandle phandle;
  ParserScriptVariableHandle qhandle;
  ParserScriptVariableHandle uhandle;

  std::list< ParserScriptVariableHandle >::iterator pit, pit2, pit_end;
  std::list< ParserScriptFunctionHandle >::iterator fit, fit2, fit_end;

  std::string name;
  std::string uname;
  std::string uname_num;

  while ( it != it_end )
  {
    // The ordering of the following two lines is important in order to avoid an internal
    // compiler error on 64-bit Windows
    uname_num = ExportToString( cnt );
    name = ( *it ).first;
    cnt++;
    uname = "$I" + uname_num;

    // Get the variable attributes
    int flags = ( *it ).second->get_flags();
    std::string type = ( *it ).second->get_type();

    // Generate an input variable
    // The input variable has the name of the variable
    // it has an unique name as well, as input and output
    // variables can share names. Likewise a variable can
    // be overwritten anytime
    vhandle = ParserScriptVariableHandle( new ParserScriptVariable( name, uname, type, flags ) );

    // Add the variable to the general list of variables
    variables.push_back( vhandle );

    // Add the variable as well into the list of named variables
    // This list is used to translate all the named variables in the tree
    named_variables[ name ] = vhandle;
    ++it;
  }

  // Phase 2: run throught the full tree and setup intermediate variables
  //  for each phase of the computation and translate constant strings and
  //  floats as well into variables.

  size_t num_expressions = program->num_expressions();
  ParserTreeHandle thandle;

  for ( size_t j = 0; j < num_expressions; j++ )
  {
    // Get the tree
    program->get_expression( static_cast< int >( j ), thandle );

    std::string type = thandle->get_type();

    std::string varname = thandle->get_varname();
    // Generate a temporary variable
    // We later convert this one into an output variable
    // if needed

    ParserNodeHandle nhandle = thandle->get_expression_tree();

    switch( nhandle->get_kind() )
    {
      case PARSER_FUNCTION_E:
      {
        uname_num = ExportToString( cnt );
        cnt++;
        uname = "$N" + uname_num;

        ohandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, type, 0 ) );

        // Build function tables and add variable description
        if ( !( this->private_->optimize_process_node( nhandle, variables, named_variables, functions,
            ohandle, cnt, error ) ) ) return false;

        variables.push_back( ohandle );
      }
        break;

      case PARSER_VARIABLE_E:
      {
        std::string varname = nhandle->get_value();
        std::map< std::string, ParserScriptVariableHandle >::iterator it =
            named_variables.find( varname );
        if ( it == named_variables.end() )
        {
          error
              = "INTERNAL ERROR - Variable name is not found in list, whereas it should be in the list.";
          return false;
        }
        ohandle = ( *it ).second;
      }
        break;
      case PARSER_CONSTANT_SCALAR_E:
      {
        // Get the value of the constant
        std::string value = nhandle->get_value();

        std::map< std::string, float >::iterator cit, cit_end;
        cit = this->private_->numerical_constants_.begin();
        cit_end = this->private_->numerical_constants_.end();
        float val;

        while ( cit != cit_end )
        {
          if ( value == ( *cit ).first )
          {
            val = ( *cit ).second;
            break;
          }
          ++cit;
        }

        if ( cit == cit_end ) 
        {
          ImportFromString( value, val );
        }

        std::list< ParserScriptVariableHandle >::iterator it, it_end;
        it = variables.begin();
        it_end = variables.end();
        ohandle.reset();

        while ( it != it_end )
        {
          if ( ( *it )->get_kind() == SCRIPT_CONSTANT_SCALAR_E )
          {
            if ( ( *it )->get_scalar_value() == val )
            {
              ohandle = *it;
              break;
            }
          }
          ++it;
        }

        if ( ohandle.get() == 0 )
        {
          uname_num = ExportToString( cnt );
          cnt++;
          uname = "$D" + uname_num;
          ohandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, val ) );
          variables.push_back( ohandle );
        }
      }
        break;
      case PARSER_CONSTANT_STRING_E:
      {
        // Get the value of the constant
        std::string value = nhandle->get_value();

        std::list< ParserScriptVariableHandle >::iterator it, it_end;
        it = variables.begin();
        it_end = variables.end();
        ohandle.reset();

        while ( it != it_end )
        {
          if ( ( *it )->get_kind() == SCRIPT_CONSTANT_SCALAR_E )
          {
            if ( ( *it )->get_string_value() == value )
            {
              ohandle = *it;
              break;
            }
          }
          ++it;
        }

        if ( ohandle.get() == 0 )
        {
          uname_num = ExportToString( cnt );
          cnt++;
          uname = "$S" + uname_num;
          ohandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, value ) );
          variables.push_back( ohandle );
        }
      }
        break;
    }

    // Phase 2b: Mark flags of the variables. Set the variables that are constant,
    // the variables that a single (executed once), and sequential (executed multiple times)
    // Run through the list from start to finish which should run through each variable
    // in the proper order
    // Update variable flags
    fit = functions.begin();
    fit_end = functions.end();

    while ( fit != fit_end )
    {
      size_t num_args = ( *fit )->num_input_vars();

      int flags = 0;
      for ( size_t j = 0; j < num_args; j++ )
      {
        phandle = ( *fit )->get_input_var( j );
        flags |= phandle->get_flags();
      }

      flags |= ( *fit )->get_flags();

      if ( flags & SCRIPT_SEQUENTIAL_VAR_E ) flags = SCRIPT_SEQUENTIAL_VAR_E;
      if ( flags & SCRIPT_SINGLE_VAR_E ) flags = SCRIPT_SINGLE_VAR_E;
      if ( flags & SCRIPT_CONST_VAR_E ) flags = SCRIPT_CONST_VAR_E;

      // Mark output variable with the proper flags
      qhandle = ( *fit )->get_output_var();
      qhandle->set_flags( flags );

      // Mark this function
      ( *fit )->set_flags( flags & ( SCRIPT_SEQUENTIAL_VAR_E | SCRIPT_SINGLE_VAR_E
          | SCRIPT_CONST_VAR_E | SCRIPT_OPTIONAL_VAR_E ) );

      ++fit;
    }

    // Add variables to named and unamed list
    // They are added here, to prevent a loop
    named_variables[ varname ] = ohandle;
    named_order[ varname ] = order;
    order++;
  }

  // Now variables and functions should contain a list of needed spaces and
  // functions contains a list in order in which expression need to be evaluated

  // Phase 3: Find and mark the output variables, so we can find them later. 
  // and follow the trail back to mark which functions and pieces are actually used

  ParserVariableList output_variables, ordered_output_variables;
  program->get_output_variables( output_variables );

  std::list < ParserScriptVariableHandle > ovariables;

  int corder = 0;
  while ( corder < order )
  {
    ParserVariableList::iterator vit, vit_end;
    vit = output_variables.begin();
    vit_end = output_variables.end();

    while ( vit != vit_end )
    {
      std::string varname = ( *vit ).first;
      int flags = ( *vit ).second->get_flags();

      std::map< std::string, int >::iterator oit;
      oit = named_order.find( varname );
      if ( oit == named_order.end() )
      {
        if ( !( flags & SCRIPT_OPTIONAL_VAR_E ) )
        {
          error
              = "INTERNAL ERROR - Could not find an output variable, whereas it should exist";
          return false;
        }
      }
      else if ( ( *oit ).second == corder )
      {
        ordered_output_variables[ varname ] = ( *vit ).second;
        break;
      }
      ++vit;
    }
    corder++;
  }

  ParserVariableList::iterator vit, vit_end;
  vit = ordered_output_variables.begin();
  vit_end = ordered_output_variables.end();

  while ( vit != vit_end )
  {
    std::string varname = ( *vit ).first;
    int flags = ( *vit ).second->get_flags();
    std::map< std::string, ParserScriptVariableHandle >::iterator nit;
    nit = named_variables.find( varname );

    if ( nit == named_variables.end() )
    {
      error = "INTERNAL ERROR - Could not find an output variable, whereas it should exist";
      return false;
    }
    //    (*nit).second->set_kind(SCRIPT_OUTPUT_E);
    ( *nit ).second->set_name( varname );

    ParserScriptVariableHandle nhandle = ( *nit ).second;

    if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
    {
      int nflags = ( *nit ).second->get_flags();
      std::string type = ( *nit ).second->get_type();
      if ( !( nflags & SCRIPT_SEQUENTIAL_VAR_E ) )
      {

        // Need to add in extra function call
        uname_num = ExportToString( cnt );
        cnt++;
        uname = "$T" + uname_num;

        uhandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, type, 
          SCRIPT_SEQUENTIAL_VAR_E | SCRIPT_USED_VAR_E | SCRIPT_CONST_VAR_E ) );
        variables.push_back( uhandle );

        // Build the function call to generate the new variable
        std::string fun_id = ParserFunctionID( "seq", type );

        ParserFunction* fun_ptr = 0;
        if ( !( program->get_catalog()->find_function( fun_id, fun_ptr ) ) )
        {
          error = "INTERNAL ERROR - Sequencer was not defined for type '"
              + ParserVariableType( type ) + "'.";
          return false;
        }

        ParserScriptFunctionHandle parhandle( nhandle->get_parent() );
        ParserScriptFunctionHandle fhandle( new ParserScriptFunction( "seq", fun_ptr ) );
        if ( nflags & SCRIPT_CONST_VAR_E ) fhandle->set_flags( SCRIPT_USED_VAR_E
            | SCRIPT_CONST_VAR_E );
        else fhandle->set_flags( SCRIPT_USED_VAR_E | SCRIPT_SINGLE_VAR_E );
        fhandle->set_input_var( 0, nhandle );
        nhandle->set_flags( SCRIPT_USED_VAR_E );
        fhandle->set_output_var( uhandle );
        uhandle->set_parent( fhandle );

        // Now search for additional occurances of whandle in the sequential list
        // and replace them with uhandle
        ParserScriptVariableHandle xhandle;

        fit = functions.begin();
        fit_end = functions.end();
        while ( fit != fit_end )
        {
          size_t num = ( *fit )->num_input_vars();
          for ( size_t k = 0; k < num; k++ )
          {
            xhandle = ( *fit )->get_input_var( k );
            if ( xhandle.get() == nhandle.get() )
            {
              ( *fit )->set_input_var( k, uhandle );
            }
          }
          ++fit;
        }

        fit = functions.begin();
        fit_end = functions.end();

        while ( fit != fit_end )
        {
          size_t num_args = ( *fit )->num_input_vars();

          int flags = 0;
          for ( size_t j = 0; j < num_args; j++ )
          {
            phandle = ( *fit )->get_input_var( j );
            flags |= phandle->get_flags();
          }

          if ( flags & SCRIPT_SEQUENTIAL_VAR_E ) flags = SCRIPT_SEQUENTIAL_VAR_E;
          if ( flags & SCRIPT_SINGLE_VAR_E ) flags = SCRIPT_SINGLE_VAR_E;
          if ( flags & SCRIPT_CONST_VAR_E ) flags = SCRIPT_CONST_VAR_E;

          // Mark output variable with the proper flags
          qhandle = ( *fit )->get_output_var();
          qhandle->set_flags( flags );

          // Mark this function
          ( *fit )->set_flags( flags & ( SCRIPT_SEQUENTIAL_VAR_E | SCRIPT_SINGLE_VAR_E
              | SCRIPT_CONST_VAR_E ) );

          ++fit;
        }

        fit = functions.begin();
        fit_end = functions.end();

        while ( fit != fit_end )
        {
          if ( ( *fit ).get() == parhandle.get() )
          {
            ++fit;
            break;
          }
          ++fit;
        }

        functions.insert( fit, fhandle );
        nhandle = uhandle;
      }
    }

    ovariables.push_back( nhandle );
    ++vit;
  }

  pit = ovariables.begin();
  pit_end = ovariables.end();
  while ( pit != pit_end )
  {
    // Mark variable as used as we actually make use of it
    ( *pit )->set_flags( SCRIPT_USED_VAR_E );
    ParserScriptFunctionHandle fhandle = ( *pit )->get_parent();
    if ( fhandle.get() ) 
    {
      this->private_->optimize_mark_used( fhandle );
    }

    ++pit;
  }

  // Phase 5: Remove duplicate expressions

  fit = functions.begin();
  fit_end = functions.end();

  std::string dependence, dependence2;

  while ( fit != fit_end )
  {
    ParserScriptVariableHandle handle = ( *fit )->get_output_var();
    handle->compute_dependence();
    dependence = handle->get_dependence();

    fit2 = functions.begin();
    while ( fit2 != fit )
    {
      ParserScriptVariableHandle nhandle = ( *fit2 )->get_output_var();

      dependence2 = nhandle->get_dependence();
      if ( dependence2 == dependence )
      {
        // The handle with which the ones in the script need to be replaced
        // Expressions are equal
        // Clear dependence, clear flags
        handle->clear_dependence();
        // Clear the used flag for this variable
        handle->clear_flags();
        ParserScriptFunctionHandle fhandle = handle->get_parent();
        if ( fhandle.get() == 0 )
        {
          error = "INTERNAL ERROR -  Duplicate input variable.";
          return false;
        }
        // Clear the function that computes the variable
        fhandle->clear_flags();
        std::list< ParserScriptFunctionHandle >::iterator hit, hit_end;
        hit = functions.begin();
        hit_end = functions.end();
        while ( hit != hit_end )
        {
          ParserScriptFunctionHandle hhandle = ( *hit );
          size_t num_input_vars = hhandle->num_input_vars();
          for ( size_t j = 0; j < num_input_vars; j++ )
          {
            if ( hhandle->get_input_var( j ) == handle )
            {
              hhandle->set_input_var( j, nhandle );
            }
          }
          ++hit;
        }
      }
      ++fit2;
    }
    ++fit;
  }

  // Phase 6: Now remove every unused variable and split the function and variable
  // lists in three: one for the pure constants and functions based on constants
  // these should be evaluated first and only once. The second one is the class
  // single variables which can be evaluated after the constants. The last class
  // is the sequential variables, which can be evaluated afterwards

  std::list < ParserScriptFunctionHandle > const_function_list;
  std::list < ParserScriptFunctionHandle > single_function_list;
  std::list < ParserScriptFunctionHandle > sequential_function_list;

  std::list < ParserScriptVariableHandle > const_variable_list;
  std::list < ParserScriptVariableHandle > single_variable_list;
  std::list < ParserScriptVariableHandle > sequential_variable_list;

  fit = functions.begin();
  fit_end = functions.end();

  // Sort functions in three different catagories
  while ( fit != fit_end )
  {

    int flags = ( *fit )->get_flags();
    // Skip over any unused function, we do not need to do any computation on
    // those
    if ( !( flags & SCRIPT_USED_VAR_E ) )
    {
      ++fit;
      continue;
    }

    if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
    {
      sequential_function_list.push_back( *fit );
    }
    else if ( flags & SCRIPT_SINGLE_VAR_E )
    {
      single_function_list.push_back( *fit );
    }
    else if ( flags & SCRIPT_CONST_VAR_E )
    {
      const_function_list.push_back( *fit );
    }

    ++fit;
  }

  pit = variables.begin();
  pit_end = variables.end();

  // Sort variables in three different catagories
  while ( pit != pit_end )
  {

    int flags = ( *pit )->get_flags();
    // Skip over any unused function, we do not need to do any computation on
    // those
    if ( !( flags & SCRIPT_USED_VAR_E ) )
    {
      ++pit;
      continue;
    }

    if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
    {
      flags = flags & ( ~( SCRIPT_SINGLE_VAR_E ) );
      ( *pit )->clear_flags();
      ( *pit )->set_flags( flags );
      sequential_variable_list.push_back( *pit );
    }
    else if ( flags & SCRIPT_SINGLE_VAR_E )
    {
      flags = flags & ( ~( SCRIPT_SEQUENTIAL_VAR_E | SCRIPT_CONST_VAR_E ) );
      ( *pit )->clear_flags();
      ( *pit )->set_flags( flags );
      single_variable_list.push_back( *pit );
    }
    else if ( flags & SCRIPT_CONST_VAR_E )
    {
      flags = flags & ( ~( SCRIPT_SINGLE_VAR_E | SCRIPT_SEQUENTIAL_VAR_E ) );
      ( *pit )->clear_flags();
      ( *pit )->set_flags( flags );
      const_variable_list.push_back( *pit );
    }
    ++pit;
  }

  // Phase 7: Add in sequence instruction for every variable that is used
  // in the sequential mode

  fit = sequential_function_list.begin();
  fit_end = sequential_function_list.end();

  ParserScriptVariableHandle whandle;
  ParserScriptVariableHandle xhandle;
  ParserScriptFunctionHandle fhandle;

  // Sort functions in three different catagories
  while ( fit != fit_end )
  {
    size_t num_input_vars = ( *fit )->num_input_vars();
    for ( size_t j = 0; j < num_input_vars; j++ )
    {
      whandle = ( *fit )->get_input_var( j );
      int flags = whandle->get_flags();
      std::string type = whandle->get_type();

      if ( !( flags & SCRIPT_SEQUENTIAL_VAR_E ) )
      {
        // Need to add in extra function call
        uname_num = ExportToString( cnt );
        cnt++;
        uname = "$T" + uname_num;

        uhandle = ParserScriptVariableHandle( new ParserScriptVariable( uname, type, 
          SCRIPT_SEQUENTIAL_VAR_E | SCRIPT_USED_VAR_E | SCRIPT_CONST_VAR_E ) );
        sequential_variable_list.push_back( uhandle );

        // Build the function call to generate the new variable
        std::string fun_id = ParserFunctionID( "seq", type );

        ParserFunction* fun_ptr = 0;
        if ( !( program->get_catalog()->find_function( fun_id, fun_ptr ) ) )
        {
          error = "INTERNAL ERROR - Sequencer was not defined for type '"
              + ParserVariableType( type ) + "'.";
          return false;
        }

        fhandle = ParserScriptFunctionHandle( new ParserScriptFunction( "seq", fun_ptr ) );
        fhandle->set_input_var( 0, whandle );
        fhandle->set_output_var( uhandle );
        uhandle->set_parent( fhandle );

        // Add these to the output of one of the other lists
        if ( flags & SCRIPT_SINGLE_VAR_E ) single_function_list.push_back( fhandle );
        else const_function_list.push_back( fhandle );

        // Now search for additional occurances of whandle in the sequential list
        // and replace them with uhandle

        fit2 = sequential_function_list.begin();
        while ( fit2 != fit_end )
        {
          size_t num = ( *fit2 )->num_input_vars();
          for ( size_t k = 0; k < num; k++ )
          {
            xhandle = ( *fit2 )->get_input_var( k );
            if ( xhandle.get() == whandle.get() )
            {
              ( *fit2 )->set_input_var( k, uhandle );
            }
          }
          ++fit2;
        }
      }
    }
    ++fit;
  }

  pit = const_variable_list.begin();
  pit_end = const_variable_list.end();
  while ( pit != pit_end )
  {
    program->add_const_var( *pit );
    ++pit;
  }

  pit = single_variable_list.begin();
  pit_end = single_variable_list.end();
  while ( pit != pit_end )
  {
    program->add_single_var( *pit );
    ++pit;
  }

  pit = sequential_variable_list.begin();
  pit_end = sequential_variable_list.end();
  while ( pit != pit_end )
  {
    program->add_sequential_var( *pit );
    ++pit;
  }

  fit = const_function_list.begin();
  fit_end = const_function_list.end();
  while ( fit != fit_end )
  {
    program->add_const_function( *fit );
    ++fit;
  }

  fit = single_function_list.begin();
  fit_end = single_function_list.end();
  while ( fit != fit_end )
  {
    program->add_single_function( *fit );
    ++fit;
  }

  fit = sequential_function_list.begin();
  fit_end = sequential_function_list.end();
  while ( fit != fit_end )
  {
    program->add_sequential_function( *fit );
    ++fit;
  }

  return true;
}

}

