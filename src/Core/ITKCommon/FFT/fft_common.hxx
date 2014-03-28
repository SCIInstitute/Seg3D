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

// File         : fft_common.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2005/11/10 14:05
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for image alignment (registration)
//                using phase correlation to find the translation vector.

#ifndef FFT_COMMON_HXX_
#define FFT_COMMON_HXX_

// local includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/FFT/fft.hxx>
#include <Core/ITKCommon/the_dynamic_array.hxx>

// system includes:
#include <math.h>
#include <list>
#include <limits.h>
#include <sstream>

#ifndef WIN32
#include <unistd.h>
#endif

// namespace access:
using namespace itk_fft;

//----------------------------------------------------------------
// DEBUG_PDF
// 
// #define DEBUG_PDF

//#ifdef DEBUG_PDF
//// FIXME: this is not thread safe:
//#define DEBUG_PDF_PFX the_text_t("PDF-")
//#define DEBUG_CLUSTERS
//#define DEBUG_MARKERS
//extern unsigned int DEBUG_COUNTER1;
//extern unsigned int DEBUG_COUNTER2;
//#else
#define DEBUG_PDF_PFX ""
//#endif


//----------------------------------------------------------------
// local_max_t
// 
class local_max_t
{
public:
  local_max_t(const double value,
              const double x,
              const double y,
              const unsigned int area):
  value_(value),
  x_(x),
  y_(y),
  area_(area)
  {}
  
  inline bool operator == (const local_max_t & lm) const
  { return (value_ == lm.value_) && (x_ == lm.x_) && (y_ == lm.y_); }
  
  inline bool operator < (const local_max_t & lm) const
  { return value_ < lm.value_; }
  
  inline bool operator > (const local_max_t & lm) const
  { return value_ > lm.value_; }
  
  // cluster value:
  double value_;
  
  // center of mass of the cluster:
  double x_;
  double y_;
  
  // area of the cluster:
  unsigned int area_;
};

//----------------------------------------------------------------
// operator << 
// 
inline std::ostream &
operator << (std::ostream & sout, const local_max_t & lm)
{
  return sout << lm.value_ << '\t'
  << lm.x_ << '\t'
  << lm.y_ << '\t'
  << lm.area_ << std::endl;
}

//----------------------------------------------------------------
// cluster_bbox_t
// 
class cluster_bbox_t
{
public:
  cluster_bbox_t()
  { reset(); }
  
  void reset()
  {
    min_[0] = std::numeric_limits<int>::max();
    min_[1] = std::numeric_limits<int>::max();
    max_[0] = std::numeric_limits<int>::min();
    max_[1] = std::numeric_limits<int>::min();
  }
  
  void update(int x, int y)
  {
    min_[0] = std::min(min_[0], x);
    min_[1] = std::min(min_[1], y);
    max_[0] = std::max(max_[0], x);
    max_[1] = std::max(max_[1], y);
  }
  
  int min_[2];
  int max_[2];
};

//----------------------------------------------------------------
// find_maxima_cm
//
// The percentage below refers to the number of pixels that fall
// below the maxima. Thus, the number of pixels above the maxima
// is 1 - percentage. This way it is possible to specify a
// thresholding value without knowing anything about the image.
// 
// The given image is thresholded, the resulting clusters/blobs
// are identified/classified, the center of mass of each cluster
// is treated as a maxima in the image.
//
// Returns the number of maxima found.
// 
extern unsigned int
find_maxima_cm(std::list<local_max_t> & max_list,
               const itk_image_t::Pointer & image,
               const double percentage = 0.9995,
               const bfs::path & prefix = DEBUG_PDF_PFX,
               const bfs::path & suffix= ".png");


//----------------------------------------------------------------
// find_correlation
// 
template <class TImage>
unsigned int
find_correlation(std::list<local_max_t> & max_list,
                 const TImage * fi,
                 const TImage * mi,
                 double lp_filter_r,
                 double lp_filter_s)
{
  itk_image_t::Pointer z0 = cast<TImage, itk_image_t>(fi);
  itk_image_t::Pointer z1 = cast<TImage, itk_image_t>(mi);
  return find_correlation<itk_image_t>(max_list,
                                       z0,
                                       z1,
                                       lp_filter_r,
                                       lp_filter_s);
}


//----------------------------------------------------------------
// find_correlation
//
template <>
unsigned int
find_correlation(std::list<local_max_t> & max_list,
                 const itk_image_t * fi,
                 const itk_image_t * mi,
                 
                 // low pass filter parameters
                 // (resampled data requires less smoothing):
                 double lp_filter_r,
                 double lp_filter_s);


//----------------------------------------------------------------
// find_correlation
//
// This is a backwards compatibility API
// 
template <class TImage>
unsigned int
find_correlation(const TImage * fi,
                 const TImage * mi,
                 std::list<local_max_t> & max_list,
                 bool resampled_data)
{
  double lp_filter_r = resampled_data ? 0.9 : 0.5;
  return find_correlation<TImage>(max_list, fi, mi, lp_filter_r, 0.1);
}


//----------------------------------------------------------------
// threshold_maxima
// 
// Discard maxima whose mass is below a given threshold ratio
// of the total mass of all maxima:
// 
void
threshold_maxima(std::list<local_max_t> & max_list,
                 const double threshold);

//----------------------------------------------------------------
// reject_negligible_maxima
// 
// Discard maxima that are worse than the best maxima by a factor
// greater than the given threshold ratio:
// 
// Returns the size of the new list.
// 
unsigned int
reject_negligible_maxima(std::list<local_max_t> & max_list,
                         const double threshold);

//----------------------------------------------------------------
// overlap_t
// 
class overlap_t
{
public:
  overlap_t():
  id_(~0),
  overlap_(0.0)
  {}
  
  overlap_t(const unsigned int id, const double overlap):
  id_(id),
  overlap_(overlap)
  {}
  
  inline bool operator == (const overlap_t & d) const
  { return id_ == d.id_; }
  
  inline bool operator < (const overlap_t & d) const
  { return overlap_ < d.overlap_; }
  
  inline bool operator > (const overlap_t & d) const
  { return overlap_ > d.overlap_; }
  
  unsigned int id_;
  double overlap_;
};

//----------------------------------------------------------------
// reject_negligible_overlap
// 
void
reject_negligible_overlap(std::list<overlap_t> & ol,
                          const double threshold);


//----------------------------------------------------------------
// estimate_displacement
//
template <class TImage>
double
estimate_displacement(const TImage * a,
                      const TImage * b,
                      const local_max_t & lm,
                      translate_transform_t::Pointer & transform,
                      image_t::PointType offset_min,
                      image_t::PointType offset_max, 
                      const double overlap_min = 0.0,
                      const double overlap_max = 1.0,
                      const mask_t * mask_a = NULL,
                      const mask_t * mask_b = NULL)
{
// FIXME:
//#ifdef DEBUG_PDF
//  the_text_t fn_debug =
//    the_text_t::number(DEBUG_COUNTER1, 3, '0') + the_text_t("-") +
//    the_text_t::number(DEBUG_COUNTER2, 1, '0') + the_text_t("-");
//#endif
  
  itk_image_t::SizeType max_sz = calc_padding<TImage>(a, b);
  const unsigned int & w = max_sz[0];
  const unsigned int & h = max_sz[1];
  
  // evaluate 4 permutations of the maxima:
  // typedef itk::NormalizedCorrelationImageToImageMetric<TImage, TImage>
  typedef itk::MeanSquaresImageToImageMetric<TImage, TImage> metric_t;
  typedef itk::LinearInterpolateImageFunction<TImage, double> interpolator_t;
  
  typedef typename TImage::SpacingType spacing_t;
  spacing_t spacing = a->GetSpacing();
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

  std::ostringstream oss;
  double best_metric = std::numeric_limits<double>::max();
  for (unsigned int i = 0; i < 4; i++)
  {
    if ( t[i][0] < offset_min[0] || t[i][0] > offset_max[0] || 
        t[i][1] < offset_min[1] || t[i][1] > offset_max[1]  )
    {
      continue;
    }
    
    translate_transform_t::Pointer ti = translate_transform_t::New();
    ti->SetOffset(t[i]);
    
// FIXME:
//#ifdef DEBUG_PDF
//    the_text_t fn = fn_debug + the_text_t::number(i) + ".png";
//    save_rgb<TImage>(fn, a, b, ti, mask_a, mask_b);
//#endif
    
    const double area_ratio = overlap_ratio<TImage>(a, b, ti);
    
    int old_precision = oss.precision();
    oss.precision(2);
    oss << i << ": " << std::setw(3) << std::fixed << area_ratio * 100.0 << "% of overlap, ";
    oss.precision(old_precision);
    
    if (area_ratio < overlap_min || area_ratio > overlap_max)
    {
      oss << "skipping..." << std::endl;
      continue;
    }
    
    // double metric = eval_metric<metric_t, interpolator_t>(ti, a, b);
    double metric = my_metric<TImage>(a,
                                      b,
                                      ti,
                                      mask_a,
                                      mask_b,
                                      overlap_min,
                                      overlap_max);
    oss << std::scientific << metric;
    if (metric < best_metric)
    {
      transform = ti;
      best_metric = metric;
      oss << " - better..." << std::endl;
      
//#ifdef DEBUG_PDF
//      save_rgb<TImage>(fn_debug + the_text_t::number(i) + "-better.png",
//			a, b, ti, mask_a, mask_b);
//#endif
    }
    else
    {
      oss << " - worse..." << std::endl;
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
template <typename TImage, typename TMask>
double
match_one_pair(const bool images_were_resampled,
               const bool use_std_mask,
               const TImage * fi,
               const TImage * mi,
               const TMask * fi_mask,
               const TMask * mi_mask,
               
               const double overlap_min,
               const double overlap_max,
               
               image_t::PointType offset_min,
               image_t::PointType offset_max,
               
               translate_transform_t::Pointer & ti,
               std::list<local_max_t> & peaks,
               unsigned int & num_peaks,
               
               // ideally this should be one, but radial distortion may
               // generate several valid peaks (up to 4), so it may be
               // necessary to consider more peaks for the unmatched images:
               const unsigned int max_peaks)
{
//#ifdef DEBUG_PDF
//  DEBUG_COUNTER1++;
//#endif
  
  ti = NULL;
  
  unsigned int total_peaks = 0;
  if (use_std_mask)
  {
    // ugh, blank out 5 percent of the image at the bottom
    // to cover up the image id:
    typename TImage::SizeType fi_sz = fi->GetLargestPossibleRegion().GetSize();
    typename TImage::SizeType mi_sz = mi->GetLargestPossibleRegion().GetSize();
    
    unsigned int fi_y = (unsigned int)(0.95 * fi_sz[1]);
    unsigned int mi_y = (unsigned int)(0.95 * mi_sz[1]);
    
    typename TImage::Pointer fi_filled = cast<TImage, TImage>(fi);
    fill<TImage>(fi_filled, 0, fi_y, fi_sz[0], fi_sz[1] - fi_y, 0);
    
    typename TImage::Pointer mi_filled = cast<TImage, TImage>(mi);
    fill<TImage>(mi_filled, 0, mi_y, mi_sz[0], mi_sz[1] - mi_y, 0);
    
    total_peaks = find_correlation<TImage>(fi_filled,
                                           mi_filled,
                                           peaks,
                                           images_were_resampled);
  }
  else
  {
    total_peaks = find_correlation<TImage>(fi,
                                           mi,
                                           peaks,
                                           images_were_resampled);
  }
  
  num_peaks = reject_negligible_maxima(peaks, 3.0);
  std::ostringstream oss;
  oss << num_peaks << '/' << total_peaks << " eligible peaks, ";
  
  if (num_peaks > max_peaks)
  {
    oss << "skipping..." << std::endl;
    CORE_LOG_MESSAGE(oss.str());
    return std::numeric_limits<double>::max();
  }
  
  // choose the best peak:
  double best_metric = std::numeric_limits<double>::max();
  
//#ifdef DEBUG_PDF
//  DEBUG_COUNTER2 = 0;
//#endif
  for (std::list<local_max_t>::iterator j = peaks.begin();
       j != peaks.end(); ++j)
  {
    oss << "evaluating permutations..." << std::endl;
    const local_max_t & lm = *j;
    
    translate_transform_t::Pointer tmp = translate_transform_t::New();
    double metric = estimate_displacement<TImage>(fi,
                                                  mi,
                                                  lm,
                                                  tmp,
                                                  offset_min,
                                                  offset_max,
                                                  overlap_min,
                                                  overlap_max,
                                                  fi_mask,
                                                  mi_mask);
    if (metric < best_metric)
    {
      best_metric = metric;
      ti = tmp;
    }
    
//#ifdef DEBUG_PDF
//    DEBUG_COUNTER2++;
//#endif
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
template <typename TImage, typename TMask>
double
match_one_pair(const bool images_were_resampled,
               const bool use_std_mask,
               const TImage * fi,
               const TImage * mi,
               const TMask * fi_mask,
               const TMask * mi_mask,
               
               const double overlap_min,
               const double overlap_max,
               
               image_t::PointType offset_min,
               image_t::PointType offset_max,
               
               const unsigned int node_id,
               translate_transform_t::Pointer & ti,
               std::pair<unsigned int, std::list<local_max_t> > & peaks,
               
               // ideally this should be one, but radial distortion may
               // generate several valid peaks (up to 4), so it may be
               // necessary to consider more peaks for the unmatched images:
               const unsigned int max_peaks)
{
  unsigned int peak_list_size = 0;
  std::list<local_max_t> peak_list;
  double m = match_one_pair<TImage, TMask>(images_were_resampled,
                                           use_std_mask,
                                           fi,
                                           mi,
                                           fi_mask,
                                           mi_mask,
                                           overlap_min,
                                           overlap_max,
                                           offset_min,
                                           offset_max,
                                           ti,
                                           peak_list,
                                           peak_list_size,
                                           max_peaks);
  
  // this info will be used when trying to match the unmatched images:
  if (peak_list_size != 0 &&
      peak_list_size <= max_peaks &&
      (peaks.second.empty() || peaks.second.size() > peak_list_size))
  {
    peaks.first = node_id;
    peaks.second.clear();
    peaks.second.splice(peaks.second.end(), peak_list);
  }
  
  return m;
}

//----------------------------------------------------------------
// match_one_pair
// 
template <typename TImage, typename TMask>
double
match_one_pair(const bool images_were_resampled,
               const bool use_std_mask,
               const TImage * fi,
               const TImage * mi,
               const TMask * fi_mask,
               const TMask * mi_mask,
               const double overlap_min,
               const double overlap_max,
               image_t::PointType offset_min,
               image_t::PointType offset_max,
               translate_transform_t::Pointer & ti)
{
  std::list<local_max_t> peaks;
  unsigned int num_peaks = 0;
  return match_one_pair<TImage, TMask>(images_were_resampled,
                                       use_std_mask,
                                       fi,
                                       mi,
                                       fi_mask,
                                       mi_mask,
                                       overlap_min,
                                       overlap_max,
                                       offset_min,
                                       offset_max,
                                       ti,
                                       peaks,
                                       num_peaks,
                                       UINT_MAX);
}


#endif // FFT_COMMON_HXX_
