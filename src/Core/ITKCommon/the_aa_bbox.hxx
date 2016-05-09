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

// File         : the_aa_bbox.hxx
// Author       : Pavel A. Koshevoy
// Created      : Mon Jun  7 22:14:00 MDT 2004
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Axis aligned 3D bounding box.

#ifndef THE_AA_BBOX_HXX_
#define THE_AA_BBOX_HXX_

// system includes:
#include <iostream>

// local includes:
#include <Core/ITKCommon/v3x1p3x1.hxx>

// namespace access:
using std::ostream;


// axis aligned bounding box layout:
// 
//        C1-------E9------C5
//        /|              /|
//       / |             / |          Z = [0  0  1]T
//     E0  |   F0      E4  |          |
//     /   e1      f1  /   |          |
//    /    |          /   E5          | reference coordinate system
//  C0-------E8------C4    |          |
//   |     |         |  F5 |          + - - - - Y = [0  1  0]T
//   | f4  c2----e10-|-----C6        /
//   |    /          |    /         /
//  E3   /  F3       E7  /         X = [1  0  0]T
//   |  e2      f2   |  E6
//   | /             | /   
//   |/              |/
//  C3------E11------C7
//  
//  Fi - face id,   fi - hidden face id.
//  Ei - edge id,   ei - hidden edge id.
//  Ci - corner id, ci - hidden corner id.
//  
//  The bounding box faces correspond to the fiew point orientation as follows:
//  F0 = top face
//  f1 = back face
//  f2 = bottom face
//  F3 = front face
//  f4 = right face
//  F5 = left face

//----------------------------------------------------------------
// the_aa_bbox_t
// 
class the_aa_bbox_t
{
public:
  the_aa_bbox_t()
  { clear(); }
  
  // reset the bounding box to be empty:
  inline void clear()
  {
    min_.assign(FLT_MAX, FLT_MAX, FLT_MAX);
    max_.assign(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  }
  
  // addition/expansion operators:
  the_aa_bbox_t & operator << (const p3x1_t & pt);
  the_aa_bbox_t & operator += (const the_aa_bbox_t & bbox);
  
  inline the_aa_bbox_t operator + (const the_aa_bbox_t & bbox) const
  {
    the_aa_bbox_t ret_val(*this);
    return ret_val += bbox;
  }
  
  // scale operators:
  the_aa_bbox_t & operator *= (const float & s);
  
  the_aa_bbox_t operator * (const float & s) const
  {
    the_aa_bbox_t result(*this);
    result *= s;
    return result;
  }
  
  // uniformly advance/retreat every face of the bounding box by value r:
  the_aa_bbox_t & operator += (const float & r);
  
  inline the_aa_bbox_t operator + (const float & r) const
  {
    the_aa_bbox_t result(*this);
    result += r;
    return result;
  }
  
  inline the_aa_bbox_t operator - (const float & r) const
  { return (*this + (-r)); }
  
  inline the_aa_bbox_t & operator -= (const float & r)
  { return (*this += (-r)); }
  
  // equality test operator:
  inline bool operator == (const the_aa_bbox_t & bbox) const
  { return ((min_ == bbox.min_) && (max_ == bbox.max_)); }
  
  inline bool operator != (const the_aa_bbox_t & bbox) const
  { return !((*this) == bbox); }
  
  // return true if the volume of this bounding box is smaller than
  // the volume of the given bounding box:
  inline bool operator < (const the_aa_bbox_t & bbox) const
  { return (volume() < bbox.volume()); }
  
  // calculate the volume of this bounding box:
  inline float volume() const
  {
    if (is_empty()) return 0.0;
    return (max_[0] - min_[0]) * (max_[1] - min_[1]) * (max_[2] - min_[2]);
  }
  
  // convert min/max into 8 bounding box corners,
  // the caller has to make sure that corner_array is of size 8:
  void corners(p3x1_t * corner_array) const;
  
  // bounding box validity tests:
  bool is_empty() const;
  
  inline bool is_singular() const
  { return (min_ == max_); }
  
  bool is_linear() const;
  bool is_planar() const;
  bool is_spacial() const;
  
  // calculate the edge length of the bounding box:
  inline float length(const unsigned int & axis_id) const
  { return (max_[axis_id] - min_[axis_id]); }
  
  // calculate the center of the bounding box:
  inline p3x1_t center() const
  { return 0.5f * (max_ + min_); }
  
  // calculate the radius of the bounding box (sphere):
  float radius(const p3x1_t & center) const;
  
  inline float radius() const
  { return 0.5f * diameter(); }
  
  inline float diameter() const
  {
    if (is_empty()) return 0;
    return (min_ - max_).norm();
  }
  
  // calculate the radius of the bounding box (cylinder):
  float radius(const p3x1_t & center, const unsigned int & axis_w_id) const;
  
  inline float radius(const unsigned int & axis_w_id) const
  { return radius(center(), axis_w_id); }
  
  // check whether a given point is contained between the faces of the
  // bounding box normal to the x, y, and z axis - if the point is
  // contained within all three, the point is inside the bounding box:
  void contains(const p3x1_t & pt,
    bool & contained_in_x,
    bool & contained_in_y,
    bool & contained_in_z) const;
  
  // check whether the bounding box contains a given point:
  inline bool contains(const p3x1_t & pt) const
  {
    bool contained_in_x = false;
    bool contained_in_y = false;
    bool contained_in_z = false;
    contains(pt, contained_in_x, contained_in_y, contained_in_z);
    return (contained_in_x && contained_in_y && contained_in_z);
  }
  
  // check whether the bounding box contains another bounding box:
  inline bool contains(const the_aa_bbox_t & bbox) const
  { return contains(bbox.min_) && contains(bbox.max_); }
  
  // check whether the bounding boxes intersect:
  bool intersects(const the_aa_bbox_t & bbox) const;
  
  // clamp this bounding box to lay within the confines of
  // a given bounding box:
  void clamp(const the_aa_bbox_t & confines);
  
  // return a copy of this bounding box clamped within the given confines:
  inline the_aa_bbox_t clamped(const the_aa_bbox_t & confines) const
  {
    the_aa_bbox_t tmp(*this);
    tmp.clamp(confines);
    return tmp;
  }
  
  // find the intersection of this bounding box with a given ray:
  bool intersects_ray(const p3x1_t & o,
          const v3x1_t & d,
          float & t_min,
          float & t_max) const;
  
  // find the axis id of the largest/smallest dimension of the bounding box:
  unsigned int largest_dimension() const;
  unsigned int smallest_dimension() const;
  
  // For debugging, dumps this bounding box into a stream:
  void dump(ostream & strm) const;
  
  // the minimum and maximum points of the bounding box:
  p3x1_t min_;
  p3x1_t max_;
};

//----------------------------------------------------------------
// operator *
// 
inline the_aa_bbox_t
operator * (float s, const the_aa_bbox_t & bbox)
{ return bbox * s; }

//----------------------------------------------------------------
// operator <<
// 
inline ostream &
operator << (ostream & strm, const the_aa_bbox_t & bbox)
{
  bbox.dump(strm);
  return strm;
}


#endif // THE_AA_BBOX_HXX_
