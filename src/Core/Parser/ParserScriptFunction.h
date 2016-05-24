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

#ifndef CORE_PARSER_PARSERSCRIPTFUNCTION_H 
#define CORE_PARSER_PARSERSCRIPTFUNCTION_H 

// STL includes
#include <string>

// Core includes
#include <Core/Parser/ParserFWD.h>

namespace Core
{

// Hide header includes, private interface and implementation
class ParserScriptFunctionPrivate;
typedef boost::shared_ptr< ParserScriptFunctionPrivate > ParserScriptFunctionPrivateHandle;

class ParserScriptFunction
{

public:
  /// Constructor
  ParserScriptFunction( std::string name, ParserFunction* function );

  /// Get the name of the function
  std::string get_name();

  /// Get the number of input variables this function depends on
  size_t num_input_vars();

  ParserScriptVariableHandle get_input_var( size_t j );

  void set_input_var( size_t j, ParserScriptVariableHandle& handle );

  ParserScriptVariableHandle get_output_var();
  void set_output_var( ParserScriptVariableHandle& handle );

  ParserFunction* get_function();

  int get_flags();
  void set_flags( int flags );
  void clear_flags();

  /// For debugging
  void print();

private:
  ParserScriptFunctionPrivateHandle private_;
};

}

#endif
