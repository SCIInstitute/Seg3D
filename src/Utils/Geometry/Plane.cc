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

#include <Utils/Geometry/Plane.h>

namespace Utils {

Plane::Plane()
  : norm_(Vector(0.0,0.0,1.0)), dist_(0.0)
{
}

Plane::Plane(const Vector& normal, double distance)
  : norm_(normal), dist_(distance)
{
}

Plane::Plane(double a, double b, double c, double d) : 
  norm_(Vector(a,b,c).normal()), 
  dist_(d/norm_.length()) 
{
}

Plane::Plane(const Point &p, const Vector &normal)
  : norm_(normal), dist_(-Dot(p, normal))
{
}

Plane::Plane(const Plane &copy)
  : norm_(copy.norm_), dist_(copy.dist_)
{
}

Plane::Plane(const Point &p1, const Point &p2, const Point &p3) :
  norm_(Cross(p2-p1,p2-p3).normal()),
  dist_(-Dot(p1, norm_))
{
}

}

