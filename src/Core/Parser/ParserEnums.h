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

#ifndef CORE_PARSER_PARSERENUMS_H 
#define CORE_PARSER_PARSERENUMS_H 

namespace Core
{

// This piece of the code defines the types the parser recognizes
// Define variable types that are used to validate and to predict
// output types.

// This part defines the attributes of the variables

enum
{
  /// A sequence denotes a vectorized variable, hence when this flag is set
  /// the function will apply the function to an array, instead of a single variable
  SCRIPT_SEQUENTIAL_VAR_E = 1,
  /// Simple variable this is the default
  SCRIPT_SINGLE_VAR_E = 2,
  /// Reserved for numerical constants
  SCRIPT_CONST_VAR_E = 4,
  /// Internal marker to mark which variables are actually used to compute the output
  SCRIPT_USED_VAR_E = 8,
  /// Internal marker telling an output is optional
  SCRIPT_OPTIONAL_VAR_E = 16
};

// Define the four classes of parser components

enum
{
  PARSER_CONSTANT_SCALAR_E = 1,
  PARSER_CONSTANT_STRING_E = 2,
  PARSER_VARIABLE_E = 3,
  PARSER_FUNCTION_E = 4
};

enum
{
  /// Define a function as sequential it will be evaluted for each instance in
  /// the sequence. This one is used for functions without parameters like rand
  /// so they trigger for each case
  PARSER_SEQUENTIAL_FUNCTION_E = 1,
  PARSER_SINGLE_FUNCTION_E = 2,
  PARSER_CONST_FUNCTION_E = 4,
  /// Variables are symmetric, this will tell the optimizer that the order of
  /// variables is not of interest
  PARSER_SYMMETRIC_FUNCTION_E = 16
};

// In the end we build script of how to compute the
// results

enum
{
  SCRIPT_INPUT_E = 1,
  SCRIPT_OUTPUT_E = 2,
  SCRIPT_CONSTANT_SCALAR_E = 4,
  SCRIPT_CONSTANT_STRING_E = 8,
  SCRIPT_VARIABLE_E = 16
};

}

#endif
