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

#ifndef CORE_PARSER_ARRAYMATHINTERPRETER_H 
#define CORE_PARSER_ARRAYMATHINTERPRETER_H 

// Core includes
#include <Core/Parser/ParserFWD.h>

namespace Core
{

class ArrayMathInterpreter
{

public:
  // The interpreter Creates executable code from the parsed code
  // The first step is setting the data sources and sinks

  //------------------------------------------------------------------------
  /// Step 0 : create program variable
  bool create_program( ArrayMathProgramHandle& mprogram, std::string& error );

  //------------------------------------------------------------------------
  /// Step 1: add sources and sinks
  bool add_data_block_source( ArrayMathProgramHandle& pprogram, 
    std::string& name, DataBlockHandle data_block, std::string& error );

  bool add_mask_data_block_source( ArrayMathProgramHandle& pprogram, 
    std::string& name, MaskDataBlockHandle mask_data_block, std::string& error );

  // Data sinks  
  bool add_data_block_sink( ArrayMathProgramHandle& pprogram, 
    std::string& name, DataBlockHandle data_block, std::string& error );

  //------------------------------------------------------------------------
  /// Step 2: translate code and generate executable code

  /// Main function for transcribing the parser output into a program that
  /// can actually be executed
  bool translate( ParserProgramHandle& pprogram, ArrayMathProgramHandle& mprogram,
    std::string& error );

  //------------------------------------------------------------------------
  /// Step 3: Set the array size

  bool set_array_size( ArrayMathProgramHandle& mprogram, size_type array_size );

  //------------------------------------------------------------------------
  /// Step 4: Run the code

  bool run( ArrayMathProgramHandle& mprogram, std::string& error );

};

}

#endif
