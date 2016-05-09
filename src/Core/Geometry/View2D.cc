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

#include <Core/Geometry/View2D.h>
#include <Core/Math/MathFunctions.h>

namespace Core {

View2D::View2D() :
  center_(0, 0, 0), scalex_(1.0), scaley_(1.0)
{
}

View2D::~View2D()
{
}

View2D::View2D(const Point& center, double scale) :
  center_(center),
  scalex_(scale),
  scaley_(scale)
{
}

View2D::View2D(const Point& center, double scalex, double scaley) :
  center_(center),
  scalex_(scalex),
  scaley_(scaley)
{
}

View2D::View2D(const View2D& copy) : 
  center_(copy.center_), 
  scalex_(copy.scalex_), 
  scaley_(copy.scaley_) 
{
}

View2D& View2D::operator=(const View2D& copy)
{
  this->center_ = copy.center_;
  this->scalex_  = copy.scalex_;
  this->scaley_  = copy.scaley_;
  return *this;
}

bool View2D::operator==(const View2D& copy) const
{
  return (this->center_ == copy.center_ && this->scalex_ == copy.scalex_ 
    && this->scaley_ == copy.scaley_); 
}

bool View2D::operator!=(const View2D& copy) const
{
  return (this->center_ != copy.center_ || this->scalex_ != copy.scalex_ ||
    this->scaley_ != copy.scaley_); 
}

void View2D::scale( double ratio )
{
  this->scalex_ *= ratio;
  this->scaley_ *= ratio;
}

void View2D::translate( const Vector& offset )
{
  double delta_x = offset[0] / this->scalex_;
  double delta_y = offset[1] / this->scaley_;
  this->center_[0] -= delta_x;
  this->center_[1] -= delta_y;
}
  
void View2D::dolly( double dz )
{
  this->center_[ 2 ] += dz;
}

void View2D::compute_clipping_planes( double aspect, double& left, double& right, 
  double& bottom, double& top ) const
{
  double clipping_width = aspect / this->scalex_ * 0.5;
  double clipping_height = 1.0 / this->scaley_ * 0.5;
  left = this->center_.x() - clipping_width;
  right = this->center_.x() + clipping_width;
  bottom = this->center_.y() - clipping_height;
  top = this->center_.y() + clipping_height;
}

std::string ExportToString( const View2D& value )
{
  return ( std::string( 1, '[' ) + ExportToString( value.center() ) + ',' + ExportToString(
      value.scalex() ) + ',' + ExportToString( value.scaley() ) + ']' );
}

bool ImportFromString( const std::string& str, View2D& value )
{
  std::vector< double > values;
  ImportFromString( str, values );
  if ( values.size() == 5 )
  {
    value = View2D( Point( values[ 0 ], values[ 1 ], values[ 2 ] ), values[ 3 ], values[ 4 ] );
    return ( true );
  }
  return ( false );
}

} // End namespace Core
