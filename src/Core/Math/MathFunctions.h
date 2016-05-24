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

#ifndef CORE_MATH_MATHFUNCTIONS_H
#define CORE_MATH_MATHFUNCTIONS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Deal with some Windows macros that shouldn't be defined. However some windows
// include files define an all lower case min and max function.
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <cstdlib>

// C includes
#include <math.h>

// STL includes
#include <limits>

// This header file contains short cuts to many math functions

// Note: To distinguish these functions from the default C functions they all
// start with an upper case letter instead of a lower case one.

// Note: Some functions like Sqrt and Pow are only defined for double or float,
// this forces the user to make an explicit choice for the precision by casting
// other inputs into this specific format. This is done to *protect* the user
// from nasty unintended implicit casts and to ensure that numeric results are
// accurate. This may seem a nuisance, but is need to guarantee a certain code
// quality.


namespace Core
{

template< class T >
inline T Min( T d1, T d2 )
{
  return d1 < d2 ? d1 : d2;
}

template< class T >
inline T Max( T d1, T d2 )
{
  return d1 > d2 ? d1 : d2;
}

template< class T >
inline T Min( T d1, T d2, T d3 )
{
  T d4 = d1 < d2 ? d1 : d2;
  d4 = d4 < d3 ? d4 : d3;
  return ( d4 );
}

template< class T >
inline T Mid( T d1, T d2, T d3 )
{
  return ( ( d1 > d2 ) ? ( ( d1 < d3 ) ? d1 : ( ( d2 > d3 ) ? d2 : d3 ) ) : ( ( d2 < d3 ) ? d2
      : ( ( d1 > d3 ) ? d1 : d3 ) ) );
}

template< class T >
inline T Max( T d1, T d2, T d3 )
{
  T d4 = d1 > d2 ? d1 : d2;
  d4 = d4 > d3 ? d4 : d3;
  return d4;
}

//------------------------------------------
// Pow, Sqrt, Cbrt, Sqr, and Exp functions
// Note: Pow, Sqrt, Exp, and Cbrt need double or float inputs, forcing the code 
// to be explicit about the precision needed for the computation.

inline double Pow( double value, double power )
{
  return pow( value, power );
}

inline double Pow( double value, int power )
{
  return pow( value, static_cast< double > ( power ) );
}

inline float Pow( float value, float power )
{
  return powf( value, power );
}

inline float Pow( float value, int power )
{
  return powf( value, static_cast< float > ( power ) );
}

inline double Sqrt( double value )
{
  return sqrt( value );
}

inline float Sqrt( float value )
{
  return sqrtf( value );
}

inline double Cbrt( double value )
{
  return ( pow( value, 1.0 / 3.0 ) );
}

inline float Cbrt( float value )
{
  return powf( value, 1.0f / 3.0f );
}

template< class T >
inline T Sqr( T value )
{
  return value * value;
}

inline double Exp( double value )
{
  return exp( value );
}

inline float Exp( float value )
{
  return expf( value );
}

//------------------------------------------
// Absolute function

template< class T >
inline T Abs( T value )
{
  return ( value < T( 0 ) ) ? -value : value;
}

//-----------------------------------------
// Sign function

template< class T >
inline int Sign( T value )
{
  if ( value == T( 0 ) ) return 0;
  return ( value < T( 0 ) ) ? -1 : 1;
}

//-----------------------------------------
// Clamp function

template< class T >
inline T Clamp( T value, T min, T max )
{
  return value <= min ? min : value >= max ? max : value;
}

//-----------------------------------------
// Interpolation function

template< class T, class G >
inline T Interpolate( T value1, T value2, G weight )
{
  return T( value2 * weight + value1 * ( G( 1 ) - weight ) );
}

//-----------------------------------------
// Extract fraction from a double or float

template< class T >
inline T Fraction( T value )
{
  return static_cast< T > ( value - static_cast< int > ( value ) );
}

//-----------------------------------------
// Round, RoundDown, Round, Ceil, Floor

template< class T >
inline int RoundDown( T value )
{
  if ( value >= T( 0 ) )
  {
    return static_cast< int > ( value );
  }
  else
  {
    if ( value == static_cast< int > ( value ) )
    {
      return -( static_cast< int > ( -value ) );
    }
    else
    {
      return -( static_cast< int > ( ( -value ) - T( 1 ) ) );
    }
  }
}

template< class T >
inline int RoundUp( T value )
{
  if ( value >= T( 0 ) )
  {
    if ( ( value - static_cast< int > ( value ) ) == T( 0 ) )
    {
      return static_cast< int > ( value );
    }
    else
    {
      return static_cast< int > ( value + T( 1 ) );
    }
  }
  else
  {
    return static_cast< int > ( value );
  }
}

template< class T >
inline int Round( T value )
{
  if ( value >= T( 0 ) )
  {
    return static_cast< int > ( value + T( 0.5 ) );
  }
  else
  {
    return static_cast< int >( value - T( 0.5 ) );
  }
}

template< class T >
inline int Floor( T value )
{
  if ( value < T( 0 ) )
  {
    int i = -static_cast< int > ( -value );
    if ( static_cast< T > ( i ) == value ) return i;
    else return i - 1;
  }
  else
  {
    return ( static_cast< int > ( value ) );
  }
}

template< class T >
inline int Ceil( T value )
{
  if ( value < T( 0 ) )
  {
    return ( -static_cast< int > ( -value ) );
  }
  else
  {
    int i = static_cast< int > ( value );
    if ( static_cast< double > ( i ) == value ) return ( i );
    else return ( i + 1 );
  }
}

//-----------------------------------------
// Double and float special types

inline bool IsNan( double val )
{
  // NOTE: Special case, IEEE says that this is true for a NaN value
  return ( val != val );
}

inline bool IsInfinite( double val )
{
  return ( val == std::numeric_limits< double >::infinity() || val == -std::numeric_limits<
      double >::infinity() );
}

inline bool IsFinite( double val )
{
  return ( ( !IsNan(val) ) && ( !IsInfinite(val) ) );
}

inline double Nan()
{
  return ( std::numeric_limits< double >::quiet_NaN() );
}

inline double Epsilon()
{
  return ( std::numeric_limits< double >::epsilon() );
}

inline bool IsNan( float val )
{
  return ( val != val );
}

inline bool IsInfinite( float val )
{
  return ( val == std::numeric_limits< float >::infinity() || val
      == -std::numeric_limits< float >::infinity() );
}

inline bool IsFinite( float val )
{
  return ( ( !IsNan(val) ) && ( !IsInfinite(val) ) );
}


inline float Nanf()
{
  return ( std::numeric_limits< float >::quiet_NaN() );
}

inline float Epsilonf()
{
  return ( std::numeric_limits< float >::epsilon() );
}

//-----------------------------------------------
// Functions for calculating nearest power of 2

inline bool IsPowerOf2( size_t value )
{
  return ( ( value & ( value - 1 ) ) == 0 );
}

// Returns a number Greater Than or Equal to value that is an exact power of 2
inline size_t NextPowerOf2( size_t value )
{
  if ( IsPowerOf2( value ) ) return value;
  size_t new_value = 2;
  while ( new_value < value )
    new_value = new_value << 1;
  return new_value;
}

// Returns a number Less Than or Equal to value that is an exact power of 2
inline size_t LargestPowerOf2( const size_t value )
{
  if ( IsPowerOf2( value ) ) return value;
  return NextPowerOf2( value ) >> 1;
}

//--------------------------------
// Trigeometry functions

inline double Pi()
{
  return 3.14159265358979323846;
}

inline float Pif()
{
  return 3.14159265358979323846f;
}

inline double Cos( double value )
{
  return cos( value );
}

inline double Sin( double value )
{
  return sin( value );
}

inline double Asin( double value )
{
  return asin( value );
}

inline double Acos( double value )
{
  return acos( value );
}

inline double Tan( double value )
{
  return tan( value );
}

inline double Cot( double value )
{
  return 1.0 / tan( value );
}

inline double Atan( double value )
{
  return atan( value );
}

inline double Atan2( double y, double x )
{
  return atan2( y, x );
}

inline double DegreeToRadian( double degree )
{
  return degree * ( Pi() / 180.0 );
}

inline double RadianToDegree( double radian )
{
  return radian * ( 180.0 / Pi() );
}

inline float Sin( float value )
{
  return sinf( value );
}

inline float Cos( float value )
{
  return cosf( value );
}

inline float Tan( float value )
{
  return tanf( value );
}

inline float Cot( float value )
{
  return 1.0f / tanf( value );
}

inline float Atan( float value )
{
  return atanf( value );
}

inline float Atan2( float y, float x )
{
  return atan2f( y, x );
}

inline float Acos( float value )
{
  return acosf( value );
}

inline float DegreeToRadian( float degree )
{
  return degree * ( Pif() / 180.0f );
}

inline float RadianToDegree( float radian )
{
  return radian * ( 180.0f / Pif() );
}

inline size_t RemoveRemainder8( size_t size )
{
  if ( sizeof (size_t) == 8 )
  {
    return size & ~(0x7ULL);
  }
  else
  {
    return size & ~(0x7UL);
  }
}

} // End namespace Core

#endif
