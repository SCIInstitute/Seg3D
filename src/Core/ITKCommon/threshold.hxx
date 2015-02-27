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

// File         : threshold.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/05 12:36
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for thresholding 2D image to isolate
//                peaks/clusters in the data.

#ifndef THRESHOLD_HXX_
#define THRESHOLD_HXX_

// local includes:
#include <Core/ITKCommon/common.hxx>


//----------------------------------------------------------------
// threshold_by_intensity_inplace
// 
// The image is thresholded, intensities below/above threshold are set
// to a background value, specified by bg_offset.
// 
// Depending on whether the image is being thresholded above or below
// the threshold, the background value is calculated as:
// 
//   a. keep pixels above threshold,
//      bg = clip - bg_offset * (max - clip)
// 
//   b. keep pixels below threshold,
//      bg = clip + bg_offset * (clip_max - min)
// 
// The function returns the background value of the thresholded image.
// 
template <class image_t>
double
threshold_by_intensity_inplace(typename image_t::Pointer & image,
			       const double clip_param,
			       const bool keep_pixels_above_threshold = true,
			       const unsigned int bins = 256,
			       const double bg_offset = 1e-3)
{
  typedef itk::ImageRegionConstIterator<image_t> iter_t;
  typedef typename image_t::IndexType index_t;
  
  // first find minima/maxima of the image:
  double v_min = std::numeric_limits<double>::max();
  double v_max = -v_min;
  
  iter_t iter(image, image->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    const double & v = iter.Get();
    v_min = std::min(v_min, v);
    v_max = std::max(v_max, v);
  }
  
  // calculate the min/max range:
  const double v_rng = v_max - v_min;
  
  if (v_rng == 0.0)
  {
    // there are no peaks in this image:
    return v_min;
  }
  
  double clip = v_min + clip_param * v_rng;
  
  // threshold the peaks:
  double background =
    keep_pixels_above_threshold
    ? clip - bg_offset * v_rng
    : clip + bg_offset * v_rng;
  
  if (keep_pixels_above_threshold)
  {
    image = threshold<image_t>(image, clip, v_max, background, v_max);
  }
  else
  {
    image = threshold<image_t>(image, v_min, clip, v_min, background);
  }
  
  return background;
}

//----------------------------------------------------------------
// threshold_by_intensity
// 
template <class image_t>
typename image_t::Pointer
threshold_by_intensity(const image_t * image,
		       const double clip_param,
		       const bool keep_pixels_above_threshold = true,
		       const unsigned int bins = 256,
		       const double bg_offset = 1e-3)
{
  typename image_t::Pointer peaks = cast<image_t, image_t>(image);
  threshold_by_intensity_inplace(peaks,
				 clip_param,
				 keep_pixels_above_threshold,
				 bins,
				 bg_offset);
  return peaks;
}

//----------------------------------------------------------------
// threshold_by_area_inplace
// 
// The clip_param below refers to the number of pixels that fall
// below/above the threshold. Thus, the number of pixels above
// the maxima is 1 - clip_param.
// 
// Thus, it is possible to specify a threshold value without
// knowing anything about the image.
// 
// The image is thresholded, intensities below/above threshold are set
// to a background value, specified by bg_offset.
// 
// Depending on whether the image is being thresholded above or below
// the threshold, the background value is calculated as:
// 
//   a. keep pixels above threshold,
//      bg = clip - bg_offset * (max - clip)
// 
//   b. keep pixels below threshold,
//      bg = clip + bg_offset * (clip_max - min)
// 
// The function returns the background value of the thresholded image
// 
template <class image_t>
double
threshold_by_area_inplace(typename image_t::Pointer & image,
			  const double clip_param,
			  const bool keep_pixels_above_threshold = true,
			  const unsigned int bins = 4096,
			  const double bg_offset = 1e-3)
{
  typedef itk::ImageRegionConstIterator<image_t> iter_t;
  typedef typename image_t::IndexType index_t;
  
  // first find minima/maxima of the image:
  double v_min = std::numeric_limits<double>::max();
  double v_max = -v_min;
  
  iter_t iter(image, image->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    const double v = iter.Get();
    v_min = std::min(v_min, v);
    v_max = std::max(v_max, v);
  }
  
  // calculate the min/max range:
  const double v_rng = v_max - v_min;
  
  if (v_rng == 0.0)
  {
    // there are no peaks in this image:
    return v_min;
  }
  
  // build a histogram:
  std::vector<unsigned int> pdf;
  pdf.assign(bins, 0);
  
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    const double v = iter.Get();
    const unsigned int bin =
      (unsigned int)(double((v - v_min) / v_rng) * double(bins - 1));
    pdf[bin]++;
  }
  
  // build the cumulative histogram:
  std::vector<unsigned int> cdf(bins);
  cdf[0] = pdf[0];
  for (unsigned int i = 1; i < bins; i++)
  {
    cdf[i] = cdf[i - 1] + pdf[i];
  }
  
  // calculate the total number of pixels in the image:
  typename image_t::SizeType size =
    image->GetLargestPossibleRegion().GetSize();
  
  double total_number_of_pixels = 1.0;
  for (unsigned int i = 0; i < size.GetSizeDimension(); i++)
  {
    total_number_of_pixels *= size[i];
  }
  
  // find the CDF bin that contains a given percentage of the total image:
  double clip = 0.0;
  for (unsigned int i = 1; i < bins; i++)
  {
    clip = v_min + (double(i) / double(bins - 1)) * v_rng;
    if (double(cdf[i]) >= clip_param * total_number_of_pixels)
    {
      break;
    }
  }
  
  // threshold the peaks:
  double background =
    keep_pixels_above_threshold
    ? clip - bg_offset * v_rng
    : clip + bg_offset * v_rng;
  
  if (keep_pixels_above_threshold)
  {
    image = threshold<image_t>(image, clip, v_max, background, v_max);
  }
  else
  {
    image = threshold<image_t>(image, v_min, clip, v_min, background);
  }
  
  return background;
}

//----------------------------------------------------------------
// threshold_by_area
// 
template <class image_t>
typename image_t::Pointer
threshold_by_area(const image_t * image,
		  const double clip_param,
		  const bool keep_pixels_above_threshold = true,
		  const unsigned int bins = 256,
		  const double bg_offset = 1e-3)
{
  typename image_t::Pointer peaks = cast<image_t, image_t>(image);
  threshold_by_area_inplace(peaks,
			    clip_param,
			    keep_pixels_above_threshold,
			    bins,
			    bg_offset);
  return peaks;
}


#endif // THRESHOLD_HXX_
