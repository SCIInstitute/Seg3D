// -*- Mode: c++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: nil -*-
// NOTE: the first line of this file sets up source code indentation rules
// for Emacs; it is also a hint to anyone modifying this file.

/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

// File         : LineMath.h
// Author       : Bradley C. Grimm
// Created      : 2009/08/19
// Copyright    : (C) 2009 University of Utah
// Description  : Helper functions and definitions for using lines and doing
//                calculations on them.

#ifndef __LINE_MATH_H__
#define __LINE_MATH_H__

#include <itkPoint.h>
#include <itkVector.h>

#include <Core/ITKCommon/Transform/the_grid_transform.hxx>

namespace ir
{

//----------------------------------------------------------------
// PointMath
//        Provides simple point functions.  Used mostly for convenience
//        and cleaner code.
class PointMath
{
public:
  static double DistanceSquared(const pnt2d_t &pt1, const pnt2d_t &pt2);
  static double Distance(const pnt2d_t &pt1, const pnt2d_t &pt2);

  static pnt2d_t VectorToPoint(const vec2d_t &pt);
  static pnt2d_t Direction(const pnt2d_t &pt1, const pnt2d_t &pt2);
  static pnt2d_t Scale(const pnt2d_t &pt1, const pnt2d_t &pt2, double factor);
  
  static pnt2d_t XYtoUV(const pnt2d_t &xy, const pnt2d_t &min, const vec2d_t &ext);
  static pnt2d_t UVtoXY(const pnt2d_t &uv, const pnt2d_t &min, const vec2d_t &ext);
};
  
//----------------------------------------------------------------
// Distance
//        Distance squared between two 2D points.
inline double PointMath::DistanceSquared(const itk::Point<double, 2> &pt1,
                                         const itk::Point<double, 2> &pt2)
{
  return (pt1[0] - pt2[0]) * (pt1[0] - pt2[0]) + 
         (pt1[1] - pt2[1]) * (pt1[1] - pt2[1]);
}
  
//----------------------------------------------------------------
// Distance
//        Distance between two 2D points.
inline double PointMath::Distance(const itk::Point<double, 2> &pt1,
                                  const itk::Point<double, 2> &pt2)
{
  return sqrt(DistanceSquared(pt1, pt2));
}
  
//----------------------------------------------------------------
// Direction
//        Gives the direction of the line (vector from pt2 to pt1)
inline itk::Point<double, 2> PointMath::Direction(const pnt2d_t &pt1, const pnt2d_t &pt2)
{
  itk::Vector<double, 2> dir;
  dir[0] = pt2[0] - pt1[0];
  dir[1] = pt2[1] - pt1[1];
  dir.Normalize();
  return PointMath::VectorToPoint(dir);
}

//----------------------------------------------------------------
// EdgeSegment
//        Provides an edge class that can be used to determine 
//        where edges intersect.
class EdgeSegment
{
public:
  EdgeSegment() { inUVSpace = true; }
  EdgeSegment(const vertex_t &pt1, const vertex_t &pt2) 
  { this->pt1 = pt1; this->pt2 = pt2; inUVSpace = true; }
  double SegmentsIntersect(const EdgeSegment& segment, pnt2d_t &intersection);

  void ToUV(const pnt2d_t &min, const vec2d_t &ext);
  void ToXY(const pnt2d_t &min, const vec2d_t &ext);

  vertex_t pt1;
  vertex_t pt2;

private:
  bool inUVSpace;
};

}

#endif


