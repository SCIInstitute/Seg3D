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

// File         : v3x1p3x1.hxx
// Author       : Pavel A. Koshevoy
// Created      : Mon Jul  1 21:53:36 MDT 2002
// Copyright    : (C) 2004-2008 University of Utah
// Description  : 2D, 3D and 3D homogeneous points/vectors.

#ifndef V3X1P3X1_HXX_
#define V3X1P3X1_HXX_

// system includes:
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <iomanip>

// namespace access:
using std::ostream;
using std::setw;
using std::endl;
using std::ios;

// forward declarations:
class m4x4_t;

// MSVC does not define M_PI by default, so I will:
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//----------------------------------------------------------------
// THE_EPSILON
// 
static const float
THE_EPSILON = 1e-6f;

//----------------------------------------------------------------
// THE_NEAR_ZERO_VECTOR_LENGTH
// 
static const float
THE_NEAR_ZERO_VECTOR_LENGTH = 1e+1f * FLT_MIN;


// shorthand, undefined at the end of the file:
#define X_ data_[0]
#define Y_ data_[1]
#define Z_ data_[2]
#define W_ data_[3]


// In order to avoid memory leaks the member functions of these classes
// will not allocate anything 'new' on the heap. All operations
// work from the stack:

//----------------------------------------------------------------
// the_duplet_t
// 
// base class for v2x1_t and p2x1_t:
// 
template<class T>
class the_duplet_t
{
public:
  the_duplet_t() {}
  
  the_duplet_t(const T * data)
  {
    X_ = data[0];
    Y_ = data[1];
  }
  
  the_duplet_t(const T & x, const T & y)
  {
    X_ = x;
    Y_ = y;
  }
  
  // comparison operator:
  inline bool operator == (const the_duplet_t<T> & d) const
  { return equal(d); }
  
  inline bool operator < (const the_duplet_t<T> & d) const
  {
    return ((X_ < d.X_) ||
      (X_ == d.X_ && Y_ < d.Y_));
  }
  
  // compare this duplet to a given duplet for equality:
  inline bool equal(const the_duplet_t<T> & d) const
  {
    return ((X_ == d.X_) &&
      (Y_ == d.Y_));
  }
  
  // assign to this duplet:
  inline void assign(const T * data)
  {
    X_ = data[0];
    Y_ = data[1];
  }
  
  inline void assign(const T & x, const T & y)
  {
    X_ = x;
    Y_ = y;
  }
  
  // scale this duplet by a given factor:
  inline void scale(const T & s)
  { X_ *= s; Y_ *= s; }
  
  // add a given duplet to this duplet:
  inline void increment(const the_duplet_t<T> & d)
  { X_ += d.X_; Y_ += d.Y_; }
  
  // subtract a given duplet from this duplet:
  inline void decrement(const the_duplet_t<T> & d)
  { X_ -= d.X_; Y_ -= d.Y_; }
  
  // this is for debugging purposes:
  inline void dump(ostream & stream, const char * type_name) const
  {
    stream << '(' << type_name << " *)(" << (void *)this << ") "
     << setw(12) << X_ << ' '
     << setw(12) << Y_;
  }
  
  // const accessors:
  inline const T & x() const { return X_; }
  inline const T & y() const { return Y_; }
  
  // non-const accessors:
  inline T & x() { return X_; }
  inline T & y() { return Y_; }
  
  // array-like accessors:
  inline const T & operator[] (const unsigned int & i) const
  { return data_[i]; }
  
  inline T & operator[] (const unsigned int & i)
  { return data_[i]; }
  
  // raw data accessors:
  inline const T * data() const
  { return data_; }
  
  inline T * data()
  { return data_; }
  
protected:
  T data_[2];
};

//----------------------------------------------------------------
// operator <<
// 
template<class T>
inline ostream &
operator << (ostream & stream, const the_duplet_t<T> & p)
{
  p.dump(stream, "the_duplet_t<T>");
  return stream;
}


//----------------------------------------------------------------
// the_triplet_t
// 
// base class for v3x1_t and p3x1_t:
// 
template<class T>
class the_triplet_t
{
  friend class m4x4_t;
  
public:
  the_triplet_t() {}
  
  the_triplet_t(const T * data)
  {
    X_ = data[0];
    Y_ = data[1];
    Z_ = data[2];
  }
  
  the_triplet_t(const T & x, const T & y, const T & z)
  {
    X_ = x;
    Y_ = y;
    Z_ = z;
  }
  
  // comparison operator:
  inline bool operator == (const the_duplet_t<T> & d) const
  { return equal(d); }
  
  inline bool operator < (const the_duplet_t<T> & d) const
  {
    return ((X_ < d.X_) ||
      (X_ == d.X_ && Y_ < d.Y_) ||
      (X_ == d.X_ && Y_ == d.Y_ && Z_ < d.Z_));
  }
  
  // compare this triplet to a given triplet for equality:
  inline bool equal(const the_triplet_t<T> & d) const
  {
    return ((X_ == d.X_) &&
      (Y_ == d.Y_) &&
      (Z_ == d.Z_));
  }
  
  // assign to this triplet:
  inline void assign(const T * data)
  {
    X_ = data[0];
    Y_ = data[1];
    Z_ = data[2];
  }
  
  inline void assign(const T & x, const T & y, const T & z)
  {
    X_ = x;
    Y_ = y;
    Z_ = z;
  }
  
  // copy data from this triplet:
  inline void get(T * data) const
  { get(data[0], data[1], data[2]); }
  
  inline void get(T & x, T & y, T & z) const
  { x = X_; y = Y_; z = Z_; }
  
  // scale this triplet by a given factor:
  inline void scale(const T & s)
  { X_ *= s; Y_ *= s; Z_ *= s; }
  
  // reverse the vector:
  inline void negate()
  { X_ = -X_; Y_ = -Y_; Z_ = -Z_; }
  
  // add a given triplet to this triplet:
  inline void increment(const the_triplet_t<T> & d)
  { X_ += d.X_; Y_ += d.Y_; Z_ += d.Z_; }
  
  // subtract a given triplet from this triplet:
  inline void decrement(const the_triplet_t<T> & d)
  { X_ -= d.X_; Y_ -= d.Y_; Z_ -= d.Z_; }
  
  // this is for debugging purposes:
  inline void dump(ostream & stream, const char * type_name) const
  {
    stream << '(' << type_name << " *)(" << (void *)this << ") "
     << setw(12) << X_ << ' '
     << setw(12) << Y_ << ' '
     << setw(12) << Z_;
  }
  
  // const accessors:
  inline const T & x() const { return X_; }
  inline const T & y() const { return Y_; }
  inline const T & z() const { return Z_; }
  
  // non-const accessors:
  inline T & x() { return X_; }
  inline T & y() { return Y_; }
  inline T & z() { return Z_; }
  
  // array-like accessors:
  inline const T & operator[] (const unsigned int & i) const
  { return data_[i]; }
  
  inline T & operator[] (const unsigned int & i)
  { return data_[i]; }
  
  // raw data accessors:
  inline const T * data() const
  { return data_; }
  
  inline T * data()
  { return data_; }
  
protected:
  T data_[3];
};

//----------------------------------------------------------------
// operator <<
// 
template<class T>
inline ostream &
operator << (ostream & stream, const the_triplet_t<T> & p)
{
  p.dump(stream, "the_triplet_t<T>");
  return stream;
}


//----------------------------------------------------------------
// the_quadruplet_t
// 
// base class for v4x1_t and p4x1_t:
// 
template<class T>
class the_quadruplet_t
{
  friend class m4x4_t;
  
public:
  the_quadruplet_t() {}
  
  the_quadruplet_t(const T * data)
  {
    X_ = data[0];
    Y_ = data[1];
    Z_ = data[2];
    W_ = data[3];
  }
  
  the_quadruplet_t(const T & x, const T & y, const T & z, const T & w)
  {
    X_ = x;
    Y_ = y;
    Z_ = z;
    W_ = w;
  }
  
  // comparison operator:
  inline bool operator == (const the_duplet_t<T> & d) const
  { return equal(d); }
  
  inline bool operator < (const the_duplet_t<T> & d) const
  {
    return ((X_ < d.X_) ||
      (X_ == d.X_ && Y_ < d.Y_) ||
      (X_ == d.X_ && Y_ == d.Y_ && Z_ < d.Z_) ||
      (X_ == d.X_ && Y_ == d.Y_ && Z_ == d.Z_ && W_ < d.W_));
  }
  
  // compare this quadruplet to a given quadruplet for equality:
  inline bool equal(const the_quadruplet_t<T> & d) const
  {
    return ((X_ == d.X_) &&
      (Y_ == d.Y_) &&
      (Z_ == d.Z_) &&
      (W_ == d.W_));
  }
  
  // assign to this quadruplet:
  inline void assign(const T * data)
  {
    X_ = data[0];
    Y_ = data[1];
    Z_ = data[2];
    W_ = data[3];
  }
  
  inline void assign(const T & x, const T & y, const T & z, const T & w)
  {
    X_ = x;
    Y_ = y;
    Z_ = z;
    W_ = w;
  }
  
  // scale this quadruplet by a given factor:
  inline void scale(const T & s)
  { X_ *= s; Y_ *= s; Z_ *= s; W_ *= s; }
  
  // add a given quadruplet to this quadruplet:
  inline void increment(const the_quadruplet_t<T> & d)
  { X_ += d.X_; Y_ += d.Y_; Z_ += d.Z_; W_ += d.W_; }
  
  // subtract a given quadruplet from this quadruplet:
  inline void decrement(const the_quadruplet_t<T> & d)
  { X_ -= d.X_; Y_ -= d.Y_; Z_ -= d.Z_; W_ -= d.W_; }
  
  // this is for debugging purposes:
  inline void dump(ostream & stream, const char * type_name) const
  {
    stream << '(' << type_name << " *)(" << (void *)this << ") "
     << setw(12) << X_ << ' '
     << setw(12) << Y_ << ' '
     << setw(12) << Z_ << ' '
     << setw(12) << W_;
  }
  
  // const accessors:
  inline const T & x() const { return X_; }
  inline const T & y() const { return Y_; }
  inline const T & z() const { return Z_; }
  inline const T & w() const { return W_; }
  
  // non-const accessors:
  inline T & x() { return X_; }
  inline T & y() { return Y_; }
  inline T & z() { return Z_; }
  inline T & w() { return W_; }
  
  // array-like accessors:
  inline const T & operator[] (const unsigned int & i) const
  { return data_[i]; }
  
  inline T & operator[] (const unsigned int & i)
  { return data_[i]; }
  
  // raw data accessors:
  inline const T * data() const
  { return data_; }
  
  inline T * data()
  { return data_; }
  
protected:
  T data_[4];
};

//----------------------------------------------------------------
// operator <<
// 
template<class T>
inline ostream &
operator << (ostream & stream, const the_quadruplet_t<T> & p)
{
  p.dump(stream, "the_quadruplet_t<T>");
  return stream;
}


//----------------------------------------------------------------
// v2x1_t
// 
class v2x1_t : public the_duplet_t<float>
{
public:
  v2x1_t() {}
  
  v2x1_t(const float * data):
    the_duplet_t<float>(data)
  {}
  
  v2x1_t(const float & x, const float & y):
    the_duplet_t<float>(x, y)
  {}
  
  // equality/inequality test operators:
  inline bool operator == (const v2x1_t & v) const { return equal(v); }
  inline bool operator != (const v2x1_t & v) const { return !equal(v); }
  
  // scale this vector:
  inline v2x1_t & operator *= (const float & s)
  { scale(s); return *this; }
  
  inline v2x1_t & operator /= (const float & s)
  { scale(1 / s); return *this; }
  
  // return a copy of this vector scaled by a given factor:
  inline const v2x1_t operator * (const float & s) const
  { v2x1_t r(*this); r *= s; return r; }
  
  inline const v2x1_t operator / (const float & s) const
  { v2x1_t r(*this); r /= s; return r; }
  
  // increment/decrement this vector:
  inline v2x1_t & operator += (const v2x1_t & v)
  { increment(v); return *this; }
  
  inline v2x1_t & operator -= (const v2x1_t & v)
  { decrement(v); return *this; }
  
  // return a copy of this vector plus/minus a given vector:
  inline const v2x1_t operator + (const v2x1_t & v) const
  { v2x1_t r(*this); r += v; return r; }
  
  inline const v2x1_t operator - (const v2x1_t & v) const
  { v2x1_t r(*this); r -= v; return r; }
  
  // return the dot product between this vector and a given factor:
  inline float operator * (const v2x1_t & v) const
  { return (X_ * v.X_ + Y_ * v.Y_); }
  
  // norm operator (returns the magnitude/length/norm of this vector):
  inline float operator ~ () const { return norm(); }
  
  // return a copy of this vector with norm 1.0, or 0.0 if it can not
  // be normalized (divided by the norm):
  inline const v2x1_t operator ! () const
  { v2x1_t r(*this); r.normalize(); return r; }
  
  // this = this / |this|
  inline bool normalize()
  {
    float n = norm();
    if (n < THE_NEAR_ZERO_VECTOR_LENGTH)
    {
      scale(0);
      return false;
    }
    
    scale(1 / n);
    return true;
  }
  
  // this is necessary in order to allow sorting:
  inline bool operator < (const v2x1_t & v) const
  { return norm_sqrd() < v.norm_sqrd(); }
  
  // the squared length of this vector
  inline float norm_sqrd() const
  { return operator * (*this); }
  
  // return the norm (length, magnitude) of this vector:
  inline float norm() const
  { return sqrtf(norm_sqrd()); }
};

//----------------------------------------------------------------
// operator -
// 
inline const v2x1_t
operator - (const v2x1_t & v)
{
  return (v * (-1.0));
}

//----------------------------------------------------------------
// operator *
// 
inline const v2x1_t
operator * (const float & s, const v2x1_t & v)
{
  return v * s;
}

//----------------------------------------------------------------
// operator <<
// 
inline ostream &
operator << (ostream & stream, const v2x1_t & v)
{
  v.dump(stream, "v2x1_t");
  return stream;
}


//----------------------------------------------------------------
// v3x1_t
//
class v3x1_t : public the_triplet_t<float>
{
public:
  v3x1_t() {}
  
  v3x1_t(const float * data):
    the_triplet_t<float>(data)
  {}
  
  v3x1_t(const float x, const float & y, const float & z):
    the_triplet_t<float>(x, y, z)
  {}
  
  v3x1_t(const v2x1_t & v, const float & z = 0.0):
    the_triplet_t<float>(v.x(), v.y(), z)
  {}
  
  // equality/inequality test operators:
  inline bool operator == (const v3x1_t & v) const { return equal(v); }
  inline bool operator != (const v3x1_t & v) const { return !equal(v); }
  
  // scale this vector:
  inline v3x1_t & operator *= (const float & s)
  { scale(s); return *this; }
  
  inline v3x1_t & operator /= (const float & s)
  { scale(1 / s); return *this; }
  
  // return a copy of this vector scaled by a given factor:
  inline const v3x1_t operator * (const float & s) const
  { v3x1_t r(*this); r *= s; return r; }
  
  inline const v3x1_t operator / (const float & s) const
  { v3x1_t r(*this); r /= s; return r; }
  
  // increment/decrement this vector:
  inline v3x1_t & operator += (const v3x1_t & v)
  { increment(v); return *this; }
  
  inline v3x1_t & operator -= (const v3x1_t & v)
  { decrement(v); return *this; }
  
  // return a copy of this vector plus/minus a given vector:
  inline const v3x1_t operator + (const v3x1_t & v) const
  { v3x1_t r(*this); r += v; return r; }
  
  inline const v3x1_t operator - (const v3x1_t & v) const
  { v3x1_t r(*this); r -= v; return r; }
  
  // return the dot product between this vector and a given vector:
  inline float operator * (const v3x1_t & v) const
  { return (X_ * v.X_ + Y_ * v.Y_ + Z_ * v.Z_); }
  
  // return the cross product between this vector and a given vector:
  inline const v3x1_t operator % (const v3x1_t & v) const
  {
    return v3x1_t((Y_ * v.Z_) - (Z_ * v.Y_),
      (Z_ * v.X_) - (X_ * v.Z_),
      (X_ * v.Y_) - (v.X_ * Y_));
  }
  
  // norm operator (returns the magnitude/length/norm of this vector):
  inline float operator ~ () const { return norm(); }
  
  // return a copy of this vector with norm 1.0, or 0.0 if it can not
  // be normalized (divided by the norm):
  inline const v3x1_t operator ! () const
  { v3x1_t r(*this); r.normalize(); return r; }
  
  // this = this / |this|
  inline bool normalize()
  {
    float n = norm();
    if (n < THE_NEAR_ZERO_VECTOR_LENGTH)
    {
      scale(0);
      return false;
    }
    
    scale(1 / n);
    return true;
  }
  
  // this is necessary in order to allow sorting:
  inline bool operator < (const v3x1_t & v) const
  { return norm_sqrd() < v.norm_sqrd(); }
  
  // the squared length of this vector
  inline float norm_sqrd() const
  { return operator * (*this); }
  
  // return the norm (length, magnitude) of this vector:
  inline float norm() const
  { return sqrtf(norm_sqrd()); }
  
  // return a vector normal to this vector:
  inline const v3x1_t normal() const
  {
    static const v3x1_t xyz[] =
    {
      v3x1_t(1.0, 0.0, 0.0),
      v3x1_t(0.0, 1.0, 0.0),
      v3x1_t(0.0, 0.0, 1.0)
    };
    
    v3x1_t unit_vec = !(*this);
    for (unsigned int i = 0; i < 3; i++)
    {
      if (fabsf(unit_vec * xyz[i]) > 0.7) continue;
      return !(unit_vec % xyz[i]);
    }
    
    // this should never happen:
    assert(false);
    return v3x1_t(0.0, 0.0, 0.0);
  }
  
  // conversion operator:
  inline operator v2x1_t() const
  { return v2x1_t(X_, Y_); }
};

//----------------------------------------------------------------
// operator -
// 
inline const v3x1_t
operator - (const v3x1_t & v)
{
  return v3x1_t(-v.x(), -v.y(), -v.z());
}

//----------------------------------------------------------------
// operator *
// 
inline const v3x1_t
operator * (const float & s, const v3x1_t & v)
{
  return v * s;
}

//----------------------------------------------------------------
// operator <<
// 
inline ostream &
operator << (ostream & stream, const v3x1_t & v)
{
  v.dump(stream, "v3x1_t");
  return stream;
}


//----------------------------------------------------------------
// p2x1_t
// 
class p2x1_t : public the_duplet_t<float>
{
public:
  p2x1_t() {}
  
  p2x1_t(const float * data):
    the_duplet_t<float>(data)
  {}
  
  p2x1_t(const float & x, const float & y):
    the_duplet_t<float>(x, y)
  {}
  
  // equality/inequality test operators:
  inline bool operator == (const p2x1_t & p) const { return equal(p); }
  inline bool operator != (const p2x1_t & p) const { return !equal(p); }
  
  // this is necessary in order to allow sorting:
  inline bool operator < (const p2x1_t & p) const
  {
    p2x1_t zero(0.0, 0.0);
    return (*this - zero) < (p - zero);
  }
  
  // scale this point:
  inline p2x1_t & operator *= (const float & s)
  { scale(s); return *this; }
  
  inline p2x1_t & operator /= (const float & s)
  { scale(1 / s); return *this; }
  
  // return a copy of this point scaled by a given factor:
  inline const p2x1_t operator * (const float & s) const
  { p2x1_t r(*this); r *= s; return r; }
  
  inline const p2x1_t operator / (const float & s) const
  { p2x1_t r(*this); r /= s; return r; }
  
  // this is used for linear combinations (in combination with scale):
  inline const p2x1_t operator + (const p2x1_t & p) const
  { p2x1_t r(*this); r.increment(p); return r; }
  
  // translate this point by a vector:
  inline p2x1_t & operator += (const v2x1_t & v)
  { increment(v); return *this; }
  
  inline p2x1_t & operator -= (const v2x1_t & v)
  { decrement(v); return *this; }
  
  // return a copy of this point translated by a given vector:
  inline const p2x1_t operator + (const v2x1_t & v) const
  { p2x1_t r(data_); r += v; return r; }
  
  inline const p2x1_t operator - (const v2x1_t & v) const
  { p2x1_t r(data_); r -= v; return r; }
  
  // return the vector difference between this point a given point:
  inline const v2x1_t operator - (const p2x1_t & p) const
  { v2x1_t r(data_); r.decrement(p); return r; }
};

//----------------------------------------------------------------
// operator +
// 
inline const p2x1_t
operator + (const v2x1_t & v, const p2x1_t & p)
{
  return p + v;
}

//----------------------------------------------------------------
// operator *
// 
inline const p2x1_t
operator * (const float & s, const p2x1_t & p)
{
  return p * s;
}

//----------------------------------------------------------------
// operator <<
// 
inline ostream &
operator << (ostream & stream, const p2x1_t & p)
{
  p.dump(stream, "p2x1_t");
  return stream;
}


//----------------------------------------------------------------
// p3x1_t
// 
class p3x1_t : public the_triplet_t<float>
{
public:
  p3x1_t() {}
  
  p3x1_t(const float * data):
    the_triplet_t<float>(data)
  {}
  
  p3x1_t(const float & x, const float & y, const float & z):
    the_triplet_t<float>(x, y, z)
  {}
  
  p3x1_t(const p2x1_t & p, const float & z = 0.0):
    the_triplet_t<float>(p.x(), p.y(), z)
  {}
  
  // equality/inequality test operators:
  inline bool operator == (const p3x1_t & p) const { return equal(p); }
  inline bool operator != (const p3x1_t & p) const { return !equal(p); }
  
  // this is necessary in order to allow sorting:
  inline bool operator < (const p3x1_t & p) const
  {
    p3x1_t zero(0.0, 0.0, 0.0);
    return (*this - zero) < (p - zero);
  }
  
  // scale this point:
  inline p3x1_t & operator *= (const float & s)
  { scale(s); return *this; }
  
  inline p3x1_t & operator /= (const float & s)
  { scale(1 / s); return *this; }
  
  // return a copy of this point scaled by a given factor:
  inline const p3x1_t operator * (const float & s) const
  { p3x1_t r(*this); r *= s; return r; }
  
  inline const p3x1_t operator / (const float & s) const
  { p3x1_t r(*this); r /= s; return r; }
  
  // this is used for linear combinations (in combination with scale):
  inline const p3x1_t operator + (const p3x1_t & p) const
  { p3x1_t r(*this); r.increment(p); return r; }
  
  // translate this point by a vector:
  inline p3x1_t & operator += (const v3x1_t & v)
  { increment(v); return *this; }
  
  inline p3x1_t & operator += (const p3x1_t & p)
  { increment(p); return *this; }
  
  inline p3x1_t & operator -= (const v3x1_t & v)
  { decrement(v); return *this; }
  
  // return a copy of this point translated by a given vector:
  inline const p3x1_t operator + (const v3x1_t & v) const
  { p3x1_t r(data_); r += v; return r; }
  
  inline const p3x1_t operator - (const v3x1_t & v) const
  { p3x1_t r(data_); r -= v; return r; }
  
  // return the vector difference between this point a given point:
  inline const v3x1_t operator - (const p3x1_t & p) const
  { v3x1_t r(data_); r.decrement(p); return r; }
  
  // conversion operator:
  inline operator p2x1_t() const
  { return p2x1_t(X_, Y_); }
};

//----------------------------------------------------------------
// operator +
// 
inline const p3x1_t
operator + (const v3x1_t & v, const p3x1_t & p)
{
  return p + v;
}

//----------------------------------------------------------------
// operator *
// 
inline const p3x1_t
operator * (const float & s, const p3x1_t & p)
{
  return p * s;
}

//----------------------------------------------------------------
// operator <<
// 
inline ostream &
operator << (ostream & stream, const p3x1_t & p)
{
  p.dump(stream, "p3x1_t");
  return stream;
}


//----------------------------------------------------------------
// p4x1_t
// 
class p4x1_t : public the_quadruplet_t<float>
{
public:
  p4x1_t() {}
  
  p4x1_t(const float * data):
    the_quadruplet_t<float>(data)
  {}
  
  p4x1_t(const float & x,
   const float & y,
   const float & z,
   const float & w):
    the_quadruplet_t<float>(x, y, z, w)
  {}
  
  p4x1_t(const p3x1_t & p, const float & w = 1.0):
    the_quadruplet_t<float>(p.x(), p.y(), p.z(), w)
  {}
  
  // homogenize this point (x = x/w, y = y/w, z = z/w, w = 1.0):
  inline void homogenize()
  { X_ = X_ / W_; Y_ = Y_ / W_; Z_ = Z_ / W_; W_ = 1.0; }
  
  // conversion operators:
  inline operator p3x1_t() const
  { return p3x1_t(X_ / W_, Y_ / W_, Z_ / W_); }
};

//----------------------------------------------------------------
// operator <<
// 
inline ostream &
operator << (ostream & stream, const p4x1_t & p)
{
  p.dump(stream, "p4x1_t");
  return stream;
}


// undefine the shorthand:
#undef X_
#undef Y_
#undef Z_
#undef W_


#endif // V3X1P3X1_HXX_
