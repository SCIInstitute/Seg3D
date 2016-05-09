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

#ifndef CORE_PARSER_PARSERFUNCTIONCATALOG_H 
#define CORE_PARSER_PARSERFUNCTIONCATALOG_H 

// STL includes
#include <string>

// Core includes
#include <Core/Parser/ParserFWD.h>

namespace Core
{

/// Hide header includes, private interface and implementation
class ParserFunctionCatalogPrivate;
typedef boost::shared_ptr< ParserFunctionCatalogPrivate > ParserFunctionCatalogPrivateHandle;

/// ParserFunctionCatalog: This class contains a list of functions that are
///              available, and hence is used for validating the program
/// The ParserFunctionCatalog serves two purposes:
///  - Tell the parser which functions are valid
///  - Tell the parser what the return type is of each function call
class ParserFunctionCatalog 
{
public:

  /// Constructor
  ParserFunctionCatalog();

  /// Add definitions of functions to the list
  /// Various calls with different amounts of arguments
  void add_function( ParserFunction* function );

  /// Retrieve a function from the data base
  bool find_function( std::string function_id, ParserFunction*& function );

  /// For debugging
  void print();

private:

  ParserFunctionCatalogPrivateHandle private_;
};

}

#endif
