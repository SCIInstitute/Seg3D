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


//----------------------------------------------------------------
// calc_tile_mosaic_bbox
//
bool
calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
                      
                      // image space bounding boxes of the tile:
                      const pnt2d_t & tile_min,
                      const pnt2d_t & tile_max,
                      
                      // mosaic space bounding boxes of the tile:
                      pnt2d_t & mosaic_min,
                      pnt2d_t & mosaic_max,
                      
                      // sample points along the image edges:
                      const unsigned int np)
{
  // initialize an empty bounding box:
  mosaic_min[0] = std::numeric_limits<double>::max();
  mosaic_min[1] = mosaic_min[0];
  mosaic_max[0] = -mosaic_min[0];
  mosaic_max[1] = -mosaic_min[0];
  
  // it happens:
  if (tile_min[0] == std::numeric_limits<double>::max() || !mosaic_to_tile)
  {
    return true;
  }
  
  base_transform_t::Pointer tile_to_mosaic = mosaic_to_tile->GetInverseTransform();
  if (tile_to_mosaic.GetPointer() == NULL)
  {
    return false;
  }
  
  double W = tile_max[0] - tile_min[0];
  double H = tile_max[1] - tile_min[1];
  
  // a temporary vector to hold the sample points:
  std::vector<pnt2d_t> xy((np + 1) * 4);
  
  // corner points:
  xy[0] = pnt2d(tile_min[0], tile_min[1]);
  xy[1] = pnt2d(tile_min[0], tile_max[1]);
  xy[2] = pnt2d(tile_max[0], tile_min[1]);
  xy[3] = pnt2d(tile_max[0], tile_max[1]);
  
  // edge points:
  for (unsigned int j = 0; j < np; j++)
  {
    const double t = double(j + 1) / double(np + 1);
    double x = tile_min[0] + t * W;
    double y = tile_min[1] + t * H;
    
    unsigned int offset = (j + 1) * 4;
    xy[offset + 0] = pnt2d(x, tile_min[1]);
    xy[offset + 1] = pnt2d(x, tile_max[1]);
    xy[offset + 2] = pnt2d(tile_min[0], y);
    xy[offset + 3] = pnt2d(tile_max[0], y);
  }
  
  // find the inverse mapping for each point, if possible:
  std::list<pnt2d_t> uv_list;
  for (unsigned int j = 0; j < xy.size(); j++)
  {
    pnt2d_t uv;
    if (find_inverse(tile_min,
                     tile_max,
                     mosaic_to_tile,
                     tile_to_mosaic.GetPointer(),
                     xy[j],
                     uv))
    {
      uv_list.push_back(uv);
    }
  }
  
  // calculate the bounding box of the inverse-mapped points:
  for (std::list<pnt2d_t>::const_iterator iter = uv_list.begin();
       iter != uv_list.end(); ++iter)
  {
    const pnt2d_t & uv = *iter;
    mosaic_min[0] = std::min(mosaic_min[0], uv[0]);
    mosaic_max[0] = std::max(mosaic_max[0], uv[0]);
    mosaic_min[1] = std::min(mosaic_min[1], uv[1]);
    mosaic_max[1] = std::max(mosaic_max[1], uv[1]);
  }
  
  return !uv_list.empty();
}
