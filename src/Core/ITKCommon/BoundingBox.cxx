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

#include <Core/ITKCommon/BoundingBox.h>


//----------------------------------------------------------------
// is_empty_bbox
// 
// Test whether a bounding box is empty (min > max)
// 
bool
is_empty_bbox(const pnt2d_t & min,
              const pnt2d_t & max)
{
  return min[0] > max[0] || min[1] > max[1];
}

//----------------------------------------------------------------
// is_singular_bbox
// 
// Test whether a bounding box is singular (min == max)
// 
bool
is_singular_bbox(const pnt2d_t & min,
                 const pnt2d_t & max)
{
  return min == max;
}

//----------------------------------------------------------------
// clamp_bbox
// 
// Restrict a bounding box to be within given limits.
// 
void
clamp_bbox(const pnt2d_t & confines_min,
           const pnt2d_t & confines_max,
           pnt2d_t & min,
           pnt2d_t & max)
{
  if (!is_empty_bbox(confines_min, confines_max))
  {
    min[0] = std::min(confines_max[0], std::max(confines_min[0], min[0]));
    min[1] = std::min(confines_max[1], std::max(confines_min[1], min[1]));
    
    max[0] = std::min(confines_max[0], std::max(confines_min[0], max[0]));
    max[1] = std::min(confines_max[1], std::max(confines_min[1], max[1]));
  }
}
