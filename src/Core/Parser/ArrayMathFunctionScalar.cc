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
#include <boost/thread/mutex.hpp>

// Core includes
#include <Core/Math/MathFunctions.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Parser/ParserFWD.h>

namespace ArrayMathFunctions
{

//--------------------------------------------------------------------------
// Simple Scalar functions

bool isnan_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( Core::IsNan( *data1 ) ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
  }

  return true;
}

bool isfinite_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( Core::IsFinite( *data1 ) ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
  }

  return true;
}

bool isinfinite_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( Core::IsInfinite( *data1 ) ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
  }

  return true;
}

bool sign_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 > 0.0f ) *data0 = 1.0f;
    else if ( *data1 < 0.0f ) *data0 = -1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
  }

  return true;
}

bool ramp_sss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data3 = pc.get_variable( 3 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    float start = *data2;
    float end = *data3;
    if ( end > start )
    {
      if ( *data1 <= start ) *data0 = 0.0f;
      else if ( *data1 >= end ) *data0 = 1.0f;
      else *data0 = ( ( *data1 - start ) / ( end - start ) );
    }
    else
    {
      if ( ( *data1 ) >= start ) *data0 = 0.0f;
      else if ( ( *data1 ) <= end ) *data0 = 1.0f;
      else *data0 = ( ( start - *data1 ) / ( start - end ) );
    }
    data0++;
    data1++;
    data2++;
    data3++;
  }

  return true;
}

bool rect_sss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data3 = pc.get_variable( 3 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    float start = *data2;
    float end = *data3;
    if ( *data1 >= start && *data1 <= end ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
    data3++;
  }

  return true;
}

bool step_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    float step = *data2;
    if ( *data1 >= step ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool not_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 ) *data0 = 0.0f;
    else *data0 = 1.0f;
    data0++;
    data1++;
  }

  return true;
}

bool inv_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = 1.0f / ( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool boolean_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
  }

  return true;
}

bool abs_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 < 0.0f ) *data0 = -( *data1 );
    else *data0 = *data1;
    data0++;
    data1++;
  }

  return true;
}

bool norm_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 < 0.0f ) *data0 = -( *data1 );
    else *data0 = *data1;
    data0++;
    data1++;
  }

  return true;
}

bool round_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = static_cast< float > ( static_cast< int > ( *data1 + 0.5f ) );
    data0++;
    data1++;
  }

  return true;
}

bool floor_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::floorf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool ceil_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::ceilf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool exp_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::expf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool pow_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::powf( *data1, *data2 );
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool sqrt_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::sqrtf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool log_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::logf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool ln_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::logf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool log2_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  float s = 1.0f / ::logf( 2.0f );
  while ( data0 != data0_end )
  {
    *data0 = ::logf( *data1 ) * s;
    data0++;
    data1++;
  }

  return true;
}

bool log10_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  float s = 1.0f / logf( 10.0f );
  while ( data0 != data0_end )
  {
    *data0 = ::logf( *data1 ) * s;
    data0++;
    data1++;
  }

  return true;
}

bool cbrt_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::powf( *data1, 1.0f / 3.0f );
    data0++;
    data1++;
  }

  return true;
}

bool sin_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::sinf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool cos_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::cosf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool tan_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::tanf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool sinh_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::sinhf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool cosh_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::coshf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool asin_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::asinf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool acos_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::acosf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool atan_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::atanf( *data1 );
    data0++;
    data1++;
  }

  return true;
}

bool atan2_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ::atan2f( *data1, *data2 );
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool asinh_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    float d = *data1;
    *data0 = ( d == 0.0f ? 0.0f : ( d > 0.0f ? 1.0f : -1.0f ) ) * ::logf( ( d < 0.0f ? -d : d ) 
      + ::sqrtf( 1.f + d * d ) );
    data0++;
    data1++;
  }

  return true;
}

bool acosh_s( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    float d = *data1;
    *data0 = ::log( d + ::sqrtf( d * d - 1.0f ) );
    data0++;
    data1++;
  }

  return true;
}

bool or_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ( *data1 || *data2 );
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool and_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ( *data1 && *data2 );
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool xor_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    *data0 = ( *data1 && ( *data2 == 0.0f ) ) ||
         ( *data2 && ( *data1 == 0.0f ) );
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool eq_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 == *data2 ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool neq_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 != *data2 ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool le_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 <= *data2 ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool ge_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 >= *data2 ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool ls_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 < *data2 ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool gt_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 > *data2 ) *data0 = 1.0f;
    else *data0 = 0.0f;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool max_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 > *data2 ) *data0 = *data1;
    else *data0 = *data2;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

bool median_sss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data3 = pc.get_variable( 3 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 > *data2 )
    {
      if ( *data3 < *data2 )
      {
        *data0 = *data2;
      }
      else
      {
        if ( *data1 < *data3 ) *data0 = *data1;
        else *data0 = *data3;
      }
    }
    else
    {
      if ( *data3 < *data1 )
      {
        *data0 = *data1;
      }
      else
      {
        if ( *data2 > *data3 ) *data0 = *data3;
        else *data0 = *data2;
      }
    }
    data0++;
    data1++;
    data2++;
    data3++;
  }

  return true;
}

bool min_ss( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data1 = pc.get_variable( 1 );
  float* data2 = pc.get_variable( 2 );
  float* data0_end = data0 + pc.get_size();

  while ( data0 != data0_end )
  {
    if ( *data1 < *data2 ) *data0 = *data1;
    else *data0 = *data2;
    data0++;
    data1++;
    data2++;
  }

  return true;
}

boost::mutex RandomMutex;

bool random_value_( Core::ArrayMathProgramCode& pc )
{
  float* data0 = pc.get_variable( 0 );
  float* data0_end = data0 + pc.get_size();

  // Random is not thread safe
  RandomMutex.lock();
  while ( data0 != data0_end )
  {
    // Note: _WIN32 is always defined for applications for Win32 and Win64 
    // (see http://msdn.microsoft.com/en-us/library/b0084kay.aspx).
#ifdef _WIN32
    // random() not available in Windows stdlib
    *data0 = static_cast< float >( rand() ) / static_cast< float >( RAND_MAX + 1 );
#else
    *data0 = static_cast< float > ( random() ) / static_cast< float > ( 0x7FFFFFFF );
#endif
    data0++;
  }
  RandomMutex.unlock();

  return true;
}

} // end namespace 

namespace Core
{

void InsertScalarArrayMathFunctionCatalog( ArrayMathFunctionCatalogHandle& catalog )
{
  // Functions
  catalog->add_function( ArrayMathFunctions::isnan_s, "isnan$S", "S" );
  catalog->add_function( ArrayMathFunctions::isfinite_s, "isfinite$S", "S" );
  catalog->add_function( ArrayMathFunctions::isinfinite_s, "isinfinite$S", "S" );
  catalog->add_function( ArrayMathFunctions::isinfinite_s, "isinf$S", "S" );
  catalog->add_function( ArrayMathFunctions::sign_s, "sign$S", "S" );
  catalog->add_function( ArrayMathFunctions::ramp_sss, "ramp$S:S:S", "S" );
  catalog->add_function( ArrayMathFunctions::rect_sss, "rect$S:S:S", "S" );
  catalog->add_function( ArrayMathFunctions::step_ss, "step$S:S", "S" );

  catalog->add_function( ArrayMathFunctions::not_s, "not$S", "S" );
  catalog->add_function( ArrayMathFunctions::inv_s, "inv$S", "S" );
  catalog->add_function( ArrayMathFunctions::boolean_s, "boolean$S", "S" );
  catalog->add_function( ArrayMathFunctions::abs_s, "abs$S", "S" );
  catalog->add_function( ArrayMathFunctions::norm_s, "norm$S", "S" );
  catalog->add_function( ArrayMathFunctions::round_s, "round$S", "S" );
  catalog->add_function( ArrayMathFunctions::floor_s, "floor$S", "S" );
  catalog->add_function( ArrayMathFunctions::ceil_s, "ceil$S", "S" );
  catalog->add_function( ArrayMathFunctions::exp_s, "exp$S", "S" );
  catalog->add_function( ArrayMathFunctions::pow_ss, "pow$S:S", "S" );
  catalog->add_function( ArrayMathFunctions::sqrt_s, "sqrt$S", "S" );
  catalog->add_function( ArrayMathFunctions::log_s, "log$S", "S" );
  catalog->add_function( ArrayMathFunctions::ln_s, "ln$S", "S" );
  catalog->add_function( ArrayMathFunctions::log2_s, "log2$S", "S" );
  catalog->add_function( ArrayMathFunctions::log10_s, "log10$S", "S" );
  catalog->add_function( ArrayMathFunctions::cbrt_s, "cbrt$S", "S" );
  catalog->add_function( ArrayMathFunctions::sin_s, "sin$S", "S" );
  catalog->add_function( ArrayMathFunctions::cos_s, "cos$S", "S" );
  catalog->add_function( ArrayMathFunctions::tan_s, "tan$S", "S" );
  catalog->add_function( ArrayMathFunctions::sinh_s, "sinh$S", "S" );
  catalog->add_function( ArrayMathFunctions::cosh_s, "cosh$S", "S" );
  catalog->add_function( ArrayMathFunctions::asin_s, "asin$S", "S" );
  catalog->add_function( ArrayMathFunctions::acos_s, "acos$S", "S" );
  catalog->add_function( ArrayMathFunctions::atan_s, "atan$S", "S" );
  catalog->add_function( ArrayMathFunctions::atan2_ss, "atan2$S:S", "S" );
  catalog->add_function( ArrayMathFunctions::asinh_s, "asinh$S", "S" );
  catalog->add_function( ArrayMathFunctions::acosh_s, "acosh$S", "S" );

  catalog->add_sym_function( ArrayMathFunctions::and_ss, "and$S:S", "S" );
  catalog->add_sym_function( ArrayMathFunctions::and_ss, "bitand$S:S", "S" );
  catalog->add_sym_function( ArrayMathFunctions::xor_ss, "xor$S:S", "S" );
  catalog->add_sym_function( ArrayMathFunctions::or_ss, "or$S:S", "S" );
  catalog->add_sym_function( ArrayMathFunctions::or_ss, "bitor$S:S", "S" );
  catalog->add_sym_function( ArrayMathFunctions::eq_ss, "eq$S:S", "S" );
  catalog->add_sym_function( ArrayMathFunctions::neq_ss, "neq$S:S", "S" );
  catalog->add_function( ArrayMathFunctions::le_ss, "le$S:S", "S" );
  catalog->add_function( ArrayMathFunctions::ge_ss, "ge$S:S", "S" );
  catalog->add_function( ArrayMathFunctions::ls_ss, "ls$S:S", "S" );
  catalog->add_function( ArrayMathFunctions::gt_ss, "gt$S:S", "S" );

  catalog->add_sym_function( ArrayMathFunctions::min_ss, "min$S:S", "S" );
  catalog->add_sym_function( ArrayMathFunctions::max_ss, "max$S:S", "S" );
  catalog->add_function( ArrayMathFunctions::median_sss, "median$S:S:S", "S" );

  catalog->add_sgl_function( ArrayMathFunctions::random_value_, "rand$", "S" );
  catalog->add_seq_function( ArrayMathFunctions::random_value_, "randv$", "S" );
}

} // end namespace
