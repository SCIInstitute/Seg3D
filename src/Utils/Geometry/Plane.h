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

#ifndef UTILS_GEOMETRY_PLANE_H
#define UTILS_GEOMETRY_PLANE_H

#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>

namespace Utils {

class Plane {
    
  public:
    Plane();
    Plane(double a, double b, double c, double d);
    Plane(const Point &p1, const Point &p2, const Point &p3);
    Plane(const Point &p, const Vector &n);
    Plane(const Vector& normal, double distance);

    Plane(const Plane &copy);
    
    double distance(const Point &p) const 
      { return Dot(p, norm_)+dist_; }
    float  distance(const PointF &p) const 
      { return Dot(p, VectorF(norm_))+static_cast<float>(dist_); }

    void flip() { norm_ = -norm_; dist_ = -dist_; }

    Point  project(const Point& p) const 
      { return p-(norm_*(dist_+Dot(p,norm_))); }
    Vector project(const Vector& v) const 
      { return v-(norm_*Dot(v,norm_)); }

    PointF  project(const PointF& p) const 
      { return PointF(p-norm_*(dist_+Dot(Vector(p),norm_))); }
    VectorF project(const VectorF& v) const 
      { return v-VectorF(norm_*Dot(Vector(v),norm_)); }

    Vector normal() const { return norm_; }
    double distance() const { return dist_; }

  private:
    Vector norm_;
    double dist_;

};

} // End namespace SCIRun


#endif
