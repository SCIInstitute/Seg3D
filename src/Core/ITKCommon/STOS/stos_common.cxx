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

// File         : stos_common.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/05/31 14:29
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for slice to slice registration.

// local includes:
#include <Core/ITKCommon/common.hxx>

// boost:
#include <boost/filesystem.hpp>

// system includes:
#include <fstream>

namespace bfs=boost::filesystem;

typedef std::list<bfs::path> path_list;

//----------------------------------------------------------------
// load_slice
// 
bool
load_slice(const char * fn_mosaic,
	   const bool & flip_mosaic,
	   const unsigned int & shrink_factor,
	   const double & clahe_slope,
	   image_t::Pointer & mosaic,
	   mask_t::Pointer & mosaic_mask,
	   bool mosaic_mask_enabled,
	   bool dont_allocate)
{
  std::cout << "    reading " << fn_mosaic;
  
  std::fstream fin;
  fin.open(fn_mosaic, std::ios::in);
  if (!fin.is_open())
  {
    std::cout << ", failed...." << std::endl;
    return false;
  }
  std::cout << std::endl;
  
  path_list fn_tiles;
  std::vector<base_transform_t::Pointer> transform;
  std::vector<image_t::ConstPointer> image;
  std::vector<mask_t::ConstPointer> mask;
  
  double pixel_spacing = 1.0;
  bool use_std_mask = false;
  load_mosaic<base_transform_t>(fin,
				pixel_spacing,
				use_std_mask,
				fn_tiles,
				transform,
        "testme");
  fin.close();
  
  unsigned int num_images = transform.size();
  assert(num_images > 1);
  assert(pixel_spacing != 0);
  
  image.resize(num_images);
  mask.resize(num_images);
  
  unsigned int i = 0;
  for (path_list::const_iterator iter = fn_tiles.begin();
       iter != fn_tiles.end(); ++iter, i++)
  {
    const bfs::path & fn_image = (*iter);
    
    // read the image:
    std::cout << std::setw(3) << i << " ";
    image[i] = std_tile<image_t>(fn_image, shrink_factor, pixel_spacing);
    
    if (use_std_mask)
    {
      mask[i] = std_mask<image_t>(image[i]);
    }
  }
  
  // assempble the mosaic:
  std::cout << "    assembling a mosaic " << std::endl;
  mosaic = make_mosaic<image_t::ConstPointer, base_transform_t::Pointer>
    (FEATHER_BLEND_E, transform, image, mask, dont_allocate);
  
  // reset the tile image origin and spacing:
  image_t::PointType origin = mosaic->GetOrigin();
  origin[0] = 0;
  origin[1] = 0;
  mosaic->SetOrigin(origin);
  
  if (mosaic_mask_enabled)
  {
    std::cout << "    assembling a mosaic mask " << std::endl;
    mosaic_mask = make_mask<base_transform_t::Pointer>(transform, mask);
    mosaic_mask->SetOrigin(origin);
  }
  
  // turn over the mosaic:
  if (flip_mosaic)
  {
    mosaic = flip<image_t>(mosaic);
    
    if (mosaic_mask_enabled)
    {
      mosaic_mask = flip<mask_t>(mosaic_mask);
    }
  }
  
  // preprocess the image with Contrast Limited Adaptive Histogram
  // Equalization algorithm, remap the intensities into the [0, 1] range:
  if (clahe_slope > 1.0)
  {
    std::cout << "    enhancing contrast with CLAHE " << std::endl;
    mosaic = CLAHE<image_t>(mosaic, 64, 64, clahe_slope, 256, 0.0, 1.0);
  }
  
  return true;
}
