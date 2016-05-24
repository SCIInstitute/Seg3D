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

// Boost includes
#include <boost/thread.hpp>

// Core includes
#include <Core/Parser/ArrayMathProgram.h> 
#include <Core/Utils/Parallel.h>

namespace Core
{

class ArrayMathProgramPrivate
{
public:

  // For parallel code
  void parallel_run( int thread, int num_threads, boost::barrier& barrier );

  // General parameters that determine how many values are computed at
  // the same time and how many processors to use
  size_type buffer_size_;
  int num_threads_;

  // The size of the array we are using
  size_type array_size_;

  // Memory buffer
  std::vector< float > buffer_;

  // Source and Sink information
  std::map< std::string, ArrayMathProgramSource > input_sources_;
  std::map< std::string, ArrayMathProgramSource > output_sinks_;

  // Variable lists
  std::vector< ArrayMathProgramVariableHandle > const_variables_;
  std::vector< ArrayMathProgramVariableHandle > single_variables_;
  std::vector< std::vector< ArrayMathProgramVariableHandle > > sequential_variables_;

  // Program code
  std::vector< ArrayMathProgramCode > const_functions_;
  std::vector< ArrayMathProgramCode > single_functions_;
  std::vector< std::vector< ArrayMathProgramCode > > sequential_functions_;

  ParserProgramHandle pprogram_;

  // Error reporting parallel code
  std::vector< size_type > error_line_;
  std::vector< bool > success_;

  typedef boost::signals2::signal< void (double) > update_progress_signal_type;

  // UPDATE_PROGRESS:
  // When new information on progress is available this signal is triggered. If this signal is 
  // triggered it should end with a value 1.0 indicating that progress reporting has finised.
  // Progress is measured between 0.0 and 1.0.
  update_progress_signal_type update_progress_signal_;
};

void ArrayMathProgramPrivate::parallel_run( int thread, int num_threads, boost::barrier& barrier )
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

  double one_percent_count = 0.01 * per_thread;
  double progress_count = 0;
  while ( offset < end )
  {
    if( thread == 0 && progress_count > one_percent_count )
    {
      // Report progress here -- only at 1% intervals
      this->update_progress_signal_( (
        static_cast< double >( offset ) / static_cast< double >( end ) ) );
      progress_count -= one_percent_count;
    }

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
    progress_count += sz;
  }

  if( thread == 0 )
  {
    // Make sure we hit 100%
    this->update_progress_signal_( 1.0 ); 
  }

  barrier.wait();
}

ArrayMathProgram::ArrayMathProgram() :
  private_( new ArrayMathProgramPrivate )
{
  // Buffer size describes how many values of a sequential variable are
  // grouped together for vectorized execution
  this->private_->buffer_size_ = 128;
  // Number of processors to use
  this->private_->num_threads_ = boost::thread::hardware_concurrency();

  // The size of the array
  this->private_->array_size_ = 1;
}

ArrayMathProgram::ArrayMathProgram( size_type array_size, size_type buffer_size,
                   int num_threads /*= -1*/ ) :
  private_( new ArrayMathProgramPrivate )
{
  // Buffer size describes how many values of a sequential variable are
  // grouped together for vectorized execution
  this->private_->buffer_size_ = buffer_size;
  // Number of processors to use
  if ( num_threads < 1 ) 
  {
    num_threads = boost::thread::hardware_concurrency();
  }
  this->private_->num_threads_ = num_threads;

  // The size of the array
  this->private_->array_size_ = array_size;
}

bool ArrayMathProgram::add_source( std::string& name, DataBlockHandle data_block )
{
  ArrayMathProgramSource ps;
  ps.set_data_block( data_block );
  this->private_->input_sources_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_source( std::string& name, MaskDataBlockHandle mask_data_block )
{
  ArrayMathProgramSource ps;
  ps.set_mask_data_block( mask_data_block );
  this->private_->input_sources_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_sink( std::string& name, DataBlockHandle data_block )
{
  ArrayMathProgramSource ps;
  ps.set_data_block( data_block );
  this->private_->output_sinks_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::find_source( std::string& name, ArrayMathProgramSource& ps )
{
  std::map< std::string, ArrayMathProgramSource >::iterator it = 
    this->private_->input_sources_.find( name );
  if ( it == this->private_->input_sources_.end() ) 
  {
    return false;
  }
  ps = ( *it ).second;
  return true;
}

bool ArrayMathProgram::find_sink( std::string& name, ArrayMathProgramSource& ps )
{
  std::map< std::string, ArrayMathProgramSource >::iterator it = 
    this->private_->output_sinks_.find( name );
  if ( it == this->private_->output_sinks_.end() ) 
  {
    return false;
  }
  ps = ( *it ).second;
  return true;
}

bool ArrayMathProgram::run_const( size_t& error_line )
{
  size_t size = this->private_->const_functions_.size();
  for ( size_t j = 0; j < size; j++ )
  {
    if ( !( this->private_->const_functions_[ j ].run() ) )
    {
      error_line = j;
      return false;
    }
  }

  return true;
}

bool ArrayMathProgram::run_single( size_t& error_line )
{
  size_t size = this->private_->single_functions_.size();
  for ( size_t j = 0; j < size; j++ )
  {
    if ( !( this->private_->single_functions_[ j ].run() ) )
    {
      error_line = j;
      return false;
    }
  }
  return true;
}

bool ArrayMathProgram::run_sequential( size_t& error_line )
{
  this->private_->error_line_.resize( this->private_->num_threads_, 0 );
  this->private_->success_.resize( this->private_->num_threads_, true );

  this->private_->update_progress_signal_.connect( 
    boost::bind( &ArrayMathProgram::update_progress, this, _1 ) );

  Parallel parallel_run_sequential( 
    boost::bind( &ArrayMathProgramPrivate::parallel_run, this->private_, _1, _2, _3 ), 
    this->private_->num_threads_ );
  parallel_run_sequential.run();

  for ( int j = 0; j < this->private_->num_threads_; j++ )
  {
    if ( this->private_->success_[ j ] == false )
    {
      error_line = this->private_->error_line_[ j ];
      return false;
    }
  }

  return true;
}

size_type ArrayMathProgram::get_buffer_size()
{
  return this->private_->buffer_size_;
}

int ArrayMathProgram::get_num_threads()
{
  return this->private_->num_threads_;
}

size_type ArrayMathProgram::get_array_size()
{
  return this->private_->array_size_;
}

void ArrayMathProgram::set_array_size( size_type array_size )
{
  this->private_->array_size_ = array_size;
}

void ArrayMathProgram::resize_const_variables( size_t sz )
{
  this->private_->const_variables_.resize( sz );
}

void ArrayMathProgram::resize_single_variables( size_t sz )
{
  this->private_->single_variables_.resize( sz );
}

void ArrayMathProgram::resize_sequential_variables( size_t sz )
{
  this->private_->sequential_variables_.resize( this->private_->num_threads_ );
  for ( int np=0; np < this->private_->num_threads_; np++ )
  {
    this->private_->sequential_variables_[ np ].resize( sz );
  }
}

void ArrayMathProgram::resize_const_functions( size_t sz )
{
  this->private_->const_functions_.resize( sz );
}

void ArrayMathProgram::resize_single_functions( size_t sz )
{
  this->private_->single_functions_.resize( sz );
}

void ArrayMathProgram::resize_sequential_functions( size_t sz )
{
  this->private_->sequential_functions_.resize( this->private_->num_threads_ );
  for ( int np=0; np < this->private_->num_threads_; np++ )
  {
    this->private_->sequential_functions_[ np ].resize( sz );
  }
}

float* ArrayMathProgram::create_buffer( size_t size )
{
  this->private_->buffer_.resize( size ); 
  return &( this->private_->buffer_[ 0 ] );
}

void ArrayMathProgram::set_const_variable( size_t j, ArrayMathProgramVariableHandle& handle )
{
  this->private_->const_variables_[ j ] = handle;
}

void ArrayMathProgram::set_single_variable( size_t j, ArrayMathProgramVariableHandle& handle )
{
  this->private_->single_variables_[ j ] = handle;
}

void ArrayMathProgram::set_sequential_variable( size_t j, size_t np, 
  ArrayMathProgramVariableHandle& handle )
{
  this->private_->sequential_variables_[ np ][ j ] = handle;
}

ArrayMathProgramVariableHandle ArrayMathProgram::get_const_variable( size_t j )
{
  return this->private_->const_variables_[ j ];
}

ArrayMathProgramVariableHandle ArrayMathProgram::get_single_variable( size_t j )
{
  return this->private_->single_variables_[ j ];
}

ArrayMathProgramVariableHandle ArrayMathProgram::get_sequential_variable( size_t j, size_t np )
{
  return this->private_->sequential_variables_[ np ][ j ];
}

void ArrayMathProgram::set_const_program_code( size_t j, ArrayMathProgramCode& pc )
{
  this->private_->const_functions_[ j ] = pc;
}

void ArrayMathProgram::set_single_program_code( size_t j, ArrayMathProgramCode& pc )
{
  this->private_->single_functions_[ j ] = pc;
}

void ArrayMathProgram::set_sequential_program_code( size_t j, size_t np, ArrayMathProgramCode& pc )
{
  this->private_->sequential_functions_[ np ][ j ] = pc;
}

void ArrayMathProgram::set_parser_program( ParserProgramHandle handle )
{
  this->private_->pprogram_ = handle;
}

ParserProgramHandle ArrayMathProgram::get_parser_program()
{
  return this->private_->pprogram_;
}

void ArrayMathProgram::update_progress( double amount )
{
  this->update_progress_signal_( amount );
}

} // end namespace
