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

#ifndef UTILS_GEOMETRY_POINT_H
#define UTILS_GEOMETRY_POINT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <vector>
#include <iosfwd>

// Utils includes
#include <Utils/Math/MathFunctions.h>

namespace Utils {

// NOTE: Vector and Point are two distinct classes to help formulate correct
// geometric expressions. 

// NOTE: The double version of Point and Vector are defined separately. As Vector
// and Point are dependent on each other, a templated version is limitted by the
// need to include templated member function, and templated member functions
// current can only be defined inside the template (a limitation of the 
// C++ language). Hence to keep code readable and organized we have two versions
// of each.

// NOTE: The need for double versions of Vector and Point stem from memory 
// concerns. A lot of the geometry is defined as a large amount of points
// without the need for double precision. Doing this in double precision is
// just a waist of memory resources. Hence for those cases we define a VectorF
// and PointF class.

class Vector;
class Point;
class VectorF;
class PointF;

class Point {

  private:
    friend class PointF;
    friend class Vector;
    friend class VectorF;

    double data_[3];

  public:
    inline Point();
    inline Point(double, double, double);
    inline Point(double, double, double, double);

    inline explicit Point(const PointF& p);
    inline explicit Point(const Vector& v);
    inline explicit Point(const VectorF& v);
    inline Point(const Point&);

    inline bool operator==(const Point&) const;
    inline bool operator!=(const Point&) const;
    inline bool operator==(const PointF&) const;
    inline bool operator!=(const PointF&) const;

    inline Point& operator=(const Point&);
    inline Point& operator=(const PointF&);

    inline Vector operator+(const Point&) const;
    inline Vector operator-(const Point&) const;
    inline Vector operator+(const PointF&) const;
    inline Vector operator-(const PointF&) const;

    inline Point operator+(const Vector&) const;
    inline Point operator-(const Vector&) const;
    inline Point operator+(const VectorF&) const;
    inline Point operator-(const VectorF&) const;
    inline Point operator*(double) const;

    inline Point& operator+=(const Vector&);
    inline Point& operator-=(const Vector&);
    inline Point& operator+=(const VectorF&);
    inline Point& operator-=(const VectorF&);
    inline Point& operator+=(const Point&);
    inline Point& operator-=(const Point&);
    inline Point& operator+=(const PointF&);
    inline Point& operator-=(const PointF&);
    inline Point& operator*=(double);
    
    inline Point& operator/=(const double);
    inline Point operator/(const double) const;
    inline Point operator-() const;

    inline double& operator[](size_t idx); 
    inline double  operator[](size_t idx) const;

    inline void addscaled(const Point&, double);
    
    inline void x(double);
    inline double x() const;
    inline void y(double);
    inline double y() const;
    inline void z(double);
    inline double z() const;
        
    friend inline double Dot(const Point&, const Point&);
    friend inline double Dot(const Vector&, const Point&);
    friend inline double Dot(const Point&, const Vector&);

    friend inline Point Min(const Point&, const Point&);
    friend inline Point Max(const Point&, const Point&);

    friend Point AffineCombination(const Point&, double,
                                   const Point&, double,
                                   const Point&, double,
                                   const Point&, double);
    friend Point AffineCombination(const Point&, double,
                                   const Point&, double,
                                   const Point&, double);
    friend Point AffineCombination(const Point&, double,
                                   const Point&, double);

    friend Point Interpolate(const Point&, const Point&, double);

    // is one point within a small interval of another?
    bool Overlap( double a, double b, double epsilon );
    bool InInterval( Point a, double epsilon );
      
    friend std::ostream& operator<<(std::ostream& os, const Point& p);
}; // end class Point


class PointF {

  private:
    friend class Point;
    friend class Vector;
    friend class VectorF;

    float data_[3];

  public:
    inline PointF();
    inline PointF(float, float, float);
    inline PointF(float, float, float, float);

    inline explicit PointF(const Point& p);
    inline explicit PointF(const Vector& v);
    inline explicit PointF(const VectorF& v);
    inline PointF(const PointF&);
    
    inline bool operator==(const Point&) const;
    inline bool operator!=(const Point&) const;
    inline bool operator==(const PointF&) const;
    inline bool operator!=(const PointF&) const;

    inline PointF& operator=(const Point&);
    inline PointF& operator=(const PointF&);

    inline Vector operator+(const Point&) const;
    inline Vector operator-(const Point&) const;
    inline VectorF operator+(const PointF&) const;
    inline VectorF operator-(const PointF&) const;

    inline Point operator+(const Vector&) const;
    inline Point operator-(const Vector&) const;
    inline PointF operator+(const VectorF&) const;
    inline PointF operator-(const VectorF&) const;
    inline PointF operator*(float) const;

    inline PointF& operator+=(const Vector&);
    inline PointF& operator-=(const Vector&);
    inline PointF& operator+=(const VectorF&);
    inline PointF& operator-=(const VectorF&);
    inline PointF& operator+=(const Point&);
    inline PointF& operator-=(const Point&);
    inline PointF& operator+=(const PointF&);
    inline PointF& operator-=(const PointF&);
    inline PointF& operator*=(float);
    
    inline PointF& operator/=(const float);
    inline PointF operator/(const float) const;
    inline PointF operator-() const;

    inline float& operator[](size_t idx); 
    inline float  operator[](size_t idx) const;

    inline void addscaled(const PointF&, float);

    inline void x(float);
    inline float x() const;
    inline void y(float);
    inline float y() const;
    inline void z(float);
    inline float z() const;
        
    friend inline float Dot(const PointF&, const PointF&);
    friend inline float Dot(const VectorF&, const PointF&);
    friend inline float Dot(const PointF&, const VectorF&);

    friend inline PointF Min(const PointF&, const PointF&);
    friend inline PointF Max(const PointF&, const PointF&);

    friend PointF AffineCombination(const PointF&, float,
                                    const PointF&, float,
                                    const PointF&, float,
                                    const PointF&, float);
    friend PointF AffineCombination(const PointF&, float,
                                    const PointF&, float,
                                    const PointF&, float);
    friend PointF AffineCombination(const PointF&, float,
                                    const PointF&, float);

    friend PointF Interpolate(const PointF&, const PointF&, float);

    // is one point within a small interval of another?
    bool Overlap( float a, float b, float epsilon );
    bool InInterval( PointF a, float epsilon );
      
    friend std::ostream& operator<<(std::ostream& os, const PointF& p);
}; // end class PointF


Point AffineCombination(const Point&, double, 
                        const Point&, double,
                        const Point&, double, 
                        const Point&, double);
Point AffineCombination(const Point&, double, 
                        const Point&, double, 
                        const Point&, double);
Point AffineCombination(const Point&, double, 
                        const Point&, double);

PointF AffineCombination(const PointF&, float, 
                         const PointF&, float,
                         const PointF&, float, 
                         const PointF&, float);
PointF AffineCombination(const PointF&, float, 
                         const PointF&, float, 
                         const PointF&, float);
PointF AffineCombination(const PointF&, float, 
                         const PointF&, float);

inline Point operator*(double d, const Point &point) { return point*d; }
inline PointF operator*(float f, const PointF &point) { return point*f; }

std::ostream& operator<<(std::ostream& os, const Point& p);
std::ostream& operator<<(std::ostream& os, const PointF& p);

} // End namespace Utils

// This cannot be above due to circular dependencies
#include <Utils/Geometry/Vector.h>

namespace Utils {

inline 
Point::Point(double x, double y, double z)
{ 
  data_[0] = x; 
  data_[1] = y; 
  data_[2] = z; 
}

inline 
PointF::PointF(float x, float y, float z)
{ 
  data_[0] = x; 
  data_[1] = y; 
  data_[2] = z; 
}


inline 
Point::Point(double x, double y, double z, double w)
{ 
  if(w==0.0)
  {
    data_[0]=0.0;
    data_[1]=0.0;
    data_[2]=0.0;
  } 
  else 
  {
    double s = 1.0/w;
    data_[0]=x*s;
    data_[1]=y*s;
    data_[2]=z*s;
  }
}

inline 
PointF::PointF(float x, float y, float z, float w)
{ 
  if(w==0.0f)
  {
    data_[0]=0.0f;
    data_[1]=0.0f;
    data_[2]=0.0f;
  } 
  else 
  {
    float s = 1.0f/w;
    data_[0]=x*s;
    data_[1]=y*s;
    data_[2]=z*s;
  }
}

inline 
Point::Point(const Vector& v)
{
  data_[0] = v.data_[0];
  data_[1] = v.data_[1]; 
  data_[2] = v.data_[2];
}

inline 
Point::Point(const PointF& p)
{
  data_[0] = static_cast<double>(p.data_[0]);
  data_[1] = static_cast<double>(p.data_[1]); 
  data_[2] = static_cast<double>(p.data_[2]);
}

inline 
Point::Point(const Point& p)
{
  data_[0] = p.data_[0];
  data_[1] = p.data_[1]; 
  data_[2] = p.data_[2];
}

inline 
Point::Point(const VectorF& v)
{
  data_[0] = static_cast<double>(v.data_[0]);
  data_[1] = static_cast<double>(v.data_[1]); 
  data_[2] = static_cast<double>(v.data_[2]);
}

inline 
PointF::PointF(const PointF& p)
{
  data_[0] = p.data_[0];
  data_[1] = p.data_[1]; 
  data_[2] = p.data_[2];
}

inline 
PointF::PointF(const Vector& v)
{
  data_[0] = static_cast<float>(v.data_[0]);
  data_[1] = static_cast<float>(v.data_[1]); 
  data_[2] = static_cast<float>(v.data_[2]);
}

inline 
PointF::PointF(const Point& p)
{
  data_[0] = static_cast<float>(p.data_[0]);
  data_[1] = static_cast<float>(p.data_[1]); 
  data_[2] = static_cast<float>(p.data_[2]);
}

inline 
PointF::PointF(const VectorF& v)
{
  data_[0] = v.data_[0];
  data_[1] = v.data_[1]; 
  data_[2] = v.data_[2];
}

inline 
Point::Point()
{
  data_[0] = 0.0;
  data_[1] = 0.0;
  data_[2] = 0.0;
}

inline 
PointF::PointF()
{
  data_[0] = 0.0f;
  data_[1] = 0.0f;
  data_[2] = 0.0f;
}

inline bool
Point::operator==(const Point& p) const
{
  return (data_[0] == p.data_[0] && 
          data_[1] == p.data_[1] &&
          data_[2] == p.data_[2]);
}

inline bool
Point::operator==(const PointF& p) const
{
  return (static_cast<float>(data_[0]) == p.data_[0] && 
          static_cast<float>(data_[1]) == p.data_[1] &&
          static_cast<float>(data_[2]) == p.data_[2]);
}

inline bool
Point::operator!=(const Point& p) const
{
  return (data_[0] != p.data_[0] || 
          data_[1] != p.data_[1] ||
          data_[2] != p.data_[2]);
}

inline bool
Point::operator!=(const PointF& p) const
{
  return (static_cast<float>(data_[0]) != p.data_[0] || 
          static_cast<float>(data_[1]) != p.data_[1] ||
          static_cast<float>(data_[2]) != p.data_[2]);
}

inline bool
PointF::operator==(const Point& p) const
{
  return (static_cast<float>(p.data_[0]) == data_[0] && 
          static_cast<float>(p.data_[1]) == data_[1] &&
          static_cast<float>(p.data_[2]) == data_[2]);
}

inline bool
PointF::operator==(const PointF& p) const
{
  return (data_[0] == p.data_[0] && 
          data_[1] == p.data_[1] &&
          data_[2] == p.data_[2]);
}

inline bool
PointF::operator!=(const Point& p) const
{
  return (static_cast<float>(p.data_[0]) != data_[0] || 
          static_cast<float>(p.data_[1]) != data_[1] ||
          static_cast<float>(p.data_[2]) != data_[2]);
}

inline bool
PointF::operator!=(const PointF& p) const
{
  return (data_[0] != p.data_[0] || 
          data_[1] != p.data_[1] ||
          data_[2] != p.data_[2]);
}

inline 
Point& Point::operator=(const Point& p)
{
  data_[0] = p.data_[0];
  data_[1] = p.data_[1]; 
  data_[2] = p.data_[2];
  return *this;
}

inline 
Point& Point::operator=(const PointF& p)
{
  data_[0] = static_cast<double>(p.data_[0]);
  data_[1] = static_cast<double>(p.data_[1]); 
  data_[2] = static_cast<double>(p.data_[2]);
  return *this;
}

inline 
PointF& PointF::operator=(const Point& p)
{
  data_[0] = static_cast<float>(p.data_[0]);
  data_[1] = static_cast<float>(p.data_[1]); 
  data_[2] = static_cast<float>(p.data_[2]);
  return *this;
}

inline 
PointF& PointF::operator=(const PointF& p)
{
  data_[0] = p.data_[0];
  data_[1] = p.data_[1]; 
  data_[2] = p.data_[2];
  return *this;
}

inline Vector 
Point::operator+(const Point& p) const
{
  return Vector(data_[0]+p.data_[0], data_[1]+p.data_[1], data_[2]+p.data_[2]);
}

inline Vector 
Point::operator-(const Point& p) const
{
  return Vector(data_[0]-p.data_[0], data_[1]-p.data_[1], data_[2]-p.data_[2]);
}

inline Point 
Point::operator+(const Vector& v) const
{
  return Point(data_[0]+v.data_[0], data_[1]+v.data_[1], data_[2]+v.data_[2]);
}

inline Point 
Point::operator-(const Vector& v) const
{
  return Point(data_[0]-v.data_[0], data_[1]-v.data_[1], data_[2]-v.data_[2]);
}

inline Vector 
Point::operator+(const PointF& p) const
{
  return Vector(data_[0]+static_cast<double>(p.data_[0]), 
                data_[1]+static_cast<double>(p.data_[1]), 
                data_[2]+static_cast<double>(p.data_[2]));
}

inline Vector 
Point::operator-(const PointF& p) const
{
  return Vector(data_[0]-static_cast<double>(p.data_[0]), 
                data_[1]-static_cast<double>(p.data_[1]), 
                data_[2]-static_cast<double>(p.data_[2]));
}

inline Point 
Point::operator+(const VectorF& v) const
{
  return Point(data_[0]+static_cast<double>(v.data_[0]), 
               data_[1]+static_cast<double>(v.data_[1]), 
               data_[2]+static_cast<double>(v.data_[2]));
}

inline Point 
Point::operator-(const VectorF& v) const
{
  return Point(data_[0]-static_cast<double>(v.data_[0]), 
               data_[1]-static_cast<double>(v.data_[1]), 
               data_[2]-static_cast<double>(v.data_[2]));
}




inline Vector 
PointF::operator+(const Point& p) const
{
  return Vector(static_cast<double>(data_[0])+p.data_[0], 
                static_cast<double>(data_[1])+p.data_[1], 
                static_cast<double>(data_[2])+p.data_[2]);
}

inline Vector 
PointF::operator-(const Point& p) const
{
  return Vector(static_cast<double>(data_[0])-p.data_[0], 
                static_cast<double>(data_[1])-p.data_[1], 
                static_cast<double>(data_[2])-p.data_[2]);
}

inline Point 
PointF::operator+(const Vector& v) const
{
  return Point(static_cast<double>(data_[0])+v.data_[0], 
               static_cast<double>(data_[1])+v.data_[1], 
               static_cast<double>(data_[2])+v.data_[2]);
}

inline Point 
PointF::operator-(const Vector& v) const
{
  return Point(static_cast<double>(data_[0])-v.data_[0], 
               static_cast<double>(data_[1])-v.data_[1], 
               static_cast<double>(data_[2])-v.data_[2]);
}

inline VectorF 
PointF::operator+(const PointF& p) const
{
  return VectorF(data_[0]+p.data_[0], 
                data_[1]+p.data_[1], 
                data_[2]+p.data_[2]);
}

inline VectorF
PointF::operator-(const PointF& p) const
{
  return VectorF(data_[0]-p.data_[0], 
                data_[1]-p.data_[1], 
                data_[2]-p.data_[2]);
}

inline PointF 
PointF::operator+(const VectorF& v) const
{
  return PointF(data_[0]+v.data_[0], 
                data_[1]+v.data_[1], 
                data_[2]+v.data_[2]);
}

inline PointF 
PointF::operator-(const VectorF& v) const
{
  return PointF(data_[0]-v.data_[0], 
                data_[1]-v.data_[1], 
                data_[2]-v.data_[2]);
}


inline Point& 
Point::operator+=(const Vector& v)
{
  data_[0]+=v.data_[0];
  data_[1]+=v.data_[1];
  data_[2]+=v.data_[2];
  return *this;
}

inline PointF& 
PointF::operator+=(const Vector& v)
{
  data_[0]+=static_cast<float>(v.data_[0]);
  data_[1]+=static_cast<float>(v.data_[1]);
  data_[2]+=static_cast<float>(v.data_[2]);
  return *this;
}

inline Point& 
Point::operator-=(const Vector& v)
{
  data_[0]-=v.data_[0];
  data_[1]-=v.data_[1];
  data_[2]-=v.data_[2];
  return *this;
}

inline PointF& 
PointF::operator-=(const Vector& v)
{
  data_[0]-=static_cast<float>(v.data_[0]);
  data_[1]-=static_cast<float>(v.data_[1]);
  data_[2]-=static_cast<float>(v.data_[2]);
  return *this;
}

inline Point& 
Point::operator+=(const Point& v)
{
  data_[0]+=v.data_[0];
  data_[1]+=v.data_[1];
  data_[2]+=v.data_[2];
  return *this;
}

inline PointF& 
PointF::operator+=(const Point& v)
{
  data_[0]+=static_cast<float>(v.data_[0]);
  data_[1]+=static_cast<float>(v.data_[1]);
  data_[2]+=static_cast<float>(v.data_[2]);
  return *this;
}

inline Point& 
Point::operator-=(const Point& v)
{
  data_[0]-=v.data_[0];
  data_[1]-=v.data_[1];
  data_[2]-=v.data_[2];
  return *this;
}

inline PointF& 
PointF::operator-=(const Point& v)
{
  data_[0]-=static_cast<float>(v.data_[0]);
  data_[1]-=static_cast<float>(v.data_[1]);
  data_[2]-=static_cast<float>(v.data_[2]);
  return *this;
}

inline Point& 
Point::operator+=(const VectorF& v)
{
  data_[0]+=static_cast<double>(v.data_[0]);
  data_[1]+=static_cast<double>(v.data_[1]);
  data_[2]+=static_cast<double>(v.data_[2]);
  return *this;
}

inline PointF& 
PointF::operator-=(const VectorF& v)
{
  data_[0]-=v.data_[0];
  data_[1]-=v.data_[1];
  data_[2]-=v.data_[2];
  return *this;
}

inline Point& 
Point::operator+=(const PointF& v)
{
  data_[0]+=static_cast<double>(v.data_[0]);
  data_[1]+=static_cast<double>(v.data_[1]);
  data_[2]+=static_cast<double>(v.data_[2]);
  return *this;
}

inline PointF& 
PointF::operator+=(const PointF& v)
{
  data_[0]+=v.data_[0];
  data_[1]+=v.data_[1];
  data_[2]+=v.data_[2];
  return *this;
}

inline Point& 
Point::operator-=(const PointF& v)
{
  data_[0]-=static_cast<double>(v.data_[0]);
  data_[1]-=static_cast<double>(v.data_[1]);
  data_[2]-=static_cast<double>(v.data_[2]);
  return *this;
}

inline PointF& 
PointF::operator-=(const PointF& v)
{
  data_[0]-=v.data_[0];
  data_[1]-=v.data_[1];
  data_[2]-=v.data_[2];
  return *this;
}


inline Point& 
Point::operator*=(double d)
{
  data_[0]*=d;
  data_[1]*=d;
  data_[2]*=d;
  return *this;
}

inline PointF& 
PointF::operator*=(float d)
{
  data_[0]*=d;
  data_[1]*=d;
  data_[2]*=d;
  return *this;
}

inline Point& 
Point::operator/=(double d)
{
  data_[0]/=d;
  data_[1]/=d;
  data_[2]/=d;
  return *this;
}

inline PointF& 
PointF::operator/=(float d)
{
  data_[0]/=d;
  data_[1]/=d;
  data_[2]/=d;
  return *this;
}

inline Point 
Point::operator-() const
{
  return Point(-data_[0], -data_[1], -data_[2]);
}

inline PointF 
PointF::operator-() const
{
  return PointF(-data_[0], -data_[1], -data_[2]);
}

inline Point 
Point::operator*(double d) const
{
  return Point(data_[0]*d, data_[1]*d, data_[2]*d);
}

inline PointF 
PointF::operator*(float d) const
{
  return PointF(data_[0]*d, data_[1]*d, data_[2]*d);
}

inline Point 
Point::operator/(double d) const
{
  double s = 1.0/d;
  return Point(data_[0]*s,data_[1]*s,data_[2]*s);
}

inline PointF 
PointF::operator/(float d) const
{
  float s = 1.0f/d;
  return PointF(data_[0]*s,data_[1]*s,data_[2]*s);
}

inline double& 
Point::operator[](size_t idx) 
{
  return data_[idx];
}

inline double 
Point::operator[](size_t idx) const 
{
  return data_[idx];
}      

inline float& 
PointF::operator[](size_t idx) 
{
  return data_[idx];
}

inline float 
PointF::operator[](size_t idx) const 
{
  return data_[idx];
}      
      
inline Point 
Interpolate(const Point& v1, const Point& v2, double weight)
{
  double weight1 = 1.0 - weight;
  return Point(v2.data_[0]*weight+v1.data_[0]*weight1,
               v2.data_[1]*weight+v1.data_[1]*weight1,
               v2.data_[2]*weight+v1.data_[2]*weight1);
}

inline PointF 
Interpolate(const PointF& v1, const PointF& v2, float weight)
{
  float weight1 = 1.0f - weight;
  return PointF(v2.data_[0]*weight+v1.data_[0]*weight1,
                v2.data_[1]*weight+v1.data_[1]*weight1,
                v2.data_[2]*weight+v1.data_[2]*weight1);
}

inline void 
Point::addscaled(const Point& p, const double scale) 
{
  data_[0] += p.data_[0] * scale;
  data_[1] += p.data_[1] * scale;
  data_[2] += p.data_[2] * scale;
}

inline void 
PointF::addscaled(const PointF& p, const float scale) 
{
  data_[0] += p.data_[0] * scale;
  data_[1] += p.data_[1] * scale;
  data_[2] += p.data_[2] * scale;
}

inline void 
Point::x(double d)
{
  data_[0]=d;
}

inline double 
Point::x() const
{
  return data_[0];
}

inline void 
Point::y(double d)
{
  data_[1]=d;
}

inline double 
Point::y() const
{
  return data_[1];
}

inline void 
Point::z(double d)
{
  data_[2]=d;
}

inline double 
Point::z() const
{
  return data_[2];
}

inline void 
PointF::x(float d)
{
  data_[0]=d;
}

inline float 
PointF::x() const
{
  return data_[0];
}

inline void 
PointF::y(float d)
{
  data_[1]=d;
}

inline float 
PointF::y() const
{
  return data_[1];
}

inline void 
PointF::z(float d)
{
  data_[2]=d;
}

inline float 
PointF::z() const
{
  return data_[2];
}

inline Point 
Min(const Point& p1, const Point& p2)
{
  return Point(Min(p1.data_[0], p2.data_[0]),
               Min(p1.data_[1], p2.data_[1]),
               Min(p1.data_[2], p2.data_[2]));
}

inline Point 
Max(const Point& p1, const Point& p2)
{
  return Point(Max(p1.data_[0], p2.data_[0]),
               Max(p1.data_[1], p2.data_[1]),
               Max(p1.data_[2], p2.data_[2]));
}

inline PointF 
Min(const PointF& p1, const PointF& p2)
{
  return PointF(Min(p1.data_[0], p2.data_[0]),
                Min(p1.data_[1], p2.data_[1]),
                Min(p1.data_[2], p2.data_[2]));
}

inline PointF 
Max(const PointF& p1, const PointF& p2)
{
  return PointF(Max(p1.data_[0], p2.data_[0]),
                Max(p1.data_[1], p2.data_[1]),
                Max(p1.data_[2], p2.data_[2]));
}

inline double 
Dot(const Point& p, const Vector& v)
{
  return p.data_[0]*v.data_[0]+p.data_[1]*v.data_[1]+p.data_[2]*v.data_[2];
}

inline double 
Dot(const Point& p1, const Point& p2)
{
  return p1.data_[0]*p2.data_[0] + p1.data_[1]*p2.data_[1] + p1.data_[2]*p2.data_[2];
}

inline double 
Dot(const Vector& p1, const Point& p2)
{
  return p1.data_[0]*p2.data_[0] + p1.data_[1]*p2.data_[1] + p1.data_[2]*p2.data_[2];
}

inline float 
Dot(const PointF& p, const VectorF& v)
{
  return p.data_[0]*v.data_[0]+p.data_[1]*v.data_[1]+p.data_[2]*v.data_[2];
}

inline float
Dot(const PointF& p1, const PointF& p2)
{
  return p1.data_[0]*p2.data_[0] + p1.data_[1]*p2.data_[1] + p1.data_[2]*p2.data_[2];
}

inline float 
Dot(const VectorF& p1, const PointF& p2)
{
  return p1.data_[0]*p2.data_[0] + p1.data_[1]*p2.data_[1] + p1.data_[2]*p2.data_[2];
}

} // End namespace Utils

#endif
