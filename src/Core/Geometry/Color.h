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

#ifndef CORE_GEOMETRY_COLOR_H
#define CORE_GEOMETRY_COLOR_H

// STL includes
#include <string>

namespace Core
{

// CLASS Color
/// This class defines an rgb color

class Color
{

  // -- constructor/destructor --
public:
  Color()
  {
    this->data_[ 0 ] = 0.0f;
    this->data_[ 1 ] = 0.0f;
    this->data_[ 2 ] = 0.0f;
  }

  Color( float r, float g, float b )
  {
    this->data_[ 0 ] = r;
    this->data_[ 1 ] = g;
    this->data_[ 2 ] = b;
  }

  Color( const Color& color )
  {
    this->data_[ 0 ] = color.data_[ 0 ];
    this->data_[ 1 ] = color.data_[ 1 ];
    this->data_[ 2 ] = color.data_[ 2 ];
  }

  ~Color()
  {
  }

  Color& operator=( const Color& color )
  {
    this->data_[ 0 ] = color.data_[ 0 ];
    this->data_[ 1 ] = color.data_[ 1 ];
    this->data_[ 2 ] = color.data_[ 2 ];

    return ( *this );
  }

  inline Color operator*( float alpha ) const
  {
    return Color( this->data_[ 0 ] * alpha, this->data_[ 1 ] * alpha, this->data_[ 2 ] * alpha );
  }

  Color operator+( const Color& rhs ) const
  {
    return Color( this->data_[ 0 ] + rhs[ 0 ], this->data_[ 1 ] + rhs[ 1 ], this->data_[ 2 ] + rhs[ 2 ] );
  }

  const Color& operator+=( const Color& rhs )
  {
    this->data_[ 0 ] += rhs.data_[ 0 ];
    this->data_[ 1 ] += rhs.data_[ 1 ];
    this->data_[ 2 ] += rhs.data_[ 2 ];
    return *this;
  }

  bool operator==( const Color& color ) const
  {
    return ( ( this->data_[ 0 ] == color.data_[ 0 ] ) && 
      ( this->data_[ 1 ] == color.data_[ 1 ] ) && 
      ( this->data_[ 2 ] == color.data_[ 2 ] ) );
  }

  bool operator!=( const Color& color ) const
  {
    return ( ( this->data_[ 0 ] != color.data_[ 0 ] ) || 
      ( this->data_[ 1 ] != color.data_[ 1 ] ) || 
      ( this->data_[ 2 ] != color.data_[ 2 ] ) );
  }

  inline const float r() const
  {
    return this->data_[ 0 ];
  }
  inline const float g() const
  {
    return this->data_[ 1 ];
  }
  inline const float b() const
  {
    return this->data_[ 2 ];
  }

  inline void r( const float r )
  {
    this->data_[ 0 ] = r;
  }
  inline void g( const float g )
  {
    this->data_[ 1 ] = g;
  }
  inline void b( const float b )
  {
    this->data_[ 2 ] = b;
  }

  const float operator[]( size_t idx ) const
  {
    return this->data_[ idx ];
  }

  float& operator[]( size_t idx )
  {
    return this->data_[ idx ];
  }

private:
  // red, green, blue
  float data_[ 3 ];
};

Color operator*( float alpha, Color color );

std::string ExportToString( const Color& value );
bool ImportFromString( const std::string& str, Color& value );

} // End namespace Core

#endif
