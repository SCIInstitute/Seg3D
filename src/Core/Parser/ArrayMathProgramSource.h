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

#ifndef CORE_PARSER_ARRAYMATHPROGRAMSOURCE_H 
#define CORE_PARSER_ARRAYMATHPROGRAMSOURCE_H 

// STL includes
#include <vector>

// Core includes
#include <Core/DataBlock/DataBlock.h>

namespace Core
{

class ArrayMathProgramSource
{

public:
  ArrayMathProgramSource() :
    bool_array_( 0 ), 
    double_array_( 0 ),
    data_block_( 0 ) {}

  void set_bool_array( std::vector< bool >* array );
  std::vector< bool >* get_bool_array();
  bool is_bool_array();

  void set_double_array( std::vector< double >* array );
  std::vector< double >* get_double_array();
  bool is_double_array();

  void set_data_block( DataBlock* data_block );
  DataBlock* get_data_block();
  bool is_data_block();

private:
  std::vector< bool >* bool_array_;
  std::vector< double >* double_array_;
  DataBlock* data_block_;
};

}

#endif
