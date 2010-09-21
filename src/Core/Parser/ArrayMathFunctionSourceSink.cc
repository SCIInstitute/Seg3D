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
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>

namespace ArrayMathFunctions
{

//--------------------------------------------------------------------------
// Source functions


bool get_scalar_ab( Core::ArrayMathProgramCode& pc )
{
  double* data0 = pc.get_variable( 0 );

  // We store the matrix pointer so we can get other properties as well
  std::vector< bool >& array( *( pc.get_bool_array( 1 ) ) );
  // One virtual call to get the data

  double* data0_end = data0 + pc.get_size();
  Core::index_type idx = pc.get_index();

  while ( data0 != data0_end )
  {
    *data0 = static_cast< double > ( array[ idx ] );
    data0++;
    idx++;
  }

  return true;
}

bool get_scalar_ad( Core::ArrayMathProgramCode& pc )
{
  double* data0 = pc.get_variable( 0 );

  // We store the matrix pointer so we can get other properties as well
  std::vector< double >& array( *( pc.get_double_array( 1 ) ) );
  // One virtual call to get the data

  double* data0_end = data0 + pc.get_size();
  Core::index_type idx = pc.get_index();

  while ( data0 != data0_end )
  {
    *data0 = static_cast< double > ( array[ idx ] );
    data0++;
    idx++;
  }

  return true;
}

bool get_scalar_data( Core::ArrayMathProgramCode& pc )
{
  // Destination 
  double* data0 = pc.get_variable( 0 );

  // Source
  Core::DataBlock& data1( *( pc.get_data_block( 1 ) ) );
  // One virtual call to get the data

  double* data0_end = data0 + pc.get_size();
  Core::index_type idx = pc.get_index();

  double val;
  while( data0 != data0_end ) 
  {
    val = data1.get_data_at( idx );
    idx++;

    *data0 = val; 
    data0++;
  }

  return true;
}

bool get_scalar_mask( Core::ArrayMathProgramCode& pc )
{
  // Destination 
  double* data0 = pc.get_variable( 0 );

  // Source
  Core::MaskDataBlock& data1( *( pc.get_mask_data_block( 1 ) ) );
  // One virtual call to get the data

  double* data0_end = data0 + pc.get_size();
  Core::index_type idx = pc.get_index();

  // Get value (on/off) from mask
  double val;
  while( data0 != data0_end ) 
  {
    val = data1.get_mask_at( idx ) ? 1 : 0;
    idx++;

    *data0 = val; 
    data0++;
  }

  return true;
}

//--------------------------------------------------------------------------
// Sink functions

bool to_bool_array_s( Core::ArrayMathProgramCode& pc )
{
  // Get the pointer to the matrix object where we need to store the data
  std::vector< bool >& array( *( pc.get_bool_array( 0 ) ) );
  double* data1 = pc.get_variable( 1 );

  double* data1_end = data1 + ( pc.get_size() );
  Core::index_type idx = pc.get_index();

  while ( data1 != data1_end )
  {
    array[ idx ] = ( *data1 != 0 );
    idx++;
    data1++;
  }

  return true;
}

bool to_double_array_s( Core::ArrayMathProgramCode& pc )
{
  // Get the pointer to the matrix object where we need to store the data
  std::vector< double >& array( *( pc.get_double_array( 0 ) ) );
  double* data1 = pc.get_variable( 1 );

  double* data1_end = data1 + ( pc.get_size() );
  Core::index_type idx = pc.get_index();

  while ( data1 != data1_end )
  {
    array[ idx ] = *data1;
    idx++;
    data1++;
  }

  return true;
}

bool to_data_block_s( Core::ArrayMathProgramCode& pc )
{
  // Get the pointer to the DataBlock object where we need to store the data
  Core::DataBlock& data0( *( pc.get_data_block( 0 ) ) );
  double* data1 = pc.get_variable( 1 );

  double* data1_end = data1 + ( pc.get_size() );
  Core::index_type idx = pc.get_index();

  while ( data1 != data1_end ) 
  {
    data0.set_data_at( idx, *data1 ); 
    idx++; 
    data1++;
  }

  return true;
}

bool to_mask_data_block_s( Core::ArrayMathProgramCode& pc )
{
  // Get the pointer to the DataBlock object where we need to store the data
  Core::MaskDataBlock& data0( *( pc.get_mask_data_block( 0 ) ) );
  double* data1 = pc.get_variable( 1 );

  double* data1_end = data1 + ( pc.get_size() );
  Core::index_type idx = pc.get_index();

  // Copy result to mask
  while ( data1 != data1_end ) 
  {
    if( *data1 )
    {
      data0.set_mask_at( idx );
    }
    else
    {
      data0.clear_mask_at( idx );
    }
    idx++; 
    data1++;
  }

  return true;
}

} //end namespace

namespace Core
{

void InsertSourceSinkArrayMathFunctionCatalog( ArrayMathFunctionCatalogHandle& catalog )
{
  // Source functions
  catalog->add_function( ArrayMathFunctions::get_scalar_ad, "get_scalar$AD", "S" );
  catalog->add_function( ArrayMathFunctions::get_scalar_ab, "get_scalar$AB", "S" );
  catalog->add_function( ArrayMathFunctions::get_scalar_data, "get_scalar$DATA", "S" );
  catalog->add_function( ArrayMathFunctions::get_scalar_mask, "get_scalar$MASK", "S" );

  // Sink functions
  catalog->add_function( ArrayMathFunctions::to_bool_array_s, "to_bool_array$S", "AB" );
  catalog->add_function( ArrayMathFunctions::to_double_array_s, "to_double_array$S", "AD" );
  catalog->add_function( ArrayMathFunctions::to_data_block_s, "to_data_block$S", "DATA" );
  catalog->add_function( ArrayMathFunctions::to_mask_data_block_s, "to_mask_data_block$S", "MASK" );
}

} // end namespace
