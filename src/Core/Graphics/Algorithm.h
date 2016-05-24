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

#ifndef CORE_GRAPHICS_ALGORITHM_H
#define CORE_GRAPHICS_ALGORITHM_H

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <vector>
#include <utility>

namespace Core
{

// SETPIXEL:
// Set the pixel at (x, y) of the buffer to the given value. 
template< class T >
void SetPixel( T* buffer, const int width, const int x, const int y, const T value )
{
  buffer[ y * width + x ] = value;
}

// FLOODFILL:
// Flood fill the buffer with the given value, starting from (x, y). The algorithm stops
// when it gets to the boundary of the buffer, or when it arrives at a point where the
// value equals to the given value.
template< class T >
void FloodFill( T* buffer, const int width, const int height, 
         const int x, const int y, const T value )
{
  std::stack< std::pair< int, int > > seed_points;
  seed_points.push( std::make_pair( x, y ) );

  while ( !seed_points.empty() )
  {
    std::pair< int, int > seed = seed_points.top();
    seed_points.pop();

    bool span_up = false;
    bool span_down = false;
    int x0 = seed.first;
    int y0 = seed.second;
    while ( x0 >= 0 && buffer[ y0 * width + x0 ] != value ) --x0;
    ++x0;
    // in case the seed point doesn't meet the criteria for filling
    x0 = std::min( x0, seed.first );  

    while ( x0 < width && buffer[ y0 * width + x0 ] != value )
    {
      SetPixel( buffer, width, x0, y0, value );

      if ( !span_down && y0 > 0 && 
        buffer[ ( y0 - 1 ) * width + x0 ] != value )
      {
        seed_points.push( std::make_pair( x0, y0 - 1 ) );
        span_down = true;
      }
      else if ( span_down && y0 > 0 &&
        buffer[ ( y0 - 1 ) * width + x0 ] == value )
      {
        span_down = false;
      }

      if ( !span_up && y0 < height - 1 &&
        buffer[ ( y0 + 1 ) * width + x0 ] != value )
      {
        seed_points.push( std::make_pair( x0, y0 + 1 ) );
        span_up = true;
      }
      else if ( span_up && y0 < height - 1 &&
        buffer[ ( y0 + 1 ) * width + x0 ] == value )
      {
        span_up = false;
      }

      x0++;
    }
  }
}

// FLOODFILL:
// Flood fill the buffer with the given value, starting from (x, y). The algorithm stops
// when it gets to the boundary of the buffer, or when it arrives at a point where the
// value equals to the given value, or when the condition evaluates to false.
template< class T, class FUNCTOR >
void FloodFill( T* buffer, const int width, const int height, 
         const int x, const int y, const T value, FUNCTOR condition )
{
  std::stack< std::pair< int, int > > seed_points;
  seed_points.push( std::make_pair( x, y ) );

  while ( !seed_points.empty() )
  {
    std::pair< int, int > seed = seed_points.top();
    seed_points.pop();

    bool span_up = false;
    bool span_down = false;
    int x0 = seed.first;
    int y0 = seed.second;
    while ( x0 >= 0 && buffer[ y0 * width + x0 ] != value && condition( x0, y0 ) ) --x0;
    ++x0;
    // in case the seed point doesn't meet the criteria for filling
    x0 = std::min( x0, seed.first );

    while ( x0 < width && buffer[ y0 * width + x0 ] != value && condition( x0, y0 ) )
    {
      SetPixel( buffer, width, x0, y0, value );

      if ( !span_down && y0 > 0 && 
        buffer[ ( y0 - 1 ) * width + x0 ] != value &&
        condition( x0, y0 -1 ) )
      {
        seed_points.push( std::make_pair( x0, y0 - 1 ) );
        span_down = true;
      }
      else if ( span_down && y0 > 0 &&
        ( buffer[ ( y0 - 1 ) * width + x0 ] == value ||
        !condition( x0, y0 -1 ) ) )
      {
        span_down = false;
      }

      if ( !span_up && y0 < height - 1 &&
        buffer[ ( y0 + 1 ) * width + x0 ] != value &&
        condition( x0, y0 + 1 ) )
      {
        seed_points.push( std::make_pair( x0, y0 + 1 ) );
        span_up = true;
      }
      else if ( span_up && y0 < height - 1 &&
        ( buffer[ ( y0 + 1 ) * width + x0 ] == value ||
        !condition( x0, y0 +1 ) ) )
      {
        span_up = false;
      }

      x0++;
    }
  }
}

} // end namespace Core

#endif
