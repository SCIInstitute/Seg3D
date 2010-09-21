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
#include <Core/Parser/ParserProgram.h> 
#include <Core/Parser/ParserScriptFunction.h>
#include <Core/Parser/ParserScriptVariable.h>
#include <Core/Parser/ParserTree.h>
#include <Core/Parser/ParserVariable.h>

namespace Core
{

// Print function for debugging
void ParserProgram::print()
{
  std::cout << "PARSED PROGRAM\n";

  std::cout << "--- INPUT VARIABLES ---\n";
  ParserVariableList::iterator it, it_end;
  it = this->input_variables_.begin();
  it_end = this->input_variables_.end();
  size_t j = 0;
  while ( it != it_end )
  {
    std::cout << "  VARIABLE " << j << ": " << ( *it ).first << "\n";
    ++it;
    j++;
  }

  std::cout << "--- OUTPUT VARIABLES ---\n";
  it = this->output_variables_.begin();
  it_end = this->output_variables_.end();
  j = 0;
  while ( it != it_end )
  {
    std::cout << "  VARIABLE " << j << ": " << ( *it ).first << "\n";
    ++it;
    j++;
  }

  std::cout << "--- EXPRESSIONS ---\n";
  for ( size_t j = 0; j < this->expressions_.size(); j++ )
  {
    // Print the original expression for comparison
    std::cout << "EXPRESSION " << j << ": " << this->expressions_[ j ].first << "\n";
    // Print the ParserTree expression
    std::cout << "TREE TO COMPUTE EXPRESSION " << j << ":\n";
    this->expressions_[ j ].second->print();
  }

  std::vector< ParserScriptVariableHandle >::iterator pit, pit_end;
  std::vector< ParserScriptFunctionHandle >::iterator fit, fit_end;

  std::cout << "\n";
  std::cout << "--- CONST VAR LIST ---\n";
  pit = this->const_variables_.begin();
  pit_end = this->const_variables_.end();
  while ( pit != pit_end )
  {
    ( *pit )->print();
    ++pit;
  }

  std::cout << "--- SINGLE VAR LIST ---\n";
  pit = this->single_variables_.begin();
  pit_end = this->single_variables_.end();
  while ( pit != pit_end )
  {
    ( *pit )->print();
    ++pit;
  }

  std::cout << "--- SEQUENTIAL VAR LIST ---\n";
  pit = this->sequential_variables_.begin();
  pit_end = this->sequential_variables_.end();
  while ( pit != pit_end )
  {
    ( *pit )->print();
    ++pit;
  }

  std::cout << "--- CONST FUNCTION LIST ---\n";
  fit = this->const_functions_.begin();
  fit_end = this->const_functions_.end();
  while ( fit != fit_end )
  {
    ( *fit )->print();
    ++fit;
  }

  std::cout << "--- SINGLE FUNCTION LIST ---\n";
  fit = this->single_functions_.begin();
  fit_end = this->single_functions_.end();
  while ( fit != fit_end )
  {
    ( *fit )->print();
    ++fit;
  }

  std::cout << "--- SEQUENTIAL FUNCTION LIST ---\n";
  fit = this->sequential_functions_.begin();
  fit_end = this->sequential_functions_.end();
  while ( fit != fit_end )
  {
    ( *fit )->print();
    ++fit;
  }
}

void ParserProgram::add_const_var( ParserScriptVariableHandle& handle )
{
  handle->set_var_number( static_cast< int >( this->const_variables_.size() ) );
  this->const_variables_.push_back( handle );
}

void ParserProgram::add_single_var( ParserScriptVariableHandle& handle )
{
  handle->set_var_number( static_cast< int >( this->single_variables_.size() ) );
  this->single_variables_.push_back( handle );
}

void ParserProgram::add_sequential_var( ParserScriptVariableHandle& handle )
{
  handle->set_var_number( static_cast< int >( this->sequential_variables_.size() ) );
  this->sequential_variables_.push_back( handle );
}

void ParserProgram::add_const_function( ParserScriptFunctionHandle& handle )
{
  this->const_functions_.push_back( handle );
}

void ParserProgram::add_single_function( ParserScriptFunctionHandle& handle )
{
  this->single_functions_.push_back( handle );
}

void ParserProgram::add_sequential_function( ParserScriptFunctionHandle& handle )
{
  this->sequential_functions_.push_back( handle );
}

size_t ParserProgram::num_const_variables()
{
  return this->const_variables_.size();
}

size_t ParserProgram::num_single_variables()
{
  return this->single_variables_.size();
}

size_t ParserProgram::num_sequential_variables()
{
  return this->sequential_variables_.size();
}

bool ParserProgram::get_const_variable( size_t j, ParserScriptVariableHandle& handle )
{
  if ( j >= this->const_variables_.size() ) return false;
  handle = this->const_variables_[ j ];
  return true;
}

bool ParserProgram::get_single_variable( size_t j, ParserScriptVariableHandle& handle )
{
  if ( j >= this->single_variables_.size() ) return false;
  handle = this->single_variables_[ j ];
  return true;
}

bool ParserProgram::get_sequential_variable( size_t j, ParserScriptVariableHandle& handle )
{
  if ( j >= this->sequential_variables_.size() ) return false;
  handle = this->sequential_variables_[ j ];
  return true;
}

size_t ParserProgram::num_const_functions()
{
  return this->const_functions_.size();
}

size_t ParserProgram::num_single_functions()
{
  return this->single_functions_.size();
}

size_t ParserProgram::num_sequential_functions()
{
  return this->sequential_functions_.size();
}

bool ParserProgram::get_const_function( size_t j, ParserScriptFunctionHandle& handle )
{
  if ( j >= this->const_functions_.size() ) return false;
  handle = this->const_functions_[ j ];
  return true;
}

bool ParserProgram::get_single_function( size_t j, ParserScriptFunctionHandle& handle )
{
  if ( j >= this->single_functions_.size() ) return false;
  handle = this->single_functions_[ j ];
  return true;
}

bool ParserProgram::get_sequential_function( size_t j, ParserScriptFunctionHandle& handle )
{
  if ( j >= this->sequential_functions_.size() ) return false;
  handle = this->sequential_functions_[ j ];
  return true;
}

void ParserProgram::add_expression( std::string& expression_string, ParserTreeHandle expression_tree )
{
  std::pair<std::string,ParserTreeHandle> expression( expression_string, expression_tree );
  this->expressions_.push_back( expression );
}

void ParserProgram::get_expression( int expression_num, std::string& expression_string, ParserTreeHandle& expression_handle )
{
  expression_string = this->expressions_[ expression_num ].first;
  expression_handle = this->expressions_[ expression_num ].second;
}

void ParserProgram::get_expression( int expression_num, ParserTreeHandle& expression_handle )
{
  expression_handle = this->expressions_[ expression_num ].second;
}

size_t ParserProgram::num_expressions()
{
  return this->expressions_.size();
}

void ParserProgram::add_input_variable( std::string name, std::string type /*= "U"*/, int flags /*= 0*/ )
{
  this->input_variables_[ name ] = ParserVariableHandle( new ParserVariable( name, type, flags ) );
}

void ParserProgram::add_output_variable( std::string name, std::string type /*= "U"*/, int flags /*= 0*/ )
{
  this->output_variables_[ name ] = ParserVariableHandle( new ParserVariable( name, type, flags ) );
}

void ParserProgram::get_input_variables( ParserVariableList& var_list )
{
  var_list = this->input_variables_;
}

void ParserProgram::get_output_variables( ParserVariableList& var_list )
{
  var_list = this->output_variables_;
}

Core::ParserFunctionCatalogHandle ParserProgram::get_catalog()
{
  return this->catalog_;
}

void ParserProgram::set_catalog( ParserFunctionCatalogHandle catalog )
{
  this->catalog_ = catalog;
}

}

