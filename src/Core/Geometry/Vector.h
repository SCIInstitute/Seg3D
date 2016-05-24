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

#ifndef CORE_GEOMETRY_VECTOR_H
#define CORE_GEOMETRY_VECTOR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <vector>
#include <iosfwd>

// Core includes
#include <Core/Utils/StringUtil.h>
#include <Core/Math/MathFunctions.h>

namespace Core
{

// Notes: see Point.h for notes

class Vector;
class Point;
class VectorF;
class PointF;

// TODO: explore templates with type restrictions

class Vector
{

private:
  friend class Point;
  friend class PointF;
  friend class VectorF;
  friend class Transform;
  friend class TransformT;

  double data_[ 3 ];

public:
  inline Vector();
  inline Vector( double, double, double );

  inline explicit Vector( const PointF& );
  inline explicit Vector( const Point& );
  inline explicit Vector( const VectorF& );
  inline Vector( const Vector& );

  bool operator==( const Vector& ) const;
  bool operator!=( const Vector& ) const;
  bool operator==( const VectorF& ) const;
  bool operator!=( const VectorF& ) const;

  inline double length() const;
  inline double length2() const;

  friend inline double Dot( const Vector&, const Vector& );
  friend inline double Dot( const Point&, const Vector& );
  friend inline double Dot( const Vector&, const Point& );

  inline Vector& operator=( const Vector& );
  inline Vector& operator=( const VectorF& );

  inline double& operator[]( size_t idx );
  inline double operator[]( size_t idx ) const;

  inline Vector operator*( const double ) const;
  inline Vector& operator*=( const double );
  inline Vector operator/( const double ) const;
  inline Vector& operator/=( const double );
  inline Vector operator+( const Vector& ) const;
  inline Vector operator+( const VectorF& ) const;
  inline Vector& operator+=( const Vector& );
  inline Vector& operator+=( const VectorF& );
  inline Vector operator-() const;
  inline Vector operator-( const Vector& ) const;
  inline Vector operator-( const VectorF& ) const;
  inline Vector operator-( const Point& ) const;
  inline Vector operator-( const PointF& ) const;
  inline Vector& operator-=( const Vector& );
  inline Vector& operator-=( const VectorF& );

  inline double normalize();
  inline Vector normal() const;

  friend inline Vector Cross( const Vector&, const Vector& );
  friend inline Vector Abs( const Vector& );

  inline void x( double );
  inline double x() const;
  inline void y( double );
  inline double y() const;
  inline void z( double );
  inline double z() const;

  bool find_orthogonal( Vector&, Vector& ) const;

  friend inline Vector Interpolate( const Vector&, const Vector&, double );

  friend std::ostream& operator<<( std::ostream& os, const Vector& p );
};

class VectorF
{

private:
  friend class Point;
  friend class PointF;
  friend class Vector;
  friend class Transform;
  friend class TransformT;

  float data_[ 3 ];

public:
  inline VectorF();
  inline VectorF( float, float, float );

  inline explicit VectorF( const PointF& );
  inline explicit VectorF( const Point& );
  inline explicit VectorF( const Vector& );
  inline VectorF( const VectorF& );

  bool operator==( const Vector& ) const;
  bool operator!=( const Vector& ) const;
  bool operator==( const VectorF& ) const;
  bool operator!=( const VectorF& ) const;

  inline float length() const;
  inline float length2() const;

  friend inline float Dot( const VectorF&, const VectorF& );
  friend inline float Dot( const PointF&, const VectorF& );
  friend inline float Dot( const VectorF&, const PointF& );

  inline VectorF& operator=( const Vector& );
  inline VectorF& operator=( const VectorF& );

  inline float& operator[]( size_t idx );
  inline float operator[]( size_t idx ) const;

  inline VectorF operator*( const float ) const;
  inline VectorF& operator*=( const float );
  inline VectorF operator/( const float ) const;
  inline VectorF& operator/=( const float );
  inline Vector operator+( const Vector& ) const;
  inline VectorF operator+( const VectorF& ) const;
  inline VectorF& operator+=( const Vector& );
  inline VectorF& operator+=( const VectorF& );
  inline VectorF operator-() const;
  inline Vector operator-( const Vector& ) const;
  inline VectorF operator-( const VectorF& ) const;
  inline Vector operator-( const Point& ) const;
  inline VectorF operator-( const PointF& ) const;
  inline VectorF& operator-=( const Vector& );
  inline VectorF& operator-=( const VectorF& );

  inline float normalize();
  inline VectorF normal() const;

  friend inline VectorF Cross( const VectorF&, const VectorF& );
  friend inline VectorF Abs( const VectorF& );

  inline void x( float );
  inline float x() const;
  inline void y( float );
  inline float y() const;
  inline void z( float );
  inline float z() const;

  bool find_orthogonal( VectorF&, VectorF& ) const;

  friend inline VectorF Interpolate( const VectorF&, const VectorF&, float );

  friend std::ostream& operator<<( std::ostream& os, const VectorF& p );
};

std::ostream& operator<<( std::ostream& os, const Vector& p );
std::ostream& operator<<( std::ostream& os, const VectorF& p );

std::string ExportToString( const Vector& value );
std::string ExportToString( const VectorF& value );

std::string ExportToString( const std::vector< Vector >& value );
std::string ExportToString( const std::vector< VectorF >& value );

bool ImportFromString( const std::string& str, Vector& value );
bool ImportFromString( const std::string& str, VectorF& value );

bool ImportFromString( const std::string& str, std::vector< Vector >& value );
bool ImportFromString( const std::string& str, std::vector< VectorF >& value );

} // End namespace Core

// Needs to be included here to avoid circular dependencies
#include <Core/Geometry/Point.h>

namespace Core
{

inline Vector::Vector()
{
  data_[ 0 ] = 0.0;
  data_[ 1 ] = 0.0;
  data_[ 2 ] = 0.0;
}

inline VectorF::VectorF()
{
  data_[ 0 ] = 0.0f;
  data_[ 1 ] = 0.0f;
  data_[ 2 ] = 0.0f;
}

inline Vector::Vector( double x, double y, double z )
{
  data_[ 0 ] = x;
  data_[ 1 ] = y;
  data_[ 2 ] = z;
}

inline VectorF::VectorF( float x, float y, float z )
{
  data_[ 0 ] = x;
  data_[ 1 ] = y;
  data_[ 2 ] = z;
}

inline Vector::Vector( const Vector& p )
{
  data_[ 0 ] = p.data_[ 0 ];
  data_[ 1 ] = p.data_[ 1 ];
  data_[ 2 ] = p.data_[ 2 ];
}

inline Vector::Vector( const Point& p )
{
  data_[ 0 ] = p.data_[ 0 ];
  data_[ 1 ] = p.data_[ 1 ];
  data_[ 2 ] = p.data_[ 2 ];
}

inline Vector::Vector( const PointF& p )
{
  data_[ 0 ] = static_cast< double > ( p.data_[ 0 ] );
  data_[ 1 ] = static_cast< double > ( p.data_[ 1 ] );
  data_[ 2 ] = static_cast< double > ( p.data_[ 2 ] );
}

inline Vector::Vector( const VectorF& v )
{
  data_[ 0 ] = static_cast< double > ( v.data_[ 0 ] );
  data_[ 1 ] = static_cast< double > ( v.data_[ 1 ] );
  data_[ 2 ] = static_cast< double > ( v.data_[ 2 ] );
}

inline VectorF::VectorF( const VectorF& p )
{
  data_[ 0 ] = p.data_[ 0 ];
  data_[ 1 ] = p.data_[ 1 ];
  data_[ 2 ] = p.data_[ 2 ];
}

inline VectorF::VectorF( const Point& p )
{
  data_[ 0 ] = static_cast< float > ( p.data_[ 0 ] );
  data_[ 1 ] = static_cast< float > ( p.data_[ 1 ] );
  data_[ 2 ] = static_cast< float > ( p.data_[ 2 ] );
}

inline VectorF::VectorF( const PointF& p )
{
  data_[ 0 ] = p.data_[ 0 ];
  data_[ 1 ] = p.data_[ 1 ];
  data_[ 2 ] = p.data_[ 2 ];
}

inline VectorF::VectorF( const Vector& v )
{
  data_[ 0 ] = static_cast< float > ( v.data_[ 0 ] );
  data_[ 1 ] = static_cast< float > ( v.data_[ 1 ] );
  data_[ 2 ] = static_cast< float > ( v.data_[ 2 ] );
}

inline bool Vector::operator==( const Vector& v ) const
{
  return ( data_[ 0 ] == v.data_[ 0 ] && data_[ 1 ] == v.data_[ 1 ] && data_[ 2 ] == v.data_[ 2 ] );
}

inline bool Vector::operator==( const VectorF& p ) const
{
  return ( static_cast< float > ( data_[ 0 ] ) == p.data_[ 0 ]
      && static_cast< float > ( data_[ 1 ] ) == p.data_[ 1 ]
      && static_cast< float > ( data_[ 2 ] ) == p.data_[ 2 ] );
}

inline bool Vector::operator!=( const Vector& p ) const
{
  return ( data_[ 0 ] != p.data_[ 0 ] || data_[ 1 ] != p.data_[ 1 ] || data_[ 2 ] != p.data_[ 2 ] );
}

inline bool Vector::operator!=( const VectorF& p ) const
{
  return ( static_cast< float > ( data_[ 0 ] ) != p.data_[ 0 ]
      || static_cast< float > ( data_[ 1 ] ) != p.data_[ 1 ]
      || static_cast< float > ( data_[ 2 ] ) != p.data_[ 2 ] );
}

inline bool VectorF::operator==( const Vector& p ) const
{
  return ( static_cast< float > ( p.data_[ 0 ] ) == data_[ 0 ]
      && static_cast< float > ( p.data_[ 1 ] ) == data_[ 1 ]
      && static_cast< float > ( p.data_[ 2 ] ) == data_[ 2 ] );
}

inline bool VectorF::operator==( const VectorF& p ) const
{
  return ( data_[ 0 ] == p.data_[ 0 ] && data_[ 1 ] == p.data_[ 1 ] && data_[ 2 ] == p.data_[ 2 ] );
}

inline bool VectorF::operator!=( const Vector& p ) const
{
  return ( static_cast< float > ( p.data_[ 0 ] ) != data_[ 0 ]
      || static_cast< float > ( p.data_[ 1 ] ) != data_[ 1 ]
      || static_cast< float > ( p.data_[ 2 ] ) != data_[ 2 ] );
}

inline bool VectorF::operator!=( const VectorF& p ) const
{
  return ( data_[ 0 ] != p.data_[ 0 ] || data_[ 1 ] != p.data_[ 1 ] || data_[ 2 ] != p.data_[ 2 ] );
}

inline double Vector::length2() const
{
  return data_[ 0 ] * data_[ 0 ] + data_[ 1 ] * data_[ 1 ] + data_[ 2 ] * data_[ 2 ];
}

inline double Vector::length() const
{
  return Sqrt( data_[ 0 ] * data_[ 0 ] + data_[ 1 ] * data_[ 1 ] + data_[ 2 ] * data_[ 2 ] );
}

inline float VectorF::length2() const
{
  return data_[ 0 ] * data_[ 0 ] + data_[ 1 ] * data_[ 1 ] + data_[ 2 ] * data_[ 2 ];
}

inline float VectorF::length() const
{
  return Sqrt( data_[ 0 ] * data_[ 0 ] + data_[ 1 ] * data_[ 1 ] + data_[ 2 ] * data_[ 2 ] );
}

inline Vector&
Vector::operator=( const Vector& v )
{
  data_[ 0 ] = v.data_[ 0 ];
  data_[ 1 ] = v.data_[ 1 ];
  data_[ 2 ] = v.data_[ 2 ];
  return *this;
}

inline Vector&
Vector::operator=( const VectorF& v )
{
  data_[ 0 ] = static_cast< double > ( v.data_[ 0 ] );
  data_[ 1 ] = static_cast< double > ( v.data_[ 1 ] );
  data_[ 2 ] = static_cast< double > ( v.data_[ 2 ] );
  return *this;
}

inline VectorF&
VectorF::operator=( const Vector& v )
{
  data_[ 0 ] = static_cast< float > ( v.data_[ 0 ] );
  data_[ 1 ] = static_cast< float > ( v.data_[ 1 ] );
  data_[ 2 ] = static_cast< float > ( v.data_[ 2 ] );
  return *this;
}

inline VectorF&
VectorF::operator=( const VectorF& v )
{
  data_[ 0 ] = v.data_[ 0 ];
  data_[ 1 ] = v.data_[ 1 ];
  data_[ 2 ] = v.data_[ 2 ];
  return *this;
}

inline double&
Vector::operator[]( size_t idx )
{
  return data_[ idx ];
}

inline double Vector::operator[]( size_t idx ) const
{
  return data_[ idx ];
}

inline float&
VectorF::operator[]( size_t idx )
{
  return data_[ idx ];
}

inline float VectorF::operator[]( size_t idx ) const
{
  return data_[ idx ];
}

inline Vector Vector::operator*( const double s ) const
{
  return Vector( data_[ 0 ] * s, data_[ 1 ] * s, data_[ 2 ] * s );
}

inline VectorF VectorF::operator*( const float s ) const
{
  return VectorF( data_[ 0 ] * s, data_[ 1 ] * s, data_[ 2 ] * s );
}

inline Vector operator*( const double s, const Vector& v )
{
  return v * s;
}

inline VectorF operator*( const float s, const VectorF& v )
{
  return v * s;
}

inline Vector Vector::operator/( const double d ) const
{
  double s = 1.0 / d;
  return Vector( data_[ 0 ] * s, data_[ 1 ] * s, data_[ 2 ] * s );
}

inline VectorF VectorF::operator/( const float d ) const
{
  float s = 1.0f / d;
  return VectorF( data_[ 0 ] * s, data_[ 1 ] * s, data_[ 2 ] * s );
}

inline Vector Vector::operator+( const Vector& v2 ) const
{
  return Vector( data_[ 0 ] + v2.data_[ 0 ], data_[ 1 ] + v2.data_[ 1 ], data_[ 2 ]
      + v2.data_[ 2 ] );
}

inline Vector Vector::operator+( const VectorF& v2 ) const
{
  return Vector( data_[ 0 ] + static_cast< double > ( v2.data_[ 0 ] ), data_[ 1 ]
      + static_cast< double > ( v2.data_[ 1 ] ), data_[ 2 ]
      + static_cast< double > ( v2.data_[ 2 ] ) );
}

inline VectorF VectorF::operator+( const VectorF& v2 ) const
{
  return VectorF( data_[ 0 ] + v2.data_[ 0 ], data_[ 1 ] + v2.data_[ 1 ], data_[ 2 ]
      + v2.data_[ 2 ] );
}

inline Vector VectorF::operator+( const Vector& v2 ) const
{
  return Vector( data_[ 0 ] + static_cast< float > ( v2.data_[ 0 ] ), data_[ 1 ]
      + static_cast< float > ( v2.data_[ 1 ] ), data_[ 2 ]
      + static_cast< float > ( v2.data_[ 2 ] ) );
}

inline Vector Vector::operator-( const Vector& v2 ) const
{
  return Vector( data_[ 0 ] - v2.data_[ 0 ], data_[ 1 ] - v2.data_[ 1 ], data_[ 2 ]
      - v2.data_[ 2 ] );
}

inline Vector Vector::operator-( const VectorF& v2 ) const
{
  return Vector( data_[ 0 ] - static_cast< double > ( v2.data_[ 0 ] ), data_[ 1 ]
      - static_cast< double > ( v2.data_[ 1 ] ), data_[ 2 ]
      - static_cast< double > ( v2.data_[ 2 ] ) );
}

inline VectorF VectorF::operator-( const VectorF& v2 ) const
{
  return VectorF( data_[ 0 ] - v2.data_[ 0 ], data_[ 1 ] - v2.data_[ 1 ], data_[ 2 ]
      - v2.data_[ 2 ] );
}

inline Vector VectorF::operator-( const Vector& v2 ) const
{
  return Vector( data_[ 0 ] - static_cast< float > ( v2.data_[ 0 ] ), data_[ 1 ]
      - static_cast< float > ( v2.data_[ 1 ] ), data_[ 2 ]
      - static_cast< float > ( v2.data_[ 2 ] ) );
}

inline Vector Vector::operator-( const Point& v2 ) const
{
  return Vector( data_[ 0 ] - v2.data_[ 0 ], data_[ 1 ] - v2.data_[ 1 ], data_[ 2 ]
      - v2.data_[ 2 ] );
}

inline Vector Vector::operator-( const PointF& v2 ) const
{
  return Vector( data_[ 0 ] - static_cast< double > ( v2.data_[ 0 ] ), data_[ 1 ]
      - static_cast< double > ( v2.data_[ 1 ] ), data_[ 2 ]
      - static_cast< double > ( v2.data_[ 2 ] ) );
}

inline Vector VectorF::operator-( const Point& v2 ) const
{
  return Vector( data_[ 0 ] - static_cast< float > ( v2.data_[ 0 ] ), data_[ 1 ]
      - static_cast< float > ( v2.data_[ 1 ] ), data_[ 2 ]
      - static_cast< float > ( v2.data_[ 2 ] ) );
}

inline VectorF VectorF::operator-( const PointF& v2 ) const
{
  return VectorF( data_[ 0 ] - v2.data_[ 0 ], data_[ 1 ] - v2.data_[ 1 ], data_[ 2 ]
      - v2.data_[ 2 ] );
}

inline Vector&
Vector::operator+=( const Vector& v2 )
{
  data_[ 0 ] += v2.data_[ 0 ];
  data_[ 1 ] += v2.data_[ 1 ];
  data_[ 2 ] += v2.data_[ 2 ];
  return *this;
}

inline Vector&
Vector::operator+=( const VectorF& v2 )
{
  data_[ 0 ] += static_cast< double > ( v2.data_[ 0 ] );
  data_[ 1 ] += static_cast< double > ( v2.data_[ 1 ] );
  data_[ 2 ] += static_cast< double > ( v2.data_[ 2 ] );
  return *this;
}

inline VectorF&
VectorF::operator+=( const VectorF& v2 )
{
  data_[ 0 ] += v2.data_[ 0 ];
  data_[ 1 ] += v2.data_[ 1 ];
  data_[ 2 ] += v2.data_[ 2 ];
  return *this;
}

inline VectorF&
VectorF::operator+=( const Vector& v2 )
{
  data_[ 0 ] += static_cast< float > ( v2.data_[ 0 ] );
  data_[ 1 ] += static_cast< float > ( v2.data_[ 1 ] );
  data_[ 2 ] += static_cast< float > ( v2.data_[ 2 ] );
  return *this;
}

inline Vector&
Vector::operator-=( const Vector& v2 )
{
  data_[ 0 ] -= v2.data_[ 0 ];
  data_[ 1 ] -= v2.data_[ 1 ];
  data_[ 2 ] -= v2.data_[ 2 ];
  return *this;
}

inline Vector&
Vector::operator-=( const VectorF& v2 )
{
  data_[ 0 ] -= static_cast< double > ( v2.data_[ 0 ] );
  data_[ 1 ] -= static_cast< double > ( v2.data_[ 1 ] );
  data_[ 2 ] -= static_cast< double > ( v2.data_[ 2 ] );
  return *this;
}

inline VectorF&
VectorF::operator-=( const VectorF& v2 )
{
  data_[ 0 ] -= v2.data_[ 0 ];
  data_[ 1 ] -= v2.data_[ 1 ];
  data_[ 2 ] -= v2.data_[ 2 ];
  return *this;
}

inline VectorF&
VectorF::operator-=( const Vector& v2 )
{
  data_[ 0 ] -= static_cast< float > ( v2.data_[ 0 ] );
  data_[ 1 ] -= static_cast< float > ( v2.data_[ 1 ] );
  data_[ 2 ] -= static_cast< float > ( v2.data_[ 2 ] );
  return *this;
}

inline Vector Vector::operator-() const
{
  return Vector( -data_[ 0 ], -data_[ 1 ], -data_[ 2 ] );
}

inline VectorF VectorF::operator-() const
{
  return VectorF( -data_[ 0 ], -data_[ 1 ], -data_[ 2 ] );
}

inline Vector Abs( const Vector& v )
{
  double x = v.data_[ 0 ] < 0 ? -v.data_[ 0 ] : v.data_[ 0 ];
  double y = v.data_[ 1 ] < 0 ? -v.data_[ 1 ] : v.data_[ 1 ];
  double z = v.data_[ 2 ] < 0 ? -v.data_[ 2 ] : v.data_[ 2 ];
  return Vector( x, y, z );
}

inline VectorF Abs( const VectorF& v )
{
  float x = v.data_[ 0 ] < 0 ? -v.data_[ 0 ] : v.data_[ 0 ];
  float y = v.data_[ 1 ] < 0 ? -v.data_[ 1 ] : v.data_[ 1 ];
  float z = v.data_[ 2 ] < 0 ? -v.data_[ 2 ] : v.data_[ 2 ];
  return VectorF( x, y, z );
}

inline Vector Cross( const Vector& v1, const Vector& v2 )
{
  return Vector( v1.data_[ 1 ] * v2.data_[ 2 ] - v1.data_[ 2 ] * v2.data_[ 1 ], v1.data_[ 2 ]
      * v2.data_[ 0 ] - v1.data_[ 0 ] * v2.data_[ 2 ], v1.data_[ 0 ] * v2.data_[ 1 ]
      - v1.data_[ 1 ] * v2.data_[ 0 ] );
}

inline VectorF Cross( const VectorF& v1, const VectorF& v2 )
{
  return VectorF( v1.data_[ 1 ] * v2.data_[ 2 ] - v1.data_[ 2 ] * v2.data_[ 1 ], v1.data_[ 2 ]
      * v2.data_[ 0 ] - v1.data_[ 0 ] * v2.data_[ 2 ], v1.data_[ 0 ] * v2.data_[ 1 ]
      - v1.data_[ 1 ] * v2.data_[ 0 ] );
}

inline Vector Interpolate( const Vector& v1, const Vector& v2, double weight )
{
  double weight1 = 1.0 - weight;
  return Vector( v2.data_[ 0 ] * weight + v1.data_[ 0 ] * weight1, v2.data_[ 1 ] * weight
      + v1.data_[ 1 ] * weight1, v2.data_[ 2 ] * weight + v1.data_[ 2 ] * weight1 );
}

inline VectorF Interpolate( const VectorF& v1, const VectorF& v2, float weight )
{
  float weight1 = 1.0f - weight;
  return VectorF( v2.data_[ 0 ] * weight + v1.data_[ 0 ] * weight1, v2.data_[ 1 ] * weight
      + v1.data_[ 1 ] * weight1, v2.data_[ 2 ] * weight + v1.data_[ 2 ] * weight1 );
}

inline Vector&
Vector::operator*=( const double d )
{
  data_[ 0 ] *= d;
  data_[ 1 ] *= d;
  data_[ 2 ] *= d;
  return *this;
}

inline VectorF&
VectorF::operator*=( const float d )
{
  data_[ 0 ] *= d;
  data_[ 1 ] *= d;
  data_[ 2 ] *= d;
  return *this;
}

inline Vector&
Vector::operator/=( const double d )
{
  double s = 1.0 / d;
  data_[ 0 ] *= s;
  data_[ 1 ] *= s;
  data_[ 2 ] *= s;
  return *this;
}

inline VectorF&
VectorF::operator/=( const float d )
{
  float s = 1.0f / d;
  data_[ 0 ] *= s;
  data_[ 1 ] *= s;
  data_[ 2 ] *= s;
  return *this;
}

inline void Vector::x( double d )
{
  data_[ 0 ] = d;
}

inline double

Vector::x() const
{
  return data_[ 0 ];
}

inline void Vector::y( double d )
{
  data_[ 1 ] = d;
}

inline double Vector::y() const
{
  return data_[ 1 ];
}

inline void Vector::z( double d )
{
  data_[ 2 ] = d;
}

inline double Vector::z() const
{
  return data_[ 2 ];
}

inline void VectorF::x( float d )
{
  data_[ 0 ] = d;
}

inline float VectorF::x() const
{
  return data_[ 0 ];
}

inline void VectorF::y( float d )
{
  data_[ 1 ] = d;
}

inline float VectorF::y() const
{
  return data_[ 1 ];
}

inline void VectorF::z( float d )
{
  data_[ 2 ] = d;
}

inline float VectorF::z() const
{
  return data_[ 2 ];
}

inline double Dot( const Vector& v1, const Vector& v2 )
{
  return v1.data_[ 0 ] * v2.data_[ 0 ] + v1.data_[ 1 ] * v2.data_[ 1 ] + v1.data_[ 2 ]
      * v2.data_[ 2 ];
}

inline float Dot( const VectorF& v1, const VectorF& v2 )
{
  return v1.data_[ 0 ] * v2.data_[ 0 ] + v1.data_[ 1 ] * v2.data_[ 1 ] + v1.data_[ 2 ]
      * v2.data_[ 2 ];
}

inline double Vector::normalize()
{
  double len = length();
  if ( len > 0.0 )
  {
    double s = 1.0 / len;
    data_[ 0 ] *= s;
    data_[ 1 ] *= s;
    data_[ 2 ] *= s;
  }
  return len;
}

inline float VectorF::normalize()
{
  float len = length();
  if ( len > 0.0f )
  {
    float s = 1.0f / len;
    data_[ 0 ] *= s;
    data_[ 1 ] *= s;
    data_[ 2 ] *= s;
  }
  return len;
}

inline Vector Vector::normal() const
{
  Vector v( *this );
  v.normalize();
  return v;
}

inline VectorF VectorF::normal() const
{
  VectorF v( *this );
  v.normalize();
  return v;
}

inline Vector Min( const Vector &v1, const Vector &v2 )
{
  return Vector( Min( v1.x(), v2.x() ), Min( v1.y(), v2.y() ), Min( v1.z(), v2.z() ) );
}

inline VectorF Min( const VectorF &v1, const VectorF &v2 )
{
  return VectorF( Min( v1.x(), v2.x() ), Min( v1.y(), v2.y() ), Min( v1.z(), v2.z() ) );
}

inline VectorF Max( const VectorF &v1, const VectorF &v2 )
{
  return VectorF( Max( v1.x(), v2.x() ), Max( v1.y(), v2.y() ), Max( v1.z(), v2.z() ) );
}

} // Core

#endif
