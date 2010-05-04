/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

Transform::Transform()
{
  load_identity();
}

Transform::Transform( const Transform& copy )
{
  this->mat_ = copy.mat_;
}

Transform& Transform::operator=( const Transform& copy )
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
  std::memcpy( data, this->mat_.data(), sizeof(double) * 16 );
}

void Transform::set( const double* data )
{
  std::memcpy( this->mat_.data(), data, sizeof(double) * 16 );
}

void Transform::load_identity()
{
  mat_ = Matrix::IDENTITY_C;
}

Transform Transform::get_inverse()
{
  Transform inv_transform;
  Invert( this->mat_, inv_transform.mat_ );
  return ( inv_transform );
}

bool Transform::operator==( const Transform& transform ) const
{
  return ( transform.mat_ == mat_ );
}

bool Transform::operator!=( const Transform& transform ) const
{
  return ( transform.mat_ != mat_ );
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
  m = Matrix::IDENTITY_C;
  m( 0, 3 ) = v.x();
  m( 1, 3 ) = v.y();
  m( 2, 3 ) = v.z();
}

// rotate into a new frame (z=shear-fixed-plane, y=projected shear vector),
// shear in y (based on value of z), rotate back to original frame
void Transform::BuildShearMatrix( Matrix& m, const Vector& s, const Plane& p )
{
  m = Matrix::IDENTITY_C;

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
  Matrix shear = Matrix::IDENTITY_C;
  shear( 1, 2 ) = yshear;
  shear( 1, 3 ) = -yshear * p.distance();

  Matrix r_inverse_mat;
  Invert( r.mat_, r_inverse_mat );
  m = r_inverse_mat * shear * r.mat_;
}

void Transform::BuildScaleMatrix( Matrix& m, const Vector& v )
{
  m = Matrix::IDENTITY_C;
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
  m = Matrix::ZERO_C;

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
  m = Matrix::ZERO_C;
  m( 0, 0 ) = f / aspect;
  m( 1, 1 ) = f;
  m( 2, 2 ) = ( zfar + znear ) / ( znear - zfar );
  m( 2, 3 ) = 2 * zfar * znear / ( znear - zfar );
  m( 3, 2 ) = -1;
}

void Transform::BuildOrthoMatrix( Matrix& m, double left, double right, double bottom, 
  double top, double nearVal, double farVal )
{
  m = Matrix::ZERO_C;
  double width = right - left;
  double height = top - bottom;
  double depth = farVal - nearVal;
  m( 0, 0 ) = 2 / width;
  m( 0, 3 ) = -( right + left ) / width;
  m( 1, 1 ) = 2 / height;
  m( 1, 3 ) = -( top + bottom ) / height;
  m( 2, 2 ) = -2 / depth;
  m( 2, 3 ) = -( farVal + nearVal ) / depth;
  m( 3, 3 ) = 1;
}

void Transform::BuildOrtho2DMatrix( Matrix& m, double left, double right, 
  double bottom, double top )
{
  Transform::BuildOrthoMatrix( m, left, right, bottom, top, -1, 1 );
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

} // namespace Core
