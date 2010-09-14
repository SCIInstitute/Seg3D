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

#ifndef CORE_PARSER_ARRAYMATHINTERPRETER_H 
#define CORE_PARSER_ARRAYMATHINTERPRETER_H 

// Boost includes 
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/Parser/ParserFunction.h>
#include <Core/Parser/ParserFWD.h>
#include <Core/Utils/Lockable.h>
#include <Core/Utils/StackBasedVector.h>

namespace Core
{

// Define class as they are not used in order

class ArrayMathIntepreter;
class ArrayMathFunction;

class ArrayMathProgram;
class ArrayMathProgramCode;
class ArrayMathProgramVariable;

// Handles for a few of the classes
// As Program is stored in a large array we do not need a handle for that
// one. These are helper classes that are located elsewhere in memory

typedef boost::shared_ptr< ArrayMathProgramVariable > ArrayMathProgramVariableHandle;
typedef boost::shared_ptr< ArrayMathProgram > ArrayMathProgramHandle;

// Depending on whether SCIRun is 64bits, the indices into the fields are
// altered to make sure we can address really large memory chunks.

// TODO How to handle this?
//#ifdef SCI_64BITS
//typedef long long       index_type;
//typedef long long       size_type;
//#else
typedef int             index_type;
typedef int             size_type;
//#endif

//-----------------------------------------------------------------------------
// Functions for databasing the function calls that make up the program

// This class is used to describe each function in the system
// It describes the name + input arguments, the return type,
// additional flags and a pointer the the actual function that
// needs to be called


class ArrayMathFunction : public ParserFunction
{
public:
  // Build a new function
  ArrayMathFunction(
    bool ( *function )( ArrayMathProgramCode& pc ),
    std::string function_id, std::string function_type, int function_flags );

  // Virtual destructor so I can do dynamic casts on this class 
  virtual ~ArrayMathFunction() {}

  // Get the pointer to the function
  bool ( *get_function() )( ArrayMathProgramCode& pc ) 
  { 
    return this->function_; 
  } 

private:
  // The function to call that needs to be called on the data
  bool ( *function_ )( ArrayMathProgramCode& pc );

};

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

class ArrayMathProgramVariable : public Lockable
{

public:
  // Constructor of the variable
  ArrayMathProgramVariable( std::string name, double* data ) :
    name_(name), 
    data_(data) 
  {}

  // Retrieve the data pointer from the central temporal
  // storage
  double* get_data();

private:
  // Name of variable
  std::string name_;

  // Where the data needs to be store
  double* data_;
};

class ArrayMathProgramSource
{

public:
  ArrayMathProgramSource() :
    bool_array_(0), 
    double_array_(0) {}

  void set_bool_array( std::vector< bool >* array );
  std::vector< bool >* get_bool_array();
  bool is_bool_array();

  void set_double_array( std::vector< double >* array );
  std::vector< double >* get_double_array();
  bool is_double_array();

private:
  std::vector< bool >* bool_array_;
  std::vector< double >* double_array_;
};

class ArrayMathProgram : public Lockable
{

public:
  
  // Default constructor
  ArrayMathProgram();

  // Constructor that allows overloading the default optimization parameters
  ArrayMathProgram( size_type array_size, size_type buffer_size, int num_proc = -1 );

  // Get the optimization parameters, these can only be set when creating the
  // object as it depends on allocated buffer sizes and those are hard to change
  // when allocated
  // Get the number of entries that are processed at once
  size_type get_buffer_size();

  // Get the number of processors
  int get_num_proc();

  // Set the size of the array to process
  size_type get_array_size();
  void set_array_size( size_type array_size );

  bool add_source( std::string& name, std::vector< bool >* array );
  bool add_source( std::string& name, std::vector< double >* array );

  bool add_sink( std::string& name, std::vector< bool >* array );
  bool add_sink( std::string& name, std::vector< double >* array );

  void resize_const_variables( size_t sz );
  void resize_single_variables( size_t sz );
  void resize_sequential_variables( size_t sz );

  void resize_const_functions( size_t sz );
  void resize_single_functions( size_t sz );
  void resize_sequential_functions( size_t sz );

  // Central buffer for all parameters
  double* create_buffer( size_t size );

  // Set variables which we use as temporal information structures
  // TODO: need to remove them at some point
  void set_const_variable( size_t j, ArrayMathProgramVariableHandle& handle );
  void set_single_variable( size_t j, ArrayMathProgramVariableHandle& handle );
  void set_sequential_variable( size_t j, size_t np, ArrayMathProgramVariableHandle& handle );

  ArrayMathProgramVariableHandle get_const_variable( size_t j );
  ArrayMathProgramVariableHandle get_single_variable( size_t j );
  ArrayMathProgramVariableHandle get_sequential_variable( size_t j, size_t np );

  // Set program code
  void set_const_program_code( size_t j, ArrayMathProgramCode& pc );
  void set_single_program_code( size_t j, ArrayMathProgramCode& pc );
  void set_sequential_program_code( size_t j, size_t np, ArrayMathProgramCode& pc );

  // Code to find the pointers that are given for sources and sinks  
  bool find_source( std::string& name, ArrayMathProgramSource& ps );
  bool find_sink( std::string& name, ArrayMathProgramSource& ps );

  bool run_const( size_t& error_line );
  bool run_single( size_t& error_line );
  bool run_sequential( size_t& error_line );

  void set_parser_program( ParserProgramHandle handle );
  ParserProgramHandle get_parser_program();

private:

  // General parameters that determine how many values are computed at
  // the same time and how many processors to use
  size_type buffer_size_;
  int num_threads_;

  // The size of the array we are using
  size_type array_size_;

  // Memory buffer
  std::vector< double > buffer_;

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

  // For parallel code
private:
  void parallel_run( int thread, int num_threads, boost::barrier& barrier );

  // Error reporting parallel code
  std::vector< size_type > error_line_;
  std::vector< bool > success_;
};

class ArrayMathInterpreter
{

public:
  // The interpreter Creates executable code from the parsed code
  // The first step is setting the data sources and sinks


  //------------------------------------------------------------------------
  // Step 0 : create program variable
  bool create_program( ArrayMathProgramHandle& mprogram, std::string& error );

  //------------------------------------------------------------------------
  // Step 1: add sources and sinks
  bool add_bool_array_source( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< bool >* array, std::string& error );

  bool add_double_array_source( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< double >* array, std::string& error );

  // Data sinks  
  bool add_bool_array_sink( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< bool >* array, std::string& error );

  bool add_double_array_sink( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< double >* array, std::string& error );

  //------------------------------------------------------------------------
  // Step 2: translate code and generate executable code

  // Main function for transcribing the parser output into a program that
  // can actually be executed
  bool translate( ParserProgramHandle& pprogram, ArrayMathProgramHandle& mprogram,
    std::string& error );

  //------------------------------------------------------------------------
  // Step 3: Set the array size

  bool set_array_size( ArrayMathProgramHandle& mprogram, size_type array_size );

  //------------------------------------------------------------------------
  // Step 4: Run the code

  bool run( ArrayMathProgramHandle& mprogram, std::string& error );

};

}

#endif
