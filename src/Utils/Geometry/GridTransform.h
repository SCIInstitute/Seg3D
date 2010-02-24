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


#ifndef UTILS_GEOMETRY_GRIDTRANSFORM_H
#define UTILS_GEOMETRY_GRIDTRANSFORM_H

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
#include <Utils/Geometry/Transform.h>

#include <Utils/Math/MathFunctions.h>

namespace Utils {

class GridTransform : public Transform
{
// -- constuctors --
  public:
    GridTransform();
    
    GridTransform(const GridTransform&);
    
    GridTransform(size_t nx, size_t ny, size_t nz,
                  const Point&, const Vector&, 
                  const Vector&, const Vector&);

    GridTransform(size_t nx, size_t ny, size_t nz,
                  const Transform& transform);

    GridTransform(size_t nx, size_t ny, size_t nz);

    GridTransform& operator=(const GridTransform& copy);

// -- comparison --
  public:
    bool operator==(const GridTransform&) const;
    bool operator!=(const GridTransform&) const;
  
// -- accessors --
  public:
    size_t nx() const { return nx_; }
    size_t ny() const { return ny_; }
    size_t nz() const { return nz_; }
  
    void nx(size_t nx) { nx_ = nx; }
    void ny(size_t ny) { ny_ = ny; }
    void nz(size_t nz) { nz_ = nz; }
  
    Transform transform() const;

// -- internal representation --  
  private:
    
    // The dimensions of the grid
    size_t    nx_;
    size_t    ny_;
    size_t    nz_;
};

Point operator*(const GridTransform& gt, const Point& d);
Vector operator*(const GridTransform& gt, const Vector& d);
PointF operator*(const GridTransform& gt, const PointF& d);
VectorF operator*(const GridTransform& gt, const VectorF& d);

} // End namespace Utils

#endif
