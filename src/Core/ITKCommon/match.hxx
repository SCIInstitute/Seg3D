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

// File         : match.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/10 15:41
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Feature matching helper functions --
//                SIFT descriptor key matching and match filtering.

#ifndef MATCH_HXX_
#define MATCH_HXX_

// local includes:
#include <Core/ITKCommon/extrema.hxx>
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/common.hxx>


//----------------------------------------------------------------
// match_t
// 
class match_t
{
public:
  match_t():
  a_(NULL),
  b_(NULL),
  error_(std::numeric_limits<double>::max()),
  r_(1.0)
  {}
  
  match_t(const descriptor_t * a,
          const descriptor_t * b,
          const double & error,
          const double & r):
  a_(a),
  b_(b),
  error_(error),
  r_(r)
  {
    assert(a != NULL && b != NULL);
  }
  
  // this will be used to sort the matches from best to worst:
  inline bool operator < (const match_t & match) const
  {
    // FIXME:
    assert(a_ != NULL && b_ != NULL);
    
    return error_ < match.error_;
  }
  
  // matched keys:
  const descriptor_t * a_;
  const descriptor_t * b_;
  
  // mismatch error in descriptor space:
  double error_;
  
  // ratio of error_ to the error_ of the next best match:
  double r_;
};


//----------------------------------------------------------------
// load_image
// 
extern image_t::Pointer
load_image(const bfs::path & fn_load,
           const unsigned int & shrink_factor,
           const double & pixel_spacing);


//----------------------------------------------------------------
// setup_pyramid
// 
extern void
setup_pyramid(pyramid_t & pyramid,
              const unsigned int index,
              const bfs::path & fn_load,
              const image_t * image,
              const mask_t * image_mask,
              const unsigned int & descriptor_version,
              unsigned int num_scales = 1,
              const bool & generate_keys = true,
              const bfs::path & fn_debug = "");

//----------------------------------------------------------------
// match_keys
// 
extern void
match_keys(const pyramid_t & a,
           const pyramid_t & b,
           std::list<match_t> & ab_list,
           std::list<const match_t *> & ab,
           const double & percentage_to_keep);

//----------------------------------------------------------------
// prefilter_matches_v1
// 
extern void
prefilter_matches_v1(const bfs::path & fn_prefix,
                     const double & peak_ratio_threshold,
                     const std::list<const match_t *> & complete,
                     std::list<const match_t *> & filtered);

//----------------------------------------------------------------
// prefilter_matches_v2
// 
extern void
prefilter_matches_v2(const bfs::path & fn_prefix,
                     const double & distortion_threshold,
                     const std::list<const match_t *> & complete,
                     std::list<const match_t *> & filtered);

//----------------------------------------------------------------
// rematch_keys
// 
extern void
rematch_keys(const bfs::path & fn_prefix,
             const pyramid_t & pa, // FIXME: remove this
             const pyramid_t & pb, // FIXME: remove this
             const std::list<descriptor_t> & a,
             const std::list<descriptor_t> & b,
             const base_transform_t * t_ab,
             const double & window_radius,
             std::list<match_t> & ab);

//----------------------------------------------------------------
// rematch_keys
// 
extern void
rematch_keys(const bfs::path & fn_prefix,
             const pyramid_t & a,
             const pyramid_t & b,
             const base_transform_t * t_ab,
             const double & window_radius,
             std::list<match_t> & ab_list,
             std::list<const match_t *> & ab,
             const double & percentage_to_keep);

extern void
bestfit_stats(const std::vector<const match_t *> & ab,
              const std::list<unsigned int> & inliers);


//----------------------------------------------------------------
// solve_for_parameters
// 
template <class transform_t>
static void
solve_for_parameters(// the transform being solved for:
                     transform_t * t_ab,
                     const unsigned int & start_with_degree,
                     const unsigned int & degrees_included,
                     
                     // match pairs:
                     const std::vector<const match_t *> & ab,
                     
                     // the list of matches that produced the best fit:
                     const std::list<unsigned int> & bestfit)
{
  std::vector<image_t::PointType> uv(bestfit.size());
  std::vector<image_t::PointType> xy(bestfit.size());
  
  unsigned int position = 0;
  for (std::list<unsigned int>::const_iterator i = bestfit.begin();
       i != bestfit.end();
       ++i, ++position)
  {
    const unsigned int & index = *i;
    const match_t * match = ab[index];
    const image_t::PointType & a_uv = match->a_->extrema_->local_coords_;
    const image_t::PointType & b_xy = match->b_->extrema_->local_coords_;
    
    uv[position] = a_uv;
    xy[position] = b_xy;
  }
  
  // fit to all detected inliers:
  t_ab->solve_for_parameters(start_with_degree, degrees_included, uv, xy);
}

//----------------------------------------------------------------
// refine_inliers
// 
template <class transform_t>
bool
refine_inliers(const transform_t * t_ab,
               const double & t,
               const std::vector<const match_t *> & ab,
               std::vector<bool> & is_inlier,
               std::list<unsigned int> & inliers,
               double & model_quality)
{
  // shortcut:
  const unsigned int & num_matches = ab.size();
  
  const double inlier_threshold = t * t;
  double inlier_error = 0.0;
  double total_error = 0.0;
  model_quality = 0.0;
  
  bool removed_inliers = false;
  bool added_inliers = false;
  for (unsigned int i = 0; i < num_matches; i++)
  {
    // transform one of the points in the match pair and see how
    // closely it lands to its counterpart:
    const match_t * match = ab[i];
    const image_t::PointType & a_uv = match->a_->extrema_->local_coords_;
    const image_t::PointType & b_xy = match->b_->extrema_->local_coords_;
    
    image_t::PointType a_xy = t_ab->TransformPoint(a_uv);
    double dx = (a_xy[0] - b_xy[0]);// / t_ab->GetXmax();
    double dy = (a_xy[1] - b_xy[1]);// / t_ab->GetYmax();
    double d2 = dx * dx + dy * dy;
    total_error += d2;
    
    // FIXME: allow for looser tolerances at the coarser resolution levels:
    // double s = integer_power<double>(2.0, match->a_->extrema_->octave_);
    // double inlier_threshold = (t * s) * (t * s);
    
    if (d2 <= inlier_threshold)
    {
      // FIXME:
      inlier_error += d2;
      // inlier_error += sqrt(d2);
      
      if (!is_inlier[i])
      {
        inliers.push_back(i);
        is_inlier[i] = true;
        added_inliers = true;
      }
    }
    else if (is_inlier[i])
    {
      inliers.remove(i);
      is_inlier[i] = false;
      removed_inliers = true;
    }
  }
  
  const unsigned int & inliers_count = inliers.size();
  
  if (inliers_count > 0)
  {
    //#if 1
    model_quality = double(inliers_count);
    //#else
    //    // model_quality = 1.0 / total_error;
    //    // model_quality = num_matches / total_error;
    //    // model_quality = inliers_count / total_error;
    //    // model_quality = inliers_count / inlier_error;
    //    /* model_quality =
    //     (double(inliers_count) * double(inliers_count)) /
    //     (1.0 + sqrt(inlier_error));
    //     */
    //#endif
  }
  
  return removed_inliers || added_inliers;
}

//----------------------------------------------------------------
// RANSAC
// 
template <class transform_t>
static bool
RANSAC(// RANSAC controls:
       const unsigned int & k,
       const unsigned int & n,
       const unsigned int & d,
       const double & t,
       
       // transform controls:
       const transform_t * initial_t_ab,
       const unsigned int & start_with_degree,
       const unsigned int & degrees_included,
       
       // match pairs:
       const std::vector<const match_t *> & ab,
       
       // pass back the results via these variables:
       typename transform_t::ParametersType & best_params,
       std::list<unsigned int> & bestfit,
       double & bestquality)
{
  const double initial_bestquality = bestquality;
  best_params = initial_t_ab->GetParameters();
  
  typename transform_t::Pointer t_ab = transform_t::New();
  t_ab->SetFixedParameters(initial_t_ab->GetFixedParameters());
  t_ab->SetParameters(initial_t_ab->GetParameters());
  
  // shortcut:
  const unsigned int & num_matches = ab.size();
  
  // at least n sample points are required to estimate the model parameters:
  if (num_matches < n) return false;
  if (degrees_included == 0) return false;
  
  // bookkeeping arrays:
  std::vector<image_t::PointType> uv(n);
  std::vector<image_t::PointType> xy(n);
  
  bool start_with_bestfit = bestfit.size() > 0;
  
  for (unsigned int i = 0; i < k; i++)
  {
    std::list<unsigned int> inliers;
    std::vector<bool> is_inlier;
    is_inlier.assign(num_matches, false);
    
    if (!start_with_bestfit)
    {
      // pick n randomly selected matches:
      for (unsigned int j = 0; j < n; j++)
      {
        // assume the matches are sorted in the order of increasing mismatch,
        // implement importance sampling to take advantage of this:
        unsigned int index;
        while (true)
        {
          // importance sampling of the matches,
          // better matches are sampled more frequently:
          double nx = double(num_matches);
          double q = drand();
          /*
           // PDF(x) = ((1 - x/nx)^2) / nx
           // CDF(x) = (x/nx - 1)^3 + 1
           double x = nx * (1.0 - pow(q, 1.0 / 3.0));
           */
          
          // uniform sampling:
          double x = nx * q;
          
          index = std::min(num_matches - 1, (unsigned int)(x));
          if (!is_inlier[index]) break;
        }
        
        inliers.push_back(index);
        is_inlier[index] = true;
        
        const match_t * match = ab[index];
        uv[j] = match->a_->extrema_->local_coords_;
        xy[j] = match->b_->extrema_->local_coords_;
      }
      
      // setup transform based on the selected matches:
      t_ab->solve_for_parameters(start_with_degree, degrees_included, uv, xy);
    }
    else
    {
      // re-initialize with previous best fit data:
      std::list<unsigned int>::const_iterator iter = bestfit.begin();
      for (unsigned int j = 0; j < bestfit.size(); j++, ++iter)
      {
        const unsigned int & id = *iter;
        inliers.push_back(id);
        is_inlier[id] = true;
      }
      
      solve_for_parameters<transform_t>(t_ab,
                                        start_with_degree,
                                        degrees_included,
                                        ab,
                                        inliers);
      start_with_bestfit = false;
    }
    
    // find other potential inliers and calculate the error measure:
    double model_quality = std::numeric_limits<double>::max();
    bool inliers_were_altered = refine_inliers<transform_t>(t_ab,
                                                            t,
                                                            ab,
                                                            is_inlier,
                                                            inliers,
                                                            model_quality);
    
    if (model_quality > bestquality)
    {
      // allow for the removal of outliers (in case the original
      // inliers were poor):
      double quality = model_quality;
      for (unsigned int j = 0; j < 100 && inliers_were_altered; j++)
      {
        if (inliers.size() < n) break;
        
        // update the transform to reflect the changes to the inliers:
        solve_for_parameters<transform_t>(t_ab,
                                          start_with_degree,
                                          degrees_included,
                                          ab,
                                          inliers);
        
        // re-calculate the error measure, update the inliers:
        inliers_were_altered = refine_inliers<transform_t>(t_ab,
                                                           t,
                                                           ab,
                                                           is_inlier,
                                                           inliers,
                                                           quality);
      }
      
      if (quality > bestquality && inliers.size() >= n)
      {
        solve_for_parameters<transform_t>(t_ab,
                                          start_with_degree,
                                          degrees_included,
                                          ab,
                                          inliers);
       	
        // save the better result:
        bestquality = quality;
        best_params = t_ab->GetParameters();
        bestfit = inliers;
        std::cout << "FIXME: quality: " << bestquality
        << ", iteration: " << i << std::endl;
      }
    }
  }
  
  return initial_bestquality < bestquality;
}

//----------------------------------------------------------------
// match
// 
// Match the pyramids with a Legendre polynomial transform of a
// given order, where order == degree of the polynomial + 1.
// 
// 2nd order == affine transform
// 3rd order == quadratic transform
// 4th order == cubic polynomial
//
template <typename transform_t>
typename transform_t::Pointer
match(const unsigned int order,
      const pyramid_t & a,
      const pyramid_t & b,
      const bfs::path & fn_debug)
{
  // a threshold value for determining when a data point fits a model:
  const double t = 2.0 * a.octave_[0].L_[0]->GetSpacing()[0]
  * (1.0 + integer_power<double>(2.0, a.octaves()));
  
  // precompute the key matches:
  std::cout << "matching " << a.fn_data_ << " to " << b.fn_data_ << std::endl;
  std::list<match_t> ab_list;
  std::list<const match_t *> ab_sorted;
  match_keys(a, b, ab_list, ab_sorted, 1.0);
  
  std::cout << "pre-filtering the key matches, t: " << t << std::endl;
  std::list<const match_t *> ab_sorted_filtered;
  // prefilter_matches_v1(fn_debug, 0.61, ab_sorted, ab_sorted_filtered);
  prefilter_matches_v2(fn_debug, 0.1, ab_sorted, ab_sorted_filtered);
  
  // maximum number of RANSAC iterations:
  // const unsigned int k = 20 * ab_sorted.size();
  const unsigned int k = 33333;
  std::cout << "RANSAC will try " << k << " iterations (instead of "
  << 20 * ab_sorted.size() << ")" << std::endl;
  
#ifdef VIS_DEBUG
  visualize_matches_v2(a, b, ab_sorted_filtered, fn_debug);
#endif
  
  // initialise the transform from pyramid a to pyramid b:
  typename transform_t::Pointer t_ab =
  setup_transform<transform_t, image_t>(b.octave_[0].L_[0]);
  
  // image center distortion will be used later to estimate the translation
  // vector (needed for stable higher order parameter estimation):
  image_t::PointType center;
  center[0] = t_ab->GetUc();
  center[1] = t_ab->GetVc();
  
  // estimate affine transform parameters:
  std::vector<const match_t *> ab;
  ab.assign(ab_sorted_filtered.begin(), ab_sorted_filtered.end());
  
  std::cout << "estimating the affine transform" << std::endl;
  typename transform_t::ParametersType params_01;
  std::list<unsigned int> bestfit;
  double bestquality = 0.0;
  
  const unsigned int initial_order = std::min(2u, order);
  const unsigned int initial_coeff =
  transform_t::count_coefficients(0, initial_order);
  
  if (RANSAC<transform_t>(k,
                          initial_coeff,
                          initial_coeff * 2,
                          t,
                          t_ab, 0, initial_order,
                          ab,
                          params_01, bestfit, bestquality))
  {
    t_ab->SetParameters(params_01);
    
    // FIXME:
    bestfit_stats(ab, bestfit);
    
    // FIXME:
#ifdef VIS_DEBUG
    visualize_best_fit(fn_debug + "d01a-",
                       a.octave_[0].L_[0],
                       b.octave_[0].L_[0],
                       t_ab,
                       ab,
                       bestfit,
                       a.octave_[0].mask_,
                       b.octave_[0].mask_);
#endif // VIS_DEBUG
    
#ifdef TRY_REMATCHING
    // re-match the keys with correct position information:
    std::cout << "re-matching the keys based on the affine transform estimate"
    << std::endl;
    rematch_keys(fn_debug, a, b, t_ab,
                 4.0 * t,
                 ab_list, ab_sorted, 1.0);
    
    // FIXME: do I really want to do this again?
    ab_sorted_filtered.clear();
    prefilter_matches_v2(fn_debug, 0.1, ab_sorted, ab_sorted_filtered);
    
#ifdef VIS_DEBUG
    if (fn_debug.size() != 0)
    {
      visualize_matches_v2(a, b, ab_sorted_filtered, fn_debug + "rematched");
    }
#endif // VIS_DEBUG
    
    // ab.assign(ab_sorted.begin(), ab_sorted.end());
    ab.assign(ab_sorted_filtered.begin(), ab_sorted_filtered.end());
    
    // re-estimate the affine transform using the rematched keys:
    std::cout << "re-estimating the affine transform" << std::endl;
    bestquality = 0.0;
    bestfit.clear();
    if (RANSAC<transform_t>(k,
                            initial_coeff,
                            initial_coeff * 2,
                            t,
                            t_ab, 0, initial_order,
                            ab,
                            params_01, bestfit, bestquality))
    {
      t_ab->SetParameters(params_01);
      
      // FIXME:
      bestfit_stats(ab, bestfit);
      
      // move the translation vector into the fixed parameters of
      // the transform and re-estimate the affine parameters:
      image_t::PointType shifted_center = t_ab->TransformPoint(center);
      double shift_x = shifted_center[0] - center[0];
      double shift_y = shifted_center[1] - center[1];
      t_ab->setup_translation(shift_x, shift_y);
      solve_for_parameters<transform_t>
      (t_ab, 0, initial_order, ab, bestfit);
      
      // FIXME:
#ifdef VIS_DEBUG
      visualize_best_fit(fn_debug + "d01b-",
                         a.octave_[0].L_[0],
                         b.octave_[0].L_[0],
                         t_ab,
                         ab,
                         bestfit,
                         a.octave_[0].mask_,
                         b.octave_[0].mask_);
#endif // VIS_DEBUG
      
      if (order > 2)
      {
        unsigned int remaining_degrees = std::min(2u, order - 2);
        unsigned int remaining_coeff = 
        transform_t::count_coefficients(2, remaining_degrees);
        
        // perform high order parameter estimation:
        std::cout << "estimating the higher order transform "
        << "(affine parameters remain fixed)" << std::endl;
        typename transform_t::ParametersType params_0123;
        
        // FIXME: perhaps this should be on a switch?
        // bestquality = 0.0;
        bestquality *= 0.75;
        
        if (RANSAC<transform_t>(k,
                                remaining_coeff,
                                remaining_coeff * 2,
                                t,
                                t_ab, 2, remaining_degrees,
                                ab,
                                params_0123, bestfit, bestquality))
        {
          t_ab->SetParameters(params_0123);
          
          // FIXME:
          bestfit_stats(ab, bestfit);
          
          // re-solve for affine transform in order to accomodate the new
          // inliers detected due to the higher order polynomial coefficients
          // of the transform, then re-solve for the higher order
          // polynomial coefficients:
          solve_for_parameters<transform_t>
          (t_ab, 0, 2, ab, bestfit);
          
          solve_for_parameters<transform_t>
          (t_ab, 2, remaining_degrees, ab, bestfit);
          
          // FIXME:
#ifdef VIS_DEBUG
          visualize_best_fit(fn_debug + "d0123-",
                             a.octave_[0].L_[0],
                             b.octave_[0].L_[0],
                             t_ab,
                             ab,
                             bestfit,
                             a.octave_[0].mask_,
                             b.octave_[0].mask_);
#endif // VIS_DEBUG
        }
      }
    }
#endif // TRY_REMATCHING
  }
  
  return t_ab;
}


#endif // MATCH_HXX_
