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

// Utils includes
#include <Utils/Math/MathFunctions.h>

// Application includes
#include <Application/Viewer/Trackball.h>

namespace Seg3D {

Trackball::Trackball() :
  width_(0), height_(0), 
  invert_y_(true), camera_mode_(true)
{
}

Trackball::~Trackball()
{
}

Utils::Quaternion 
Trackball::map_mouse_move_to_rotation(int x0, int y0, int x1, int y1)
{
  Utils::Vector v0 = project_point_on_sphere(x0, y0);
  Utils::Vector v1 = project_point_on_sphere(x1, y1);
  
  Utils::Vector axis = Utils::Cross(v0, v1);
  double angle = Utils::Acos(Dot(v0, v1));
  
  // negate the angle for camera rotation
  if (camera_mode_)
  {
    angle = -angle;
  }
  
  return Utils::Quaternion(axis, angle);
}

Utils::Vector 
Trackball::project_point_on_sphere( int x, int y )
{
  Utils::Vector v(x*2.0/width_-1.0, (invert_y_? (height_-y) : y)*2.0/height_-1.0, 0.0);
  double len2 = v.length2();
  v[2] = len2 >= 1.0 ? 0.0 : Utils::Sqrt(1.0 - len2);
  v.normalize();
  
  return v;
}



} // End namespace Seg3D