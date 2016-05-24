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

#ifndef CORE_GEOMETRY_VIEW2D_H
#define CORE_GEOMETRY_VIEW2D_H

#include <Core/Utils/EnumClass.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>

namespace Core
{

// Forward declaration
class View2D;

class View2D
{
public:

  View2D();
  View2D( const Point& center, double scale );
  View2D( const Point& center, double scalex, double scaley );

  ~View2D();

  View2D( const View2D& );
  View2D& operator=( const View2D& );

  // compare 2 views; are they exactly the same?
  bool operator==( const View2D& ) const;
  bool operator!=( const View2D& ) const;

  inline const Point& center() const
  {
    return this->center_;
  }

  inline Point& center()
  {
    return this->center_;
  }

  inline double scalex() const
  {
    return scalex_;
  }

  inline void scalex( double scalex )
  {
    scalex_ = scalex;
  }

  inline double scaley() const
  {
    return scaley_;
  }

  inline void scaley( double scaley )
  {
    scaley_ = scaley;
  }

  void scale( double ratio );
  void translate( const Vector& offset );
  void dolly( double dz );
  void compute_clipping_planes( double aspect, double& left, double& right, 
    double& bottom, double& top ) const;

private:
  // Center point
  Point center_;

  // Size of slice displayed in x direction (if negative the axis needs to be flipped)
  double scalex_;

  // Size of slice displayed in y direction (if negative the axis needs to be flipped)
  double scaley_;

};

std::string ExportToString( const View2D& value );
bool ImportFromString( const std::string& str, View2D& value );

} // End namespace Core

#endif
