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

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <Core/Utils/Log.h>
#include <Core/Geometry/Matrix.h>
#include <Core/Math/MathFunctions.h>

namespace Core
{

Matrix::Matrix( const double mat[ 4 ][ 4 ] ) 
{
  for ( size_t i = 0;  i < 4 ; i++ ) 
  {
    for ( size_t j = 0;  j < 4 ; j++)
    {
      data_[ i + 4*j ] = mat[ i ][ j ];
    }
  }
}

Matrix::Matrix( const Matrix& mat )
{
  memcpy( data_, mat.data_, 16 * sizeof(double) );
}

Matrix::Matrix( const MatrixF& mat )
{
  for ( size_t j = 0;  j < 16 ; j++ ) data_[ j ] = static_cast<double>( mat.data_[ j ] );
}

Matrix& Matrix::operator=( const Matrix& mat )
{
  memcpy( data_, mat.data_, 16 * sizeof(double) );
  return *this;
}

Matrix& Matrix::operator=( const MatrixF& mat )
{
  for ( size_t j = 0;  j < 16 ; j++ ) data_[ j ] = static_cast<double>( mat.data_[ j ] );
  return *this; 
}


Vector Matrix::operator *( const Vector &rhs ) const
{
  return Vector( data_[ 0 ] * rhs[ 0 ] + data_[ 4 ] * rhs[ 1 ] + data_[ 8 ] * rhs[ 2 ], 
    data_[ 1 ] * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ], 
    data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ] + data_[ 10 ] * rhs[ 2 ] );
}

VectorF Matrix::operator *( const VectorF &rhs ) const
{
  return VectorF( static_cast< float > ( data_[ 0 ]  * rhs[ 0 ] + data_[ 4 ]
      * rhs[ 1 ] +  data_[ 8 ] * rhs[ 2 ] ), static_cast< float > ( data_[ 1 ]
      * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ] ),
      static_cast< float > ( data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ]
          + data_[ 10 ] * rhs[ 2 ] ) );
}

Point Matrix::operator*( const Point& rhs ) const
{
  double scale = 1.0 / (data_[ 3 ] * rhs[ 0 ] + data_[ 7 ] * rhs[ 1 ] + data_[ 11 ] * rhs[ 2 ] + data_[ 15 ] );

  return Point( scale * ( data_[ 0 ] * rhs[ 0 ] + data_[ 4 ] * rhs[ 1 ] + data_[ 8 ] * rhs[ 2 ] + data_[ 12 ] ), 
    scale * (data_[ 1 ] * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ] + data_[ 13 ] ), 
    scale * (data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ] + data_[ 10 ] * rhs[ 2 ] + data_[ 14 ] ) );
}

PointF Matrix::operator*( const PointF& rhs ) const
{
  double scale = 1.0 / (data_[ 3 ] * rhs[ 0 ] + data_[ 7 ] * rhs[ 1 ] + data_[ 11 ] * rhs[ 2 ] + data_[ 15 ] );

  return PointF( static_cast< float > ( scale * ( data_[ 0 ] * rhs[ 0 ] + data_[ 4 ] * rhs[ 1 ] + data_[ 8 ] * rhs[ 2 ] + data_[ 12 ] ) ), 
    static_cast< float > ( scale * (data_[ 1 ] * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ] + data_[ 13 ] ) ), 
    static_cast< float > ( scale * (data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ] + data_[ 10 ] * rhs[ 2 ] + data_[ 14 ] ) ) );
}

Matrix Matrix::operator*( const Matrix& rhs ) const
{
  Matrix new_mat;
  
  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
      size_t m = 4 * j;
      new_mat.data_[ i + m ] = 0.0;
      for ( size_t k = 0; k < 4 ; k++ )
      {
        new_mat.data_[ i + m ] += data_[ i + 4 * k ] * rhs.data_[ k + m ];
      }
    }
  }
  
  return new_mat;
}

Matrix& Matrix::operator*=( const Matrix& rhs )
{
  double tmp[16];
  memcpy( tmp, data_, 16 * sizeof( double ) );
  
  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
      size_t m = 4 * j;
      data_ [ i + m ] = 0.0;
      for ( size_t k = 0; k < 4 ; k++ )
      {
        data_[ i + m ] += tmp[ i + 4 * k ] * rhs.data_[ k + m ];
      }
    }
  }
  
  return *this;
}

bool Matrix::operator==( const Matrix& mat ) const
{
  for ( size_t j = 0; j < 16; j++ )
    if ( data_[ j ] != mat.data_[ j ] ) return false;
  return true;
}

bool Matrix::operator!=( const Matrix& mat ) const
{
  for ( size_t j = 0; j < 16; j++ )
    if ( data_[ j ] != mat.data_[ j ] ) return true;
  return false;
}

bool Matrix::Invert( const Matrix& mat, Matrix& inverse_mat )
{
  boost::numeric::ublas::matrix< double > A( 4, 4 );
  boost::numeric::ublas::matrix< double > inverse( 4, 4 );
  
  
  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
      A( i, j ) = mat.data_[ i + 4 * j ];
      if ( i == j ) inverse( i, j ) = 1.0;
      else inverse( i, j ) = 0.0;
    }
  }

  boost::numeric::ublas::permutation_matrix< std::size_t > pm( 4 );

  if ( boost::numeric::ublas::lu_factorize( A, pm ) != 0 ) return false;

  // If the determinant of A is less than a small value, 
  // consider it singular.
  double det = A( 1, 1 ) * A( 2, 2 ) * A( 3, 3 ) /  A( 0, 0 );
  if ( Abs( det ) < 1e-7 )
  {
//    return false;
  }

  try
  {
    boost::numeric::ublas::lu_substitute( A, pm, inverse );
  }
  catch ( ... )
  {
    CORE_LOG_ERROR( "Could not invert matrix" );
    return false;
  }
  
  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
       inverse_mat.data_[ i + 4 * j ] = inverse( i, j );
    }
  }

  return true;
}

void Matrix::Transpose( const Matrix& mat, Matrix& trans )
{
  for ( size_t j = 0;  j < 4 ; j++ )
  {
    for ( size_t i = 0;  i < 4 ; i++ ) 
    {
      trans.data_[ i + 4 * j ] = mat.data_[ j + 4 * i ];
    }
  }
}

Matrix Matrix::Zero()
{
  return Matrix();
}

Matrix Matrix::Identity()
{
  Matrix mat;
  mat.data_[ 0 ] = 1.0;
  mat.data_[ 5 ] = 1.0;
  mat.data_[ 10 ] = 1.0;
  mat.data_[ 15 ] = 1.0;
  return mat;
}


MatrixF::MatrixF( const MatrixF& mat )
{
  memcpy( data_, mat.data_, 16 * sizeof( float ) );
}

MatrixF::MatrixF( const Matrix& mat )
{
  for ( size_t j = 0;  j < 16 ; j++ ) data_[ j ] = static_cast<float>( mat.data_[ j ] );
}

MatrixF& MatrixF::operator=( const MatrixF& mat )
{
  memcpy( data_, mat.data_, 16 * sizeof( float ) );
  return *this;
}

MatrixF& MatrixF::operator=( const Matrix& mat )
{
  for ( size_t j = 0;  j < 16 ; j++ ) data_[ j ] = static_cast<float>( mat.data_[ j ] );
  return *this; 
}



Vector MatrixF::operator *( const Vector &rhs ) const
{
  return Vector( data_[ 0 ] * rhs[ 0 ] + data_[ 4 ] * rhs[ 1 ] + data_[ 8 ] * rhs[ 2 ], 
    data_[ 1 ] * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ], 
    data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ] + data_[ 10 ] * rhs[ 2 ] );
}

VectorF MatrixF::operator *( const VectorF &rhs ) const
{
  return VectorF( data_[ 0 ]  * rhs[ 0 ] + data_[ 4 ]
      * rhs[ 1 ] +  data_[ 8 ] * rhs[ 2 ],  data_[ 1 ]
      * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ],
      data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ] + data_[ 10 ] * rhs[ 2 ] );
}

Point MatrixF::operator*( const Point& rhs ) const
{
  double scale = 1.0 / (data_[ 3 ] * rhs[ 0 ] + data_[ 7 ] * rhs[ 1 ] + data_[ 11 ] * rhs[ 2 ] + data_[ 15 ] );

  return Point( scale * ( data_[ 0 ] * rhs[ 0 ] + data_[ 4 ] * rhs[ 1 ] + data_[ 8 ] * rhs[ 2 ] + data_[ 12 ] ), 
    scale * (data_[ 1 ] * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ] + data_[ 13 ] ), 
    scale * (data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ] + data_[ 10 ] * rhs[ 2 ] + data_[ 14 ] ) );
}

PointF MatrixF::operator*( const PointF& rhs ) const
{
  float scale = 1.0f / (data_[ 3 ] * rhs[ 0 ] + data_[ 7 ] * rhs[ 1 ] + data_[ 11 ] * rhs[ 2 ] + data_[ 15 ] );

  return PointF( scale * ( data_[ 0 ] * rhs[ 0 ] + data_[ 4 ] * rhs[ 1 ] + data_[ 8 ] * rhs[ 2 ] + data_[ 12 ] ), 
    scale * (data_[ 1 ] * rhs[ 0 ] + data_[ 5 ] * rhs[ 1 ] + data_[ 9 ] * rhs[ 2 ] + data_[ 13 ] ), 
    scale * (data_[ 2 ] * rhs[ 0 ] + data_[ 6 ] * rhs[ 1 ] + data_[ 10 ] * rhs[ 2 ] + data_[ 14 ] ) );
}

MatrixF MatrixF::operator*( const MatrixF& rhs ) const
{
  MatrixF new_mat;
  
  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
      size_t m = 4 * j;
      new_mat.data_[ i + m ] = 0.0f;
      for ( size_t k = 0; k < 4 ; k++ )
      {
        new_mat.data_[ i + m ] += data_[ i + 4 * k ] * rhs.data_[ k + m ];
      }
    }
  }
  
  return new_mat;
}

MatrixF& MatrixF::operator*=( const MatrixF& rhs )
{
  float tmp[16];
  memcpy( tmp, data_, 16 * sizeof( float ) );
  
  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
      size_t m = 4 * j;
      data_ [ i + m ] = 0.0f;
      for ( size_t k = 0; k < 4 ; k++ )
      {
        data_[ i + m ] += tmp[ i + 4 * k ] * rhs.data_[ k + m ];
      }
    }
  }
  
  return *this;
}

bool MatrixF::operator==( const MatrixF& mat ) const
{
  for ( size_t j = 0; j < 16; j++ )
    if ( data_[ j ] != mat.data_[ j ] ) return false;
  return true;
}

bool MatrixF::operator!=( const MatrixF& mat ) const
{
  for ( size_t j = 0; j < 16; j++ )
    if ( data_[ j ] != mat.data_[ j ] ) return true;
  return false;
}

bool MatrixF::Invert( const MatrixF& mat, MatrixF& inverse_mat )
{
  boost::numeric::ublas::matrix< float > A( 4, 4 );
  boost::numeric::ublas::matrix< float > inverse( 4, 4 );
  
  
  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
      A( i, j ) = mat.data_[ i + 4 * j ];
      if ( i == j ) inverse( i, j ) = 1.0f;
      else inverse( i, j ) = 0.0f;
    }
  }

  boost::numeric::ublas::permutation_matrix< std::size_t > pm( 4 );

  if ( boost::numeric::ublas::lu_factorize( A, pm ) != 0 ) return false;

  // If the determinant of A is less than a small value, 
  // consider it singular.
  float det = A( 1, 1 ) * A( 2, 2 ) * A( 3, 3 ) /  A( 0, 0 );
  if ( Abs( det ) < 1e-7f )
  {
//    return false;
  }

  boost::numeric::ublas::lu_substitute( A, pm, inverse );

  for ( size_t j = 0; j < 4; j++ )
  {
    for ( size_t i = 0; i < 4; i++ )
    {
       inverse_mat.data_[ i + 4 * j ] = inverse( i, j );
    }
  }

  return true;
}

void MatrixF::Transpose( const MatrixF& mat, MatrixF& trans )
{
  for ( size_t j = 0;  j < 4 ; j++ )
  {
    for ( size_t i = 0;  i < 4 ; i++ ) 
    {
      trans.data_[ i + 4 * j ] = mat.data_[ j + 4 * i ];
    }
  }
}

MatrixF MatrixF::Zero()
{
  return MatrixF();
}

MatrixF MatrixF::Identity()
{
  MatrixF mat;
  mat.data_[ 0 ] = 1.0f;
  mat.data_[ 5 ] = 1.0f;
  mat.data_[ 10 ] = 1.0f;
  mat.data_[ 15 ] = 1.0f;
  return mat;
}

} // End namespace Core
