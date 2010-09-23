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

// Core includes
#include <Core/Parser/ArrayMathProgram.h> 
#include <Core/Utils/Parallel.h>

namespace Core
{

bool ArrayMathProgram::add_source( std::string& name, DataBlock* data_block )
{
  ArrayMathProgramSource ps;
  ps.set_data_block( data_block );
  this->input_sources_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_source( std::string& name, MaskDataBlock* mask_data_block )
{
  ArrayMathProgramSource ps;
  ps.set_mask_data_block( mask_data_block );
  this->input_sources_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_sink( std::string& name, DataBlock* data_block )
{
  ArrayMathProgramSource ps;
  ps.set_data_block( data_block );
  this->output_sinks_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_sink( std::string& name, MaskDataBlock* mask_data_block )
{
  ArrayMathProgramSource ps;
  ps.set_mask_data_block( mask_data_block );
  this->output_sinks_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::find_source( std::string& name, ArrayMathProgramSource& ps )
{
  std::map< std::string, ArrayMathProgramSource >::iterator it = this->input_sources_.find( name );
  if ( it == this->input_sources_.end() ) 
  {
    return false;
  }
  ps = ( *it ).second;
  return true;
}

bool ArrayMathProgram::find_sink( std::string& name, ArrayMathProgramSource& ps )
{
  std::map< std::string, ArrayMathProgramSource >::iterator it = this->output_sinks_.find( name );
  if ( it == this->output_sinks_.end() ) 
  {
    return false;
  }
  ps = ( *it ).second;
  return true;
}

bool ArrayMathProgram::run_const( size_t& error_line )
{
  size_t size = this->const_functions_.size();
  for ( size_t j = 0; j < size; j++ )
  {
    if ( !( this->const_functions_[ j ].run() ) )
    {
      error_line = j;
      return false;
    }
  }

  return true;
}

bool ArrayMathProgram::run_single( size_t& error_line )
{
  size_t size = this->single_functions_.size();
  for ( size_t j = 0; j < size; j++ )
  {
    if ( !( this->single_functions_[ j ].run() ) )
    {
      error_line = j;
      return false;
    }
  }
  return true;
}

bool ArrayMathProgram::run_sequential( size_t& error_line )
{
  this->error_line_.resize( this->num_threads_, 0 );
  this->success_.resize( this->num_threads_, true );

  Parallel parallel_run_sequential( 
    boost::bind( &ArrayMathProgram::parallel_run, this, _1, _2, _3 ), this->num_threads_ );
  parallel_run_sequential.run();

  for ( int j = 0; j < this->num_threads_; j++ )
  {
    if ( this->success_[ j ] == false )
    {
      error_line = this->error_line_[ j ];
      return false;
    }
  }

  return true;
}

void ArrayMathProgram::parallel_run( int thread, int num_threads, boost::barrier& barrier )
{
  index_type per_thread = this->array_size_ / num_threads;
  index_type start = thread * per_thread;
  index_type end = ( thread + 1 ) * per_thread;
  index_type offset, sz;
  if ( thread + 1 == num_threads ) 
  {
    end = this->array_size_;
  }

  offset = start;
  this->success_[ thread ] = true;

  while ( offset < end )
  {
    sz = this->buffer_size_;
    if ( offset + sz >= end ) 
    {
      sz = end - offset;
    }

    size_t size = this->sequential_functions_[ thread ].size();
    for ( size_t j = 0; j < size; j++ )
    {
      this->sequential_functions_[ thread ][ j ].set_index( offset );
      this->sequential_functions_[ thread ][ j ].set_size( sz );
    }
    for ( size_t j = 0; j < size; j++ )
    {
      if ( !( this->sequential_functions_[ thread ][ j ].run() ) )
      {
        this->error_line_[ thread ] = j;
        this->success_[ thread ] = false;
      }
    }
    offset += sz;
  }

  barrier.wait();
}

ArrayMathProgram::ArrayMathProgram() 
{
  // Buffer size describes how many values of a sequential variable are
  // grouped together for vectorized execution
  this->buffer_size_ = 128;
  // Number of processors to use
  this->num_threads_ = boost::thread::hardware_concurrency();

  // The size of the array
  this->array_size_ = 1;
}

ArrayMathProgram::ArrayMathProgram( size_type array_size, size_type buffer_size,
  int num_threads /*= -1*/ ) 
{
  // Buffer size describes how many values of a sequential variable are
  // grouped together for vectorized execution
  this->buffer_size_ = buffer_size;
  // Number of processors to use
  if ( num_threads < 1 ) 
  {
    num_threads = boost::thread::hardware_concurrency();
  }
  this->num_threads_ = num_threads;

  // The size of the array
  this->array_size_ = array_size;
}

size_type ArrayMathProgram::get_buffer_size()
{
  return this->buffer_size_;
}

int ArrayMathProgram::get_num_threads()
{
  return this->num_threads_;
}

size_type ArrayMathProgram::get_array_size()
{
  return this->array_size_;
}

void ArrayMathProgram::set_array_size( size_type array_size )
{
  this->array_size_ = array_size;
}

void ArrayMathProgram::resize_const_variables( size_t sz )
{
  this->const_variables_.resize( sz );
}

void ArrayMathProgram::resize_single_variables( size_t sz )
{
  this->single_variables_.resize( sz );
}

void ArrayMathProgram::resize_sequential_variables( size_t sz )
{
  this->sequential_variables_.resize( this->num_threads_ );
  for ( int np=0; np < this->num_threads_; np++ )
  {
    this->sequential_variables_[ np ].resize( sz );
  }
}

void ArrayMathProgram::resize_const_functions( size_t sz )
{
  this->const_functions_.resize( sz );
}

void ArrayMathProgram::resize_single_functions( size_t sz )
{
  this->single_functions_.resize( sz );
}

void ArrayMathProgram::resize_sequential_functions( size_t sz )
{
  this->sequential_functions_.resize( this->num_threads_ );
  for ( int np=0; np < this->num_threads_; np++ )
  {
    this->sequential_functions_[ np ].resize( sz );
  }
}

double* ArrayMathProgram::create_buffer( size_t size )
{
  this->buffer_.resize( size ); 
  return &( this->buffer_[ 0 ] );
}

void ArrayMathProgram::set_const_variable( size_t j, ArrayMathProgramVariableHandle& handle )
{
  this->const_variables_[ j ] = handle;
}

void ArrayMathProgram::set_single_variable( size_t j, ArrayMathProgramVariableHandle& handle )
{
  this->single_variables_[ j ] = handle;
}

void ArrayMathProgram::set_sequential_variable( size_t j, size_t np, 
  ArrayMathProgramVariableHandle& handle )
{
  this->sequential_variables_[ np ][ j ] = handle;
}

ArrayMathProgramVariableHandle ArrayMathProgram::get_const_variable( size_t j )
{
  return this->const_variables_[ j ];
}

ArrayMathProgramVariableHandle ArrayMathProgram::get_single_variable( size_t j )
{
  return this->single_variables_[ j ];
}

ArrayMathProgramVariableHandle ArrayMathProgram::get_sequential_variable( size_t j, size_t np )
{
  return this->sequential_variables_[ np ][ j ];
}

void ArrayMathProgram::set_const_program_code( size_t j, ArrayMathProgramCode& pc )
{
  this->const_functions_[ j ] = pc;
}

void ArrayMathProgram::set_single_program_code( size_t j, ArrayMathProgramCode& pc )
{
  this->single_functions_[ j ] = pc;
}

void ArrayMathProgram::set_sequential_program_code( size_t j, size_t np, ArrayMathProgramCode& pc )
{
  this->sequential_functions_[ np ][ j ] = pc;
}

void ArrayMathProgram::set_parser_program( ParserProgramHandle handle )
{
  this->pprogram_ = handle;
}

ParserProgramHandle ArrayMathProgram::get_parser_program()
{
  return this->pprogram_;
}

} // end namespace
