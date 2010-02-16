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

#ifndef UTILS_GEOMETRY_VIEW2D_H
#define UTILS_GEOMETRY_VIEW2D_H

#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>

namespace Utils {

// Forward declaration
class View2D;

class View2D {
  public:
    View2D();
    View2D(const Point& center, double scale);
    View2D(const Point& center, double scalex, double scaley);

    ~View2D();

    View2D(const View2D&);
    View2D& operator=(const View2D&);

    // compare 2 views; are they exactly the same?
    bool operator==(const View2D&) const;
    bool operator!=(const View2D&) const;

    inline Point center() const                { return center_; }
    inline void center(const Point& center)    { center_ = center; }

    inline double scalex() const                { return scalex_; }
    inline void   scalex(double scalex)         { scalex_ = scalex; }

    inline double scaley() const                { return scaley_; }
    inline void   scaley(double scaley)         { scaley_ = scaley; }
    
  private:
    // Center point
    Point  center_;
    
    // Size of slice displayed in x direction (if negative the axis needs to be flipped)
    double scalex_;

    // Size of slice displayed in y direction (if negative the axis needs to be flipped)
    double scaley_;
    };

} // End namespace Utils

#endif
