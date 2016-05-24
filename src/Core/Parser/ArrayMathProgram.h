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

#ifndef CORE_PARSER_ARRAYMATHPROGRAM_H 
#define CORE_PARSER_ARRAYMATHPROGRAM_H 

// Core includes
#include <Core/Parser/ArrayMathProgramCode.h>
#include <Core/Parser/ArrayMathProgramSource.h>
#include <Core/Parser/ParserFWD.h>
#include <Core/Utils/Lockable.h>

namespace Core
{

// Hide header includes, private interface and implementation
class ArrayMathProgramPrivate;
typedef boost::shared_ptr< ArrayMathProgramPrivate > ArrayMathProgramPrivateHandle;

class ArrayMathProgram : public Lockable
{

public:

  // Default constructor
  ArrayMathProgram();

  /// Constructor that allows overloading the default optimization parameters
  ArrayMathProgram( size_type array_size, size_type buffer_size, int num_threads = -1 );

  /// Get the optimization parameters, these can only be set when creating the
  /// object as it depends on allocated buffer sizes and those are hard to change
  /// when allocated
  /// Get the number of entries that are processed at once
  size_type get_buffer_size();

  /// Get the number of threads
  int get_num_threads();

  /// Set the size of the array to process
  size_type get_array_size();
  void set_array_size( size_type array_size );

  bool add_source( std::string& name, DataBlockHandle data_block );
  bool add_source( std::string& name, MaskDataBlockHandle mask_data_block );

  bool add_sink( std::string& name, DataBlockHandle data_block );

  void resize_const_variables( size_t sz );
  void resize_single_variables( size_t sz );
  void resize_sequential_variables( size_t sz );

  void resize_const_functions( size_t sz );
  void resize_single_functions( size_t sz );
  void resize_sequential_functions( size_t sz );

  /// Central buffer for all parameters
  float* create_buffer( size_t size );

  /// Set variables which we use as temporal information structures
  // TODO: need to remove them at some point
  void set_const_variable( size_t j, ArrayMathProgramVariableHandle& handle );
  void set_single_variable( size_t j, ArrayMathProgramVariableHandle& handle );
  void set_sequential_variable( size_t j, size_t np, ArrayMathProgramVariableHandle& handle );

  ArrayMathProgramVariableHandle get_const_variable( size_t j );
  ArrayMathProgramVariableHandle get_single_variable( size_t j );
  ArrayMathProgramVariableHandle get_sequential_variable( size_t j, size_t np );

  /// Set program code
  void set_const_program_code( size_t j, ArrayMathProgramCode& pc );
  void set_single_program_code( size_t j, ArrayMathProgramCode& pc );
  void set_sequential_program_code( size_t j, size_t np, ArrayMathProgramCode& pc );

  /// Code to find the pointers that are given for sources and sinks  
  bool find_source( std::string& name, ArrayMathProgramSource& ps );
  bool find_sink( std::string& name, ArrayMathProgramSource& ps );

  bool run_const( size_t& error_line );
  bool run_single( size_t& error_line );
  bool run_sequential( size_t& error_line );

  void set_parser_program( ParserProgramHandle handle );
  ParserProgramHandle get_parser_program();

  typedef boost::signals2::signal< void (double) > update_progress_signal_type;

  // UPDATE_PROGRESS:
  /// When new information on progress is available this signal is triggered. If this signal is 
  /// triggered it should end with a value 1.0 indicating that progress reporting has finised.
  /// Progress is measured between 0.0 and 1.0.
  update_progress_signal_type update_progress_signal_;

private:
  void update_progress( double amount );

  ArrayMathProgramPrivateHandle private_;
};

}

#endif
