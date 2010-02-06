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

#ifndef UTILS_GEOMETRY_VIEW3D_H
#define UTILS_GEOMETRY_VIEW3D_H

#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>

namespace Utils {

// Forward declaration
class View3D;
class Quaternion;

class View3D {
  public:
    View3D();
    View3D(const Point&, const Point&, const Vector&, double);
    ~View3D();

    View3D(const View3D&);
    View3D& operator=(const View3D&);
    
    // compare 2 views; are they exactly the same?
    bool operator==(const View3D&);
    bool operator!=(const View3D&);

    inline Point eyep() const                { return eyep_; }
    inline void eyep(const Point& eyep)      { eyep_ = eyep; }

    inline Point lookat() const              { return lookat_; }
    inline void lookat(const Point& lookat)  { lookat_ = lookat; }

    inline Vector up() const                 { return up_; }
    inline void up(const Vector& up)         { up_ = up;}

    inline double fov() const                { return fov_; }
    inline void fov(double fov)              { fov_ = fov; }
    
    void rotate(const Quaternion& rotation);
    
  private:
    // Eye point
    Point                    eyep_;
    // Look at this point in space
    Point                    lookat_;
    // Vector pointing to the up position
    Vector                   up_;
    // Field of view
    double                   fov_;
};

} // End namespace Utils

#endif
