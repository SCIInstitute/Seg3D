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


#ifndef UTILS_GEOMETRY_TRANSFORM_H
#define UTILS_GEOMETRY_TRANSFORM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <ostream>

// Utils includes
#include <Utils/Geometry/Matrix.h>
#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Plane.h>

#include <Utils/Math/MathFunctions.h>

namespace Utils {

//Forward declaration
class Transform;

// Class definition
class Transform
{
  public:
    Transform();
    Transform(const Transform&);
    Transform(const Point&, const Vector&, const Vector&, const Vector&);

    Transform& operator=(const Transform& copy);

    void load_identity();
    void load_basis(const Point&,const Vector&, const Vector&, const Vector&);
    void load_frame(const Vector&, const Vector&, const Vector&);
    Transform invert();

    void post_transform(const Transform&);
    void pre_transform(const Transform&);

    void post_mult_matrix(const Matrix& m);
    void pre_mult_matrix(const Matrix& m);

    void pre_permute(int xmap, int ymap, int zmap);
    void post_permute(int xmap, int ymap, int zmap);

    void pre_scale(const Vector&);
    void post_scale(const Vector&);

    void pre_shear(const Vector&, const Plane&);
    void post_shear(const Vector&, const Plane&);

    void pre_rotate(double, const Vector& axis);
    void post_rotate(double, const Vector& axis);

    void pre_translate(const Vector&);
    void post_translate(const Vector&);

    // Returns true if the rotation happened, false otherwise.
    bool rotate(const Vector& from, const Vector& to);

    void perspective(const Point& eyep, const Point& lookat, const Vector& up, 
                  double fovy, double znear, double zfar, double aspect);

    const Matrix& get_matrix() const;

    void get(double* data) const;
    void set(const double* data);

    Point   project(const Point& p) const;
    Vector  project(const Vector& p) const;
    PointF  project(const PointF& p) const;
    VectorF project(const VectorF& p) const;

    bool operator==(const Transform&) const;
    bool operator!=(const Transform&) const;

  public:

    static void BuildPermute(Matrix& m, int xmap, int ymap, int zmap, bool pre);
    static void BuildRotate(Matrix& m, double angle, const Vector& axis);
    static void BuildShear(Matrix& m, const Vector& s, const Plane& p);
    static void BuildScale(Matrix& m, const Vector& v);
    static void BuildTranslate(Matrix& m, const Vector& v);

  protected:
    Matrix mat_;
};

Point operator*(const Transform& t, const Point& d);
Vector operator*(const Transform& t, const Vector& d);
PointF operator*(const Transform& t, const PointF& d);
VectorF operator*(const Transform& t, const VectorF& d);

} // End namespace Utils

#endif
