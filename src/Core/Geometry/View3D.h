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

#ifndef CORE_GEOMETRY_VIEW3D_H
#define CORE_GEOMETRY_VIEW3D_H

#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>

namespace Core
{

// Forward declaration
class View3D;
class Quaternion;

class View3D
{
public:
  View3D();
  View3D( const Point& eyep, const Point& lookat, const Vector& up, double fov );
  ~View3D();

  View3D( const View3D& );
  View3D& operator=( const View3D& );

  // compare 2 views; are they exactly the same?
  bool operator==( const View3D& ) const;
  bool operator!=( const View3D& ) const;

  inline Point eyep() const
  {
    return this->eyep_;
  }
  inline void eyep( const Point& eyep )
  {
    this->eyep_ = eyep;
  }

  inline Point lookat() const
  {
    return this->lookat_;
  }
  inline void lookat( const Point& lookat )
  {
    this->lookat_ = lookat;
  }

  inline Vector up() const
  {
    return this->up_;
  }
  inline void up( const Vector& up )
  {
    this->up_ = up;
  }

  inline double fov() const
  {
    return this->fovy_;
  }
  inline void fov( double fov )
  {
    this->fovy_ = fov;
  }

  void rotate( const Vector& axis, double angle );
  void scale( double ratio );
  void translate( const Vector& offset );
  void compute_clipping_planes( const BBox& bbox, double& znear, double& zfar );

private:

  // Eye point
  Point eyep_;
  // Look at this point in space
  Point lookat_;
  // Vector pointing to the up position
  Vector up_;
  // Field of view in y direction
  double fovy_;

};

std::string ExportToString( const View3D& value );
bool ImportFromString( const std::string& str, View3D& value );

} // End namespace Core

#endif
