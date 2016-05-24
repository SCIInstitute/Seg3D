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

// STL includes
#include <iostream>

// Boost includes
#include <boost/thread.hpp>

// Core includes
#include <Core/Parser/ArrayMathFunction.h>
#include <Core/Parser/ArrayMathInterpreter.h>
#include <Core/Parser/ArrayMathProgram.h>
#include <Core/Parser/ArrayMathProgramVariable.h>
#include <Core/Parser/ParserEnums.h>
#include <Core/Parser/ParserProgram.h>
#include <Core/Parser/ParserScriptFunction.h>
#include <Core/Parser/ParserScriptVariable.h>

namespace Core
{

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
    else if ( type == "DATA" || type == "MASK" ) 
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
  size_type buffer_size = mprogram->get_buffer_size();
  int num_threads = mprogram->get_num_threads();

  for ( int nt = 0; nt < num_threads; nt++ )
  {
    for ( size_t j = 0; j < num_sequential_variables; j++ )
    {
      pprogram->get_sequential_variable( j, vhandle );
      std::string type = vhandle->get_type();
      flags = vhandle->get_flags();

      if ( ( flags & SCRIPT_CONST_VAR_E ) && ( nt > 0 ) )
      {
        continue;
      }
      if ( type == "S" )
      {
        buffer_mem += 1 * buffer_size;
      }
      else if ( type == "DATA" || type == "MASK" )
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
  float* buffer = mprogram->create_buffer( buffer_mem );

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
        float val = vhandle->get_scalar_value();
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

  for ( int nt = 0; nt < num_threads; nt++ )
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
      if ( ( flags & SCRIPT_CONST_VAR_E ) && ( nt > 0 ) ) 
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
        for ( int p = 0; p < num_threads; p++ )
        {
          mprogram->set_sequential_variable( j, p, pvhandle );
        }
      }
      else
      {
        mprogram->set_sequential_variable( j, nt, pvhandle );
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
      else if ( type == "DATA" )
      {
        mprogram->find_source( name, ps );
        if ( ps.is_data_block() )
        {
          pc.set_data_block( i + 1, ps.get_data_block() );
        }
        else
        {
          error
            = "INTERNAL ERROR - Variable is of DataBlock type, but given source is not a DataBlock.";
          return false;
        }
      }
      else if ( type == "MASK" )
      {
        mprogram->find_source( name, ps );
        if ( ps.is_mask_data_block() )
        {
          pc.set_mask_data_block( i + 1, ps.get_mask_data_block() );
        }
        else
        {
          error
            = "INTERNAL ERROR - Variable is of MaskDataBlock type, but given source is not a MaskDataBlock.";
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
    else if ( type == "DATA" )
    {
      mprogram->find_sink( name, ps );
      if ( ps.is_data_block() )
      {
        pc.set_data_block( 0, ps.get_data_block() );
      }
      else
      {
        error
          = "INTERNAL ERROR - Variable is of DataBlock type, but given sink is not a DataBlock.";
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
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return false;
      }
    }

    mprogram->set_single_program_code( j, pc );
  }

  // Process sequential list
  for ( int nt = 0; nt < num_threads; nt++ )
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
        pc.set_variable( 0, mprogram->get_sequential_variable( onum, nt )->get_data() );
      }
      else if ( type == "DATA" )
      {
        mprogram->find_sink( name, ps );
        if ( ps.is_data_block() )
        {
          pc.set_data_block( 0, ps.get_data_block() );
        }
        else
        {
          error
            = "INTERNAL ERROR - Variable is of DataBlock type, but given source is not a DataBlock.";
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
                mprogram->get_sequential_variable( inum, nt )->get_data() );
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
        else if ( type == "DATA" )
        {
          mprogram->find_source( name, ps );
          if ( ps.is_data_block() )
          {
            pc.set_data_block( i + 1, ps.get_data_block() );
          }
          else
          {
            error
              = "INTERNAL ERROR - Variable is of DataBlock type, but given source is not a DataBlock.";
            return false;
          }
        }
        else if ( type == "MASK" )
        {
          mprogram->find_source( name, ps );
          if ( ps.is_mask_data_block() )
          {
            pc.set_mask_data_block( i + 1, ps.get_mask_data_block() );
          }
          else
          {
            error
              = "INTERNAL ERROR - Variable is of MaskDataBlock type, but given source is not a MaskDataBlock.";
            return false;
          }
        }
        else
        {
          error = "INTERNAL ERROR - Encountered unknown type.";
          return false;
        }
      }
      mprogram->set_sequential_program_code( j, nt, pc );
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

bool ArrayMathInterpreter::add_data_block_source( ArrayMathProgramHandle& pprogram, 
  std::string& name, DataBlockHandle data_block, std::string& error )
{
  if ( !( create_program( pprogram, error ) ) ) 
  {
    return false;
  }
  return pprogram->add_source( name, data_block );
}

bool ArrayMathInterpreter::add_mask_data_block_source( ArrayMathProgramHandle& pprogram, 
  std::string& name, MaskDataBlockHandle mask_data_block, std::string& error )
{
  if ( !( create_program( pprogram, error ) ) ) 
  {
    return false;
  }
  return pprogram->add_source( name, mask_data_block );
}

bool ArrayMathInterpreter::add_data_block_sink( ArrayMathProgramHandle& pprogram, 
  std::string& name, DataBlockHandle data_block, std::string& error )
{
  if ( !( create_program( pprogram, error ) ) )
  {
    return false;
  }
  return pprogram->add_sink( name, data_block );
}

bool ArrayMathInterpreter::set_array_size( ArrayMathProgramHandle& pprogram, size_type array_size )
{
  pprogram->set_array_size( array_size );
  return true;
}

} // end namespace
