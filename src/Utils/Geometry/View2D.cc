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
  center_(0, 0, 0), scalex_(1.0), scaley_(1.0),
  width_(0), height_(0)
{
}

View2D::~View2D()
{
}

View2D::View2D(const Point& center, double scale) :
  center_(center),
  scalex_(scale),
  scaley_(scale),
  width_(0), height_(0)
{
}

View2D::View2D(const Point& center, double scalex, double scaley) :
  center_(center),
  scalex_(scalex),
  scaley_(scaley), width_(0), height_(0)
{
}

View2D::View2D(const View2D& copy) : 
  center_(copy.center_), 
  scalex_(copy.scalex_), 
  scaley_(copy.scaley_) ,
  width_(copy.width_),
  height_(copy.height_)
{
}

View2D& View2D::operator=(const View2D& copy)
{
  this->center_ = copy.center_;
  this->scalex_  = copy.scalex_;
  this->scaley_  = copy.scaley_;
  this->width_ = copy.width_;
  this->height_ = copy.height_;
  return *this;
}

bool
View2D::operator==(const View2D& copy) const
{
  return (this->center_ == copy.center_ && this->scalex_ == copy.scalex_ 
    && this->scaley_ == copy.scaley_ && this->width_ == copy.width_ 
    && this->height_ == copy.height_); 
}

bool
View2D::operator!=(const View2D& copy) const
{
  return (this->center_ != copy.center_ || this->scalex_ != copy.scalex_ ||
    this->scaley_ != copy.scaley_ || this->width_ != copy.width_ ||
    this->height_ != copy.height_); 
}

void View2D::scale( double ratio )
{
  this->scalex_ *= ratio;
  this->scaley_ *= ratio;
}

void View2D::translate( const Vector& offset )
{
  double delta_x = offset[0] / this->width_ / this->scalex_;
  double delta_y = offset[1] / this->height_ / this->scaley_;
  this->center_[0] += delta_x;
  this->center_[1] += delta_y;
}

void View2D::resize( int width, int height )
{
  this->width_ = width;
  this->height_ = height;
}

void View2D::get_clipping_planes( double& left, double& right, double& bottom, double& top ) const
{
  double dimension = Min(this->width_, this->height_);
  double clipping_width = this->width_ / dimension / this->scalex_ * 0.5;
  double clipping_height = this->height_ / dimension / this->scaley_ * 0.5;
  left = this->center_.x() - clipping_width;
  right = this->center_.x() + clipping_width;
  bottom = this->center_.y() - clipping_height;
  top = this->center_.y() + clipping_height;
}

} // End namespace Utils
