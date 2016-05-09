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

#ifndef CORE_PARSER_ARRAYMATHPROGRAMSOURCE_H 
#define CORE_PARSER_ARRAYMATHPROGRAMSOURCE_H 

// Core includes
#include <Core/Parser/ParserFWD.h>

namespace Core
{

// Hide header includes, private interface and implementation
class ArrayMathProgramSourcePrivate;
typedef boost::shared_ptr< ArrayMathProgramSourcePrivate > ArrayMathProgramSourcePrivateHandle;

class ArrayMathProgramSource
{

public:
  ArrayMathProgramSource();

  void set_data_block( DataBlockHandle data_block );
  DataBlock* get_data_block();
  bool is_data_block();

  void set_mask_data_block( MaskDataBlockHandle mask_data_block );
  MaskDataBlock* get_mask_data_block();
  bool is_mask_data_block();

private:
  ArrayMathProgramSourcePrivateHandle private_;
};

}

#endif
