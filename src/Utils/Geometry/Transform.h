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
#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Plane.h>

#include <Utils/Math/MathFunctions.h>

namespace Utils {

class Transform
{
  public:
    Transform();
    Transform(const Transform&);
    Transform(const Point&, const Vector&, const Vector&, const Vector&);
    Transform(const std::vector<double>& values);

    Transform& operator=(const Transform& copy);

    void load_basis(const Point&,const Vector&, const Vector&, const Vector&);
    void load_frame(const Vector&, const Vector&, const Vector&);

    void post_trans(const Transform&);
    void pre_trans(const Transform&);
      
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

    Point   unproject(const Point& p) const;
    Vector  unproject(const Vector& p) const;
    PointF  unproject(const PointF& p) const;
    VectorF unproject(const VectorF& p) const;

    Point   project(const Point& p) const;
    Vector  project(const Vector& p) const;
    PointF  project(const PointF& p) const;
    VectorF project(const VectorF& p) const;
    Vector  project_normal(const Vector&) const;
    VectorF project_normal(const VectorF&) const;
 
    void get(double*) const;
    void get_trans(double*) const;
    void set(double*);
    void set_trans(double*);
    
    void invert();
    void load_identity();
    void perspective(const Point& eyep, const Point& lookat,
         const Vector& up, double fov, double znear, double zfar,
         int xres, int yres);
         
    friend Point   operator*(Transform &t, const Point &d);
    friend Vector  operator*(Transform &t, const Vector &d); 
    friend PointF  operator*(Transform &t, const PointF &d);
    friend VectorF operator*(Transform &t, const VectorF &d); 

  private:

    void install_mat(double[4][4]);
    void build_permute(double m[4][4], int, int, int, bool pre);
    void build_rotate(double m[4][4], double, const Vector&);
    void build_shear(double mat[4][4], const Vector&, const Plane&);
    void build_scale(double m[4][4], const Vector&);
    void build_translate(double m[4][4], const Vector&);
    void pre_mulmat(const double[4][4]);
    void post_mulmat(const double[4][4]);
    void load_identity(double[4][4]);

    void compute_imat();

    double mat_[4][4];
    double imat_[4][4];
};

Point operator*(Transform &t, const Point &d);
Vector operator*(Transform &t, const Vector &d);
PointF operator*(Transform &t, const PointF &d);
VectorF operator*(Transform &t, const VectorF &d);

} // End namespace Utils

#endif
