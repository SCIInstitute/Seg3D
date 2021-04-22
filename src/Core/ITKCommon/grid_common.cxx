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

// File         : grid_common.cxx
// Author       : Pavel A. Koshevoy
// Created      : Wed Jan 10 09:31:00 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : code used to refine mesh transform control points

// the includes:
#include <Core/ITKCommon/grid_common.hxx>


//----------------------------------------------------------------
// setup_grid_transform
// 
bool
setup_grid_transform(the_grid_transform_t & transform,
                     unsigned int rows,
                     unsigned int cols,
                     const pnt2d_t & tile_min,
                     const pnt2d_t & tile_max,
                     const mask_t * tile_mask,
                     base_transform_t::ConstPointer mosaic_to_tile,
                     unsigned int max_iterations,
                     double min_step_scale,
                     double min_error_sqrd,
                     unsigned int pick_up_pace_steps)
{
  const itk::GridTransform * gt =
    dynamic_cast<const itk::GridTransform *>(mosaic_to_tile.GetPointer());
  
  std::vector<pnt2d_t> xy_arr((rows + 1) * (cols + 1));
  std::vector<pnt2d_t> xy_apx((rows + 1) * (cols + 1));
  
  image_t::Pointer dx = make_image<image_t>(cols + 1,
                                            rows + 1,
                                            1.0,
                                            0.0);
  image_t::Pointer dy = make_image<image_t>(cols + 1,
                                            rows + 1,
                                            1.0,
                                            0.0);
  
  typedef itk::LegendrePolynomialTransform<double, 1> approx_transform_t;
  
  // the mosaic to tile transform is typically more stable:
  approx_transform_t::Pointer mosaic_to_tile_approx;
  
  if (gt == nullptr)
  {
    mosaic_to_tile_approx =
      approx_transform<approx_transform_t>(tile_min,
                                           tile_max,
                                           tile_mask,
                                           mosaic_to_tile.GetPointer(),
                                           16,    // samples per edge
                                           1,     // landmark generator version
                                           true); // iterative refinement
  }
  
  // temporaries:
  vec2d_t tile_ext = tile_max - tile_min; 
  pnt2d_t uv;
  pnt2d_t pq;
  
  for (unsigned int row = 0; row <= rows; row++)
  {
    pq[1] = double(row) / double(rows);
    uv[1] = tile_min[1] + tile_ext[1] * pq[1];
    for (unsigned int col = 0; col <= cols; col++)
    {
      pq[0] = double(col) / double(cols);
      uv[0] = tile_min[0] + tile_ext[0] * pq[0];
      
      // shortcut:
      unsigned int index = row * (cols + 1) + col;
      pnt2d_t & xy = xy_arr[index];
      pnt2d_t & xy_approx = xy_apx[index];
      
      if (gt == nullptr)
      {
        // general transform:
        if (!find_inverse(tile_min,
                          tile_max,
                          mosaic_to_tile_approx.GetPointer(),
                          uv,
                          xy_approx,
                          max_iterations,
                          min_step_scale,
                          min_error_sqrd,
                          pick_up_pace_steps))
        {
          // we are screwed:
          return false;
        }
        
        if (!find_inverse(tile_min,
                          tile_max,
                          mosaic_to_tile.GetPointer(),
                          uv,
                          xy,
                          max_iterations,
                          min_step_scale,
                          min_error_sqrd,
                          pick_up_pace_steps))
        {
          xy = xy_approx;
        }
        
        pnt2d_t uv2 = mosaic_to_tile->TransformPoint(xy);
        
        // verify that the point maps back correctly within some tolerance:
        vec2d_t e_uv = uv2 - uv;
        
        // verify that the approximate and exact aren't too far apart:
        vec2d_t e_xy = xy_approx - xy;
        
        double e_uv_absolute = e_uv.GetSquaredNorm();
        
        // FIXME: this is an idea -- if the exact transform give wildely
        // differing result from the approximate transform, we may be able
        // remove such outliers via a median filter. To do that, we have to
        // maintain an image of the approximate/exact result differences:
        image_t::IndexType ix;
        ix[0] = col;
        ix[1] = row;
        dx->SetPixel(ix, e_xy[0]);
        dy->SetPixel(ix, e_xy[1]);
        
        // FIXME: this is a temporary crutch, the method outlined above
        // should be used instead:
        static const double uv_tolerance = 1e-6;
        if (e_uv_absolute > uv_tolerance)
        {
          xy = xy_approx;
        }
      }
      else
      {
        // discontinuous transform -- this is a more stable way to resample
        // the transformation mesh:
        bool ok = gt->transform_.transform_inv(pq, xy);
        assert(ok);
        assert(xy[0] == xy[0] && xy[1] == xy[1]);
        if (!ok) return false;
      }
    }
  }
  
//#if 0
//  save<native_image_t>(cast<image_t, native_image_t>
//                       (remap_min_max<image_t>(dx)),
//                       "init-error-x.tif");
//  
//  save<native_image_t>(cast<image_t, native_image_t>
//                       (remap_min_max<image_t>(dy)),
//                       "init-error-y.tif");
//#endif
  
  transform.setup(rows, cols, tile_min, tile_max, xy_arr);
  return true;
}

//----------------------------------------------------------------
// setup_mesh_transform
// 
bool
setup_mesh_transform(the_mesh_transform_t & transform,
                     unsigned int rows,
                     unsigned int cols,
                     const pnt2d_t & tile_min,
                     const pnt2d_t & tile_max,
                     const mask_t * tile_mask,
                     base_transform_t::ConstPointer mosaic_to_tile,
                     unsigned int max_iterations,
                     double min_step_scale,
                     double min_error_sqrd,
                     unsigned int pick_up_pace_steps)
{
  const itk::GridTransform * gt =
    dynamic_cast<const itk::GridTransform *>(mosaic_to_tile.GetPointer());
  
  std::vector<pnt2d_t> xy_arr((rows + 1) * (cols + 1));
  std::vector<pnt2d_t> xy_apx((rows + 1) * (cols + 1));
  
  image_t::Pointer dx = make_image<image_t>(cols + 1,
                                            rows + 1,
                                            1.0,
                                            0.0);
  image_t::Pointer dy = make_image<image_t>(cols + 1,
                                            rows + 1,
                                            1.0,
                                            0.0);
  
  typedef itk::LegendrePolynomialTransform<double, 1> approx_transform_t;
  
  // the mosaic to tile transform is typically more stable:
  approx_transform_t::Pointer mosaic_to_tile_approx;
  
  if (gt == nullptr)
  {
    mosaic_to_tile_approx =
      approx_transform<approx_transform_t>(tile_min,
                                           tile_max,
                                           tile_mask,
                                           mosaic_to_tile.GetPointer(),
                                           16,    // samples per edge
                                           1,     // landmark generator version
                                           true); // iterative refinement
  }
  
  // temporaries:
  vec2d_t tile_ext = tile_max - tile_min; 
  pnt2d_t uv;
  pnt2d_t pq;

  std::vector<pnt2d_t> uv_list((rows + 1) * (cols + 1));
  
  for (unsigned int row = 0; row <= rows; row++)
  {
    pq[1] = double(row) / double(rows);
    uv[1] = tile_min[1] + tile_ext[1] * pq[1];
    for (unsigned int col = 0; col <= cols; col++)
    {
      pq[0] = double(col) / double(cols);
      uv[0] = tile_min[0] + tile_ext[0] * pq[0];
      
      // shortcut:
      unsigned int index = row * (cols + 1) + col;
      pnt2d_t & xy = xy_arr[index];
      pnt2d_t & xy_approx = xy_apx[index];
      uv_list[index] = pq;
      
      if (gt == nullptr)
      {
        // general transform:
        if (!find_inverse(tile_min,
                          tile_max,
                          mosaic_to_tile_approx.GetPointer(),
                          uv,
                          xy_approx,
                          max_iterations,
                          min_step_scale,
                          min_error_sqrd,
                          pick_up_pace_steps))
        {
          // we are screwed:
          return false;
        }
        
        if (!find_inverse(tile_min,
                          tile_max,
                          mosaic_to_tile.GetPointer(),
                          uv,
                          xy,
                          max_iterations,
                          min_step_scale,
                          min_error_sqrd,
                          pick_up_pace_steps))
        {
          xy = xy_approx;
        }
        
        pnt2d_t uv2 = mosaic_to_tile->TransformPoint(xy);
        
        // verify that the point maps back correctly within some tolerance:
        vec2d_t e_uv = uv2 - uv;
        
        // verify that the approximate and exact aren't too far apart:
        vec2d_t e_xy = xy_approx - xy;
        
        double e_uv_absolute = e_uv.GetSquaredNorm();
        
        // FIXME: this is an idea -- if the exact transform give wildely
        // differing result from the approximate transform, we may be able
        // remove such outliers via a median filter. To do that, we have to
        // maintain an image of the approximate/exact result differences:
        image_t::IndexType ix;
        ix[0] = col;
        ix[1] = row;
        dx->SetPixel(ix, e_xy[0]);
        dy->SetPixel(ix, e_xy[1]);
        
        // FIXME: this is a temporary crutch, the method outlined above
        // should be used instead:
        static const double uv_tolerance = 1e-6;
        if (e_uv_absolute > uv_tolerance)
        {
          xy = xy_approx;
        }
      }
      else
      {
        // discontinuous transform -- this is a more stable way to resample
        // the transformation mesh:
        bool ok = gt->transform_.transform_inv(pq, xy);
        assert(ok);
        assert(xy[0] == xy[0] && xy[1] == xy[1]);
        if (!ok) return false;
      }
    }
  }
  
//#if 0
//  save<native_image_t>(cast<image_t, native_image_t>
//                       (remap_min_max<image_t>(dx)),
//                       "init-error-x.tif");
//  
//  save<native_image_t>(cast<image_t, native_image_t>
//                       (remap_min_max<image_t>(dy)),
//                       "init-error-y.tif");
//#endif
  
  transform.setup(tile_min, tile_max, uv_list, xy_arr);
  return true;
}
