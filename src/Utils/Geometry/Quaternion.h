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

#ifndef UTILS_GEOMETRY_QUATERNION_H
#define UTILS_GEOMETRY_QUATERNION_H

// Utils includes
#include <Utils/Math/MathFunctions.h>

namespace Utils {

class Vector;

// CLASS QUATERNION
//
// NOTE: Class Quaternion will always keep itself normalized, since we are 
// using Quaternion for rotation only. Similarly, only functions and operators necessary 
// for performing rotations are implemented.

class Quaternion {

  public:
    
    // default constructor, makes an identity quaternion
    inline Quaternion();
    
    inline Quaternion(double w, double x, double y, double z);
    
    // copy constructor
    inline Quaternion(const Quaternion& src);
    
    //inline explicit Quaternion(const Vector& vec);
    
    // construct a quaternion from the given axis and angle of rotation
    Quaternion(const Vector& axis, double angle);
    
    // construct a quaternion from the given rotation transformation
    // NOTE: if the matrix is not pure rotation, the result is undefined
    // NOTE: the matrix should be column major
    explicit Quaternion(const double matrix[4][4]);
    
    inline Quaternion get_conjugate() const;
    inline void conjugate();
    
    // convert the quaternion to a column major matrix
    void to_matrix(double matrix[4][4]) const;
    
    void from_matrix(const double matrix[4][4]);
    
    Vector rotate(const Vector& vec) const;
    
    inline Quaternion operator*(const Quaternion& rhs) const;
    inline Quaternion& operator*=(const Quaternion& rhs);
        
    inline bool operator==(const Quaternion& rhs) const;
    
    inline Quaternion& operator=(const Quaternion& rhs);
    
  private:
  
    friend Quaternion Slerp(const Quaternion&, const Quaternion&, double, bool);

    inline double normalize();
    inline double norm() const;
    
    // the real part
    double w_;
    
    // the imaginary part
    double x_;
    double y_;
    double z_;
    
    // NUMERIC_ZERO:
    // any value less than or equal to it will be considered zero
    static const double NUMERIC_EPSILON;
};

inline 
Quaternion::Quaternion() :
  w_(1), x_(0), y_(0), z_(0)
{
}

inline
Quaternion::Quaternion(double w, double x, double y, double z) :
  w_(w), x_(x), y_(y), z_(z)
{
  normalize();
}

inline 
Quaternion::Quaternion(const Quaternion &src) :
  w_(src.w_), x_(src.x_), y_(src.y_), z_(src.z_)
{
}

inline double 
Quaternion::normalize()
{
  double len = norm();
  if (len > NUMERIC_EPSILON)
  {
    double s = 1.0/len;
    w_ *= s;
    x_ *= s;
    y_ *= s;
    z_ *= s;
  }
  else
  {
    len = 0.0;
    w_ = 1;
    x_ = y_ = z_ = 0.0;
  }
  
  return len;
}

inline double
Quaternion::norm() const
{
  return Sqrt(w_*w_ + x_*x_ + y_*y_ + z_*z_);
}

inline Quaternion
Quaternion::get_conjugate() const
{
  Quaternion res;
  res.w_ = w_;
  res.x_ = -x_;
  res.y_ = -y_;
  res.z_ = -z_;

  return res;
}

inline void
Quaternion::conjugate()
{
  x_ = -x_;
  y_ = -y_;
  z_ = -z_;
}

inline Quaternion 
Quaternion::operator*(const Quaternion& rhs) const
{
  Quaternion result;
  result.x_ = x_*rhs.w_ + y_*rhs.z_ - z_*rhs.y_ + w_*rhs.x_;
  result.y_ = -x_*rhs.z_ + y_*rhs.w_ + z_*rhs.x_ + w_*rhs.y_;
  result.z_ = x_*rhs.y_ - y_*rhs.x_ + z_*rhs.w_ + w_*rhs.z_;
  result.w_ = -x_*rhs.x_ - y_*rhs.y_ - z_*rhs.z_ + w_*rhs.w_;
  
  return result;
}

inline Quaternion& 
Quaternion::operator*=(const Quaternion& rhs)
{
  *this = (*this) * rhs;
  
  return (*this);
}

inline bool 
Quaternion::operator==(const Quaternion& rhs) const
{
  return (w_==rhs.w_ && x_==rhs.x_ && y_==rhs.y_ && z_==rhs.z_);
}

inline Quaternion& 
Quaternion::operator=(const Quaternion& rhs)
{
  w_ = rhs.w_;
  x_ = rhs.x_;
  y_ = rhs.y_;
  z_ = rhs.z_;
  
  return (*this);
}

// Spherical linear interpolation between two quaternions
Quaternion Slerp(const Quaternion& from, const Quaternion& to, double t, bool shortest_path = true);

} // end namespace Utils

#endif