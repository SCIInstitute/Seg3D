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

// STL includes
#include <iostream>

// Boost includes
#include <boost/thread.hpp>

// Core includes
#include <Core/Parser/ArrayMathInterpreter.h> 
#include <Core/Parser/ParserEnums.h>
#include <Core/Parser/ParserProgram.h>
#include <Core/Parser/ParserScriptFunction.h>
#include <Core/Parser/ParserScriptVariable.h>
#include <Core/Utils/Parallel.h>
//#include <Core/Parser/ArrayMathFunctionCatalog.h>

namespace Core
{

ArrayMathFunction::ArrayMathFunction( bool( *function )( ArrayMathProgramCode& pc ),
    std::string function_id, std::string function_type, int function_flags ) :
  ParserFunction( function_id, function_type, function_flags )
{
  this->function_ = function;
}

bool ArrayMathInterpreter::create_program( ArrayMathProgramHandle& mprogram, std::string& error )
{
  if ( mprogram.get() == 0 )
  {
    mprogram = ArrayMathProgramHandle( new ArrayMathProgram() );

    if ( mprogram.get() == 0 )
    {
      error = "INTERNAL ERROR - Could not allocate ArrayMathProgram.";
      return false;
    }
  }
  return true;
}

// -------------------------------------------------------------------------
// Translate a parser script

bool ArrayMathInterpreter::translate( ParserProgramHandle& pprogram,
    ArrayMathProgramHandle& mprogram, std::string& error )
{
  // Create program is needed
  if ( !( create_program( mprogram, error ) ) )
  {
    return false;
  }

  mprogram->set_parser_program( pprogram );

  // -------------------------------------------------------------------------
  // Process const part of the program

  // Determine the buffer sizes that need to be allocated

  // Get the number of variables/function calls involved  
  size_t num_const_variables = pprogram->num_const_variables();
  size_t num_const_functions = pprogram->num_const_functions();
  size_t num_single_variables = pprogram->num_single_variables();
  size_t num_single_functions = pprogram->num_single_functions();
  size_t num_sequential_variables = pprogram->num_sequential_variables();
  size_t num_sequential_functions = pprogram->num_sequential_functions();

  // Reserve space for const part of the program
  mprogram->resize_const_variables( num_const_variables );
  mprogram->resize_const_functions( num_const_functions );
  mprogram->resize_single_variables( num_single_variables );
  mprogram->resize_single_functions( num_single_functions );
  mprogram->resize_sequential_variables( num_sequential_variables );
  mprogram->resize_sequential_functions( num_sequential_functions );

  // Variable part
  ParserScriptVariableHandle chandle;
  ParserScriptVariableHandle vhandle;
  ParserScriptVariableHandle phandle;
  ParserScriptFunctionHandle fhandle;
  int onum, inum, kind;
  std::string type, name;
  int flags;

  size_type buffer_mem = 0;
  // Determine how many space we need to reserve for const variables
  for ( size_t j = 0; j < num_const_variables; j++ )
  {
    pprogram->get_const_variable( j, vhandle );
    std::string type = vhandle->get_type();

    if ( type == "S" )
    {
      buffer_mem += 1;
    }
    else if ( type == "AB" || type == "AD" )
    {
      buffer_mem += 0;
    }
    else
    {
      error
          = "INTERNAL ERROR - Encountered a type that is not supported by ArrayMath functions.";
      return false;
    }
  }

  // Determine how many space we need to reserve for single variables
  for ( size_t j = 0; j < num_single_variables; j++ )
  {
    pprogram->get_single_variable( j, vhandle );
    std::string type = vhandle->get_type();

    if ( type == "S" )
    {
      buffer_mem += 1;
    }
    else if ( type == "AB" || type == "AD" )
    {
      buffer_mem += 0;
    }
    else
    {
      error
          = "INTERNAL ERROR - Encountered a type that is not supported by ArrayMath functions.";
      return false;
    }
  }

  // Determine how many space we need to reserve for sequential variables
  int buffer_size = mprogram->get_buffer_size();
  int num_proc = mprogram->get_num_proc();

  for ( int np = 0; np < num_proc; np++ )
  {
    for ( size_t j = 0; j < num_sequential_variables; j++ )
    {
      pprogram->get_sequential_variable( j, vhandle );
      std::string type = vhandle->get_type();
      flags = vhandle->get_flags();

      if ( ( flags & SCRIPT_CONST_VAR_E ) && ( np > 0 ) )
      {
        continue;
      }
      if ( type == "S" )
      {
        buffer_mem += 1 * buffer_size;
      }
      else if ( type == "AB" || type == "AD" )
      {
        buffer_mem += 0;
      }
      else
      {
        error
            = "INTERNAL ERROR - Encountered a type that is not supported by ArrayMath functions.";
        return false;
      }
    }
  }

  // Now assign buffers to variables

  // Get all the constants in one piece of memory
  // All memory management is inside the ArrayMathProgram
  // We only get the pointer to actually insert all the pieces
  double* buffer = mprogram->create_buffer( buffer_mem );

  for ( size_t j = 0; j < num_const_variables; j++ )
  {
    // Get the next constant variable
    pprogram->get_const_variable( j, vhandle );

    // Determine the name of the variable
    name = vhandle->get_name();

    // Determine the type to see what we need to do
    type = vhandle->get_type();

    // Determine the type of the variable
    kind = vhandle->get_kind();

    ArrayMathProgramVariableHandle pvhandle;
    if ( type == "S" )
    {
      // Insert constant variables directly into the buffer
      // This variable should be read only, hence we should be
      // able to store it right away
      if ( kind == SCRIPT_CONSTANT_SCALAR_E )
      {
        double val = vhandle->get_scalar_value();
        buffer[ 0 ] = val;
      }

      // Generate a new program variable
      pvhandle = ArrayMathProgramVariableHandle( new ArrayMathProgramVariable( name, buffer ) );
      buffer += 1;
    }
    else
    {
      // Generate a new program variable where the value is stored
      // This is a source or sinkl that does not need memory
      pvhandle = ArrayMathProgramVariableHandle( new ArrayMathProgramVariable( name, 0 ) );
    }

    // Add this variable to the code 
    mprogram->set_const_variable( j, pvhandle );
  }

  for ( size_t j = 0; j < num_single_variables; j++ )
  {
    // Get the next constant variable
    pprogram->get_single_variable( j, vhandle );

    // Determine the name of the variable
    name = vhandle->get_name();

    // Determine the type to see what we need to do
    type = vhandle->get_type();

    // Determine the type of the variable
    kind = vhandle->get_kind();

    ArrayMathProgramVariableHandle pvhandle;
    if ( type == "S" )
    {
      // Generate a new program variable
      pvhandle = ArrayMathProgramVariableHandle( new ArrayMathProgramVariable( name, buffer ) );
      buffer += 1;
    }
    else
    {
      // Generate a new program variable where the value is stored
      // This is a source or sinkl that does not need memory
      pvhandle = ArrayMathProgramVariableHandle( new ArrayMathProgramVariable( name, 0 ) );
    }

    // Add this variable to the code 
    mprogram->set_single_variable( j, pvhandle );
  }

  for ( int np = 0; np < num_proc; np++ )
  {
    for ( size_t j = 0; j < num_sequential_variables; j++ )
    {
      // Get the next constant variable
      pprogram->get_sequential_variable( j, vhandle );

      // Determine the name of the variable
      name = vhandle->get_name();

      // Determine the type to see what we need to do
      type = vhandle->get_type();

      // Determine the type of the variable
      kind = vhandle->get_kind();

      flags = vhandle->get_flags();
      if ( ( flags & SCRIPT_CONST_VAR_E ) && ( np > 0 ) ) 
      {
        continue;
      }

      ArrayMathProgramVariableHandle pvhandle;
      if ( type == "S" )
      {
        // Generate a new program variable
        pvhandle = 
          ArrayMathProgramVariableHandle( new ArrayMathProgramVariable( name, buffer ) );
        buffer += 1 * buffer_size;
      }
      else
      {
        // Generate a new program variable where the value is stored
        // This is a source or sinkl that does not need memory
        pvhandle = ArrayMathProgramVariableHandle( new ArrayMathProgramVariable( name, 0 ) );
      }

      // Add this variable to the code 
      if ( flags & SCRIPT_CONST_VAR_E )
      {
        for ( int p = 0; p < num_proc; p++ )
        {
          mprogram->set_sequential_variable( j, p, pvhandle );
        }
      }
      else
      {
        mprogram->set_sequential_variable( j, np, pvhandle );
      }
    }
  }

  // Function part
  ArrayMathProgramSource ps;

  for ( size_t j = 0; j < num_const_functions; j++ )
  {
    pprogram->get_const_function( j, fhandle );
    // Set the function pointer
    ArrayMathFunction* func = dynamic_cast< ArrayMathFunction* > ( fhandle->get_function() );
    ArrayMathProgramCode pc( func->get_function() );
    pc.set_size( 1 );
    pc.set_index( 0 );

    ParserScriptVariableHandle ohandle = fhandle->get_output_var();
    onum = ohandle->get_var_number();
    type = ohandle->get_type();
    name = ohandle->get_name();
    flags = ohandle->get_flags();

    if ( type == "S" )
    {
      if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
      {
        // These are sequenced variables and hence all multi threaded buffers
        // are equal
        pc.set_variable( 0, mprogram->get_sequential_variable( onum, 0 )->get_data() );
        pc.set_size( buffer_size );
      }
      else if ( flags & SCRIPT_SINGLE_VAR_E )
      {
        pc.set_variable( 0, mprogram->get_single_variable( onum )->get_data() );
      }
      else if ( flags & SCRIPT_CONST_VAR_E )
      {
        pc.set_variable( 0, mprogram->get_const_variable( onum )->get_data() );
      }
    }
    else if ( type == "AB" )
    {
      mprogram->find_sink( name, ps );
      if ( ps.is_bool_array() )
      {
        pc.set_bool_array( 0, ps.get_bool_array() );
      }
      else
      {
        error
            = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
        return false;
      }
    }
    else if ( type == "AD" )
    {
      mprogram->find_sink( name, ps );
      if ( ps.is_double_array() )
      {
        pc.set_double_array( 0, ps.get_double_array() );
      }
      else
      {
        error
            = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
        return false;
      }
    }
    else
    {
      error = "INTERNAL ERROR - Encountered unknown type.";
      return false;
    }

    size_t num_input_vars = fhandle->num_input_vars();
    for ( size_t i = 0; i < num_input_vars; i++ )
    {
      ParserScriptVariableHandle ihandle = fhandle->get_input_var( i );
      name = ihandle->get_name();
      inum = ihandle->get_var_number();
      type = ihandle->get_type();
      flags = ihandle->get_flags();
      if ( type == "S" )
      {
        if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
        {
          pc.set_variable( i + 1,
            mprogram->get_sequential_variable( inum, 0 )->get_data() );
        }
        else if ( flags & SCRIPT_SINGLE_VAR_E )
        {
          pc.set_variable( i + 1, mprogram->get_single_variable( inum )->get_data() );
        }
        else if ( flags & SCRIPT_CONST_VAR_E )
        {
          pc.set_variable( i + 1, mprogram->get_const_variable( inum )->get_data() );
        }

      }
      else if ( type == "AB" )
      {
        mprogram->find_source( name, ps );
        if ( ps.is_bool_array() )
        {
          pc.set_bool_array( i + 1, ps.get_bool_array() );
        }
        else
        {
          error
              = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
          return false;
        }
      }
      else if ( type == "AD" )
      {
        mprogram->find_source( name, ps );
        if ( ps.is_double_array() )
        {
          pc.set_double_array( i + 1, ps.get_double_array() );
        }
        else
        {
          error
              = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
          return false;
        }
      }
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return false;
      }
    }
    mprogram->set_const_program_code( j, pc );
  }

  for ( size_t j = 0; j < num_single_functions; j++ )
  {
    pprogram->get_single_function( j, fhandle );

    // Set the function pointer
    ArrayMathFunction* func = dynamic_cast< ArrayMathFunction* > ( fhandle->get_function() );
    ArrayMathProgramCode pc( func->get_function() );

    pc.set_size( 1 );
    pc.set_index( 0 );

    ParserScriptVariableHandle ohandle = fhandle->get_output_var();
    onum = ohandle->get_var_number();
    type = ohandle->get_type();
    name = ohandle->get_name();
    flags = ohandle->get_flags();

    if ( type == "S" )
    {
      if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
      {
        // These are sequenced variables and hence all multi threaded buffers
        // are equal
        pc.set_size( buffer_size );
        pc.set_variable( 0, mprogram->get_sequential_variable( onum, 0 )->get_data() );
      }
      else if ( flags & SCRIPT_SINGLE_VAR_E )
      {
        pc.set_variable( 0, mprogram->get_single_variable( onum )->get_data() );
      }
      else if ( flags & SCRIPT_CONST_VAR_E )
      {
        pc.set_variable( 0, mprogram->get_const_variable( onum )->get_data() );
      }

    }
    else if ( type == "AB" )
    {
      mprogram->find_sink( name, ps );
      if ( ps.is_bool_array() )
      {
        pc.set_bool_array( 0, ps.get_bool_array() );
      }
      else
      {
        error
            = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
        return false;
      }
    }
    else if ( type == "AD" )
    {
      mprogram->find_sink( name, ps );
      if ( ps.is_double_array() )
      {
        pc.set_double_array( 0, ps.get_double_array() );
      }
      else
      {
        error
            = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
        return false;
      }
    }
    else
    {
      error = "INTERNAL ERROR - Encountered unknown type.";
      return false;
    }

    size_t num_input_vars = fhandle->num_input_vars();
    for ( size_t i = 0; i < num_input_vars; i++ )
    {
      ParserScriptVariableHandle ihandle = fhandle->get_input_var( i );
      name = ihandle->get_name();
      inum = ihandle->get_var_number();
      type = ihandle->get_type();
      flags = ihandle->get_flags();
      if ( type == "S" )
      {
        if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
        {
          pc.set_variable( i + 1,
              mprogram->get_sequential_variable( inum, 0 )->get_data() );
        }
        else if ( flags & SCRIPT_SINGLE_VAR_E )
        {
          pc.set_variable( i + 1, mprogram->get_single_variable( inum )->get_data() );
        }
        else if ( flags & SCRIPT_CONST_VAR_E )
        {
          pc.set_variable( i + 1, mprogram->get_const_variable( inum )->get_data() );
        }
      }
      else if ( type == "AB" )
      {
        mprogram->find_source( name, ps );
        if ( ps.is_bool_array() )
        {
          pc.set_bool_array( i + 1, ps.get_bool_array() );
        }
        else
        {
          error
              = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
          return false;
        }
      }
      else if ( type == "AD" )
      {
        mprogram->find_source( name, ps );
        if ( ps.is_double_array() )
        {
          pc.set_double_array( i + 1, ps.get_double_array() );
        }
        else
        {
          error
              = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
          return false;
        }
      }
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return false;
      }
    }

    mprogram->set_single_program_code( j, pc );
  }

  // Process sequential list
  for ( int np = 0; np < num_proc; np++ )
  {
    for ( size_t j = 0; j < num_sequential_functions; j++ )
    {
      pprogram->get_sequential_function( j, fhandle );

      // Set the function pointer
      ArrayMathFunction* func = dynamic_cast< ArrayMathFunction* > ( fhandle->get_function() );
      ArrayMathProgramCode pc( func->get_function() );

      ParserScriptVariableHandle ohandle = fhandle->get_output_var();
      onum = ohandle->get_var_number();
      type = ohandle->get_type();
      name = ohandle->get_name();

      if ( type == "S" )
      {
        pc.set_variable( 0, mprogram->get_sequential_variable( onum, np )->get_data() );
      }
      else if ( type == "AB" )
      {
        mprogram->find_sink( name, ps );
        if ( ps.is_bool_array() )
        {
          pc.set_bool_array( 0, ps.get_bool_array() );
        }
        else
        {
          error
              = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
          return false;
        }
      }
      else if ( type == "AD" )
      {
        mprogram->find_sink( name, ps );
        if ( ps.is_double_array() )
        {
          pc.set_double_array( 0, ps.get_double_array() );
        }
        else
        {
          error
              = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
          return false;
        }
      }
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return false;
      }

      size_t num_input_vars = fhandle->num_input_vars();
      for ( size_t i = 0; i < num_input_vars; i++ )
      {
        ParserScriptVariableHandle ihandle = fhandle->get_input_var( i );
        name = ihandle->get_name();
        inum = ihandle->get_var_number();
        type = ihandle->get_type();
        flags = ihandle->get_flags();

        if ( type == "S" )
        {
          if ( flags & SCRIPT_SEQUENTIAL_VAR_E )
          {
            if ( flags & SCRIPT_CONST_VAR_E ) pc.set_variable( i + 1,
                mprogram->get_sequential_variable( inum, 0 )->get_data() );
            else pc.set_variable( i + 1,
                mprogram->get_sequential_variable( inum, np )->get_data() );
          }

          else if ( flags & SCRIPT_SINGLE_VAR_E )
          {
            pc.set_variable( i + 1, mprogram->get_single_variable( inum )->get_data() );
          }
          else if ( flags & SCRIPT_CONST_VAR_E )
          {
            pc.set_variable( i + 1, mprogram->get_const_variable( inum )->get_data() );
          }
        }
        else if ( type == "AB" )
        {
          mprogram->find_source( name, ps );
          if ( ps.is_bool_array() )
          {
            pc.set_bool_array( i + 1, ps.get_bool_array() );
          }
          else
          {
            error
                = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
            return false;
          }
        }
        else if ( type == "AD" )
        {
          mprogram->find_source( name, ps );
          if ( ps.is_double_array() )
          {
            pc.set_double_array( i + 1, ps.get_double_array() );
          }
          else
          {
            error
                = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
            return false;
          }
        }
        else
        {
          error = "INTERNAL ERROR - Encountered unknown type.";
          return false;
        }
      }
      mprogram->set_sequential_program_code( j, np, pc );
    }
  }

  return true;
}

bool ArrayMathInterpreter::run( ArrayMathProgramHandle& mprogram, std::string& error )
{
  // This does not optimally make use of the parser, in principal the
  // const part does not need to be generated after it is done once
  // Like wise the single part. For now we rerun everything every time
  ParserProgramHandle pprogram = mprogram->get_parser_program();
  ParserScriptFunctionHandle fhandle;

  // First: run the const ones
  size_t error_line;
  if ( !( mprogram->run_const( error_line ) ) )
  {
    pprogram->get_const_function( error_line, fhandle );
    error = " RUNTIME ERROR - Function '" + fhandle->get_name()
        + "' crashed for unknown reason";
    return false;
  }

  // Second: run the single ones
  if ( !( mprogram->run_single( error_line ) ) )
  {
    pprogram->get_single_function( error_line, fhandle );
    error = " RUNTIME ERROR - Function '" + fhandle->get_name()
        + "' crashed for unknown reason";
    return false;
  }

  // Third: run the sequential ones
  if ( !( mprogram->run_sequential( error_line ) ) )
  {
    pprogram->get_sequential_function( error_line, fhandle );
    error = " RUNTIME ERROR - Function '" + fhandle->get_name()
        + "' crashed for unknown reason";
    return false;
  }

  return true;
}

bool ArrayMathInterpreter::add_bool_array_source( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< bool >* array, std::string& error )
{
  if ( !( create_program( pprogram, error ) ) ) 
  {
    return false;
  }
  return pprogram->add_source( name, array );
}

bool ArrayMathInterpreter::add_double_array_source( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< double >* array, std::string& error )
{
  if ( !( create_program( pprogram, error ) ) ) 
  {
    return false;
  }
  return pprogram->add_source( name, array );
}

bool ArrayMathInterpreter::add_bool_array_sink( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< bool >* array, std::string& error )
{
  if ( !( create_program( pprogram, error ) ) ) 
  {
    return false;
  }
  return pprogram->add_sink( name, array );
}

bool ArrayMathInterpreter::add_double_array_sink( ArrayMathProgramHandle& pprogram,
    std::string& name, std::vector< double >* array, std::string& error )
{
  if ( !( create_program( pprogram, error ) ) )
  {
    return false;
  }
  return pprogram->add_sink( name, array );
}

bool ArrayMathInterpreter::set_array_size( ArrayMathProgramHandle& pprogram, size_type array_size )
{
  pprogram->set_array_size( array_size );
  return true;
}


bool ArrayMathProgram::add_source( std::string& name, std::vector< bool >* array )
{
  ArrayMathProgramSource ps;
  ps.set_bool_array( array );
  this->input_sources_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_source( std::string& name, std::vector< double >* array )
{
  ArrayMathProgramSource ps;
  ps.set_double_array( array );
  this->input_sources_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_sink( std::string& name, std::vector< bool >* array )
{
  ArrayMathProgramSource ps;
  ps.set_bool_array( array );
  this->output_sinks_[ name ] = ps;
  return true;
}

bool ArrayMathProgram::add_sink( std::string& name, std::vector< double >* array )
{
  ArrayMathProgramSource ps;
  ps.set_double_array( array );
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

int ArrayMathProgram::get_num_proc()
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

void ArrayMathProgramCode::print()
{
  std::cout << "function_ = " << this->function_ << "\n";
  for ( size_t j = 0; j < this->variables_.size(); j++ )
  {
    std::cout << "variable[" << j << "]_ = " << this->variables_[ j ] << "\n";
  }
}

ArrayMathProgramCode::ArrayMathProgramCode( bool ( *function )( ArrayMathProgramCode& pc ) ) :
  function_(function), 
  index_(0), 
  size_(1)
{

}

ArrayMathProgramCode::ArrayMathProgramCode() :
  function_(0), 
  index_(0), 
  size_(1)
{

}


double* ArrayMathProgramVariable::get_data()
{
  return this->data_;
}


void ArrayMathProgramSource::set_bool_array( std::vector< bool >* array )
{
  this->bool_array_ = array;
}


std::vector< bool >* ArrayMathProgramSource::get_bool_array()
{
  return this->bool_array_;
}


bool ArrayMathProgramSource::is_bool_array()
{
  return this->bool_array_ != 0;
}


void ArrayMathProgramSource::set_double_array( std::vector< double >* array )
{
  this->double_array_ = array;
}


std::vector< double >* ArrayMathProgramSource::get_double_array()
{
  return this->double_array_;
}


bool ArrayMathProgramSource::is_double_array()
{
  return this->double_array_ != 0;
}

} // end namespace
