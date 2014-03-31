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

// File         : pyramid.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/04 12:34
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Difference of Gaussians multi-scale image pyramid used for
//                SIFT key and descriptor generation.

#ifndef PYRAMID_HXX_
#define PYRAMID_HXX_

// local includes:
#include <Core/ITKCommon/extrema.hxx>
#include <Core/ITKCommon/common.hxx>

// ITK includes:
#include <itkGradientImageFilter.h>

#include <boost/filesystem.hpp>

// system includes:
#include <list>
#include <vector>

namespace bfs=boost::filesystem;


//----------------------------------------------------------------
// gradient_filter_t
// 
typedef itk::GradientImageFilter<image_t> gradient_filter_t;

//----------------------------------------------------------------
// gradient_image_t
// 
typedef gradient_filter_t::OutputImageType gradient_image_t;

//----------------------------------------------------------------
// gradient_t
// 
typedef gradient_image_t::PixelType gradient_t;


//----------------------------------------------------------------
// octave_t
// 
// NOTE: the following comments are by Sebastian Nowozin (or are
//       based on his comments from autopano-2.4 source code):
// 
// Generate DoG maps. The maps are organized like this:
//    0: D(s0)
//    1: D(k * s0)
//    2: D(k^2 * s0)
//   ...
//    s: D(k^s * s0)     = D(2 * s0)
//  s+1: D(k^(s+1) * s0) = D(k * 2 * s0)
// 
// So, we can start peak searching at 1 to s, and have a DoG map into
// each direction.
// 
// NOTE: Gaussian G(sigma) has the following property:
// G(sigma_1) convoled with G(sigma_2) == G(sqrt(sigma_1^2 + sigma_2^2))
// 
// Therefore, we have:
// 
// G(k^{p+1}) == G(k^p) convolved with G(sigma),
// 
// Our goal is to compute every iterations sigma value so this
// equation iteratively produces the next level.
// 
//      sigma	= sqrt{(k^{p+1})^2 - (k^p)^2}
//		= sqrt{k^{2p+2} - k^{2p}}
//		= sqrt{k^2p * (k^2 - 1)}
//		= k^p * sqrt{k^2 - 1}
// 
class octave_t
{
public:
  void setup(const unsigned int & octave,
             const image_t * L0,
             const mask_t * mask,
             const double & s0,
             const double & k,
             const unsigned int s,
             const bool & make_keys);
  
  // number of scales in this octave:
  inline unsigned int scales() const
  { return gL_.size(); }
  
  // for each scale, find the minima and maxima points of the DoF images:
  void detect_extrema(const unsigned int & pyramid,
                      const unsigned int & octave,
                      const double percent_below_threshold,
                      const bool threshold_by_area,
                      const bfs::path & fn_prefix = "");
  
  // NOTE: this operates on the results produced by detect_extrema:
  void generate_keys();
  
  // NOTE: this operates on the results produced by generate_keys:
  void generate_descriptors(const unsigned int & descriptor_version);
  
  // count the number of extrema points detected within this octave:
  unsigned int count_extrema() const;
  
  // count the number of keys detected within this octave:
  unsigned int count_keys() const;
  
  // image mask (eroded to step away from the mosaic boundaries):
  mask_t::ConstPointer mask_;
  mask_t::ConstPointer mask_eroded_;
  
  // smoothed images:
  std::vector<image_t::Pointer> L_;
  
  // DoG images, D[i] = L[i + 1] - L[i]:
  std::vector<image_t::Pointer> D_;
  
  // gradient vectors (in polar coordinates):
  std::vector<gradient_image_t::Pointer> gL_;
  
  // FIXME: minima/maxima images:
  std::vector<image_t::Pointer> raw_min_;
  std::vector<image_t::Pointer> raw_max_;
  
  // sigma[i] -> sigma used to generate L[i]:
  std::vector<double> sigma_;
  
  // extrema:
  std::vector<std::list<extrema_t> > extrema_min_;
  std::vector<std::list<extrema_t> > extrema_max_;
  
  // keys:
  std::vector<std::list<descriptor_t> > keys_min_;
  std::vector<std::list<descriptor_t> > keys_max_;
  
  // min/max sampling window radius:
  static const double r0_;
  static const double r1_;
};

//----------------------------------------------------------------
// pyramid_t
// 
class pyramid_t
{
public:
  void setup(const image_t * initial_image,
             const mask_t * mask,
             const double &  initial_sigma,
             const unsigned int s,
             const double min_edge = 96.0,
             const bool & make_keys = true);
  
  // number of octaves in this pyramid:
  inline unsigned int octaves() const
  { return octave_.size(); }
  
  // helper functions:
  bool remove_lowest_resolution_octave(const unsigned int how_many = 1);
  bool remove_highest_resolution_octave(const unsigned int how_many = 1);
  
  // find the extrema points within each octave:
  void detect_extrema(const unsigned int & pyramid,
                      const double percent_below_threshold,
                      const bool threshold_by_area = true,
                      const bfs::path & fn_prefix = "");
  
  // NOTE: this operates on the results produced by detect_extrema:
  void generate_keys();
  
  // NOTE: this operates on the results produced by generate_keys:
  void generate_descriptors(const unsigned int & descriptor_version);
  
  // count the number of extrema points detected within this pyramid:
  unsigned int count_extrema() const;
  
  // count the number of keys detected within this pyramid:
  unsigned int count_keys() const;
  
  // save a bunch of images with the keys marked on them in color:
  void debug(const bfs::path & fn_prefix) const;
  
  // use this to speed up pyramid setup:
  bool save(const bfs::path & fn_save) const;
  bool load(const bfs::path & fn_save);
  
  // image pyramid octaves:
  std::vector<octave_t> octave_;
  
  // file name of the image that originated this pyramid:
  bfs::path fn_data_;
};

//----------------------------------------------------------------
// load_pyramid
// 
extern void
load_pyramid(const bfs::path & fn_load,
             pyramid_t & pyramid,
             image_t::Pointer & mosaic,
             mask_t::Pointer & mosaic_mask);


#endif // PYRAMID_HXX_
