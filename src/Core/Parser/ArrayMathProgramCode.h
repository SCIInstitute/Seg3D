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

#ifndef CORE_PARSER_ARRAYMATHPROGRAMCODE_H 
#define CORE_PARSER_ARRAYMATHPROGRAMCODE_H 

// STL includes
#include <vector>

// Boost includes 
#include <boost/function.hpp>

// Core includes
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/Parser/ParserFWD.h> // Needed for index_type
#include <Core/Utils/StackBasedVector.h>

namespace Core
{

class ArrayMathProgramCode;
typedef boost::function< bool( ArrayMathProgramCode& pc ) > ArrayMathFunctionObject;

//-----------------------------------------------------------------------------
// Code segment class, all the function calls are based on this class
// providing the program with input and output variables all located in one
// piece of memory. Although this class points to auxilary memory block,
// an effort is made to store them all in the same array, to minimize
// page swapping

class ArrayMathProgramCode
{
public:
  
  // Constructor
  ArrayMathProgramCode( ArrayMathFunctionObject function );

  ArrayMathProgramCode();

  /// Set the function pointer
  inline void set_function( ArrayMathFunctionObject function )
  {
    this->function_ = function;
  }

  /// Get the function pointer
  inline ArrayMathFunctionObject get_function()
  {
    return this->function_;
  }

  /// Tell the progam where to temporary space has been allocated
  /// for this part of the program
  inline void set_variable( size_t j, float* variable )
  {
    if ( j >= this->variables_.size() ) this->variables_.resize( j + 1 );
    this->variables_[ j ] = reinterpret_cast< void* >( variable );
  }

  inline void set_data_block( size_t j, DataBlock* data_block )
  {
    if ( j >= this->variables_.size() ) this->variables_.resize( j + 1 );
    this->variables_[ j ] = reinterpret_cast< void* >( data_block ); 
  }

  inline void set_mask_data_block( size_t j, MaskDataBlock* mask_data_block )
  {
    if ( j >= this->variables_.size() ) this->variables_.resize( j + 1 );
    this->variables_[ j ] = reinterpret_cast< void* >( mask_data_block ); 
  }

  /// Set the index, we keep this in the list so the program knows which
  /// element we need to process.
  inline void set_index( index_type index )
  { 
    this->index_ = index;
  }

  /// Set the size of the array that needs to be processed
  inline void set_size( size_type size )
  { 
    this->size_ = size;
  }

  inline size_type get_size()
  { 
    return this->size_;
  }

  /// These functions are called by the actual code segments
  /// For Scalar, Vector and Tensor buffers
  inline float* get_variable( size_t j )
  { 
    return reinterpret_cast< float* >( this->variables_[ j ] );
  }

  inline DataBlock* get_data_block( size_t j )
  { 
    return reinterpret_cast< DataBlock* >( this->variables_[ j ] );
  }

  inline MaskDataBlock* get_mask_data_block( size_t j )
  { 
    return reinterpret_cast< MaskDataBlock* >( this->variables_[ j ] );
  }

  // Get the current index
  inline index_type get_index()
  { 
    return this->index_;
  }

  // Run this code segment
  // Run time errors are reported by returning a false,
  // After which we can look in the parser script to see
  // which function failed
  inline bool run()
  { 
    return this->function_( *this );
  }

  void print();

private:
  // This is the minimal information needed to run the parsed program
  // In order improve performance, all the buffers and instructions are
  // grouped together so they fit in a few pages of the memory manager

  /// Function call to evaluate this piece of the code
  ArrayMathFunctionObject function_;

  /// Location of where the data is stored
  StackBasedVector < void*, 3 > variables_;

  /// Index in where we are in the selection
  index_type index_;

  /// Sequence size
  size_type size_;
};

}

#endif
