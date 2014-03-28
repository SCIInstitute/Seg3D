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

// File         : match.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/10 15:41
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Feature matching helper functions --
//                SIFT descriptor key matching and match filtering.

// local includes:
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/visualize.hxx>
#include <Core/ITKCommon/extrema.hxx>
#include <Core/ITKCommon/threshold.hxx>
#include <Core/ITKCommon/tree.hxx>
#include <Core/ITKCommon/histogram.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/the_text.hxx>


//----------------------------------------------------------------
// VIS_DEBUG
// 
#define VIS_DEBUG

//----------------------------------------------------------------
// TRY_REMATCHING
// 
#define TRY_REMATCHING


//----------------------------------------------------------------
// wrap
//
template <class wrapper_t>
static void
wrap(wrapper_t & wrapper, const descriptor_t * key)
{
  wrapper.key_ = key;
}


//----------------------------------------------------------------
// update_target_space_data
// 
static void
update_target_space_data(const pyramid_t & a, const base_transform_t * t_ab)
{
  static itk::Vector<double, 2> x_axis = vec2d(1.0, 0.0);
  
  for (unsigned int i = 0; i < a.octaves(); i++)
  {
    const octave_t & octave = a.octave_[i];
    for (unsigned int j = 0; j < octave.scales(); j++)
    {
      // update the target space extrema coordinates:
      const std::list<extrema_t> & extrema_min = octave.extrema_min_[j];
      const std::list<extrema_t> & extrema_max = octave.extrema_max_[j];
      std::list<extrema_t>::const_iterator iter;
      
      for (iter = extrema_min.begin(); iter != extrema_min.end(); ++iter)
      {
        const extrema_t & ext = *iter;
        ext.target_coords_ = t_ab->TransformPoint(ext.local_coords_);
      }
      
      for (iter = extrema_max.begin(); iter != extrema_max.end(); ++iter)
      {
        const extrema_t & ext = *iter;
        ext.target_coords_ = t_ab->TransformPoint(ext.local_coords_);
      }
      
      // update the target space descriptor orientation:
      const std::list<descriptor_t> & keys_min = octave.keys_min_[j];
      const std::list<descriptor_t> & keys_max = octave.keys_max_[j];
      std::list<descriptor_t>::const_iterator jter;
      
      for (jter = keys_min.begin(); jter != keys_min.end(); ++jter)
      {
        const descriptor_t & key = *jter;
        image_t::PointType x =
        t_ab->TransformPoint(key.extrema_->local_coords_ + x_axis);
        
        itk::Vector<double, 2> x_axis_rotated =
        x - key.extrema_->target_coords_;
        
        double rotation = calc_angle(x_axis_rotated[0], x_axis_rotated[1]);
        key.target_orientation_ =
        clamp_angle(key.local_orientation_ + rotation);
      }
      
      for (jter = keys_max.begin(); jter != keys_max.end(); ++jter)
      {
        const descriptor_t & key = *jter;
        image_t::PointType x =
        t_ab->TransformPoint(key.extrema_->local_coords_ + x_axis);
        
        itk::Vector<double, 2> x_axis_rotated =
        x - key.extrema_->target_coords_;
        
        double rotation = calc_angle(x_axis_rotated[0], x_axis_rotated[1]);
        key.target_orientation_ =
        clamp_angle(key.local_orientation_ + rotation);
      }
    }
  }
}


//----------------------------------------------------------------
// match_keys
//
static void
match_keys(const std::list<descriptor_t> & a,
           const std::list<descriptor_t> & b,
           std::list<match_t> & ab)
{
  //#if 0
  //  typedef tree_t<KEY_SIZE, key_wrapper_t, double> key_tree_t;
  //  
  //  // build the tree from the A list:
  //  key_tree_t tree;
  //  std::vector<key_wrapper_t> tree_pt(a.size());
  //  {
  //    std::list<descriptor_t>::const_iterator iter = a.begin();
  //    for (unsigned int i = 0; i < a.size(); i++, ++iter)
  //    {
  //      wrap(tree_pt[i], &(*iter));
  //    }
  //    
  //    tree.setup(&(tree_pt[0]), tree_pt.size());
  //  }
  //  
  //  // query the tree from the B list:
  //  for (std::list<descriptor_t>::const_iterator i = b.begin();
  //       i != b.end(); ++i)
  //  {
  //    key_wrapper_t query;
  //    wrap(query, &(*i));
  //    
  //    // get the nearest neighbor match:
  //    double distance = std::numeric_limits<double>::max();
  //    std::list<key_tree_t::nn_t> nn_sorted;
  //    if (!tree.nn(query, distance, nn_sorted, 2 * (KEY_SIZE), 2))
  //    {
  //      continue;
  //    }
  //    
  //    const key_tree_t::nn_t & nn0 = *(nn_sorted.begin());
  //    const double & d0 = nn0.dist_;
  //    double r = 1.0;
  //    if (nn_sorted.size() == 2)
  //    {
  //      const key_tree_t::nn_t & nn1 = *(++nn_sorted.begin());
  //      const double & d1 = nn1.dist_;
  //      r = d0 / d1;
  //      
  //      // if (r > 0.5) continue;
  //    }
  //    
  //    for (unsigned int j = 0; j < nn0.node_->num_pts_; j++)
  //    {
  //      match_t rec(nn0.node_->points_[j].key_, query.key_, distance, r);
  //      ab.push_back(rec);
  //    }
  //  }
  //#else
  // find closest and second closest matches:
  key_wrapper_t query;
  key_wrapper_t nn0;
  key_wrapper_t nn1;
  key_wrapper_t tmp;
  
  for (std::list<descriptor_t>::const_iterator i = b.begin();
       i != b.end(); ++i)
  {
    wrap(query, &(*i));
    
    double ds0 = std::numeric_limits<double>::max();
    double ds1 = ds0;
    for (std::list<descriptor_t>::const_iterator j = ++(a.begin());
         j != a.end(); ++j)
    {
      wrap(tmp, &(*j));
      double ds = calc_euclidian_distance_sqrd<KEY_SIZE>
      (query.key_->descriptor_, tmp.key_->descriptor_);
      if (ds >= ds0) continue;
      
      nn1 = nn0;
      ds1 = ds0;
      
      nn0 = tmp;
      ds0 = ds;
    }
    
    if (ds0 == std::numeric_limits<double>::max()) continue;
    
    double d0 = sqrt(ds0);
    double d1 = sqrt(ds1);
    double r = d0 / d1;
    match_t rec(nn0.key_, query.key_, d0, r);
    ab.push_back(rec);
  }
  //#endif
  
  // sort the matches:
  ab.sort();
}

//----------------------------------------------------------------
// match_post_proc
// 
static void
match_post_proc(std::list<match_t> & ab_list,
                std::list<const match_t *> & ab,
                const double & percentage_to_keep)
{
  ab_list.sort();
  
  unsigned int num_to_keep =
  static_cast<unsigned int>(static_cast<double>(ab_list.size()) * percentage_to_keep);
  
  std::list<match_t>::const_iterator iter = ab_list.begin();
  for (unsigned int i = 0; i < num_to_keep; i++, ++iter)
  {
    ab.push_back(&(*iter));
  }
}

//----------------------------------------------------------------
// match_keys
// 
void
match_keys(const pyramid_t & a,
           const pyramid_t & b,
           std::list<match_t> & ab_list,
           std::list<const match_t *> & ab,
           const double & percentage_to_keep)
{
  // try to find matching keys among two pyramids:
  const unsigned int num_octaves = std::min(a.octaves(), b.octaves());
  
  // FIXME:
  const int start_octave = 0;// std::max(0, int(num_octaves) - 2);
  
  for (unsigned int i = start_octave; i < num_octaves; i++)
  {
    const octave_t & ao = a.octave_[i];
    const octave_t & bo = b.octave_[i];
    
    const unsigned int num_scales = ao.scales();
    assert(num_scales == bo.scales());
    for (unsigned int j = 0; j < num_scales; j++)
    {
      match_keys(ao.keys_min_[j], bo.keys_min_[j], ab_list);
      match_keys(ao.keys_max_[j], bo.keys_max_[j], ab_list);
    }
  }
  
  match_post_proc(ab_list, ab, percentage_to_keep);
}

//----------------------------------------------------------------
// remove_mismatches
//
template <class transform_t>
static void
remove_mismatches(const transform_t * t_ab,
                  const double & inlier_threshold,
                  const std::vector<const match_t *> & ab,
                  std::vector<const match_t *> & ab_revised)
{
  std::list<const match_t *> ab_tmp;
  
  unsigned int num_matches = ab.size();
  for (unsigned int i = 0; i < num_matches; i++)
  {
    // transform one of the points in the match pair and see how
    // closely it lands to its counterpart:
    const match_t * match = ab[i];
    const image_t::PointType & a_uv = match->a_->extrema_->local_coords_;
    const image_t::PointType & b_xy = match->b_->extrema_->local_coords_;
    
    image_t::PointType a_xy = t_ab->TransformPoint(a_uv);
    double dx = (a_xy[0] - b_xy[0]);
    double dy = (a_xy[1] - b_xy[1]);
    double d2 = dx * dx + dy * dy;
    
    if (d2 <= inlier_threshold)
    {
      ab_tmp.push_back(match);
    }
  }
  
  // copy the revised match list into the array:
  num_matches = ab_tmp.size();
  ab_revised.resize(num_matches);
  std::list<const match_t *>::const_iterator iter = ab_tmp.begin();
  for (unsigned int i = 0; i < num_matches; i++, ++iter)
  {
    ab_revised[i] = *iter;
  }
}

//----------------------------------------------------------------
// prefilter_matches_v1
// 
// 1. for each match, store the change in key location and
//    key orientation in a 3D histogram.
// 2. discard matches which map to histogram bins which are less
//    than a given percentage of the maximum histogram peak.
// 
void
prefilter_matches_v1(const std::string & fn_prefix,
                     const double & peak_ratio_threshold,
                     const std::list<const match_t *> & complete,
                     std::list<const match_t *> & filtered)
{
  const unsigned int & num_matches = complete.size();
  
  // find the range of key position changes (2D location + orientation):
  double dx_min = std::numeric_limits<double>::max();
  double dy_min = std::numeric_limits<double>::max();
  double da_min = std::numeric_limits<double>::max();
  double dx_max = -dx_min;
  double dy_max = -dy_min;
  double da_max = -da_min;
  
  std::vector<double> dx(num_matches);
  std::vector<double> dy(num_matches);
  std::vector<double> da(num_matches);
  
  std::list<const match_t *>::const_iterator iter = complete.begin();
  for (unsigned int i = 0; i < num_matches; i++, ++iter)
  {
    const descriptor_t * a0 = (*iter)->a_;
    const descriptor_t * a1 = (*iter)->b_;
    
    dx[i] = a1->extrema_->local_coords_[0] - a0->extrema_->local_coords_[0];
    dy[i] = a1->extrema_->local_coords_[1] - a0->extrema_->local_coords_[1];
    da[i] = fmod(a1->local_orientation_ - a0->local_orientation_ +
                 2.0 * TWO_PI, TWO_PI);
    
    dx_min = std::min(dx[i], dx_min);
    dy_min = std::min(dy[i], dy_min);
    da_min = std::min(da[i], da_min);
    
    dx_max = std::max(dx[i], dx_max);
    dy_max = std::max(dy[i], dy_max);
    da_max = std::max(da[i], da_max);
  }
  
  // FIXME:
  da_min = 0.0;
  da_max = TWO_PI;
  
  // allocate a 3D histogram:
//#if 1
  static const unsigned int dx_bins = 16;
  static const unsigned int dy_bins = 16;
  static const unsigned int da_bins = 16;
//#else
//  static const unsigned int dx_bins = 32;
//  static const unsigned int dy_bins = 32;
//  static const unsigned int da_bins = 32;
//#endif
  
  typedef itk::Image<unsigned int, 3> volume_t;
  volume_t::RegionType::SizeType sz;
  sz[0] = dx_bins;
  sz[1] = dy_bins;
  sz[2] = da_bins;
  volume_t::Pointer histogram = make_image<volume_t>(sz);
  
  // these are temporary variable used to update the histtogram bins:
  static unsigned int dx_bin[3];
  static double dx_w[3];
  
  static unsigned int dy_bin[3];
  static double dy_w[3];
  
  static unsigned int da_bin[3];
  static double da_w[3];
  
  static volume_t::IndexType index;
  
  // update the histogram:
  iter = complete.begin();
  for (unsigned int i = 0; i < num_matches; i++, ++iter)
  {
    // update the histogram:
    index[0] =
    calc_histogram_donations(dx_bins, dx_min, dx_max, dx[i], dx_bin, dx_w);
    
    index[1] =
    calc_histogram_donations(dy_bins, dy_min, dy_max, dy[i], dy_bin, dy_w);
    
    index[2] =
    calc_histogram_donations(da_bins, da_min, da_max, da[i], da_bin, da_w);
    
    histogram->SetPixel(index, histogram->GetPixel(index) + 1);
  }
  
  // FIXME:
  save<volume_t>(histogram, fn_prefix + "histogram.mha");
  
  // find the min/max of the histogram:
  volume_t::PixelType min = std::numeric_limits<volume_t::PixelType>::max();
  volume_t::PixelType max = -min;
  double avg = image_min_max<volume_t>(histogram, min, max);
  double rng = max - min;
  std::cout << "min: " << min << std::endl
  << "max: " << max << std::endl
  << "avg: " << avg << std::endl
  << "rng: " << rng << std::endl;
  
  // FIXME:
  volume_t::Pointer tmp = make_image<volume_t>(sz);
  
//#if 0
//  unsigned int cutoff =
//  (unsigned int)(2.0 * ceil(double(num_matches) /
//                            double(dx_bins * dy_bins * da_bins)));
//#else
  unsigned int cutoff = (unsigned int)(peak_ratio_threshold * rng + min);
//#endif
  
  // discard matches that map to bins below a given threshold:
  iter = complete.begin();
  for (unsigned int i = 0; i < num_matches; i++, ++iter)
  {
    // update the histogram:
    index[0] =
    calc_histogram_donations(dx_bins, dx_min, dx_max, dx[i], dx_bin, dx_w);
    
    index[1] =
    calc_histogram_donations(dy_bins, dy_min, dy_max, dy[i], dy_bin, dy_w);
    
    index[2] =
    calc_histogram_donations(da_bins, da_min, da_max, da[i], da_bin, da_w);
    
    unsigned int bin_count = histogram->GetPixel(index);
    if (bin_count < cutoff) continue;
    
    tmp->SetPixel(index, tmp->GetPixel(index) + 1);
    filtered.push_back(*iter);
  }
  
  // FIXME:
  save<volume_t>(tmp, fn_prefix + "histogram-thresholded.mha");
  
  // FIXME:
  std::cout << "filtered from " << complete.size() << " to " << filtered.size()
  << std::endl;
}

//----------------------------------------------------------------
// prefilter_matches_v2
// 
// 1. for every matching point, find its nearest neighbor.
// 2. find the scale distortion of the matching point pair.
// 3. discard points for which the closest point pair is distorted
//    by more than a given percentage.
// 
void
prefilter_matches_v2(const std::string & fn_prefix,
                     const double & distortion_threshold,
                     const std::list<const match_t *> & complete,
                     std::list<const match_t *> & filtered)
{
  const unsigned int & num_matches = complete.size();
  
  static image_t::PointType origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  
  // caclulate distances between points:
  std::vector<double> distance(num_matches);
  
  std::list<const match_t *>::const_iterator iter = complete.begin();
  for (unsigned int i = 0; i < num_matches; i++, ++iter)
  {
    const image_t::PointType & a0 = (*iter)->a_->extrema_->local_coords_;
    const image_t::PointType & a1 = (*iter)->b_->extrema_->local_coords_;
    
    itk::Vector<double, 2> v0 = (a0 - origin);
    itk::Vector<double, 2> v1 = (a1 - origin);
    
    distance[i] = v1.GetNorm() - v0.GetNorm();
  }
  
  iter = complete.begin();
  for (unsigned int i = 0; i < num_matches; i++, ++iter)
  {
    const image_t::PointType & a0 = (*iter)->a_->extrema_->local_coords_;
    const image_t::PointType & a1 = (*iter)->b_->extrema_->local_coords_;
    
    // find the nearest neighbor:
    double best_d2 = std::numeric_limits<double>::max();
    std::list<const match_t *>::const_iterator best_jter;
    unsigned int best_nn = ~0;
    
    std::list<const match_t *>::const_iterator jter = complete.begin();
    for (unsigned int j = 0; j < num_matches; j++)
    {
      if (i == j) continue;
      
      const image_t::PointType & b0 = (*jter)->a_->extrema_->local_coords_;
      const double d2 = (b0 - a0).GetSquaredNorm();
      
      // skip same extrema points:
      if (d2 == 0.0) continue;
      
      if (d2 < best_d2)
      {
        best_d2 = d2;
        best_nn = j;
        best_jter = jter;
      }
    }
    
    // this should never happen:
    if (best_nn == ~0u) continue;
    
    const image_t::PointType & b0 = (*best_jter)->a_->extrema_->local_coords_;
    const image_t::PointType & b1 = (*best_jter)->b_->extrema_->local_coords_;
    
    // find the canonical transform of this point pair:
    itk::Vector<double, 2> a = (a1 - a0);
    itk::Vector<double, 2> b = (b1 - b0);
    
    double norm_0 = (b0 - a0).GetSquaredNorm();
    double norm_1 = (b1 - a1).GetSquaredNorm();
    double distortion = 1.0 - ((1e-3 + std::min(norm_0, norm_1)) /
                               (1e-3 + std::max(norm_0, norm_1)));
    
    // discard matches that distort the closest point pair more than 10%:
    if (distortion > distortion_threshold) continue;
    
    filtered.push_back(*iter);
  }
  
  // FIXME:
  std::cout << "filtered from " << complete.size() << " to " << filtered.size()
  << std::endl;
}


//----------------------------------------------------------------
// rematch_keys
// 
void
rematch_keys(const std::string & fn_prefix,
             const pyramid_t & pa, // FIXME: remove this
             const pyramid_t & pb, // FIXME: remove this
             const std::list<descriptor_t> & a,
             const std::list<descriptor_t> & b,
             const base_transform_t * t_ab,
             const double & window_radius,
             std::list<match_t> & ab)
{
//#if 1
  typedef tree_t<2, ext_wrapper_t, double> ext_tree_t;
  typedef tree_t<KEY_SIZE, key_wrapper_t, double> key_tree_t;
  typedef node_t<KEY_SIZE, key_wrapper_t, double> key_node_t;
  
  itk::Vector<double, 2> x_axis;
  x_axis[0] = 1.0;
  x_axis[1] = 0.0;
  
  // keys from pyramid A are stored in a kd-tree:
  ext_tree_t key_tree;
  std::vector<ext_wrapper_t> key_pt(a.size());
  {
    std::list<descriptor_t>::const_iterator iter = a.begin();
    for (unsigned int i = 0; i < a.size(); i++, ++iter)
    {
      wrap(key_pt[i], &(*iter));
    }
    
    key_tree.setup(&(key_pt[0]), key_pt.size());
  }
  
  // query the tree from the B pyramid key list:
  for (std::list<descriptor_t>::const_iterator i = b.begin();
       i != b.end(); ++i)
  {
    ext_wrapper_t query;
    wrap(query, &(*i));
    
    // get the neighbors:
    std::list<ext_tree_t::nn_t> nn_sorted;
    if (!key_tree.neighbors(query, window_radius, nn_sorted, key_pt.size()))
    {
      continue;
    }
    
    // match the query key to the neighbors by matching their feature vectors:
    std::vector<key_wrapper_t> tree_pt;
    key_tree_t tree;
    {
      std::list<key_wrapper_t> tree_pt_list;
      std::list<ext_tree_t::nn_t>::const_iterator iter = nn_sorted.begin();
      for (unsigned int j = 0; j < nn_sorted.size(); j++, ++iter)
      {
        const ext_tree_t::nn_t & nn = *iter;
        for (unsigned int k = 0; k < nn.node_->num_pts_; k++)
        {
          // skip keys with mismatched orientation:
          const ext_wrapper_t & nn_match = nn.node_->points_[k];
          const descriptor_t & nn_key = *(nn_match.key_);
          
          // make sure the key orientations match:
          double orientation_difference =
          clamp_angle(nn_key.target_orientation_ -
                      query.key_->local_orientation_);
          
          if (orientation_difference > M_PI) orientation_difference -= TWO_PI;
          if (fabs(orientation_difference) / M_PI > 0.09) continue;
          
          key_wrapper_t wrapper;
          wrap(wrapper, &nn_key);
          tree_pt_list.push_back(wrapper);
        }
      }
      if (tree_pt_list.empty()) continue;
      
      tree_pt.assign(tree_pt_list.begin(), tree_pt_list.end());
      tree.setup(&(tree_pt[0]), tree_pt.size());
    }
    
    // get the nearest neighbor match:
    {
      key_wrapper_t query2;
      wrap(query2, query.key_);
      
      double distance = std::numeric_limits<double>::max();
      std::list<key_tree_t::nn_t> nn_sorted;
      if (!tree.nn(query2, distance, nn_sorted, 2 * (KEY_SIZE), 2))
      {
        continue;
      }
      
      const key_tree_t::nn_t & nn0 = *(nn_sorted.begin());
      const double & d0 = nn0.dist_;
      double r = 1.0;
      if (nn_sorted.size() == 2)
      {
        const key_tree_t::nn_t & nn1 = *(++nn_sorted.begin());
        const double & d1 = nn1.dist_;
        r = d0 / d1;
        
        // if (r > 0.5) continue;
      }
      
      for (unsigned int j = 0; j < nn0.node_->num_pts_; j++)
      {
        const key_wrapper_t & matching_key = nn0.node_->points_[j];
        match_t rec(matching_key.key_, query2.key_, distance, r);
        ab.push_back(rec);
      }
    }
  }
//#else
//  // find closest and second closest matches:
//  key_wrapper_t query;
//  key_wrapper_t nn0;
//  key_wrapper_t nn1;
//  key_wrapper_t tmp;
//  
//  for (std::list<descriptor_t>::const_iterator i = b.begin();
//       i != b.end(); ++i)
//  {
//    wrap(query, &(*i));
//    
//    double ds0 = std::numeric_limits<double>::max();
//    double ds1 = ds0;
//    for (std::list<descriptor_t>::const_iterator j = ++(a.begin());
//         j != a.end(); ++j)
//    {
//      wrap(tmp, &(*j));
//      double ds = calc_euclidian_distance_sqrd<KEY_SIZE>(query, tmp);
//      if (ds >= ds0) continue;
//      
//      nn1 = nn0;
//      ds1 = ds0;
//      
//      nn0 = tmp;
//      ds0 = ds;
//    }
//    
//    double d0 = sqrt(ds0);
//    double d1 = sqrt(ds1);
//    double r = d0 / d1;
//    match_t rec(nn0.key_, query.key_, d0, r);
//    ab.push_back(rec);
//  }
//  
//#endif
  
  // sort the matches:
  ab.sort();
  
  std::cout << "re-matched " << ab.size() << " keypoints" << std::endl;
}

//----------------------------------------------------------------
// rematch_keys
// 
void
rematch_keys(const std::string & fn_prefix,
             const pyramid_t & a,
             const pyramid_t & b,
             const base_transform_t * t_ab,
             const double & window_radius,
             std::list<match_t> & ab_list,
             std::list<const match_t *> & ab,
             const double & percentage_to_keep)
{
  update_target_space_data(a, t_ab);
  
  ab_list.clear();
  ab.clear();
  
  // try to find matching keys among two pyramids:
  const unsigned int num_octaves = std::min(a.octaves(), b.octaves());
  for (unsigned int i = 0; i < num_octaves; i++)
  {
    const octave_t & ao = a.octave_[i];
    const octave_t & bo = b.octave_[i];
    
    const unsigned int num_scales = ao.scales();
    assert(num_scales == bo.scales());
    
    // const double window_scale = integer_power<double>(2.0, i);
    for (unsigned int j = 0; j < num_scales; j++)
    {
      rematch_keys(fn_prefix,
                   a,
                   b,
                   ao.keys_min_[j],
                   bo.keys_min_[j],
                   t_ab,
                   window_radius, // * window_scale,
                   ab_list);
      rematch_keys(fn_prefix,
                   a,
                   b,
                   ao.keys_max_[j],
                   bo.keys_max_[j],
                   t_ab,
                   window_radius, // * window_scale,
                   ab_list);
    }
  }
  
  match_post_proc(ab_list, ab, percentage_to_keep);
}


//----------------------------------------------------------------
// load_image
// 
image_t::Pointer
load_image(const std::string & fn_load,
           const unsigned int & shrink_factor,
           const double & pixel_spacing)
{
  std::cout << "    ";
  image_t::Pointer image = load<image_t>(fn_load);
  
  // reset the tile image origin and spacing:
  image_t::PointType origin = image->GetOrigin();
  origin[0] = 0;
  origin[1] = 0;
  image->SetOrigin(origin);
  
  image_t::SpacingType spacing = image->GetSpacing();
  spacing[0] = 1;
  spacing[1] = 1;
  image->SetSpacing(spacing);
  
  //#if 0
  //  // FIXME: this may be unnecessary, but Lowe recommended it (somewhere):
  //  if (shrink_factor > 1)
  //  {
  //    shrink_factor /= 2;
  //  }
  //  else
  //  {
  //    std::cout << "    up-sampling " << std::endl;
  //    image = resize<image_t>(image, 2.0);
  //    spacing[0] = 0.5;
  //    spacing[1] = 0.5;
  //    image->SetSpacing(spacing);
  //  }
  //#endif
  
  // don't blur the images unnecessarily:
  if (shrink_factor > 1)
  {
    std::cout << "    down-sampling " << std::endl;
    image = shrink<image_t>(image, shrink_factor);
  }
  
  if (pixel_spacing != 1.0)
  {
    typedef image_t::SpacingType sp_t;
    sp_t sp = image->GetSpacing();
    sp[0] *= pixel_spacing;
    sp[1] *= pixel_spacing;
    image->SetSpacing(sp);
  }
  
  // preprocess the image with Contrast Limited Adaptive Histogram
  // Equalization algorithm, remap the intensities into the [0, 1] range:
  std::cout << "    enhancing contrast with CLAHE " << std::endl;
  image = CLAHE<image_t>(image, 255, 255, 3.0, 256, 0.0, 1.0);
  
  return image;
}



//----------------------------------------------------------------
// setup_pyramid
// 
void
setup_pyramid(pyramid_t & pyramid,
              const unsigned int index,
              const std::string & fn_load,
              const image_t * image,
              const mask_t * image_mask,
              const unsigned int & descriptor_version,
              unsigned int num_scales,
              const bool & generate_keys,
              const std::string & fn_debug)
{
  std::string prefix = fn_debug;
  if (fn_debug.size() != 0)
  {
    prefix += the_text_t::number(index);
  }
  
  // reset the pyramid:
  pyramid = pyramid_t();
  pyramid.fn_data_ = fn_load;
  
  std::cout << "    building the pyramid" << std::endl;
  pyramid.setup(image, image_mask, 0.8, num_scales, 128, generate_keys);
  
  if (generate_keys)
  {
    std::cout << "    detecting extrema points: ";
    pyramid.detect_extrema(index, 0.9975, true, prefix);
    std::cout << pyramid.count_extrema() << std::endl;
    
    if (descriptor_version != ~0u)
    {
      std::cout << "    generating keys: ";
      pyramid.generate_keys();
      std::cout << pyramid.count_keys() << std::endl;
      
      std::cout << "    generating descriptors" << std::endl;
      pyramid.generate_descriptors(descriptor_version);
    }
  }
  
  // FIXME:
  if (prefix.size() != 0) pyramid.debug(prefix);
  
  std::cout << std::endl;
}

//----------------------------------------------------------------
// bestfit_stats
// 
void
bestfit_stats(const std::vector<const match_t *> & ab,
              const std::list<unsigned int> & inliers)
{
  // indexed based on the ratio of the distance (in descriptor space)
  // between the query point and its closest neighbor to the distance
  // between the query point and its second closest neighbor:
  std::vector<double> pdf_hit;
  pdf_hit.assign(70, 0.0);
  
  for (std::list<unsigned int>::const_iterator iter = inliers.begin();
       iter != inliers.end(); ++iter)
  {
    const double & r = ab[*iter]->r_;
    unsigned int idx = (unsigned int)(r * double(pdf_hit.size() - 1));
    pdf_hit[idx] += 1;
  }
  
  std::cout << "hit PDF: " << std::endl;
  dump(std::cout, &pdf_hit[0], pdf_hit.size(), 10, true);
}
