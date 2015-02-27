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

// File         : visualize.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/05 11:09
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for visualizing unmatched
//                and matched SIFT keys and descriptors.

#ifndef VISUALIZE_HXX_
#define VISUALIZE_HXX_

// local includes:
#include <Core/ITKCommon/extrema.hxx>
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/draw.hxx>

// ITK includes:
#include <itkVector.h>

// system includes:
#include <list>
#include <vector>
#include <string>


//----------------------------------------------------------------
// draw_feature_key
// 
extern void
draw_feature_key(native_image_t::Pointer * image,
                 const double & x0,
                 const double & y0,
                 const double & direction,
                 const double & radius,
                 const xyz_t & color);

//----------------------------------------------------------------
// draw_feature_vector
// 
// x0, y0 are expressed in physical coordinates:
// 
extern void
draw_feature_vector(native_image_t::Pointer * image,
                    double x0,
                    double y0,
                    const double & direction,
                    const xyz_t & color,
                    const std::vector<double> & feature,
                    double scale = 1.0);

//----------------------------------------------------------------
// draw_keys
// 
extern void
draw_keys(native_image_t::Pointer * image,
          const std::list<descriptor_t> & keys,
          const xyz_t & rgb,
          const double & r0,
          const double & r1,
          const unsigned int octave = 0);

//----------------------------------------------------------------
// visualize_matches
// 
// visualize the matching keys:
// 
extern void
visualize_matches(const pyramid_t & a,
                  const pyramid_t & b,
                  const unsigned int & io, // octave index
                  const unsigned int & is, // scale index
                  const std::list<match_t> & ab,
                  const bfs::path & fn_prefix,
                  unsigned int num_keys = ~0);

//----------------------------------------------------------------
// visualize_best_fit
// 
extern void
visualize_best_fit(const bfs::path & fn_prefix,
                   const image_t * a_img,
                   const image_t * b_img,
                   const base_transform_t * t_ab,
                   const std::vector<const match_t *> & ab,
                   const std::list<unsigned int> & inliers,
                   const mask_t * a_mask = NULL,
                   const mask_t * b_mask = NULL);

//----------------------------------------------------------------
// visualize_matches_v2
// 
// visualize the matching keys:
// 
extern void
visualize_matches_v2(const pyramid_t & a,
                     const pyramid_t & b,
                     const std::list<const match_t *> & ab,
                     const bfs::path & fn_prefix,
                     unsigned int num_keys = ~0);

//----------------------------------------------------------------
// visualize_nn
// 
extern void
visualize_nn(const bfs::path & fn_prefix,
             const pyramid_t & a,
             const pyramid_t & b,
             const ext_wrapper_t & b_key_wrapper,
             const std::vector<ext_wrapper_t> & a_key_wrappers,
             const double & window_radius);


//----------------------------------------------------------------
// dump
// 
extern void
dump(std::ostream & so,
     const double * data,
     const unsigned int & size,
     const unsigned int & height = 10,
     const bool & normalize = false);


//----------------------------------------------------------------
// dump
// 
extern void
dump(std::ostream & so,
     const double * data_a,
     const double * data_b,
     const unsigned int & size,
     const unsigned int & height = 10,
     const bool & normalize = false);


#endif // VISUALIZE_HXX_
