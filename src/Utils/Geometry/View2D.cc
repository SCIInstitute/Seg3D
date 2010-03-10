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

#include <Utils/Geometry/View2D.h>
#include <Utils/Math/MathFunctions.h>

namespace Utils {

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

bool
View2D::operator==(const View2D& copy) const
{
  return (this->center_ == copy.center_ && this->scalex_ == copy.scalex_ 
    && this->scaley_ == copy.scaley_); 
}

bool
View2D::operator!=(const View2D& copy) const
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

void View2D::flip(FlipDirectionType direction)
{
  if (direction == FlipDirectionType::HORIZONTAL_E)
  {
    this->scalex_ = -this->scalex_;
  }
  else
  {
    this->scaley_ = -this->scaley_;
  }
}

} // End namespace Utils
