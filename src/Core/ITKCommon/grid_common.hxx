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

// File         : grid_common.hxx
// Author       : Pavel A. Koshevoy
// Created      : Wed Jan 10 09:31:00 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : code used to refine mesh transform control points

#ifndef GRID_COMMON_HXX_
#define GRID_COMMON_HXX_

// the includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/FFT/fft_common.hxx>
#include <Core/ITKCommon/Transform/the_grid_transform.hxx>
#include <Core/ITKCommon/Transform/itkGridTransform.h>
#include <Core/ITKCommon/Optimizers/itkRegularStepGradientDescentOptimizer2.h>
#include <Core/ITKCommon/Optimizers/optimizer_observer.hxx>

#include <Core/Utils/Log.h>

#include <boost/filesystem.hpp>

// system includes:
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <time.h>

// ITK includes:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImageRegistrationMethod.h>
#include <itkMultiResolutionImageRegistrationMethod.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>

namespace bfs=boost::filesystem;


//----------------------------------------------------------------
// DEBUG_REFINE_ONE_POINT
//
// #define DEBUG_REFINE_ONE_POINT

//----------------------------------------------------------------
// DEBUG_ESTIMATE_DISPLACEMENT
//
// #define DEBUG_ESTIMATE_DISPLACEMENT

#if defined(DEBUG_REFINE_ONE_POINT) || defined(DEBUG_ESTIMATE_DISPLACEMENT)
static int COUNTER = 0;
#endif


//----------------------------------------------------------------
// optimizer_t
//
typedef itk::RegularStepGradientDescentOptimizer2 optimizer_t;


//----------------------------------------------------------------
// setup_grid_transform
//
extern bool
setup_grid_transform(the_grid_transform_t & transform,
         unsigned int rows,
         unsigned int cols,
         const pnt2d_t & tile_min,
         const pnt2d_t & tile_max,
         const mask_t * tile_mask,
         base_transform_t::ConstPointer mosaic_to_tile,
         unsigned int max_iterations = 100,
         double min_step_scale = 1e-12,
         double min_error_sqrd = 1e-16,
         unsigned int pick_up_pace_steps = 5 // successful steps to pick up pace
);

//----------------------------------------------------------------
// setup_mesh_transform
//
extern bool
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
                     unsigned int pick_up_pace_steps);

//----------------------------------------------------------------
// estimate_displacement
//
template <class TImage>
void
estimate_displacement(double & best_metric, // current best metric
                      translate_transform_t::Pointer & best_transform,

                      // origin offset of the small neighborhood of image A
                      // within the large image A:
                      const vec2d_t & offset,

                      const TImage * a, // large image A
                      const TImage * b, // small image B

                      // this could be the size of B (ususally when matching
                      // small neighborhoods in A and B), or the
                      // maximum dimesions of A and B (plain image matching):
                      const typename TImage::SizeType & period_sz,

                      const local_max_t & lm,
                      const double overlap_min = 0.05,
                      const double overlap_max = 1.0,
                      const mask_t * mask_a = nullptr,
                      const mask_t * mask_b = nullptr,

                      const unsigned int num_perms = 4)
{
  vec2d_t best_offset = best_transform->GetOffset();

  const unsigned int & w = period_sz[0];
  const unsigned int & h = period_sz[1];

  typedef typename TImage::SpacingType spacing_t;
  spacing_t spacing = b->GetSpacing();
  double sx = spacing[0];
  double sy = spacing[1];

  // evaluate 4 permutation of the maxima:
  const double x = lm.x_;
  const double y = lm.y_;

  const vec2d_t t[] = {
    vec2d(sx * x, sy * y),
    vec2d(sx * (x - w), sy * y),
    vec2d(sx * x, sy * (y - h)),
    vec2d(sx * (x - w), sy * (y - h))
  };

#ifdef DEBUG_ESTIMATE_DISPLACEMENT
  static const bfs::path fn_save("/tmp/estimate_displacement-");
  static int COUNTER2 = 0;
  bfs::path suffix =
    the_text_t::number(COUNTER, 3, '0') + "-" +
    the_text_t::number(COUNTER2, 3, '0') + "-";
  COUNTER2++;

  // FIXME:
  {
    translate_transform_t::Pointer ti = translate_transform_t::New();
    ti->SetOffset(-offset);
    save_rgb<TImage>(fn_save + suffix + "0-init.png",
                     a,
                     b,
                     ti,
                     mask_a,
                     mask_b);
  }
#endif

  std::ostringstream oss;
  for (unsigned int i = 0; i < num_perms; i++)
  {
    translate_transform_t::Pointer ti = translate_transform_t::New();
    ti->SetOffset(t[i] - offset);

    double area_ratio = overlap_ratio<TImage>(a, b, ti);
    oss << i << ": " << std::setw(3) << static_cast<int>(area_ratio * 100.0) << "% of overlap, ";

    if (area_ratio < overlap_min || area_ratio > overlap_max)
    {
      oss << "skipping..." << std::endl;
      continue;
    }

//#if 0
//    typedef itk::LinearInterpolateImageFunction<TImage, double> interpolator_t;
//    // typedef itk::MeanSquaresImageToImageMetric<TImage, TImage> metric_t;
//    typedef itk::NormalizedCorrelationImageToImageMetric<TImage, TImage>
//      metric_t;
//    double metric = eval_metric<metric_t, interpolator_t>(ti,
//                a,
//                b,
//                mask_a,
//                mask_b);
//#else
    double metric = my_metric<TImage>(area_ratio,
                                      a,
                                      b,
                                      ti,
                                      mask_a,
                                      mask_b,
                                      overlap_min,
                                      overlap_max);
    //#endif

    oss << metric;
    if (metric < best_metric)
    {
      best_offset = t[i];
      best_metric = metric;
      oss << " - better..." << std::endl;

#ifdef DEBUG_ESTIMATE_DISPLACEMENT
      // FIXME:
      save_rgb<TImage>(fn_save + suffix +
                       the_text_t::number(i + 1) + "-perm.png",
                       a,
                       b,
                       ti,
                       mask_a,
                       mask_b);
#endif
    }
    else
    {
      oss << " - worse..." << std::endl;
    }
  }

  CORE_LOG_MESSAGE(oss.str());
  best_transform->SetOffset(best_offset);
}


//----------------------------------------------------------------
// match_one_pair
//
// match two images, find the best matching transform and
// return the corresponding match metric value
//
template <class TImage>
double
match_one_pair(translate_transform_t::Pointer & best_transform,
               const TImage * fi,
               const mask_t * fi_mask,

               const TImage * mi,
               const mask_t * mi_mask,

               // this could be the size of B (ususally when matching
               // small neighborhoods in A and B), or the
               // maximum dimesions of A and B (plain image matching):
               const typename TImage::SizeType & period_sz,

               const double & overlap_min,
               const double & overlap_max,

               image_t::PointType offset_min,
               image_t::PointType offset_max,

               const double & lp_filter_r,
               const double & lp_filter_s,

               const unsigned int max_peaks,
               const bool & consider_zero_displacement)
{
  static const vec2d_t offset = vec2d(0, 0);
  best_transform = nullptr;

  std::list<local_max_t> max_list;
  unsigned int total_peaks = find_correlation<TImage>(max_list,
                                                      fi,
                                                      mi,
                                                      lp_filter_r,
                                                      lp_filter_s);

  unsigned int num_peaks = reject_negligible_maxima(max_list, 2.0);
  std::ostringstream oss;
  oss << num_peaks << '/' << total_peaks << " eligible peaks, ";

  if (num_peaks > max_peaks)
  {
    oss << "skipping..." << std::endl;
    CORE_LOG_MESSAGE(oss.str());
    return std::numeric_limits<double>::max();
  }

  double best_metric = std::numeric_limits<double>::max();
  best_transform = translate_transform_t::New();
  best_transform->SetIdentity();

  if (consider_zero_displacement)
  {
    // make sure to consider the no-displacement case:
    estimate_displacement<TImage>(best_metric,
                                  best_transform,
                                  offset,
                                  fi,
                                  mi,
                                  period_sz,
                                  local_max_t(0, 0, 0, 0),
                                  0,
                                  1,
                                  fi_mask,
                                  mi_mask,
                                  1); // don't try permutations
  }

  // choose the best peak:
  for (std::list<local_max_t>::iterator j = max_list.begin();
       j != max_list.end(); ++j)
  {
    oss << "evaluating permutations..." << std::endl;
    const local_max_t & lm = *j;

    double prev_metric = best_metric;
    vec2d_t prev_offset = best_transform->GetOffset();

    estimate_displacement<TImage>(best_metric,
                                  best_transform,
                                  offset,
                                  fi,
                                  mi,
                                  period_sz,
                                  lm, // local maxima record
                                  overlap_min,
                                  overlap_max,
                                  fi_mask,
                                  mi_mask);

    // re-evaluate the overlap in the context of the small neighborhoods:
    double overlap = overlap_ratio(fi, mi, best_transform);
    if (overlap < overlap_min || overlap > overlap_max)
    {
      best_metric = prev_metric;
      best_transform->SetOffset(prev_offset);
    }
  }
  CORE_LOG_MESSAGE(oss.str());

  return best_metric;
}


//----------------------------------------------------------------
// match_one_pair
//
// match two images, find the best matching transform and
// return the corresponding match metric value
//
template <class TImage>
double
match_one_pair(translate_transform_t::Pointer & best_transform,
               const TImage * fi_large,
               const mask_t * fi_large_mask,

               const TImage * fi,
               const mask_t * /* fi_mask */,

               const TImage * mi,
               const mask_t * mi_mask,

               // origin offset of the small fixed image neighborhood
               // within the full image:
               const vec2d_t & offset,

               const double & overlap_min,
               const double & overlap_max,

               //image_t::PointType offset_min,
               //image_t::PointType offset_max,

               const double & lp_filter_r,
               const double & lp_filter_s,

               const unsigned int max_peaks,
               const bool & consider_zero_displacement)
{
  best_transform = nullptr;

  std::list<local_max_t> max_list;
  unsigned int total_peaks = find_correlation<TImage>(max_list,
                                                      fi,
                                                      mi,
                                                      lp_filter_r,
                                                      lp_filter_s);

  unsigned int num_peaks = reject_negligible_maxima(max_list, 2.0);
  std::ostringstream oss;
  oss << num_peaks << '/' << total_peaks << " eligible peaks, ";

  if (num_peaks > max_peaks)
  {
    oss  << "skipping..." << std::endl;
    CORE_LOG_MESSAGE(oss.str());
    return std::numeric_limits<double>::max();
  }

  double best_metric = std::numeric_limits<double>::max();
  best_transform = translate_transform_t::New();
  best_transform->SetIdentity();

  typename TImage::SizeType period_sz = mi->GetLargestPossibleRegion().GetSize();

  if (consider_zero_displacement)
  {
    // make sure to consider the no-displacement case:
    estimate_displacement<TImage>(best_metric,
                                  best_transform,
                                  offset,
                                  fi_large,
                                  mi,
                                  period_sz,
                                  local_max_t(0, 0, 0, 0),
                                  0,
                                  1,
                                  fi_large_mask,
                                  mi_mask,
                                  1); // don't try permutations
  }

  // choose the best peak:
  for (std::list<local_max_t>::iterator j = max_list.begin();
       j != max_list.end(); ++j)
  {
    oss << "evaluating permutations..." << std::endl;
    const local_max_t & lm = *j;

    double prev_metric = best_metric;
    vec2d_t prev_offset = best_transform->GetOffset();

    estimate_displacement<TImage>(best_metric,
                                  best_transform,
                                  offset,
                                  fi_large,
                                  mi,
                                  period_sz,
                                  lm, // local maxima record
                                  overlap_min,
                                  overlap_max,
                                  fi_large_mask,
                                  mi_mask);

    // re-evaluate the overlap in the context of the small neighborhoods:
    double overlap = overlap_ratio(fi, mi, best_transform);
    if (overlap < overlap_min || overlap > overlap_max)
    {
      best_metric = prev_metric;
      best_transform->SetOffset(prev_offset);
    }
  }
  CORE_LOG_MESSAGE(oss.str());

  return best_metric;
}


//----------------------------------------------------------------
// refine_one_point_fft
//
template <typename TImage>
bool
refine_one_point_fft(vec2d_t & shift,
                     const pnt2d_t & origin,

                     // the large images and their masks:
                     const TImage * tile_0,
                     const mask_t * mask_0,

                     // the extracted small neighborhoods and their masks:
                     const TImage * img_0,
                     const mask_t * msk_0,
                     const TImage * img_1,
                     const mask_t * msk_1)
{
  // feed the two neighborhoods into the FFT translation estimator:
  translate_transform_t::Pointer translate;
  double metric =
  match_one_pair<TImage>(translate, // best translation transform

                         // fixed image in full:
                         tile_0,
                         mask_0,

                         // fixed image small neighborhood:
                         img_0,
                         msk_0,

                         // moving image small neighborhood:
                         img_1,
                         msk_1,

                         // origin offset of the small fixed image
                         // neighborhood in the large image:
                         vec2d(origin[0], origin[1]),

                         0.25,// overlap min
                         1.0, // overlap max
                         0.5, // low pass filter r
                         0.1, // low pass filter s
                         10,  // number of peaks
                         true); // consider the no-displacement case

#ifdef DEBUG_REFINE_ONE_POINT
  static const bfs::path fn_save("/tmp/refine_one_point_fft-");
  bfs::path suffix = the_text_t::number(COUNTER, 3, '0');

  save_composite(fn_save + suffix + "-a.png",
                 img_0,
                 img_1,
                 identity_transform_t::New(),
                 true);
#endif // DEBUG_REFINE_ONE_POINT

#if defined(DEBUG_REFINE_ONE_POINT) || defined(DEBUG_ESTIMATE_DISPLACEMENT)
  COUNTER++;
#endif

  if (metric == std::numeric_limits<double>::max())
  {
    return false;
  }

#ifdef DEBUG_REFINE_ONE_POINT
  save_composite(fn_save + suffix + "-b.png",
                 img_0,
                 img_1,
                 translate.GetPointer(),
                 true);
#endif // DEBUG_REFINE_ONE_POINT

  shift = -translate->GetOffset();
  return true;
}


//----------------------------------------------------------------
// refine_one_point_helper
//
template <typename TImage>
bool
refine_one_point_helper(const pnt2d_t & center,
                        const pnt2d_t & origin,
                        const double & min_overlap,

                        // the large images and their masks:
                        const TImage * tile_0,
                        const mask_t * mask_0,
                        const TImage * tile_1,
                        const mask_t * mask_1,

                        // transform that maps from the space of
                        // tile 0 to the space of tile 1:
                        const base_transform_t * t_01,

                        // the extracted small neighborhoods and their masks:
                        TImage * img_0,
                        mask_t * msk_0,
                        TImage * img_1,
                        mask_t * msk_1,

                        // neighborhood size and pixel spacing:
                        const typename TImage::SizeType & sz,
                        const typename TImage::SpacingType & sp)
{
  // setup the image interpolators:
  typedef itk::LinearInterpolateImageFunction<TImage, double> interpolator_t;
  typename interpolator_t::Pointer interpolator[] = {
    interpolator_t::New(),
    interpolator_t::New()
  };

  interpolator[0]->SetInputImage(tile_0);
  interpolator[1]->SetInputImage(tile_1);

  if (!interpolator[0]->IsInsideBuffer(center))
  {
    // don't bother extracting neigborhoods if the neighborhood center
    // doesn't fall inside both images:
    return false;
  }

  // temporaries:
  pnt2d_t mosaic_pt;
  pnt2d_t tile_pt;
  typename TImage::IndexType index;

  // keep count of pixel in the neighborhood:
  unsigned int area[] = { 0, 0 };

  // extract a neighborhood of the given point from both tiles:
  for (unsigned int y = 0; y < sz[1]; y++)
  {
    mosaic_pt[1] = origin[1] + double(y) * sp[1];
    index[1] = y;
    for (unsigned int x = 0; x < sz[0]; x++)
    {
      mosaic_pt[0] = origin[0] + double(x) * sp[0];
      index[0] = x;

      // fixed image:
      tile_pt = mosaic_pt;
      if (interpolator[0]->IsInsideBuffer(tile_pt) &&
          pixel_in_mask<mask_t>(mask_0, tile_pt))
      {
        double p = interpolator[0]->Evaluate(tile_pt);
        img_0->SetPixel(index, (unsigned char)(std::min(255.0, p)));
        msk_0->SetPixel(index, 1);
        area[0]++;
      }
      else
      {
        img_0->SetPixel(index, 0);
        msk_0->SetPixel(index, 0);
      }

      // moving image:
      tile_pt = t_01->TransformPoint(mosaic_pt);
      if (interpolator[1]->IsInsideBuffer(tile_pt) &&
          pixel_in_mask<mask_t>(mask_1, tile_pt))
      {
        double p = interpolator[1]->Evaluate(tile_pt);
        img_1->SetPixel(index, (unsigned char)(std::min(255.0, p)));
        msk_1->SetPixel(index, 1);
        area[1]++;
      }
      else
      {
        img_1->SetPixel(index, 0);
        msk_1->SetPixel(index, 0);
      }
    }
  }

  // skip points which don't have enough neighborhood information:
  double max_area = double(sz[0] * sz[1]);
  double a[] = {
    double(area[0]) / max_area,
    double(area[1]) / max_area
  };

  if (a[0] < min_overlap || a[1] < min_overlap)
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------
// refine_one_point_fft
//
template <typename TImage>
bool
refine_one_point_fft(vec2d_t & shift,
                     const pnt2d_t & center,
                     const pnt2d_t & origin,
                     const double & min_overlap,

                     // the large images and their masks:
                     const TImage * tile_0,
                     const mask_t * mask_0,
                     const TImage * tile_1,
                     const mask_t * mask_1,

                     // transform that maps from the space of
                     // tile 0 to the space of tile 1:
                     const base_transform_t * t_01,

                     // the extracted small neighborhoods and their masks:
                     TImage * img_0,
                     mask_t * msk_0,
                     TImage * img_1,
                     mask_t * msk_1,

                     // neighborhood size and pixel spacing:
                     const typename TImage::SizeType & sz,
                     const typename TImage::SpacingType & sp)
{
  if (!refine_one_point_helper<TImage>(center,
                                       origin,
                                       min_overlap,
                                       tile_0,
                                       mask_0,
                                       tile_1,
                                       mask_1,
                                       t_01,
                                       img_0,
                                       msk_0,
                                       img_1,
                                       msk_1,
                                       sz,
                                       sp))
  {
    return false;
  }

  return refine_one_point_fft<TImage>(shift,
                                      origin,
                                      tile_0,
                                      mask_0,
                                      img_0,
                                      msk_0,
                                      img_1,
                                      msk_1);
}

//----------------------------------------------------------------
// refine_one_point_fft
//
template <typename TImage>
bool
refine_one_point_fft(vec2d_t & shift,
                     const pnt2d_t & center,
                     const double & min_overlap,

                     const TImage * tile_0,
                     const mask_t * mask_0,
                     const TImage * tile_1,
                     const mask_t * mask_1,

                     // transform that maps from the space of
                     // tile 0 to the space of tile 1:
                     const base_transform_t * t_01,

                     // neighborhood size:
                     const unsigned int & neighborhood)
{
  typename TImage::SpacingType sp = tile_1->GetSpacing();
  typename TImage::SizeType sz;
  sz[0] = neighborhood;
  sz[1] = neighborhood;

  pnt2d_t origin(center);
  origin[0] -= 0.5 * double(sz[0]) * sp[0];
  origin[1] -= 0.5 * double(sz[1]) * sp[1];

  typename TImage::Pointer img[] = {
    make_image<TImage>(sz),
    make_image<TImage>(sz)
  };

  mask_t::Pointer msk[] = {
    make_image<mask_t>(sz),
    make_image<mask_t>(sz)
  };

  img[0]->SetSpacing(sp);
  img[1]->SetSpacing(sp);
  msk[0]->SetSpacing(sp);
  msk[1]->SetSpacing(sp);

  return refine_one_point_fft<TImage>(shift,
                                      center,
                                      origin,
                                      min_overlap,
                                      tile_0,
                                      mask_0,
                                      tile_1,
                                      mask_1,
                                      t_01,
                                      img[0],
                                      msk[0],
                                      img[1],
                                      msk[1],
                                      sz,
                                      sp);
}

//----------------------------------------------------------------
// refine_one_point_fft
//
template <typename TImage>
bool
refine_one_point_fft(vec2d_t & shift,

                     // the extracted small neighborhoods and their masks:
                     const TImage * img_0,
                     const mask_t * msk_0,
                     const TImage * img_1,
                     const mask_t * msk_1)
{
  typename TImage::SizeType period_sz =
  img_1->GetLargestPossibleRegion().GetSize();

  // feed the two neighborhoods into the FFT translation estimator:
  translate_transform_t::Pointer translate;
  double metric =
  match_one_pair<TImage>(translate, // best translation transform

                         // fixed image small neighborhood:
                         img_0,
                         msk_0,

                         // moving image small neighborhood:
                         img_1,
                         msk_1,

                         period_sz,

                         0.25,// overlap min
                         1.0, // overlap max
                         0.5, // low pass filter r
                         0.1, // low pass filter s
                         10,  // number of peaks
                         true); // consider the no-displacement case

#ifdef DEBUG_REFINE_ONE_POINT
  static const bfs::path fn_save("/tmp/refine_one_point_fft-");
  bfs::path suffix = the_text_t::number(COUNTER, 3, '0');

  save_composite(fn_save + suffix + "-a.png",
                 img_0,
                 img_1,
                 identity_transform_t::New(),
                 true);
#endif // DEBUG_REFINE_ONE_POINT

#if defined(DEBUG_REFINE_ONE_POINT) || defined(DEBUG_ESTIMATE_DISPLACEMENT)
  COUNTER++;
#endif

  if (metric == std::numeric_limits<double>::max())
  {
    return false;
  }

#ifdef DEBUG_REFINE_ONE_POINT
  save_composite(fn_save + suffix + "-b.png",
                 img_0,
                 img_1,
                 translate.GetPointer(),
                 true);
#endif // DEBUG_REFINE_ONE_POINT

  shift = -translate->GetOffset();
  return true;
}


//----------------------------------------------------------------
// extract
//
// Return the number of pixels in the region in the mask:
//
template <typename TImage, typename TMask>
unsigned int
extract(const typename TImage::PointType & origin,

        const TImage * tile,
        const TMask * mask,

        TImage * tile_region,
        TMask *  mask_region,

        const typename TImage::PixelType & bg =
        itk::NumericTraits<typename TImage::PixelType>::Zero)
{
  tile_region->SetOrigin(origin);
  mask_region->SetOrigin(origin);

  // setup the image interpolator:
  typedef itk::LinearInterpolateImageFunction<TImage, double> interpolator_t;
  typename interpolator_t::Pointer interpolator = interpolator_t::New();
  interpolator->SetInputImage(tile);

  // temporary:
  typename TImage::PointType tile_pt;
  typename TImage::PixelType tile_val;
  typename TMask::IndexType mask_ix;
  typename TMask::PixelType mask_val;

  static const typename TMask::PixelType mask_min =
  itk::NumericTraits<typename TMask::PixelType>::Zero;

  static const typename TMask::PixelType mask_max =
  itk::NumericTraits<typename TMask::PixelType>::One;

  unsigned int num_pixels = 0;

  typedef itk::ImageRegionIteratorWithIndex<TImage> itex_t;
  itex_t itex(tile_region, tile_region->GetLargestPossibleRegion());
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    tile_val = bg;
    mask_val = mask_min;

    tile_region->TransformIndexToPhysicalPoint(itex.GetIndex(), tile_pt);
    if (interpolator->IsInsideBuffer(tile_pt))
    {
      mask_val = mask_max;
      if (mask != nullptr)
      {
        mask->TransformPhysicalPointToIndex(tile_pt, mask_ix);
        mask_val = mask->GetPixel(mask_ix);
      }

      if (mask_val != mask_min)
      {
        tile_val =
        (typename TImage::PixelType)(interpolator->Evaluate(tile_pt));
        num_pixels++;
      }
    }

    itex.Set(tile_val);
    mask_region->SetPixel(itex.GetIndex(), mask_val);
  }

  return num_pixels;
}


//----------------------------------------------------------------
// refine_one_point_helper
//
template <typename TImage>
bool
refine_one_point_helper(// the large images and their masks:
                        const TImage * tile_0,
                        const mask_t * mask_0,
                        const TImage * tile_1,
                        const mask_t * mask_1,


                        // mosaic space neighborhood center:
                        const pnt2d_t & center,
                        pnt2d_t & origin,

                        // minimum acceptable neighborhood overlap ratio:
                        const double & min_overlap,

                        // the extracted small neighborhoods and their masks:
                        TImage * img_0,
                        mask_t * msk_0,
                        TImage * img_1,
                        mask_t * msk_1)
{
  const typename TImage::SizeType & sz =
  img_0->GetLargestPossibleRegion().GetSize();
  const typename TImage::SpacingType & sp =
  img_0->GetSpacing();

  origin[0] = center[0] - (double(sz[0]) * sp[0]) / 2;
  origin[1] = center[1] - (double(sz[1]) * sp[1]) / 2;

  pnt2d_t corner[] = {
    origin + vec2d(sz[0] * sp[0], 0),
    origin + vec2d(sz[0] * sp[0], sz[1] * sp[1]),
    origin + vec2d(0, sz[1] * sp[1])
  };

  // make sure both tiles overlap the region:
  typename TImage::IndexType tmp_ix;
  if (!(tile_0->TransformPhysicalPointToIndex(origin, tmp_ix) ||
        tile_0->TransformPhysicalPointToIndex(corner[0], tmp_ix) ||
        tile_0->TransformPhysicalPointToIndex(corner[1], tmp_ix) ||
        tile_0->TransformPhysicalPointToIndex(corner[2], tmp_ix)) ||
      !(tile_1->TransformPhysicalPointToIndex(origin, tmp_ix) ||
        tile_1->TransformPhysicalPointToIndex(corner[0], tmp_ix) ||
        tile_1->TransformPhysicalPointToIndex(corner[1], tmp_ix) ||
        tile_1->TransformPhysicalPointToIndex(corner[2], tmp_ix)))
  {
    return false;
  }

  // keep count of pixel in the neighborhood:
  unsigned int area[] = { 0, 0 };

  area[0] = extract<TImage, mask_t>(origin,
                                    tile_0,
                                    mask_0,
                                    img_0,
                                    msk_0);

  area[1] = extract<TImage, mask_t>(origin,
                                    tile_1,
                                    mask_1,
                                    img_1,
                                    msk_1);

  // skip points which don't have enough neighborhood information:
  double max_area = double(sz[0] * sz[1]);
  double a[] = {
    double(area[0]) / max_area,
    double(area[1]) / max_area
  };

  if (a[0] < min_overlap || a[1] < min_overlap)
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------
// tiles_intersect
//
template <typename TImage>
bool
tiles_intersect(// the large images and their masks:
                const TImage * tile_0,
                const TImage * tile_1,

                // transforms from mosaic space to tile space:
                const base_transform_t * forward_0,
                const base_transform_t * forward_1,

                // mosaic space neighborhood center:
                const pnt2d_t & origin,

                // neighborhood size and pixel spacing:
                const typename TImage::SizeType & sz,
                const typename TImage::SpacingType & sp)
{
  pnt2d_t corner[] = {
    origin,
    origin + vec2d(sz[0] * sp[0], 0),
    origin + vec2d(sz[0] * sp[0], sz[1] * sp[1]),
    origin + vec2d(0, sz[1] * sp[1])
  };

  // temporary:
  typename TImage::IndexType tmp_ix;
  pnt2d_t tmp_pt;

  // make sure both tiles overlap the region:
  bool in[] = { false, false };

  for (unsigned int i = 0; i < 4 && (!in[0] || !in[1]) ; i++)
  {
    tmp_pt = forward_0->TransformPoint(corner[i]);
    in[0] = in[0] || tile_0->TransformPhysicalPointToIndex(tmp_pt, tmp_ix);

    tmp_pt = forward_1->TransformPoint(corner[i]);
    in[1] = in[1] || tile_1->TransformPhysicalPointToIndex(tmp_pt, tmp_ix);
  }

  return in[0] && in[1];
}

//----------------------------------------------------------------
// refine_one_point_helper
//
template <typename TImage>
bool
refine_one_point_helper(// the large images and their masks:
                        const TImage * tile_0,
                        const mask_t * mask_0,
                        const TImage * tile_1,
                        const mask_t * mask_1,

                        // transforms from mosaic space to tile space:
                        const base_transform_t * forward_0,
                        const base_transform_t * forward_1,

                        // mosaic space neighborhood center:
                        const pnt2d_t & center,

                        // minimum acceptable neighborhood overlap ratio:
                        const double & min_overlap,

                        // neighborhood size and pixel spacing:
                        const typename TImage::SizeType & sz,
                        const typename TImage::SpacingType & sp,

                        // the extracted small neighborhoods and their masks:
                        TImage * img_0_large,
                        mask_t * msk_0_large,
                        TImage * img_0,
                        mask_t * msk_0,
                        TImage * img_1,
                        mask_t * msk_1)
{
  pnt2d_t origin = center;
  origin[0] -= (double(sz[0]) * sp[0]) / 2;
  origin[1] -= (double(sz[1]) * sp[1]) / 2;

  if (!tiles_intersect<TImage>(tile_0,
                               tile_1,
                               forward_0,
                               forward_1,
                               origin,
                               sz,
                               sp))
  {
    return false;
  }

  // setup the image interpolators:
  typedef itk::LinearInterpolateImageFunction<TImage, double> interpolator_t;
  typename interpolator_t::Pointer interpolator[] = {
    interpolator_t::New(),
    interpolator_t::New()
  };

  interpolator[0]->SetInputImage(tile_0);
  interpolator[1]->SetInputImage(tile_1);

  // temporaries:
  pnt2d_t mosaic_pt;
  pnt2d_t tile_pt;
  typename TImage::IndexType index;

  // keep count of pixel in the neighborhood:
  unsigned int area[] = { 0, 0 };

  // extract a neighborhood of the given point from both tiles:
  for (unsigned int y = 0; y < sz[1]; y++)
  {
    mosaic_pt[1] = origin[1] + double(y) * sp[1];
    index[1] = y;
    for (unsigned int x = 0; x < sz[0]; x++)
    {
      mosaic_pt[0] = origin[0] + double(x) * sp[0];
      index[0] = x;

      // fixed image:
      tile_pt = forward_0->TransformPoint(mosaic_pt);
      if (interpolator[0]->IsInsideBuffer(tile_pt) &&
          pixel_in_mask<mask_t>(mask_0, tile_pt))
      {
        double p = interpolator[0]->Evaluate(tile_pt);
        img_0->SetPixel(index, (unsigned char)(std::min(255.0, p)));
        msk_0->SetPixel(index, 1);
        area[0]++;
      }
      else
      {
        img_0->SetPixel(index, 0);
        msk_0->SetPixel(index, 0);
      }

      // moving image:
      tile_pt = forward_1->TransformPoint(mosaic_pt);
      if (interpolator[1]->IsInsideBuffer(tile_pt) &&
          pixel_in_mask<mask_t>(mask_1, tile_pt))
      {
        double p = interpolator[1]->Evaluate(tile_pt);
        img_1->SetPixel(index, (unsigned char)(std::min(255.0, p)));
        msk_1->SetPixel(index, 1);
        area[1]++;
      }
      else
      {
        img_1->SetPixel(index, 0);
        msk_1->SetPixel(index, 0);
      }
    }
  }

  // skip points which don't have enough neighborhood information:
  double max_area = double(sz[0] * sz[1]);
  double a[] = {
    double(area[0]) / max_area,
    double(area[1]) / max_area
  };

  if (a[0] < min_overlap || a[1] < min_overlap)
  {
    return false;
  }

  if (img_0_large != nullptr)
  {
    // extract the larger neighborhood from the fixed tile:
    pnt2d_t origin_large(center);
    origin_large[0] -= sz[0] * sp[0];
    origin_large[1] -= sz[1] * sp[1];

    for (unsigned int y = 0; y < sz[1] * 2; y++)
    {
      mosaic_pt[1] = origin_large[1] + double(y) * sp[1];
      index[1] = y;
      for (unsigned int x = 0; x < sz[0] * 2; x++)
      {
        mosaic_pt[0] = origin_large[0] + double(x) * sp[0];
        index[0] = x;

        // fixed image:
        tile_pt = forward_0->TransformPoint(mosaic_pt);
        if (interpolator[0]->IsInsideBuffer(tile_pt) &&
            pixel_in_mask<mask_t>(mask_0, tile_pt))
        {
          double p = interpolator[0]->Evaluate(tile_pt);
          img_0_large->SetPixel(index, (unsigned char)(std::min(255.0, p)));
          msk_0_large->SetPixel(index, 1);
        }
        else
        {
          img_0_large->SetPixel(index, 0);
          msk_0_large->SetPixel(index, 0);
        }
      }
    }
  }

  return true;
}


//----------------------------------------------------------------
// refine_one_point_fft
//
template <typename TImage>
bool
refine_one_point_fft(vec2d_t & shift,

                     // fixed tile, in mosaic space:
                     const TImage * tile_0,
                     const mask_t * mask_0,

                     // moving tile, in mosaic space:
                     const TImage * tile_1,
                     const mask_t * mask_1,

                     // mosaic space neighborhood center:
                     const pnt2d_t & center,

                     // minimum acceptable neighborhood overlap ratio:
                     const double & min_overlap,

                     // the extracted small neighborhoods and their masks:
                     TImage * img_0,
                     mask_t * msk_0,
                     TImage * img_1,
                     mask_t * msk_1)
{
  pnt2d_t origin;
  if (!refine_one_point_helper<TImage>(tile_0,
                                       mask_0,
                                       tile_1,
                                       mask_1,
                                       center,
                                       origin,
                                       min_overlap,
                                       img_0,
                                       msk_0,
                                       img_1,
                                       msk_1))
  {
    return false;
  }

  return refine_one_point_fft<TImage>(shift,
                                      pnt2d(0, 0), // origin,
                                      tile_0,
                                      mask_0,
                                      img_0,
                                      msk_0,
                                      img_1,
                                      msk_1);
}


//----------------------------------------------------------------
// refine_one_point_fft
//
template <typename TImage>
bool
refine_one_point_fft(vec2d_t & shift,

                     // the large images and their masks:
                     const TImage * tile_0,
                     const mask_t * mask_0,
                     const TImage * tile_1,
                     const mask_t * mask_1,

                     // transforms from mosaic space to tile space:
                     const base_transform_t * forward_0,
                     const base_transform_t * forward_1,

                     // mosaic space neighborhood center:
                     const pnt2d_t & center,

                     // minimum acceptable neighborhood overlap ratio:
                     const double & min_overlap,

                     // neighborhood size and pixel spacing:
                     const typename TImage::SizeType & sz,
                     const typename TImage::SpacingType & sp,

                     // the extracted larger neighborhood:
                     TImage * img_0_large,
                     mask_t * msk_0_large,

                     // the extracted small neighborhoods and their masks:
                     TImage * img_0,
                     mask_t * msk_0,
                     TImage * img_1,
                     mask_t * msk_1)
{
  if (!refine_one_point_helper<TImage>(tile_0,
                                       mask_0,
                                       tile_1,
                                       mask_1,
                                       forward_0,
                                       forward_1,
                                       center,
                                       min_overlap,
                                       sz,
                                       sp,
                                       img_0_large,
                                       msk_0_large,
                                       img_0,
                                       msk_0,
                                       img_1,
                                       msk_1))
  {
    return false;
  }

  pnt2d_t origin;
  origin[0] = sz[0] * sp[0] / 2;
  origin[1] = sz[1] * sp[1] / 2;
  return refine_one_point_fft<TImage>(shift,
                                      origin,
                                      img_0_large,
                                      msk_0_large,
                                      img_0,
                                      msk_0,
                                      img_1,
                                      msk_1);
}


//----------------------------------------------------------------
// refine_one_point_fft
//
template <typename TImage>
bool
refine_one_point_fft(vec2d_t & shift,

                     // the large images and their masks:
                     const TImage * tile_0,
                     const mask_t * mask_0,
                     const TImage * tile_1,
                     const mask_t * mask_1,

                     // transforms from mosaic space to tile space:
                     const base_transform_t * forward_0,
                     const base_transform_t * forward_1,

                     // mosaic space neighborhood center:
                     const pnt2d_t & center,

                     // minimum acceptable neighborhood overlap ratio:
                     const double & min_overlap,

                     // neighborhood size and pixel spacing:
                     const typename TImage::SizeType & sz,
                     const typename TImage::SpacingType & sp,

                     // the extracted small neighborhoods and their masks:
                     TImage * img_0,
                     mask_t * msk_0,
                     TImage * img_1,
                     mask_t * msk_1)
{
  pnt2d_t origin(center);
  origin[0] -= sz[0] * sp[0] / 2;
  origin[1] -= sz[1] * sp[1] / 2;

  if (!refine_one_point_helper<TImage>(tile_0,
                                       mask_0,
                                       tile_1,
                                       mask_1,
                                       forward_0,
                                       forward_1,
                                       origin,
                                       min_overlap,
                                       sz,
                                       sp,
                                       nullptr,
                                       nullptr,
                                       img_0,
                                       msk_0,
                                       img_1,
                                       msk_1))
  {
    return false;
  }

  return refine_one_point_fft<TImage>(shift,
                                      img_0,
                                      msk_0,
                                      img_1,
                                      msk_1);
}


//----------------------------------------------------------------
// refine_one_pair
//
template <class TTransform, class TImage>
double
refine_one_pair(typename TTransform::Pointer & t01,

                const TImage * i0,
                const mask_t * m0,

                const TImage * i1,
                const mask_t * m1,

                const unsigned int levels,
                const unsigned int iterations,
                const double & min_step,
                const double & max_step)
{
  // typedef itk::MultiResolutionImageRegistrationMethod<TImage, TImage>
  typedef itk::ImageRegistrationMethod<TImage, TImage>
  registration_t;

  // setup the registration object:
  typename registration_t::Pointer registration = registration_t::New();

  // setup the image interpolator:
  typedef itk::LinearInterpolateImageFunction<TImage, double> interpolator_t;
  registration->SetInterpolator(interpolator_t::New());

  // setup the optimizer:
  optimizer_t::Pointer optimizer = optimizer_t::New();
  registration->SetOptimizer(optimizer);
  optimizer_observer_t<optimizer_t>::Pointer observer =
  optimizer_observer_t<optimizer_t>::New();
  optimizer->AddObserver(itk::IterationEvent(), observer);
  optimizer->SetMinimize(true);
  optimizer->SetNumberOfIterations(iterations);
  optimizer->SetMinimumStepLength(min_step);
  optimizer->SetMaximumStepLength(max_step);
  optimizer->SetGradientMagnitudeTolerance(1e-6);
  optimizer->SetRelaxationFactor(5e-1);
  optimizer->SetPickUpPaceSteps(5);
  optimizer->SetBackTracking(true);

  // FIXME: this is probably unnecessary:
  typedef optimizer_t::ScalesType optimizer_scales_t;
  optimizer_scales_t parameter_scales(t01->GetNumberOfParameters());
  parameter_scales.Fill(1.0);

  try { optimizer->SetScales(parameter_scales); }
  catch (itk::ExceptionObject &) {}

  // setup the image-to-image metric:
  typedef itk::NormalizedCorrelationImageToImageMetric<TImage, TImage>
  metric_t;

  typename metric_t::Pointer metric = metric_t::New();
  registration->SetMetric(metric);

  registration->SetTransform(t01);
  registration->SetInitialTransformParameters(t01->GetParameters());

  // setup the masks:
  typedef itk::ImageMaskSpatialObject<2> mask_so_t;
  mask_t::ConstPointer fi_mask = m0;
  if (m0 != nullptr)
  {
    mask_so_t::Pointer fi_mask_so = mask_so_t::New();
    fi_mask_so->SetImage(fi_mask);
    metric->SetFixedImageMask(fi_mask_so);
  }

  mask_t::ConstPointer mi_mask = m1;
  if (m1 != nullptr)
  {
    mask_so_t::Pointer mi_mask_so = mask_so_t::New();
    mi_mask_so->SetImage(mi_mask);
    metric->SetMovingImageMask(mi_mask_so);
  }

  // setup the fixed and moving image:
  typename TImage::ConstPointer fi = i0;
  typename TImage::ConstPointer mi = i1;

  registration->SetFixedImageRegion(fi->GetLargestPossibleRegion());
  registration->SetFixedImage(fi);
  registration->SetMovingImage(mi);

  // setup the multi-resolution image pyramids:
  // registration->SetNumberOfLevels(levels);

  // evaluate the metric before the registration:
  double metric_before =
  eval_metric<metric_t, interpolator_t>(t01, fi, mi, fi_mask, mi_mask);
  assert(metric_before != std::numeric_limits<double>::max());

  typename TTransform::ParametersType params_before = t01->GetParameters();
  std::ostringstream oss;

  // perform the registration:
  try
  {
    oss << std::endl;
    registration->Update();
  }
  catch (itk::ExceptionObject & exception)
  {
    oss << "image registration threw an exception: "
    << std::endl << exception.what() << std::endl;
  }
  t01->SetParameters(optimizer->GetBestParams());

  // evaluate the metric after the registration:
  double metric_after =
  eval_metric<metric_t, interpolator_t>(t01, fi, mi, fi_mask, mi_mask);

  typename TTransform::ParametersType params_after = t01->GetParameters();

  oss << "BEFORE: " << metric_before << std::endl
  << "AFTER:  " << metric_after << std::endl;

  if (metric_before <= metric_after || metric_after != metric_after)
  {
    oss << "NOTE: minimization failed, ignoring registration results..." << std::endl;
    t01->SetParameters(params_before);
    CORE_LOG_MESSAGE(oss.str());
    return metric_before;
  }

  CORE_LOG_MESSAGE(oss.str());
  return metric_after;
}


//----------------------------------------------------------------
// refine_one_point_nofft
//
template <typename TImage>
bool
refine_one_point_nofft(vec2d_t & shift,
                       const pnt2d_t & origin,

                       // the large images and their masks:
                       const TImage * tile_0,
                       const mask_t * mask_0,

                       // the extracted small neighborhood with masks:
                       const TImage * img_1,
                       const mask_t * msk_1)
{
  // feed the two neighborhoods into the optimizer translation estimator:
  translate_transform_t::Pointer translate = translate_transform_t::New();
  vec2d_t offset = vec2d(origin[0], origin[1]);
  translate->SetOffset(-offset);

  double metric =
  refine_one_pair<translate_transform_t, TImage>
  (translate,
   tile_0,
   mask_0,
   img_1,
   msk_1,
   3,   // number of pyramid levels
   100, // number of iterations
   1e-8,  // min step
   1e+4); // max step

  std::ostringstream oss;

  shift = -(translate->GetOffset() + offset);
  oss << "shift: " << shift << std::endl;
  CORE_LOG_MESSAGE(oss.str());

#ifdef DEBUG_REFINE_ONE_POINT
  static const bfs::path fn_save("/tmp/refine_one_point_nofft-");
  bfs::path suffix = the_text_t::number(COUNTER, 3, '0');
  {
    translate_transform_t::Pointer t = translate_transform_t::New();
    t->SetOffset(-offset);
    save_composite(fn_save + suffix + "-a.png",
                   tile_0,
                   img_1,
                   t.GetPointer(),
                   true);
  }
#endif // DEBUG_REFINE_ONE_POINT

#if defined(DEBUG_REFINE_ONE_POINT) || defined(DEBUG_ESTIMATE_DISPLACEMENT)
  COUNTER++;
#endif

  if (metric == std::numeric_limits<double>::max())
  {
    return false;
  }

#ifdef DEBUG_REFINE_ONE_POINT
  save_composite(fn_save + suffix + "-b.png",
                 tile_0,
                 img_1,
                 translate.GetPointer(),
                 true);
#endif // DEBUG_REFINE_ONE_POINT

  return true;
}


//----------------------------------------------------------------
// refine_one_point_nofft
//
template <typename TImage>
bool
refine_one_point_nofft(vec2d_t & shift,
                       const pnt2d_t & center,
                       const pnt2d_t & origin,
                       const double & min_overlap,

                       // the large images and their masks:
                       const TImage * tile_0,
                       const mask_t * mask_0,
                       const TImage * tile_1,
                       const mask_t * mask_1,

                       // transform that maps from the space of
                       // tile 0 to the space of tile 1:
                       const base_transform_t * t_01,

                       // the extracted small neighborhoods and their masks:
                       TImage * img_0,
                       mask_t * msk_0,
                       TImage * img_1,
                       mask_t * msk_1,

                       // neighborhood size and pixel spacing:
                       const typename TImage::SizeType & sz,
                       const typename TImage::SpacingType & sp)
{
  if (!refine_one_point_helper<TImage>(center,
                                       origin,
                                       min_overlap,
                                       tile_0,
                                       mask_0,
                                       tile_1,
                                       mask_1,
                                       t_01,
                                       img_0,
                                       msk_0,
                                       img_1,
                                       msk_1,
                                       sz,
                                       sp))
  {
    return false;
  }

  // feed the two neighborhoods into the optimizer translation estimator:
  translate_transform_t::Pointer translate = translate_transform_t::New();
  translate->SetIdentity();

  double metric =
  refine_one_pair<translate_transform_t, TImage>
  (translate,
   img_0,
   msk_0,
   img_1,
   msk_1,
   3,   // number of pyramid levels
   100, // number of iterations
   1e-8,  // min step
   1e+4); // max step

#ifdef DEBUG_REFINE_ONE_POINT
  static const bfs::path fn_save("/tmp/refine_one_point_nofft-");
  bfs::path suffix = the_text_t::number(COUNTER, 3, '0');
  save_composite(fn_save + suffix + "-a.png",
                 img_0.GetPointer(),
                 img_1.GetPointer(),
                 identity_transform_t::New(),
                 true);
#endif // DEBUG_REFINE_ONE_POINT

#if defined(DEBUG_REFINE_ONE_POINT) || defined(DEBUG_ESTIMATE_DISPLACEMENT)
  COUNTER++;
#endif

  if (metric == std::numeric_limits<double>::max())
  {
    return false;
  }

#ifdef DEBUG_REFINE_ONE_POINT
  save_composite(fn_save + suffix + "-b.png",
                 img_0.GetPointer(),
                 img_1.GetPointer(),
                 translate.GetPointer(),
                 true);
#endif // DEBUG_REFINE_ONE_POINT

  shift = -translate->GetOffset();
  return true;
}

//----------------------------------------------------------------
// refine_one_point_nofft
//
template <typename TImage>
bool
refine_one_point_nofft(vec2d_t & shift,
                       const pnt2d_t & center,
                       const double & min_overlap,
                       const TImage * tile_0,
                       const mask_t * mask_0,
                       const TImage * tile_1,
                       const mask_t * mask_1,

                       // transform that maps from the space of
                       // tile 0 to the space of tile 1:
                       const base_transform_t * t_01,

                       // neighborhood size:
                       const unsigned int & neighborhood)
{
  typename TImage::SpacingType sp = tile_1->GetSpacing();
  typename TImage::SizeType sz;
  sz[0] = neighborhood;
  sz[1] = neighborhood;

  pnt2d_t origin(center);
  origin[0] -= 0.5 * double(sz[0]) * sp[0];
  origin[1] -= 0.5 * double(sz[1]) * sp[1];

  typename TImage::Pointer img[] = {
    make_image<TImage>(sz),
    make_image<TImage>(sz)
  };

  mask_t::Pointer msk[] = {
    make_image<mask_t>(sz),
    make_image<mask_t>(sz)
  };

  img[0]->SetSpacing(sp);
  img[1]->SetSpacing(sp);
  msk[0]->SetSpacing(sp);
  msk[1]->SetSpacing(sp);

  return refine_one_point_nofft<TImage>(shift,
                                        center,
                                        origin,
                                        min_overlap,
                                        tile_0,
                                        mask_0,
                                        tile_1,
                                        mask_1,
                                        t_01,
                                        img[0],
                                        msk[0],
                                        img[1],
                                        msk[1],
                                        sz,
                                        sp);
}


#endif // GRID_COMMON_HXX_
