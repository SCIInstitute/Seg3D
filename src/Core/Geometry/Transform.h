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

#ifndef CORE_GEOMETRY_TRANSFORM_H
#define CORE_GEOMETRY_TRANSFORM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <ostream>

// Core includes
#include <Core/Geometry/Matrix.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Plane.h>

#include <Core/Math/MathFunctions.h>

namespace Core
{

//Forward declaration
class Transform;
class TransformF;

// Class definition
class Transform
{
  friend class TransformF;
public:
  Transform();
  Transform( const Transform& );
  Transform( const TransformF& );
  
  Transform( const Point&, const Vector&, const Vector&, const Vector& );

  Transform& operator=( const Transform& copy );
  Transform& operator=( const TransformF& copy );

  void load_identity();
  void load_basis( const Point&, const Vector&, const Vector&, const Vector& );
  void load_frame( const Vector&, const Vector&, const Vector& );
  void load_matrix( const Matrix& m );

  Transform get_inverse() const;

  void post_transform( const Transform& );
  void pre_transform( const Transform& );

  void post_mult_matrix( const Matrix& m );
  void pre_mult_matrix( const Matrix& m );

  void pre_permute( int xmap, int ymap, int zmap );
  void post_permute( int xmap, int ymap, int zmap );

  void pre_scale( const Vector& );
  void post_scale( const Vector& );

  void pre_shear( const Vector&, const Plane& );
  void post_shear( const Vector&, const Plane& );

  void pre_rotate( double, const Vector& axis );
  void post_rotate( double, const Vector& axis );

  void pre_translate( const Vector& );
  void post_translate( const Vector& );

  // Returns true if the rotation happened, false otherwise.
  bool rotate( const Vector& from, const Vector& to );

  const Matrix& get_matrix() const;

  void get( double* data ) const;
  void set( const double* data );

  Point project( const Point& p ) const;
  Vector project( const Vector& p ) const;
  PointF project( const PointF& p ) const;
  VectorF project( const VectorF& p ) const;

  bool operator==( const Transform& ) const;
  bool operator!=( const Transform& ) const;

  bool is_axis_aligned() const;

  // TODO: Need to look at this public definition, most of these are internal versions
public:

  static void BuildPermuteMatrix( Matrix& m, int xmap, int ymap, int zmap, bool pre );
  static void BuildRotateMatrix( Matrix& m, double angle, const Vector& axis );
  static void BuildShearMatrix( Matrix& m, const Vector& s, const Plane& p );
  static void BuildScaleMatrix( Matrix& m, const Vector& v );
  static void BuildTranslateMatrix( Matrix& m, const Vector& v );
  static void BuildViewMatrix( Matrix& m, const Point& eyep, const Point& lookat, const Vector& up );
  static void BuildPerspectiveMatrix( Matrix& m, double fovy, double aspect, 
          double znear, double zfar );
  static void BuildOrthoMatrix( Matrix& m, double left, double right, double bottom, 
          double top, double nearVal, double farVal );
  static void BuildOrtho2DMatrix( Matrix& m, double left, double right, double bottom, double top );

  static const int TRANSFORM_LENGTH;

protected:
  // rotation matrix
  Matrix mat_;
};

Point operator*( const Transform& t, const Point& d );
Vector operator*( const Transform& t, const Vector& d );
PointF operator*( const Transform& t, const PointF& d );
VectorF operator*( const Transform& t, const VectorF& d );

std::string ExportToString( const Transform& value );
bool ImportFromString( const std::string& str, Transform& value );


// Class definition
class TransformF
{
  friend class Transform;

public:
  TransformF();
  TransformF( const TransformF& );
  TransformF( const Transform& );

  TransformF( const PointF&, const VectorF&, const VectorF&, const VectorF& );

  TransformF& operator=( const TransformF& copy );
  TransformF& operator=( const Transform& copy );

  void load_identity();
  void load_basis( const PointF&, const VectorF&, const VectorF&, const VectorF& );
  void load_frame( const VectorF&, const VectorF&, const VectorF& );
  void load_matrix( const MatrixF& m );

  TransformF get_inverse();

  void post_transform( const TransformF& );
  void pre_transform( const TransformF& );

  void post_mult_matrix( const MatrixF& m );
  void pre_mult_matrix( const MatrixF& m );

  void pre_permute( int xmap, int ymap, int zmap );
  void post_permute( int xmap, int ymap, int zmap );

  void pre_scale( const VectorF& );
  void post_scale( const VectorF& );

  void pre_shear( const VectorF&, const Plane& );
  void post_shear( const VectorF&, const Plane& );

  void pre_rotate( float, const VectorF& axis );
  void post_rotate( float, const VectorF& axis );

  void pre_translate( const VectorF& );
  void post_translate( const VectorF& );

  // Returns true if the rotation happened, false otherwise.
  bool rotate( const VectorF& from, const VectorF& to );

  const MatrixF& get_matrix() const;

  void get( float* data ) const;
  void set( const float* data );

  Point project( const Point& p ) const;
  Vector project( const Vector& p ) const;
  PointF project( const PointF& p ) const;
  VectorF project( const VectorF& p ) const;

  bool operator==( const TransformF& ) const;
  bool operator!=( const TransformF& ) const;

  bool is_axis_aligned() const;

public:  
  static void BuildPermuteMatrix( MatrixF& m, int xmap, int ymap, int zmap, bool pre );
  static void BuildRotateMatrix( MatrixF& m, float angle, const VectorF& axis );
  static void BuildShearMatrix( MatrixF& m, const VectorF& s, const Plane& p );
  static void BuildScaleMatrix( MatrixF& m, const VectorF& v );
  static void BuildTranslateMatrix( MatrixF& m, const VectorF& v );
  static void BuildViewMatrix( MatrixF& m, const PointF& eyep, const PointF& lookat, const VectorF& up );
  static void BuildPerspectiveMatrix( MatrixF& m, float fovy, float aspect, 
          float znear, float zfar );
  static void BuildOrthoMatrix( MatrixF& m, float left, float right, float bottom, 
          float top, float nearVal, float farVal );
  static void BuildOrtho2DMatrix( MatrixF& m, float left, float right, float bottom, float top );
  
  static const int TRANSFORM_LENGTH;

protected:
  // rotation matrix
  MatrixF mat_;
};

std::ostream& operator<<( std::ostream& os, const Transform& t );
std::ostream& operator<<( std::ostream& os, const TransformF& t );

} // End namespace Core

#endif
