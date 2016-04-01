/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef CORE_GEOMETRY_MATRIX_H
#define CORE_GEOMETRY_MATRIX_H

#include <cstring>

#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>

namespace Core
{

class Matrix;
class MatrixF;

class Matrix
{
  friend class MatrixF;

public:
  inline Matrix() 
  {
    for ( size_t j = 0;  j < 16 ; j++ ) data_[ j ] = 0.0;
  }

  Matrix( const double mat[ 4 ][ 4 ] );

  Matrix( const Matrix& mat );
  Matrix( const MatrixF& mat );

  ~Matrix()
  {
  }

  Matrix& operator=( const Matrix& mat ); 
  Matrix& operator=( const MatrixF& mat );

  inline double* data()
  {
    return data_;
  }

  inline const double* data() const
  {
    return data_;
  }

  inline void data( const double mat[ 4 ][ 4 ] )
  {
    for ( size_t i = 0;  i < 4 ; i++ ) 
    {
      for ( size_t j = 0;  j < 4 ; j++ )
      {
        data_[ i + 4*j ] = mat[ i ][ j ];
      }
    }
  }

  inline double& operator()( size_t i, size_t j )
  {
    return data_[ i + 4*j ];
  }

  inline const double& operator()( size_t i, size_t j ) const
  {
    return data_[ i + 4*j ];
  }

  Vector operator*( const Vector& rhs ) const;
  VectorF operator*( const VectorF& rhs ) const;
  Point operator*( const Point& rhs ) const;
  PointF operator*( const PointF& rhs ) const;

  Matrix operator*( const Matrix& rhs ) const;
  Matrix& operator*=( const Matrix& rhs );

  bool operator==( const Matrix& mat ) const;
  bool operator!=( const Matrix& mat ) const;

private:
  double data_[16];

  // static funtions
public:
  // INVERT:
  /// Compute the inverse of the input matrix using LU decomposition
  static bool Invert( const Matrix& mat, Matrix& inverse );

  // TRANSPOSE:
  /// Transpose a matrix
  static void Transpose( const Matrix& mat, Matrix& trans );
  
  // ZERO:
  /// Create a zero matrix
  static Matrix Zero();
  
  // IDENTITY:
  /// Create an identity matrix
  static Matrix Identity();
};


class MatrixF
{
  friend class Matrix;
public:
  inline MatrixF() 
  {
    for ( size_t j = 0;  j < 16 ; j++ ) data_[ j ] = 0.0f;
  }

  MatrixF( const double mat[ 4 ][ 4 ] );

  MatrixF( const Matrix& mat );
  MatrixF( const MatrixF& mat );

  ~MatrixF()
  {
  }

  MatrixF& operator=( const Matrix& mat );  
  MatrixF& operator=( const MatrixF& mat );
  
  inline float* data()
  {
    return data_;
  }

  inline const float* data() const
  {
    return data_;
  }

  inline void data( const float mat[ 4 ][ 4 ] )
  {
    for ( size_t i = 0;  i < 4 ; i++ ) 
    {
      for ( size_t j = 0;  j < 4 ; j++ )
      {
        data_[ i + 4*j ] = mat[ i ][ j ];
      }
    }
  }

  inline float& operator()( size_t i, size_t j )
  {
    return data_[ i + 4*j ];
  }

  inline const float& operator()( size_t i, size_t j ) const
  {
    return data_[ i + 4*j ];
  }

  Vector operator*( const Vector& rhs ) const;
  VectorF operator*( const VectorF& rhs ) const;
  Point operator*( const Point& rhs ) const;
  PointF operator*( const PointF& rhs ) const;

  MatrixF operator*( const MatrixF& rhs ) const;
  MatrixF& operator*=( const MatrixF& rhs );

  bool operator==( const MatrixF& mat ) const;
  bool operator!=( const MatrixF& mat ) const;

private:
  float data_[16];

  // static funtions
public:

  // INVERT:
  /// Compute the inverse of the input matrix using LU decomposition
  /// (boost ublas lu_factorize)
  static bool Invert( const MatrixF& mat, MatrixF& inverse );

  // TRANSPOSE:
  /// Transpose a matrix
  static void Transpose( const MatrixF& mat, MatrixF& trans );
  
  // ZERO:
  /// Create a zero matrix
  static MatrixF Zero();
  
  // IDENTITY:
  /// Create an identity matrix
  static MatrixF Identity();
};



} // End namespace Core

#endif
