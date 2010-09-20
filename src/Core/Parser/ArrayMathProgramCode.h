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

#ifndef CORE_PARSER_ARRAYMATHPROGRAMCODE_H 
#define CORE_PARSER_ARRAYMATHPROGRAMCODE_H 

// STL includes
#include <vector>

// Core includes
#include <Core/Parser/ParserFWD.h> // Needed for index_type
#include <Core/Utils/StackBasedVector.h>

namespace Core
{

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
  ArrayMathProgramCode( bool ( *function )( ArrayMathProgramCode& pc ) );

  ArrayMathProgramCode();

  // Set the function pointer
  inline void set_function( bool ( *function )( ArrayMathProgramCode& pc ) )
  {
    this->function_ = function;
  }

  // Get the function pointer
  inline bool ( *get_function() )( ArrayMathProgramCode& pc )
  {
    return this->function_;
  }

  // Tell the progam where to temporary space has been allocated
  // for this part of the program
  inline void set_variable( size_t j, double* variable )
  {
    if ( j >= this->variables_.size() ) this->variables_.resize( j + 1 );
    this->variables_[ j ] = reinterpret_cast< void* >( variable );
  }

  inline void set_bool_array( size_t j, std::vector< bool >* array )
  {
    if ( j >= this->variables_.size() ) this->variables_.resize( j + 1 );
    this->variables_[ j ] = array;
  }

  inline void set_double_array( size_t j, std::vector< double >* array )
  {
    if ( j >= this->variables_.size() ) this->variables_.resize( j + 1 );
    this->variables_[ j ] = array;
  }

  // Set the index, we keep this in the list so the program knows which
  // element we need to process.
  inline void set_index( index_type index )
  { 
    this->index_ = index;
  }

  // Set the size of the array that needs to be processed
  inline void set_size( size_type size )
  { 
    this->size_ = size;
  }

  inline size_type get_size()
  { 
    return this->size_;
  }

  // These functions are called by the actual code segments
  // For Scalar, Vector and Tensor buffers
  inline double* get_variable( size_t j )
  { 
    return reinterpret_cast< double* >( this->variables_[ j ] );
  }

  inline std::vector< bool >* get_bool_array( size_t j )
  { 
    return reinterpret_cast< std::vector< bool >* >( this->variables_[ j ] );
  }

  inline std::vector< double >* get_double_array( size_t j )
  { 
    return reinterpret_cast< std::vector< double >* >( this->variables_[ j ] );
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

  // Function call to evaluate this piece of the code
  bool ( *function_ )( ArrayMathProgramCode& pc );

  // Location of where the data is stored
  StackBasedVector < void*, 3 > variables_;

  // Index in where we are in the selection
  index_type index_;

  // Sequence size
  size_type size_;
};

}

#endif
