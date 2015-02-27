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

// File         : IRPruner.cpp
// Author       : Bradley C. Grimm
// Created      : 2009/05/15 11:52
// Copyright    : (C) 2009 University of Utah
// Description  : Used to prune uninteresting tiles.

// system includes:
#include <stdio.h>

// the includes:
#include <Core/ITKCommon/FFT/fft_common.hxx>
#include <Core/ITKCommon/mosaic_layout_common.hxx>
#include <Core/ITKCommon/IRPruner.h>

// TODO: replace cout and cerr with logging

void IRPruner::PruneImages(ImageList & image_names,
                           std::vector<base_transform_t::Pointer> &transform,
                           const unsigned int &shrink_factor,
                           const double &pixel_spacing,
                           const bool &use_std_mask,
                           const unsigned int &tile_size,
                           const double &intensity_tolerance,
                           float overlap_percent)
{
  double min_interest = std::numeric_limits<unsigned int>::max();
  double max_interest = std::numeric_limits<unsigned int>::min();

  ImageList passed;
  ImageList failed;

  unsigned int i = 0;
  for (ImageList::const_iterator fn_iter = image_names.begin(); fn_iter != image_names.end(); ++fn_iter, i++)
  {
    // Load in an image...
    image_t::Pointer image = std_tile<image_t>(*fn_iter, 
                                               shrink_factor, 
                                               pixel_spacing);

    mask_t::ConstPointer mask;
    if (use_std_mask)
    {
      mask = std_mask<image_t>(image);
    }

    image_t::RegionType image_region = image->GetBufferedRegion();
    image_t::SizeType image_size = image_region.GetSize();
    image_t::IndexType image_index = image_region.GetIndex();

    image_t::SizeType chunk_size;
    double full_count = 0.0;
    double interest_level = 0.0;
    
    float left_crop = image_size[0] * overlap_percent;
    float right_crop = image_size[0] * (1.0f - overlap_percent);
    
    float top_crop = image_size[1] * overlap_percent;
    float bottom_crop = image_size[1] * (1.0f - overlap_percent); 

    // Iterate through the image in small chunks...
    for (unsigned int x = 0, xid = 0; x < image_size[0]; x += tile_size, xid++)
    {
      if ( x > left_crop && (x+tile_size) < right_crop )
        continue;

      for (unsigned int y = 0, yid = 0; y < image_size[1]; y += tile_size, yid++)
      {
        if ( y > top_crop && (y+tile_size) < bottom_crop )
          continue;

        image_t::RegionType chunk_region;
        image_t::IndexType chunk_index;
        chunk_index[0] = x + image_index[0];
        chunk_index[1] = y + image_index[1];
        chunk_region.SetIndex( chunk_index );

        chunk_size[0] = std::min(tile_size, static_cast<unsigned int>(image_size[0] - x));
        chunk_size[1] = std::min(tile_size, static_cast<unsigned int>(image_size[1] - y));
        chunk_region.SetSize( chunk_size );

        image_t::IndexType index;
        typedef itk::ImageRegionIteratorWithIndex<image_t> iter_t;
        iter_t iter(image, chunk_region);
        
        // Find the average intensity on this chunk.
        double avg = 0.0, count = 0.0;
        for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
        {
          index = iter.GetIndex();
          double pixel_value = image->GetPixel(index);
          avg += pixel_value;
          count += 1.0;
          full_count += 1.0;
        }

        avg /= count;

        // Find how far each value is from the average.
        for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
        {
          index = iter.GetIndex();

          double pixel_value = image->GetPixel(index);
          interest_level += std::abs(pixel_value - avg);
        }
      }
    }

    double interest = (interest_level / full_count);
    if ( interest < min_interest )
      min_interest = interest;
    if ( interest > max_interest )
      max_interest = interest;
    
    if ( transform.size() != 0 )
    {
      // Save out those that passed the test.
      if ( interest > intensity_tolerance )
      {
        std::cout << "Passed the test with " << interest << ":       " << (*fn_iter) << std::endl;
        passed.push_back((*fn_iter));
      }
      else
      {
        std::cout << "Failed the test "  << interest << ":       " << (*fn_iter) << std::endl;
        failed.push_back((*fn_iter));
        if ( transform.size() > i )
          transform.erase(transform.begin()+i,transform.begin()+i+1);
        i--;
      }
    }
    else
      std::cout << (*fn_iter) << ": " << interest << std::endl;

    interest_level = 0.0;
  }
  
  if ( transform.size() != 0 )
  {
    std::cout << "Images that passed:" << std::endl;
    for (ImageList::const_iterator fn_iter = passed.begin();
         fn_iter != passed.end(); ++fn_iter)
    {
      std::cout << "  " << (*fn_iter) << std::endl;
    }

    std::cout << std::endl << "Images that failed:" << std::endl;
    for (ImageList::const_iterator fn_iter = failed.begin(); fn_iter != failed.end(); ++fn_iter)
    {
      std::cout << "  " << (*fn_iter) << std::endl;
      image_names.remove( *fn_iter );
    }

    if ( min_interest > intensity_tolerance )
    {
      std::cout << "No images were pruned.  " 
        << "To prune an image, a tolerance of at least: "
      << min_interest << " is needed." << std::endl;

    }
  }
}