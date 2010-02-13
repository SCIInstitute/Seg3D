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

// STL includes
#include <numeric>

// Utils includes
#include <Utils/Geometry/Vector.h>
#include <Utils/Geometry/Quaternion.h>
#include <Utils/Math/MathFunctions.h>
#include <Utils/Geometry/Matrix.h>

namespace Utils {

const double Quaternion::NUMERIC_EPSILON = std::numeric_limits<double>::epsilon()*10.0;

Quaternion::Quaternion(const Vector& axis, double angle)
{
  Vector tmp = axis;
  double len = tmp.normalize();
  if (len > NUMERIC_EPSILON)
  {
    double half_angle = angle * 0.5;
    double sin_half_angle = Sin(half_angle);
    w_ = Cos(half_angle);
    x_ = tmp[0] * sin_half_angle;
    y_ = tmp[1] * sin_half_angle;
    z_ = tmp[2] * sin_half_angle;    
  }
  else
  {
    w_ = 1;
    x_ = y_ = z_ = 0.0;
  }
}

Quaternion::Quaternion(const Matrix& matrix)
{
  from_matrix(matrix);
}

void 
Quaternion::to_matrix(Matrix& matrix) const
{
  double x2 = x_ + x_;
  double y2 = y_ + y_;
  double z2 = z_ + z_;
  
  double xx = x_ * x2;
  double xy = x_ * y2;
  double xz = x_ * z2;
  double xw = x2 * w_;
  
  double yy = y_ * y2;
  double yz = y_ * z2;
  double yw = y2 * w_;
  
  double zz = z_ * z2;
  double zw = z2 * w_;
  
  matrix(0, 0) = 1.0 - (yy + zz);
  matrix(0, 1) = xy - zw;
  matrix(0, 2) = xz + yw;
  
  matrix(1, 0) = xy + zw;
  matrix(1, 1) = 1.0 - (xx + zz);
  matrix(1, 2) = yz - xw;
  
  matrix(2, 0) = xz - yw;
  matrix(2, 1) = yz + xw;
  matrix(2, 2) = 1.0 - (xx + yy);
  
  matrix(0, 3) = matrix(1, 3) = matrix(2, 3) 
    = matrix(3, 0) = matrix(3, 1) = matrix(3, 2) = 0.0;
  matrix(3, 3) = 1.0;
}

void 
Quaternion::from_matrix(const Matrix& matrix)
{
  double trace = matrix(0, 0) + matrix(1, 1) + matrix(2, 2);

  if (trace > 0)
  {
    double s = 0.5 / Sqrt(trace + 1.0);
    w_ = 0.25 / s;
    x_ = (matrix(2, 1) - matrix(1, 2)) * s;
    y_ = (matrix(0, 2) - matrix(2, 0)) * s;
    z_ = (matrix(1, 0) - matrix(0, 1)) * s;
  }
  else if (matrix(0, 0) > matrix(1, 1) && matrix(0, 0) > matrix(2, 2))
  {
    double s = Sqrt(1.0 + matrix(0, 0) - matrix(1, 1) - matrix(2, 2));
    x_ = 0.5 * s;
    s = 0.5 / s;
    w_ = (matrix(2, 1) - matrix(1, 2)) * s;
    y_ = (matrix(1, 0) + matrix(0, 1)) * s;
    z_ = (matrix(0, 2) + matrix(2, 0)) * s;
  }
  else if (matrix(1, 1) > matrix(2, 2))
  {
    double s = Sqrt(1.0 + matrix(1, 1) - matrix(0, 0) - matrix(2, 2));
    y_ = 0.5 * s;
    s = 0.5 / s;
    w_ = (matrix(0, 2) - matrix(2, 0)) * s;
    x_ = (matrix(0, 1) + matrix(1, 0)) * s;
    z_ = (matrix(1, 2) + matrix(2, 1)) * s;
  } 
  else
  {
    double s = Sqrt(1.0 + matrix(2, 2) - matrix(0, 0) - matrix(1, 1));
    z_ = 0.5 * s;
    s = 0.5 / s;
    w_ = (matrix(1, 0) - matrix(0, 1)) * s;
    x_ = (matrix(0, 2) + matrix(2, 0)) * s;
    y_ = (matrix(1, 2) + matrix(2, 1)) * s;
  }
  
  normalize();
}

Vector 
Quaternion::rotate(const Vector& vec) const
{
  Matrix matrix;
  to_matrix(matrix);
    
  Vector result;
  result[0] = matrix(0, 0) * vec[0] + matrix(0, 1) * vec[1] + matrix(0, 2) * vec[2];
  result[1] = matrix(1, 0) * vec[0] + matrix(1, 1) * vec[1] + matrix(1, 2) * vec[2];
  result[2] = matrix(2, 0) * vec[0] + matrix(2, 1) * vec[1] + matrix(2, 2) * vec[2];
  
  return result;
}

Quaternion 
Slerp(const Quaternion& from, const Quaternion& to, double t, bool shortest_path)
{
  Quaternion result;
  
  const Quaternion& p = from;
  Quaternion q;
  
  double cos_theta = from.w_*to.w_ + from.x_*to.x_ + from.y_*to.y_ + from.z_*to.z_;
  if (shortest_path && cos_theta < 0.0)
  {
    cos_theta = -cos_theta;
    q.w_ = -to.w_;
    q.x_ = -to.x_;
    q.y_ = -to.y_;
    q.z_ = -to.z_;
  } 
  else
  {
    q = to;
  }
  
  double ratio_p, ratio_q;
  if (1.0 - cos_theta > 0.0001)
  {
    double theta = Acos(cos_theta);
    double sin_theta = Sin(theta);
    ratio_p = Sin((1.0 -t)*theta) / sin_theta;
    ratio_q = Sin(t*theta) / sin_theta;
  }
  else
  {
    // very close, do linear interpolation
    ratio_p = 1.0 - t;
    ratio_q = t;
  }
  
  result.w_ = from.w_*ratio_p + to.w_*ratio_q;
  result.x_ = from.x_*ratio_p + to.x_*ratio_q;
  result.y_ = from.y_*ratio_p + to.y_*ratio_q;
  result.z_ = from.z_*ratio_p + to.z_*ratio_q;
  
  return result;
}

} // end namespace Utils
