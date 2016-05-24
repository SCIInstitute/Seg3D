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

// Core includes
#include <Core/Parser/ArrayMathFunctionCatalog.h>

namespace ArrayMathFunctions
{

//--------------------------------------------------------------------------
// Add functions

// Add scalar + scalar
bool add_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = *data1 + *data2;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

//--------------------------------------------------------------------------
// Sub functions

// Sub scalar - scalar
bool sub_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = *data1 - *data2;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

//--------------------------------------------------------------------------  
// Neg functions

// Negate scalar
bool neg_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data1_end = data1 + pc.get_size();

  while ( data1 != data1_end )
  {
    *data0 = -( *data1 );
    data0++;
    data1++;
  }

  return true;
}

//--------------------------------------------------------------------------
// Mult functions

// Mult scalar * scalar
bool mult_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = *data1 * *data2;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

//--------------------------------------------------------------------------  
// Div functions

// Div scalar / scalar
bool div_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = *data1 / *data2;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

//--------------------------------------------------------------------------  
// Rem functions

// Rem scalar / scalar
bool rem_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = fmod( *data1, *data2 );
    data0++;
    data1++;
    data2++;
  }

  return true;
}

//--------------------------------------------------------------------------  
// Sequence functions (translation from arrays of size 1 into arrays of size n)

// Sequence scalar
bool seq_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  float val = data1[ 0 ];
  while ( data0 != data0_end )
  {
    *data0 = val;
    data0++;
  }

  return true;
}

//--------------------------------------------------------------------------
// Assign functions

bool assign_sss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data3 = pc.get_variable( 3 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    int idx = static_cast< int > ( *data3 + 0.5 );
    if ( idx != 0 ) return ( false );
    *data0 = *data2;
    data0++;
    data1++;
    data2++;
    data3++;
  }

  return true;
}

//--------------------------------------------------------------------------
// Subs functions

// Subs scalar scalar
bool subs_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    int idx = static_cast< int > ( *data2 + 0.5 );
    if ( idx != 0 ) return ( false );
    *data0 = *data1;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

// Select functions
bool select_sss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data3 = pc.get_variable( 3 );
  float* data1_end = data1 + pc.get_size();

  while ( data1 != data1_end )
  {
    if ( *data1 )
    {
      *data0 = *data2;
    }
    else
    {
      *data0 = *data3;
    }
    data0++;
    data1++;
    data2++;
    data3++;
  }

  return true;
}

} // end namsespace 

namespace Core
{

void InsertBasicArrayMathFunctionCatalog( ArrayMathFunctionCatalogHandle& catalog )
{
  // Add add functions to database
  catalog->add_sym_function( ArrayMathFunctions::add_ss, "add$S:S", "S" );

  // Add sub functions to database
  catalog->add_function( ArrayMathFunctions::sub_ss, "sub$S:S", "S" );

  // Add neg function to database
  catalog->add_function( ArrayMathFunctions::neg_s, "neg$S", "S" );

  // Add mult functions to database
  catalog->add_sym_function( ArrayMathFunctions::mult_ss, "mult$S:S", "S" );

  // Add div functions to database
  catalog->add_function( ArrayMathFunctions::div_ss, "div$S:S", "S" );

  // Add rem functions to database
  catalog->add_function( ArrayMathFunctions::rem_ss, "rem$S:S", "S" );

  // Add assign functions to database
  catalog->add_function( ArrayMathFunctions::assign_sss, "assign$S:S:S", "S" );

  // Add subs functions to database
  catalog->add_function( ArrayMathFunctions::subs_ss, "subs$S:S", "S" );

  // Add sequencer code: translate from single to sequence of data
  catalog->add_function( ArrayMathFunctions::seq_s, "seq$S", "S" );

  catalog->add_function( ArrayMathFunctions::select_sss, "select$S:S:S", "S" );
}

} // end namespace
