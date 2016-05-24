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

#include <Core/Geometry/Transform.h>
#include <Core/Geometry/Point.h>

namespace Core
{

const int Transform::TRANSFORM_LENGTH = 16;
const int TransformF::TRANSFORM_LENGTH = 16;

Transform::Transform()
{
  load_identity();
}

Transform::Transform( const Transform& copy )
{
  this->mat_ = copy.mat_;
}

Transform::Transform( const TransformF& copy )
{
  this->mat_ = copy.mat_;
}

Transform& Transform::operator=( const Transform& copy )
{
  this->mat_ = copy.mat_;
  return ( *this );
}

Transform& Transform::operator=( const TransformF& copy )
{
  this->mat_ = copy.mat_;
  return ( *this );
}

Transform::Transform( const Point& p, const Vector& i, const Vector& j, const Vector& k )
{
  load_basis( p, i, j, k );
}

void Transform::load_basis( const Point &p, const Vector &x, const Vector &y, const Vector &z )
{
  load_frame( x, y, z );
  pre_translate( Vector( p ) );
}

void Transform::load_frame( const Vector& x, const Vector& y, const Vector& z )
{
  mat_( 3, 3 ) = 1.0;
  mat_( 0, 3 ) = mat_( 1, 3 ) = mat_( 2, 3 ) = 0.0;
  mat_( 3, 0 ) = mat_( 3, 1 ) = mat_( 3, 2 ) = 0.0;

  mat_( 0, 0 ) = x.x();
  mat_( 1, 0 ) = x.y();
  mat_( 2, 0 ) = x.z();

  mat_( 0, 1 ) = y.x();
  mat_( 1, 1 ) = y.y();
  mat_( 2, 1 ) = y.z();

  mat_( 0, 2 ) = z.x();
  mat_( 1, 2 ) = z.y();
  mat_( 2, 2 ) = z.z();
}

void Transform::load_matrix( const Matrix& m )
{
  this->mat_ = m;
}

void Transform::post_transform( const Transform& trans )
{
  this->mat_ *= trans.mat_;
}

void Transform::pre_transform( const Transform& trans )
{
  this->mat_ = trans.mat_ * this->mat_;
}

void Transform::post_mult_matrix( const Matrix& m )
{
  this->mat_ *= m;
}

void Transform::pre_mult_matrix( const Matrix& m )
{
  this->mat_ = m * this->mat_;
}

void Transform::pre_scale( const Vector& v )
{
  Matrix m;
  Transform::BuildScaleMatrix( m, v );

  this->mat_ = m * this->mat_;
}

void Transform::post_scale( const Vector& v )
{
  Matrix m;
  Transform::BuildScaleMatrix( m, v );

  this->mat_ *= m;
}

void Transform::pre_shear( const Vector& s, const Plane& p )
{
  Matrix m;
  Transform::BuildShearMatrix( m, s, p );

  this->mat_ = m * this->mat_;
}

void Transform::post_shear( const Vector& s, const Plane& p )
{
  Matrix m;
  Transform::BuildShearMatrix( m, s, p );

  this->mat_ *= m;
}

void Transform::pre_translate( const Vector& v )
{
  Matrix m;
  Transform::BuildTranslateMatrix( m, v );

  this->mat_ = m * this->mat_;
}

void Transform::post_translate( const Vector& v )
{
  Matrix m;
  Transform::BuildTranslateMatrix( m, v );

  this->mat_ *= m;
}

void Transform::pre_rotate( double angle, const Vector& axis )
{
  Matrix m;
  Transform::BuildRotateMatrix( m, angle, axis );

  this->mat_ = m * this->mat_;
}

void Transform::post_rotate( double angle, const Vector& axis )
{
  Matrix m;
  Transform::BuildRotateMatrix( m, angle, axis );

  this->mat_ *= m;
}

bool Transform::rotate( const Vector& from, const Vector& to )
{
  Vector t( to );
  t.normalize();
  Vector f( from );
  f.normalize();
  Vector axis( Cross( f, t ) );

  if ( axis.length2() < 1.0e-8 )
  {
    // Vectors are too close to each other to get a stable axis of
    // rotation, so return.
    return false;
  }

  double sinth = axis.length();
  double costh = Dot( f, t );
  if ( Abs( sinth ) < 1.0e-9 )
  {
    if ( costh > 0.0 ) return false; // no rotate;
    else
    {
      // from and to are in opposite directions, find an axis of rotation
      // Try the Z axis first.  This will fail if from is along Z, so try
      // Y next.  Then rotate 180 degrees.
      axis = Cross( from, Vector( 0.0, 0.0, 1.0 ) );
      if ( axis.length2() < 1.0e-9 ) axis = Cross( from, Vector( 0.0, 1.0, 0.0 ) );
      axis.normalize();
      post_rotate( Pi(), axis );
    }
  }
  else
  {
    post_rotate( Atan2( sinth, costh ), axis.normal() );
  }
  return true;
}

void Transform::pre_permute( int xmap, int ymap, int zmap )
{
  Matrix m;
  Transform::BuildPermuteMatrix( m, xmap, ymap, zmap, true );

  this->mat_ = m * this->mat_;
}

void Transform::post_permute( int xmap, int ymap, int zmap )
{
  Matrix m;
  Transform::BuildPermuteMatrix( m, xmap, ymap, zmap, false );

  this->mat_ *= m;
}

Point Transform::project( const Point& p ) const
{
  return this->mat_ * p;
}

PointF Transform::project( const PointF& p ) const
{
  return this->mat_ * p;
}

Vector Transform::project( const Vector& v ) const
{
  return this->mat_ * v;
}

VectorF Transform::project( const VectorF& v ) const
{
  return this->mat_ * v;
}

const Matrix& Transform::get_matrix() const
{
  return this->mat_;
}

void Transform::get( double* data ) const
{
  std::memcpy( data, this->mat_.data(), sizeof(double) * TRANSFORM_LENGTH );
}

void Transform::set( const double* data )
{
  std::memcpy( this->mat_.data(), data, sizeof(double) * TRANSFORM_LENGTH );
}

void Transform::load_identity()
{
  mat_ = Matrix::Identity();
}

Transform Transform::get_inverse() const
{
  Transform inv_transform;
  Matrix::Invert( this->mat_, inv_transform.mat_ );
  return ( inv_transform );
}

bool Transform::operator==( const Transform& transform ) const
{
  Vector ex( 1.0, 0.0, 0.0 );
  Vector ey( 0.0, 1.0, 0.0 );
  Vector ez( 0.0, 0.0, 1.0 );
  Point origin( 0.0, 0.0, 0.0 );

  const double epsilon = 1e-4;

  Vector ex1 = project( ex ); Vector ex2 = transform.project( ex );
  double lex1 = ex1.normalize(); double lex2 = ex2.normalize();
  if ( lex1 + lex2 > 0.0 && ( Abs( ( lex1 - lex2) / ( lex1 + lex2 ) ) > epsilon || Abs ( Dot( ex1, ex2 ) - 1.0 ) > epsilon ) ) return false;

  Vector ey1 = project( ey ); Vector ey2 = transform.project( ey );
  double ley1 = ey1.normalize(); double ley2 = ey2.normalize();
  if ( ley1 + ley2 > 0.0 && ( Abs( ( ley1 - ley2 ) / ( ley1 + ley2 ) ) > epsilon || Abs ( Dot( ey1, ey2 ) - 1.0 ) > epsilon ) ) return false;

  Vector ez1 = project( ez ); Vector ez2 = transform.project( ez );
  double lez1 = ez1.normalize(); double lez2 = ez2.normalize();
  if ( lez1 + lez2 > 0.0 && ( Abs( ( lez1 - lez2 ) / ( lez1 + lez2 ) ) > epsilon || Abs ( Dot( ez1, ez2 ) - 1.0 ) > epsilon ) ) return false;
  
  if ( ( project( origin ) - transform.project( origin ) ).length() > epsilon ) return false;

  return true;
}

bool Transform::operator!=( const Transform& transform ) const
{
  return !( *this == transform );
}

bool Transform::is_axis_aligned() const
{
  bool found_x_axis = false;
  bool found_y_axis = false;
  bool found_z_axis = false;

  Vector ex(1.0,0.0,0.0);
  Vector ey(0.0,1.0,0.0);
  Vector ez(0.0,0.0,1.0);
  Vector test;
  
  double target = 1.0 - 1.0e-6;

  test = project( ex ); 
  test.normalize();
  if ( Dot( test, ex ) > target ) found_x_axis = true;
  else if ( Dot( test, ey ) > target ) found_y_axis = true;
  else if ( Dot( test, ez ) > target ) found_z_axis = true;

  test = project( ey ); 
  test.normalize();
  if ( Dot( test, ex ) > target ) found_x_axis = true;
  else if ( Dot( test, ey ) > target ) found_y_axis = true;
  else if ( Dot( test, ez ) > target ) found_z_axis = true;

  test = project( ez ); 
  test.normalize();
  if ( Dot( test, ex ) > target ) found_x_axis = true;
  else if ( Dot( test, ey ) > target ) found_y_axis = true;
  else if ( Dot( test, ez ) > target ) found_z_axis = true;

  return ( found_x_axis && found_y_axis && found_z_axis ); 
}

void Transform::BuildTranslateMatrix( Matrix& m, const Vector& v )
{
  m = Matrix::Identity();
  m( 0, 3 ) = v.x();
  m( 1, 3 ) = v.y();
  m( 2, 3 ) = v.z();
}

// rotate into a new frame (z=shear-fixed-plane, y=projected shear vector),
// shear in y (based on value of z), rotate back to original frame
void Transform::BuildShearMatrix( Matrix& m, const Vector& s, const Plane& p )
{
  m = Matrix::Identity();

  Vector sv( p.project( s ) ); // s projected onto p
  Vector dn( s - sv ); // difference (in normal direction) between s and sv
  double d = Dot( dn, p.normal() );

  // shear vector lies in shear fixed plane, return identity.
  if ( Abs( d ) < 1.0e-8 )
  {
    return;
  }

  double yshear = sv.length() / d; // compute the length of the shear vector,
  // after the normal-to-shear-plane component
  // has been made unit-length.
  Vector svn( sv );
  svn.normalize(); // normalized vector for building orthonormal basis

  Vector su( Cross( svn, p.normal() ) );

  // the rotation to take the z-axis to the shear normal
  // and the y-axis to the projected shear vector
  Transform r;
  r.load_frame( su, svn, p.normal() );

  // the shear matrix in the new frame
  Matrix shear = Matrix::Identity();
  shear( 1, 2 ) = yshear;
  shear( 1, 3 ) = -yshear * p.distance();

  Matrix r_inverse_mat;
  Matrix::Invert( r.mat_, r_inverse_mat );
  m = r_inverse_mat * shear * r.mat_;
}

void Transform::BuildScaleMatrix( Matrix& m, const Vector& v )
{
  m = Matrix::Identity();
  m( 0, 0 ) = v.x();
  m( 1, 1 ) = v.y();
  m( 2, 2 ) = v.z();
}

void Transform::BuildRotateMatrix( Matrix& m, double angle, const Vector& axis )
{
  double sintheta = Sin( angle );
  double costheta = Cos( angle );
  double ux = axis.x();
  double uy = axis.y();
  double uz = axis.z();

  m( 0, 0 ) = ux * ux + costheta * ( 1.0 - ux * ux );
  m( 0, 1 ) = ux * uy * ( 1.0 - costheta ) - uz * sintheta;
  m( 0, 2 ) = uz * ux * ( 1.0 - costheta ) + uy * sintheta;
  m( 0, 3 ) = 0.0;

  m( 1, 0 ) = ux * uy * ( 1.0 - costheta ) + uz * sintheta;
  m( 1, 1 ) = uy * uy + costheta * ( 1 - uy * uy );
  m( 1, 2 ) = uy * uz * ( 1.0 - costheta ) - ux * sintheta;
  m( 1, 3 ) = 0.0;

  m( 2, 0 ) = uz * ux * ( 1.0 - costheta ) - uy * sintheta;
  m( 2, 1 ) = uy * uz * ( 1.0 - costheta ) + ux * sintheta;
  m( 2, 2 ) = uz * uz + costheta * ( 1 - uz * uz );
  m( 2, 3 ) = 0.0;

  m( 3, 0 ) = 0.0;
  m( 3, 1 ) = 0.0;
  m( 3, 2 ) = 0.0;
  m( 3, 3 ) = 1.0;
}

void Transform::BuildPermuteMatrix( Matrix& m, int xmap, int ymap, int zmap, bool pre )
{
  m = Matrix::Zero();

  m( 3, 3 ) = 1.0;
  int x = xmap < 0 ? ( -1 - xmap ) : ( xmap - 1 );
  int y = ymap < 0 ? ( -1 - ymap ) : ( ymap - 1 );
  int z = zmap < 0 ? ( -1 - zmap ) : ( zmap - 1 );

  if ( pre )
  {
    // for each row, set the mapped row
    m( 0, x ) = Sign( xmap ) * 1.0;
    m( 1, y ) = Sign( ymap ) * 1.0;
    m( 2, z ) = Sign( zmap ) * 1.0;
  }
  else
  {
    // for each column, set the mapped column
    m( x, 0 ) = Sign( xmap ) * 1.0;
    m( y, 1 ) = Sign( ymap ) * 1.0;
    m( z, 2 ) = Sign( zmap ) * 1.0;
  }
}

void Transform::BuildViewMatrix( Matrix& m, const Point& eyep, 
  const Point& lookat, const Vector& up )
{
  Vector z( eyep - lookat );
  z.normalize();

  Vector x( Cross( up, z ) );
  x.normalize();

  Vector y( Cross( z, x ) );

  Transform tf( eyep, x, y, z );
  tf = tf.get_inverse();
  m = tf.get_matrix();
}

void Transform::BuildPerspectiveMatrix( Matrix& m, double fovy, double aspect,
  double znear, double zfar )
{
  double f = Cot( DegreeToRadian( fovy ) * 0.5 );
  m = Matrix::Zero();
  m( 0, 0 ) = f / aspect;
  m( 1, 1 ) = f;
  m( 2, 2 ) = ( zfar + znear ) / ( znear - zfar );
  m( 2, 3 ) = 2.0 * zfar * znear / ( znear - zfar );
  m( 3, 2 ) = -1.0;
}

void Transform::BuildOrthoMatrix( Matrix& m, double left, double right, double bottom, 
  double top, double nearVal, double farVal )
{
  m = Matrix::Zero();
  double width = right - left;
  double height = top - bottom;
  double depth = farVal - nearVal;
  m( 0, 0 ) = 2.0 / width;
  m( 0, 3 ) = -( right + left ) / width;
  m( 1, 1 ) = 2.0 / height;
  m( 1, 3 ) = -( top + bottom ) / height;
  m( 2, 2 ) = -2.0 / depth;
  m( 2, 3 ) = -( farVal + nearVal ) / depth;
  m( 3, 3 ) = 1.0;
}

void Transform::BuildOrtho2DMatrix( Matrix& m, double left, double right, 
  double bottom, double top )
{
  Transform::BuildOrthoMatrix( m, left, right, bottom, top, -1.0, 1.0 );
}

Point operator*( const Transform& t, const Point& d )
{
  return t.project( d );
}

Vector operator*( const Transform& t, const Vector& d )
{
  return t.project( d );
}

PointF operator*( const Transform& t, const PointF& d )
{
  return t.project( d );
}

VectorF operator*( const Transform& t, const VectorF& d )
{
  return t.project( d );
}

std::string ExportToString( const Transform& value )
{
  std::vector< double > trans( Transform::TRANSFORM_LENGTH );
  value.get( &trans[ 0 ] );
  return ExportToString( trans );
}

bool ImportFromString( const std::string& str, Transform& value )
{
  std::vector< double > values;
  ImportFromString( str, values );
  if ( values.size() == Transform::TRANSFORM_LENGTH )
  {
    value.set( &values[ 0 ] );
    return ( true );
  }
  return false;
}



TransformF::TransformF()
{
  load_identity();
}

TransformF::TransformF( const TransformF& copy )
{
  this->mat_ = copy.mat_;
}

TransformF::TransformF( const Transform& copy )
{
  this->mat_ = copy.mat_;
}

TransformF& TransformF::operator=( const TransformF& copy )
{
  this->mat_ = copy.mat_;
  return *this;
}

TransformF& TransformF::operator=( const Transform& copy )
{
  this->mat_ = copy.mat_;
  return *this;
}

TransformF::TransformF( const PointF& p, const VectorF& i, const VectorF& j, const VectorF& k )
{
  load_basis( p, i, j, k );
}

void TransformF::load_basis( const PointF &p, const VectorF &x, const VectorF &y, const VectorF &z )
{
  load_frame( x, y, z );
  pre_translate( VectorF( p ) );
}

void TransformF::load_frame( const VectorF& x, const VectorF& y, const VectorF& z )
{
  mat_( 3, 3 ) = 1.0f;
  mat_( 0, 3 ) = mat_( 1, 3 ) = mat_( 2, 3 ) = 0.0f;
  mat_( 3, 0 ) = mat_( 3, 1 ) = mat_( 3, 2 ) = 0.0f;

  mat_( 0, 0 ) = x.x();
  mat_( 1, 0 ) = x.y();
  mat_( 2, 0 ) = x.z();

  mat_( 0, 1 ) = y.x();
  mat_( 1, 1 ) = y.y();
  mat_( 2, 1 ) = y.z();

  mat_( 0, 2 ) = z.x();
  mat_( 1, 2 ) = z.y();
  mat_( 2, 2 ) = z.z();
}

void TransformF::load_matrix( const MatrixF& m )
{
  this->mat_ = m;
}

void TransformF::post_transform( const TransformF& trans )
{
  this->mat_ *= trans.mat_;
}

void TransformF::pre_transform( const TransformF& trans )
{
  this->mat_ = trans.mat_ * this->mat_;
}

void TransformF::post_mult_matrix( const MatrixF& m )
{
  this->mat_ *= m;
}

void TransformF::pre_mult_matrix( const MatrixF& m )
{
  this->mat_ = m * this->mat_;
}

void TransformF::pre_scale( const VectorF& v )
{
  MatrixF m;
  TransformF::BuildScaleMatrix( m, v );

  this->mat_ = m * this->mat_;
}

void TransformF::post_scale( const VectorF& v )
{
  MatrixF m;
  TransformF::BuildScaleMatrix( m, v );

  this->mat_ *= m;
}

void TransformF::pre_shear( const VectorF& s, const Plane& p )
{
  MatrixF m;
  TransformF::BuildShearMatrix( m, s, p );

  this->mat_ = m * this->mat_;
}

void TransformF::post_shear( const VectorF& s, const Plane& p )
{
  MatrixF m;
  TransformF::BuildShearMatrix( m, s, p );

  this->mat_ *= m;
}

void TransformF::pre_translate( const VectorF& v )
{
  MatrixF m;
  TransformF::BuildTranslateMatrix( m, v );

  this->mat_ = m * this->mat_;
}

void TransformF::post_translate( const VectorF& v )
{
  MatrixF m;
  TransformF::BuildTranslateMatrix( m, v );

  this->mat_ *= m;
}

void TransformF::pre_rotate( float angle, const VectorF& axis )
{
  MatrixF m;
  TransformF::BuildRotateMatrix( m, angle, axis );

  this->mat_ = m * this->mat_;
}

void TransformF::post_rotate( float angle, const VectorF& axis )
{
  MatrixF m;
  TransformF::BuildRotateMatrix( m, angle, axis );

  this->mat_ *= m;
}

bool TransformF::rotate( const VectorF& from, const VectorF& to )
{
  VectorF t( to );
  t.normalize();
  VectorF f( from );
  f.normalize();
  VectorF axis( Cross( f, t ) );

  if ( axis.length2() < 1.0e-8f )
  {
    // Vectors are too close to each other to get a stable axis of
    // rotation, so return.
    return false;
  }

  float sinth = axis.length();
  float costh = Dot( f, t );
  if ( Abs( sinth ) < 1.0e-9f )
  {
    if ( costh > 0.0f ) return false; // no rotate;
    else
    {
      // from and to are in opposite directions, find an axis of rotation
      // Try the Z axis first.  This will fail if from is along Z, so try
      // Y next.  Then rotate 180 degrees.
      axis = Cross( from, VectorF( 0.0f, 0.0f, 1.0f ) );
      if ( axis.length2() < 1.0e-9f ) axis = Cross( from, VectorF( 0.0f, 1.0f, 0.0f ) );
      axis.normalize();
      post_rotate( static_cast<float>( Pi() ), axis );
    }
  }
  else
  {
    post_rotate( Atan2( sinth, costh ), axis.normal() );
  }
  return true;
}

void TransformF::pre_permute( int xmap, int ymap, int zmap )
{
  MatrixF m;
  TransformF::BuildPermuteMatrix( m, xmap, ymap, zmap, true );

  this->mat_ = m * this->mat_;
}

void TransformF::post_permute( int xmap, int ymap, int zmap )
{
  MatrixF m;
  TransformF::BuildPermuteMatrix( m, xmap, ymap, zmap, false );

  this->mat_ *= m;
}

Point TransformF::project( const Point& p ) const
{
  return this->mat_ * p;
}

PointF TransformF::project( const PointF& p ) const
{
  return this->mat_ * p;
}

Vector TransformF::project( const Vector& v ) const
{
  return this->mat_ * v;
}

VectorF TransformF::project( const VectorF& v ) const
{
  return this->mat_ * v;
}

const MatrixF& TransformF::get_matrix() const
{
  return this->mat_;
}

void TransformF::get( float* data ) const
{
  std::memcpy( data, this->mat_.data(), sizeof( float ) * TRANSFORM_LENGTH );
}

void TransformF::set( const float* data )
{
  std::memcpy( this->mat_.data(), data, sizeof( float ) * TRANSFORM_LENGTH );
}

void TransformF::load_identity()
{
  mat_ = MatrixF::Identity();
}

TransformF TransformF::get_inverse()
{
  TransformF inv_transform;
  MatrixF::Invert( this->mat_, inv_transform.mat_ );
  return ( inv_transform );
}

bool TransformF::operator==( const TransformF& transform ) const
{
  return ( transform.mat_ == mat_ );
}

bool TransformF::operator!=( const TransformF& transform ) const
{
  return ( transform.mat_ != mat_ );
}

bool TransformF::is_axis_aligned() const
{
  bool found_x_axis = false;
  bool found_y_axis = false;
  bool found_z_axis = false;

  VectorF ex(1.0f,0.0f,0.0f);
  VectorF ey(0.0f,1.0f,0.0f);
  VectorF ez(0.0f,0.0f,1.0f);
  VectorF test;
  
  float target = 1.0f - 1.0e-6f;

  test = project( ex ); 
  test.normalize();
  if ( Dot( test, ex ) > target ) found_x_axis = true;
  else if ( Dot( test, ey ) > target ) found_y_axis = true;
  else if ( Dot( test, ez ) > target ) found_z_axis = true;

  test = project( ey ); 
  test.normalize();
  if ( Dot( test, ex ) > target ) found_x_axis = true;
  else if ( Dot( test, ey ) > target ) found_y_axis = true;
  else if ( Dot( test, ez ) > target ) found_z_axis = true;

  test = project( ez ); 
  test.normalize();
  if ( Dot( test, ex ) > target ) found_x_axis = true;
  else if ( Dot( test, ey ) > target ) found_y_axis = true;
  else if ( Dot( test, ez ) > target ) found_z_axis = true;

  return ( found_x_axis && found_y_axis && found_z_axis ); 
}

void TransformF::BuildTranslateMatrix( MatrixF& m, const VectorF& v )
{
  m = MatrixF::Identity();
  m( 0, 3 ) = v.x();
  m( 1, 3 ) = v.y();
  m( 2, 3 ) = v.z();
}

// rotate into a new frame (z=shear-fixed-plane, y=projected shear vector),
// shear in y (based on value of z), rotate back to original frame
void TransformF::BuildShearMatrix( MatrixF& m, const VectorF& s, const Plane& p )
{
  m = Matrix::Identity();

  VectorF sv( p.project( s ) ); // s projected onto p
  VectorF dn( s - sv ); // difference (in normal direction) between s and sv
  VectorF normal( p.normal() );
  float d = Dot( dn, normal );

  // shear vector lies in shear fixed plane, return identity.
  if ( Abs( d ) < 1.0e-8f )
  {
    return;
  }

  float yshear = sv.length() / d; // compute the length of the shear vector,
  // after the normal-to-shear-plane component
  // has been made unit-length.
  VectorF svn( sv );
  svn.normalize(); // normalized vector for building orthonormal basis

  VectorF su( Cross( svn, normal ) );

  // the rotation to take the z-axis to the shear normal
  // and the y-axis to the projected shear vector
  TransformF r;
  r.load_frame( su, svn, normal );

  // the shear matrix in the new frame
  MatrixF shear = MatrixF::Identity();
  shear( 1, 2 ) = yshear;
  shear( 1, 3 ) = -yshear * static_cast< float >( p.distance() );

  MatrixF r_inverse_mat;
  MatrixF::Invert( r.mat_, r_inverse_mat );
  m = r_inverse_mat * shear * r.mat_;
}

void TransformF::BuildScaleMatrix( MatrixF& m, const VectorF& v )
{
  m = MatrixF::Identity();
  m( 0, 0 ) = v.x();
  m( 1, 1 ) = v.y();
  m( 2, 2 ) = v.z();
}

void TransformF::BuildRotateMatrix( MatrixF& m, float angle, const VectorF& axis )
{
  float sintheta = Sin( angle );
  float costheta = Cos( angle );
  float ux = axis.x();
  float uy = axis.y();
  float uz = axis.z();

  m( 0, 0 ) = ux * ux + costheta * ( 1.0f - ux * ux );
  m( 0, 1 ) = ux * uy * ( 1.0f - costheta ) - uz * sintheta;
  m( 0, 2 ) = uz * ux * ( 1.0f - costheta ) + uy * sintheta;
  m( 0, 3 ) = 0.0f;

  m( 1, 0 ) = ux * uy * ( 1.0f - costheta ) + uz * sintheta;
  m( 1, 1 ) = uy * uy + costheta * ( 1.0f - uy * uy );
  m( 1, 2 ) = uy * uz * ( 1.0f - costheta ) - ux * sintheta;
  m( 1, 3 ) = 0.0f;

  m( 2, 0 ) = uz * ux * ( 1.0f - costheta ) - uy * sintheta;
  m( 2, 1 ) = uy * uz * ( 1.0f - costheta ) + ux * sintheta;
  m( 2, 2 ) = uz * uz + costheta * ( 1.0f - uz * uz );
  m( 2, 3 ) = 0.0f;

  m( 3, 0 ) = 0.0f;
  m( 3, 1 ) = 0.0f;
  m( 3, 2 ) = 0.0f;
  m( 3, 3 ) = 1.0f;
}

void TransformF::BuildPermuteMatrix( MatrixF& m, int xmap, int ymap, int zmap, bool pre )
{
  m = MatrixF::Zero();

  m( 3, 3 ) = 1.0f;
  int x = xmap < 0 ? ( -1 - xmap ) : ( xmap - 1 );
  int y = ymap < 0 ? ( -1 - ymap ) : ( ymap - 1 );
  int z = zmap < 0 ? ( -1 - zmap ) : ( zmap - 1 );

  if ( pre )
  {
    // for each row, set the mapped row
    m( 0, x ) = Sign( xmap ) * 1.0f;
    m( 1, y ) = Sign( ymap ) * 1.0f;
    m( 2, z ) = Sign( zmap ) * 1.0f;
  }
  else
  {
    // for each column, set the mapped column
    m( x, 0 ) = Sign( xmap ) * 1.0f;
    m( y, 1 ) = Sign( ymap ) * 1.0f;
    m( z, 2 ) = Sign( zmap ) * 1.0f;
  }
}

void TransformF::BuildViewMatrix( MatrixF& m, const PointF& eyep, 
  const PointF& lookat, const VectorF& up )
{
  VectorF z( eyep - lookat );
  z.normalize();

  VectorF x( Cross( up, z ) );
  x.normalize();

  VectorF y( Cross( z, x ) );

  TransformF tf( eyep, x, y, z );
  tf = tf.get_inverse();
  m = tf.get_matrix();
}

void TransformF::BuildPerspectiveMatrix( MatrixF& m, float fovy, float aspect,
  float znear, float zfar )
{
  float f = Cot( DegreeToRadian( fovy ) * 0.5f );
  m = MatrixF::Zero();
  m( 0, 0 ) = f / aspect;
  m( 1, 1 ) = f;
  m( 2, 2 ) = ( zfar + znear ) / ( znear - zfar );
  m( 2, 3 ) = 2.0f * zfar * znear / ( znear - zfar );
  m( 3, 2 ) = -1.0f;
}

void TransformF::BuildOrthoMatrix( MatrixF& m, float left, float right, float bottom, 
  float top, float nearVal, float farVal )
{
  m = MatrixF::Zero();
  float width = right - left;
  float height = top - bottom;
  float depth = farVal - nearVal;
  m( 0, 0 ) = 2.0f / width;
  m( 0, 3 ) = -( right + left ) / width;
  m( 1, 1 ) = 2.0f / height;
  m( 1, 3 ) = -( top + bottom ) / height;
  m( 2, 2 ) = -2.0f / depth;
  m( 2, 3 ) = -( farVal + nearVal ) / depth;
  m( 3, 3 ) = 1.0f;
}

void TransformF::BuildOrtho2DMatrix( MatrixF& m, float left, float right, 
  float bottom, float top )
{
  TransformF::BuildOrthoMatrix( m, left, right, bottom, top, -1.0f, 1.0f );
}

Point operator*( const TransformF& t, const Point& d )
{
  return t.project( d );
}

Vector operator*( const TransformF& t, const Vector& d )
{
  return t.project( d );
}

PointF operator*( const TransformF& t, const PointF& d )
{
  return t.project( d );
}

VectorF operator*( const TransformF& t, const VectorF& d )
{
  return t.project( d );
}

std::ostream& operator<<( std::ostream& os, const Transform& t )
{
  Matrix mat = t.get_matrix();
  os << "transform:" << std::endl
     << mat(0, 0) << " " << mat(0, 1) << " " << mat(0, 2) << " " << mat(0, 3) << std::endl;
  os << mat(1, 0) << " " << mat(1, 1) << " " << mat(1, 2) << " " << mat(1, 3) << std::endl;
  os << mat(2, 0) << " " << mat(2, 1) << " " << mat(2, 2) << " " << mat(2, 3) << std::endl;
  os << mat(3, 0) << " " << mat(3, 1) << " " << mat(3, 2) << " " << mat(3, 3) << std::endl;
  return os;
}

std::ostream& operator<<( std::ostream& os, const TransformF& t )
{
  MatrixF mat = t.get_matrix();
  os << "transform:" << std::endl
     << mat(0, 0) << " " << mat(0, 1) << " " << mat(0, 2) << " " << mat(0, 3) << std::endl;
  os << mat(1, 0) << " " << mat(1, 1) << " " << mat(1, 2) << " " << mat(1, 3) << std::endl;
  os << mat(2, 0) << " " << mat(2, 1) << " " << mat(2, 2) << " " << mat(2, 3) << std::endl;
  os << mat(3, 0) << " " << mat(3, 1) << " " << mat(3, 2) << " " << mat(3, 3) << std::endl;
  return os;
}

std::string ExportToString( const TransformF& value )
{
  std::vector< float > trans( TransformF::TRANSFORM_LENGTH );
  value.get( &trans[ 0 ] );
  return ExportToString( trans );
}

bool ImportFromString( const std::string& str, TransformF& value )
{
  std::vector< float > values;
  ImportFromString( str, values );
  if ( values.size() == TransformF::TRANSFORM_LENGTH )
  {
    value.set( &values[ 0 ] );
    return true;
  }
  return false;
}

} // namespace Core
