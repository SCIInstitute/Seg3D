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

#ifndef CORE_PARSER_PARSERVARIABLE_H 
#define CORE_PARSER_PARSERVARIABLE_H 

// STL includes
#include <string>

// Boost includes 
#include <boost/shared_ptr.hpp>

namespace Core
{

// Hide header includes, private interface and implementation
class ParserVariablePrivate;
typedef boost::shared_ptr< ParserVariablePrivate > ParserVariablePrivateHandle;

// Render human readable output
std::string ParserVariableType( std::string type );

/// ParserVariable: A variable for the the parser program. This class contains
///              the names of the variables that are defined at the start of the
///              program and which need to be defined at the end of the program
class ParserVariable
{
public:
  ParserVariable( std::string name, std::string type );
  ParserVariable( std::string name, std::string type, int flags );

  /// Retrieve the type of the variable
  std::string get_type();

  /// Set the type of the variable
  void set_type( std::string type );

  /// Retrieve the flags of the variable
  int get_flags();

  /// Get the name of the variable
  std::string get_name();

private:
  ParserVariablePrivateHandle private_;
};

}

#endif
