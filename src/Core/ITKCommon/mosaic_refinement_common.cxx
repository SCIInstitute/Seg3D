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

// File         : mosaic_refinement_common.cxx
// Author       : Pavel A. Koshevoy
// Created      : Mon Nov  3 20:26:25 MST 2008
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for automatic mosaic refinement.

// local includes:
#include <Core/ITKCommon/mosaic_refinement_common.hxx>


//----------------------------------------------------------------
// regularize_displacements
// 
void
regularize_displacements(// computed displacement vectors of the moving image
                         // grid transform control points, in mosaic space:
                         std::vector<vec2d_t> & xy_shift,
                         std::vector<double> & mass,
                         
                         image_t::Pointer & dx,
                         image_t::Pointer & dy,
                         image_t::Pointer & db,
                         
                         // median filter radius:
                         const unsigned int & median_radius)
{
  // shortcuts:
  image_t::RegionType::SizeType sz = dx->GetLargestPossibleRegion().GetSize();
  unsigned int mesh_cols = sz[0];
  unsigned int mesh_rows = sz[1];
  
  // denoise
  if (median_radius > 0)
  {
    dx = median<image_t>(dx, median_radius);
    dy = median<image_t>(dy, median_radius);
    // db = median<image_t>(db, median_radius);
  }
  
  // extend (fill in gaps):
  typedef itk::ImageRegionConstIteratorWithIndex<image_t> iter_t;
  iter_t iter(dx, dx->GetLargestPossibleRegion());
  image_t::Pointer dx_blurred = cast<image_t, image_t>(dx);
  image_t::Pointer dy_blurred = cast<image_t, image_t>(dy);
  image_t::Pointer db_blurred = cast<image_t, image_t>(db);
  
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    image_t::IndexType index = iter.GetIndex();
    if (!db->GetPixel(index))
    {
      static const double max_w = 3.0;
      double w = 0.0;
      double px = 0.0;
      double py = 0.0;
      
      // keep expanding the neighborhood until at least one
      // successful sample is found:
      
      int max_x = std::max(int(index[0]), int(mesh_cols - 1 - index[0]));
      int max_y = std::max(int(index[1]), int(mesh_rows - 1 - index[1]));
      int max_r = std::min(1, std::max(max_x, max_y));
      for (int r = 1; r <= max_r && w < max_w; r++)
      {
        image_t::IndexType ix;
        int x0 = index[0] - r;
        int x1 = index[0] + r;
        int y0 = index[1] - r;
        int y1 = index[1] + r;
        
        int d = 2 * r + 1;
        for (int o = 0; o < d; o++)
        {
          ix[0] = x0;
          ix[1] = y0 + o + 1;
          if (ix[0] >= 0 && ix[0] < int(mesh_cols) &&
              ix[1] >= 0 && ix[1] < int(mesh_rows) &&
              db->GetPixel(ix))
          {
            px += dx->GetPixel(ix);
            py += dy->GetPixel(ix);
            w += 1.0;
          }
          
          ix[0] = x1;
          ix[1] = y0 + o;
          if (ix[0] >= 0 && ix[0] < int(mesh_cols) &&
              ix[1] >= 0 && ix[1] < int(mesh_rows) &&
              db->GetPixel(ix))
          {
            px += dx->GetPixel(ix);
            py += dy->GetPixel(ix);
            w += 1.0;
          }
          
          ix[0] = x0 + o;
          ix[1] = y0;
          if (ix[0] >= 0 && ix[0] < int(mesh_cols) &&
              ix[1] >= 0 && ix[1] < int(mesh_rows) &&
              db->GetPixel(ix))
          {
            px += dx->GetPixel(ix);
            py += dy->GetPixel(ix);
            w += 1.0;
          }
          
          ix[0] = x0 + o + 1;
          ix[1] = y1;
          if (ix[0] >= 0 && ix[0] < int(mesh_cols) &&
              ix[1] >= 0 && ix[1] < int(mesh_rows) &&
              db->GetPixel(ix))
          {
            px += dx->GetPixel(ix);
            py += dy->GetPixel(ix);
            w += 1.0;
          }
        }
      }
      
      if (w != 0.0)
      {
        dx_blurred->SetPixel(index, px / w);
        dy_blurred->SetPixel(index, py / w);
        db_blurred->SetPixel(index, 1);
      }
    }
  }
  
  // blur:
  dx_blurred = smooth<image_t>(dx_blurred, 1.0);
  dy_blurred = smooth<image_t>(dy_blurred, 1.0);
  // db_blurred = smooth<image_t>(db_blurred, 1.0);
  
  dx = dx_blurred;
  dy = dy_blurred;
  db = db_blurred;
  
  // update the mesh displacement field:
  iter = iter_t(dx, dx->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    image_t::IndexType index = iter.GetIndex();
    unsigned int i = index[0] + index[1] * mesh_cols;
    
    xy_shift[i][0] = dx->GetPixel(index);
    xy_shift[i][1] = dy->GetPixel(index);
    mass[i] += db->GetPixel(index);
  }
}
