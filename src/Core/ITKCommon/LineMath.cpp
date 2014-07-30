// -*- Mode: c++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: nil -*-
// NOTE: the first line of this file sets up source code indentation rules
// for Emacs; it is also a hint to anyone modifying this file.

/*  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// File         : LineMath.cpp
// Author       : Bradley C. Grimm
// Created      : 2009/08/19
// Copyright    : (C) 2009 University of Utah
// License      : GPLv2
// Description  : Helper functions and definitions for using lines and doing
//                calculations on them.

// Local Includes
#include <Core/ITKCommon/LineMath.h>

using namespace ir;

//----------------------------------------------------------------
// VectorToPoint
//        Converts the data structure from an itk Vector to an itk pt.
itk::Point<double, 2> PointMath::VectorToPoint(const itk::Vector<double, 2> &pt)
{
  itk::Point<double, 2> new_pt;
  new_pt[0] = pt[0];
  new_pt[1] = pt[1];
  return new_pt;
}

//----------------------------------------------------------------
// Scale
//        Gives the direction of the line (vector from pt2 to pt1)
itk::Point<double, 2> PointMath::Scale(const pnt2d_t &pt1, const pnt2d_t &pt2, double factor)
{
    double new_distance = 
      ir::PointMath::Distance(pt1, pt2) * factor;
    
    itk::Point<double, 2> new_point = PointMath::Direction(pt1, pt2);
    new_point[0] *= new_distance;
    new_point[1] *= new_distance;
    new_point[0] += pt1[0];
    new_point[1] += pt1[1];

    return new_point;
}

//----------------------------------------------------------------
// ToUV
//        Converts the the given point to a range between 0.0 - 1.0.
pnt2d_t PointMath::XYtoUV(const pnt2d_t &xy, const pnt2d_t &min, const vec2d_t &ext)
{
  pnt2d_t uv = xy;
  uv[0] -= min[0];
  uv[1] -= min[1];
  uv[0] /= ext[0];
  uv[1] /= ext[1];
  return uv;
}

//----------------------------------------------------------------
// ToXY
//        Converts the given point into the given xy range.
pnt2d_t PointMath::UVtoXY(const pnt2d_t &uv, const pnt2d_t &min, const vec2d_t &ext)
{
  pnt2d_t xy = uv;
  xy[0] *= ext[0];
  xy[1] *= ext[1];
  xy[0] += min[0];
  xy[1] += min[1];
  return xy;
}


//----------------------------------------------------------------
// SegmentsIntersect
//        Returns if two lines intersect.  If they do it also gives
//        the intersection point.
//        Note: This compares the xy of one to the uv of the other.
double EdgeSegment::SegmentsIntersect(const EdgeSegment& segment, 
                                      itk::Point<double, 2> &intersection)
{
  // Code inspired from online forum posting.
  itk::Point<double, 2> DP;
  itk::Point<double, 2> QA;
  itk::Point<double, 2> QB;
  
  double d;
  double la;
  double lb;

  //........................................

  DP[0] = segment.pt1.uv_[0] - pt1.xy_[0]; 
  DP[1] = segment.pt1.uv_[1] - pt1.xy_[1];
  
  QA[0] = pt2.xy_[0] - pt1.xy_[0]; 
  QA[1] = pt2.xy_[1] - pt1.xy_[1];
  
  QB[0] = segment.pt2.uv_[0] - segment.pt1.uv_[0]; 
  QB[1] = segment.pt2.uv_[1] - segment.pt1.uv_[1];

  d  =   QA[1] * QB[0] - QB[1] * QA[0] ;
  la = ( QB[0] * DP[1] - QB[1] * DP[0] ) / d ;
  lb = ( QA[0] * DP[1] - QA[1] * DP[0] ) / d ;

  // if intersection exist   0 <= la  <= 1 and  0 <= lb  <= 1
  if ( 0 <= la && la <= 1 && 0 <= lb && lb <= 1 )
  {
    intersection[0] = pt1.xy_[0] + la * QA[0];
    intersection[1] = pt1.xy_[1] + la * QA[1];

    return PointMath::Distance(pt1.xy_, intersection);
  }
  return std::numeric_limits<double>::max();
}

//----------------------------------------------------------------
// ToUV
//        Converts the uv parameter of both end points into a range
//        between 0.0 - 1.0.  Once in UV space, calling this function
//        does nothing.
void EdgeSegment::ToUV(const pnt2d_t &min, const vec2d_t &ext)
{
  if ( !inUVSpace )
  {
    pt1.uv_ = PointMath::XYtoUV(pt1.uv_, min, ext);
    pt2.uv_ = PointMath::XYtoUV(pt2.uv_, min, ext);
    inUVSpace = true;
  }
}

//----------------------------------------------------------------
// ToXY
//        Converts the uv parameter of both end points into the given
//        xy range.  Once in XY space, calling this function
//        does nothing.
void EdgeSegment::ToXY(const pnt2d_t &min, const vec2d_t &ext)
{
  if ( inUVSpace )
  {
    pt1.uv_ = PointMath::UVtoXY(pt1.uv_, min, ext);
    pt2.uv_ = PointMath::UVtoXY(pt2.uv_, min, ext);
    inUVSpace = false;
  }
}