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

#ifndef CORE_PARSER_PARSERSCRIPTVARIABLE_H 
#define CORE_PARSER_PARSERSCRIPTVARIABLE_H 

// STL includes
#include <string>

// Core includes
#include <Core/Parser/ParserFWD.h>

namespace Core
{

// Hide header includes, private interface and implementation
class ParserScriptVariablePrivate;
typedef boost::shared_ptr< ParserScriptVariablePrivate > ParserScriptVariablePrivateHandle;

class ParserScriptVariable
{

public:

  /// Create input variable
  ParserScriptVariable( std::string name, std::string uname, std::string type, int flags );

  /// Create output variable
  ParserScriptVariable( std::string uname, std::string type, int flags );

  /// Create scalar const variable
  ParserScriptVariable( std::string uname, float value );

  /// Create string const variable
  ParserScriptVariable( std::string uname, std::string value );

  /// Set/get parent
  void set_parent( ParserScriptFunctionHandle& handle );
  ParserScriptFunctionHandle get_parent();

  /// Get kind of the variable
  int get_kind();
  void set_kind( int kind );

  std::string get_type();
  void set_type( std::string type );

  /// Get/set flags of the variable
  int get_flags();
  void set_flags( int flags );
  void clear_flags();

  void set_const_var();
  void set_single_var();
  void set_sequential_var();

  bool is_const_var();
  bool is_single_var();
  bool is_sequential_var();

  /// Get the name and the unique name
  std::string get_name();
  void set_name( std::string name );

  std::string get_uname();
  void set_uname( std::string uname );

  /// Get the constant values
  float get_scalar_value();
  std::string get_string_value();

  /// Dependence of this variable
  void compute_dependence();

  std::string get_dependence();
  void clear_dependence();

  int get_var_number();
  void set_var_number( int var_number );

  /// For debugging
  void print();

private:
  ParserScriptVariablePrivateHandle private_;
};

}

#endif
