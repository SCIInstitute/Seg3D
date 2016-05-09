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

#include <Core/Geometry/Matrix.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/View3D.h>
#include <Core/Geometry/Quaternion.h>
#include <Core/Math/MathFunctions.h>

namespace Core
{

View3D::View3D() :
  eyep_( 0, 0, 1 ), lookat_( 0, 0, 0 ), up_( 0, 1, 0 ), fovy_( 60.0 )
{
}

View3D::~View3D()
{
}

View3D::View3D( const Point& eyep, const Point& lookat, const Vector& up, double fov ) :
  eyep_( eyep ), lookat_( lookat ), up_( up ), fovy_( fov )
{
}

View3D::View3D( const View3D& copy ) :
  eyep_( copy.eyep_ ), lookat_( copy.lookat_ ), up_( copy.up_ ), fovy_( copy.fovy_ )
{
}

View3D& View3D::operator=( const View3D& copy )
{
  this->eyep_ = copy.eyep_;
  this->lookat_ = copy.lookat_;
  this->up_ = copy.up_;
  this->fovy_ = copy.fovy_;

  return *this;
}

bool View3D::operator==( const View3D& copy ) const
{
  return ( this->eyep_ == copy.eyep_ && this->lookat_ == copy.lookat_ && this->up_ == copy.up_
      && this->fovy_ == copy.fovy_ );
}

bool View3D::operator!=( const View3D& copy ) const
{
  return ( this->eyep_ != copy.eyep_ || this->lookat_ != copy.lookat_ || this->up_ != copy.up_
      || this->fovy_ != copy.fovy_ );
}

void View3D::rotate( const Vector& axis, double angle )
{
  Vector z( this->eyep_ - this->lookat_ );
  double eye_distance = z.normalize();
  Vector x( Cross( this->up_, z ) );
  x.normalize();
  Vector y( Cross( z, x ) );

  // Convert the quaternion to matrix
  // NOTE: Invert the angle for rotating the eye
  Quaternion quat( axis, DegreeToRadian( -angle ) );
  Matrix mat;
  quat.to_matrix( mat );

  Transform view_trans;
  view_trans.load_frame( x, y, z );
  view_trans.post_mult_matrix( mat );
  const Matrix& view_mat = view_trans.get_matrix();

  this->up_ = Vector( view_mat( 0, 1 ), view_mat( 1, 1 ), view_mat( 2, 1 ) );
  this->eyep_ = this->lookat_ + eye_distance * Vector( view_mat( 0, 2 ), view_mat( 1, 2 ),
      view_mat( 2, 2 ) );
}

void View3D::scale( double ratio )
{
  this->eyep_ = this->lookat_ + ( this->eyep_ - this->lookat_ ) / ratio;
}

void View3D::translate( const Vector& offset )
{
  this->eyep_ -= offset;
  this->lookat_ -= offset;
}

void View3D::compute_clipping_planes( const BBox& bbox, double& znear, double& zfar )
{
  Vector eye_vec = this->lookat_ - this->eyep_;
  eye_vec.normalize();
  
  Point cmin = bbox.min();
  Point cmax = bbox.max();

  Point pt = cmin;
  znear = zfar = Dot( eye_vec, pt - this->eyep_ );
  pt = cmax;
  double dist = Dot( eye_vec, pt - this->eyep_ );
  znear = Min( znear, dist );
  zfar = Max( zfar, dist );

  for ( int i = 0; i < 3; i++ )
  {
    pt = cmin;
    pt[ i ] = cmax[ i ];
    dist = Dot( eye_vec, pt - this->eyep_ );
    znear = Min( znear, dist );
    zfar = Max( zfar, dist );

    pt = cmax;
    pt[ i ] = cmin[ i ];
    dist = Dot( eye_vec, pt - this->eyep_ );
    znear = Min( znear, dist );
    zfar = Max( zfar, dist );
  }

  // Offset the clipping planes by a small value
  double range = Core::Max( znear, zfar - znear );
  znear -= range * 0.001;
  zfar += range * 0.001;
  znear = Core::Max( znear, 1e-2 * zfar );
}

std::string ExportToString( const View3D& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.eyep() ) + ',' + ExportToString(
      value.lookat() ) + ',' + ExportToString( value.up() ) + ',' + ExportToString(
      value.fov() ) + ']' );
}

bool ImportFromString( const std::string& str, View3D& value )
{
  std::vector< double > values;
  ImportFromString( str, values );
  if ( values.size() == 10 )
  {
    value
        = View3D( Point( values[ 0 ], values[ 1 ], values[ 2 ] ), Point( values[ 3 ],
            values[ 4 ], values[ 5 ] ), Vector( values[ 6 ], values[ 7 ], values[ 8 ] ),
            values[ 9 ] );
    return ( true );
  }
  return ( false );
}

} // End namespace Core
