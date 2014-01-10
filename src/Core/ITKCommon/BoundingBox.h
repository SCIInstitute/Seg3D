/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ITKCOMMON_BOUNDINGBOX_H
#define CORE_ITKCOMMON_BOUNDINGBOX_H

#include <itkPoint.h>
#include <itkVector.h>

#include <Core/ITKCommon/itkTypes.h>

//----------------------------------------------------------------
// is_empty_bbox
// 
// Test whether a bounding box is empty (min > max)
// 
extern bool
is_empty_bbox(const pnt2d_t & min,
              const pnt2d_t & max);

//----------------------------------------------------------------
// is_singular_bbox
// 
// Test whether a bounding box is singular (min == max)
// 
extern bool
is_singular_bbox(const pnt2d_t & min,
                 const pnt2d_t & max);

//----------------------------------------------------------------
// clamp_bbox
// 
// Restrict a bounding box to be within given limits.
// 
extern void
clamp_bbox(const pnt2d_t & confines_min,
           const pnt2d_t & confines_max,
           pnt2d_t & min,
           pnt2d_t & max);

//----------------------------------------------------------------
// update_bbox
// 
// Expand the bounding box to include a given point.
// 
inline static void
update_bbox(pnt2d_t & min, pnt2d_t & max, const pnt2d_t & pt)
{
  if (min[0] > pt[0]) min[0] = pt[0];
  if (min[1] > pt[1]) min[1] = pt[1];
  if (max[0] < pt[0]) max[0] = pt[0];
  if (max[1] < pt[1]) max[1] = pt[1];
}


//----------------------------------------------------------------
// bbox_overlap
// 
// Test whether two bounding boxes overlap.
// 
inline bool
bbox_overlap(const pnt2d_t & min_box1, 
             const pnt2d_t & max_box1, 
             const pnt2d_t & min_box2,
             const pnt2d_t & max_box2)
{
  return
  max_box1[0] > min_box2[0] && 
  min_box1[0] < max_box2[0] &&
  max_box1[1] > min_box2[1] && 
  min_box1[1] < max_box2[1];
}

//----------------------------------------------------------------
// inside_bbox
// 
// Test whether a given point is inside the bounding box.
// 
inline bool
inside_bbox(const pnt2d_t & min, const pnt2d_t & max, const pnt2d_t & pt)
{
  return
  min[0] <= pt[0] &&
  pt[0] <= max[0] &&
  min[1] <= pt[1] &&
  pt[1] <= max[1];
}

#endif