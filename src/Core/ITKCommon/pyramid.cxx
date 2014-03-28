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

// File         : pyramid.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/04 12:39
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Difference of Gaussians multi-scale image pyramid used for
//                SIFT key and descriptor generation.

// local includes:
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/visualize.hxx>
#include <Core/ITKCommon/cluster.hxx>
#include <Core/ITKCommon/threshold.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/histogram.hxx>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>

#include <Core/Utils/Log.h>

// ITK includes:
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>

#include <boost/filesystem.hpp>

// system includes:
#include <fstream>

namespace bfs=boost::filesystem;


//----------------------------------------------------------------
// LOG_2
// 
static const double LOG_2 = log(2.0);

//----------------------------------------------------------------
// threshold
// 
template <typename data_t>
void
threshold(std::vector<data_t> & vec, const data_t & max)
{
  const unsigned int len = vec.size();
  for (unsigned int i = 0; i < len; i++)
  {
    vec[i] = std::min(max, vec[i]);
  }
}


//----------------------------------------------------------------
// get_neighbors
//
//  N0 N1 N2     x-1,y-1  x,y-1  x+1,y-1
//  N3 N4 N5     x-1,y    x,y    x+1,y
//  N6 N7 N8     x-1,y+1  x,y+1  x+1,y+1
//
template <typename data_t>
static void
get_neighbors(const image_t::ConstPointer & image,
              const image_t::IndexType & index,
              data_t * N)
{
  image_t::RegionType::SizeType sz =
  image->GetLargestPossibleRegion().GetSize();
  
  for (int x = -1; x <= 1; x++)
  {
    image_t::IndexType xy(index);
    xy[0] += x;
    for (int y = -1; y <= 1; y++)
    {
      xy[1] += y;
      unsigned int i = (y + 1) * 3 + x + 1;
      
      if (xy[0] < 0 || image_size_value_t(xy[0]) >= sz[0] ||
          xy[1] < 0 || image_size_value_t(xy[1]) >= sz[1])
      {
        N[i] = std::numeric_limits<double>::quiet_NaN();
      }
      else
      {
        N[i] = data_t(image->GetPixel(xy));
      }
    }
  }
}

//----------------------------------------------------------------
// calculate_curvature_ratio
//
// This function assumes that the index is at least 1 pixel away
// from the image boundaries:
// 
static double
calculate_curvature_ratio(const image_t * D1,
                          const image_t::IndexType & index)
{
  // get the neighbors:
  static double N[9];
  get_neighbors<double>(D1, index, N);
  
  // Compute the Hessian matrix of the Difference-of-Gaussians image
  // at the given coordinates:
  double Dxx = (N[5] - 2.0 * N[4] + N[3]);
  double Dyy = (N[7] - 2.0 * N[4] + N[1]);
  double Dxy = (N[8] - N[6] + N[0] - N[2]) / 4.0;
  
  double TrH = Dxx + Dyy;		// twice the mean curvature
  double DetH = Dxx * Dyy - Dxy * Dxy;	// gaussian curvature
  double ratio = TrH * TrH / DetH;
  
  return ratio;
}


//----------------------------------------------------------------
// find_maxima_cm_v2
// 
// Find the center-of-mass of data peaks.
// 
bool
find_maxima_cm_v2(const double * data,
                  const unsigned int & size,
                  std::list<centerofmass_t<1> > & max_list)
{
  std::vector<double> peaks;
  peaks.assign(&data[0], &data[size]);
  
  if (!isolate_orientation_histogram_peaks(&peaks[0], size, true))
  {
    // could not isolate any peaks, probably a flat histogram:
    return false;
  }
  
  return identify_clusters_cm(&peaks[0], size, max_list);
}

//----------------------------------------------------------------
// vectors_from_orientation_histogram
// 
bool
vectors_from_orientation_histogram(std::list<gradient_t> & dm,
                                   const double * orientation,
                                   const unsigned int & bins)
{
  std::list<centerofmass_t<1> > peaks;
  if (!find_maxima_cm_v2(orientation, bins, peaks))
  {
    // the histogram has no peaks:
    return false;
  }
  
  // convert the histogram peaks into radians:
  for (std::list<centerofmass_t<1> >::iterator i = peaks.begin();
       i != peaks.end(); ++i)
  {
    const centerofmass_t<1> & peak = *i;
    gradient_t vec;
    vec[0] = TWO_PI * fmod((peak[0] + 0.5) / double(bins), 1.0);
    vec[1] = peak.mass_;
    dm.push_back(vec);
  }
  
  return true;
}

//----------------------------------------------------------------
// identify_orientations
// 
bool
identify_orientations(const gradient_image_t * gL,
                      const double & xc,
                      const double & yc,
                      const double & sigma,
                      const unsigned int bins,
                      const unsigned int max_peaks,
                      std::list<gradient_t> & direction_magnitude)
{
  // NOTE: the following comments are by Sebastian Nowozin (or are
  //       based on his comments from autopano-2.4 source code):
  // 
  // From "Image Processing, Analysis and Machine Vision", pp. 84:
  // 'Pixels more distant from the center of the operator have smaller
  // influence, and pixels farther than 3 * sigma from the center have
  // negligible influence.'
  // 
  // So, the gaussian window radius == 3 * sigma:
  // 
  const unsigned int r = static_cast<unsigned int>(floor(3.0 * sigma + 0.5));
  const double max_r2 = static_cast<double>(r * r);
  
  const gradient_image_t::RegionType::SizeType sz =
  gL->GetLargestPossibleRegion().GetSize();
  unsigned int x0 = static_cast<unsigned int>(std::max(int(1), int(xc - r)));
  unsigned int y0 = static_cast<unsigned int>(std::max(int(1), int(yc - r)));
  unsigned int x1 = static_cast<unsigned int>(std::min(int(sz[0] - 2), int(xc + r)));
  unsigned int y1 = static_cast<unsigned int>(std::min(int(sz[1] - 2), int(yc + r)));
  
  image_t::RegionType rn;
  image_t::RegionType::SizeType rn_sz;
  rn_sz[0] = x1 - x0 + 1;
  rn_sz[1] = y1 - y0 + 1;
  rn.SetSize(rn_sz);
  
  image_t::IndexType rn_ix;
  rn_ix[0] = x0;
  rn_ix[1] = y0;
  rn.SetIndex(rn_ix);
  
  // build a 36 bin histogram of gradient directions in the neighborhood of
  // the given point:
  std::vector<double> orientation;
  orientation.assign(bins, 0.0);
  
  const double two_sigma_sqrd = 2.0 * sigma * sigma;
  
  typedef itk::ImageRegionConstIteratorWithIndex<gradient_image_t> itex_t;
  itex_t itex(gL, rn);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // calculate the gaussian weight:
    const image_t::IndexType & ix = itex.GetIndex();
    double dx = ix[0] - xc;
    double dy = ix[1] - yc;
    double r2 = dx * dx + dy * dy;
    if (r2 > max_r2) continue;
    
    // since overall scaling is not important in identifying the highest bins
    // of a histogram, I will omit the proper normalization of the gaussian:
    double gaussian_weight = exp(- r2 / two_sigma_sqrd);
    
    gradient_t angle_magnitude = itex.Get();
    update_orientation_histogram(&(orientation[0]),
                                 bins,
                                 angle_magnitude[0],
                                 angle_magnitude[1] * gaussian_weight);
  }
  
  // FIXME: is this really necessary?
  smoothout_orientation_histogram(&orientation[0], bins, 3);
  
  // get the vectors:
  if (!vectors_from_orientation_histogram(direction_magnitude,
                                          &orientation[0],
                                          bins))
  {
    return false;
  }
  
  if (direction_magnitude.size() > max_peaks)
  {
    // too many peaks, probably not enough contrast:
    return false;
  }
  
  return true;
}


//----------------------------------------------------------------
// generate_feature_vector_v0
// 
static void
generate_feature_vector_v0(// gradient image:
                           const gradient_image_t * gL,
                           
                           // sampling window radius:
                           const double & R,
                           
                           // the feature key:
                           descriptor_t & key)
{
  static const unsigned int bins = 8;
  static const unsigned int rows = static_cast<unsigned int>(sqrt(double(KEY_SIZE / bins)));
  static const unsigned int cols = rows;
  
  static const double sqrt_two = sqrt(2.0);
  
  // contrinution buffers:
  static unsigned int col_addr[3];
  static unsigned int row_addr[3];
  static double col_w[3];
  static double row_w[3];
  
  // weight sigma:
  const double weight_sigma = R / 3.0;
  const double two_sigma_sqrd = 2.0 * weight_sigma * weight_sigma;
  const double max_r2 = static_cast<double>(R * R);
  const double r_quadrant = R / sqrt_two;
  
  // find the region falling under the descriptor:
  const gradient_image_t::RegionType::SizeType sz =
  gL->GetLargestPossibleRegion().GetSize();
  
  unsigned int x0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[0] - R), 0);
  unsigned int y0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[1] - R), 0);
  unsigned int x1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[0] + R), int(sz[0] - 1));
  unsigned int y1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[1] + R), int(sz[1] - 1));
  
  image_t::RegionType rn;
  
  image_t::RegionType::SizeType rn_sz;
  rn_sz[0] = x1 - x0 + 1;
  rn_sz[1] = y1 - y0 + 1;
  rn.SetSize(rn_sz);
  
  image_t::IndexType rn_ix;
  rn_ix[0] = x0;
  rn_ix[1] = y0;
  rn.SetIndex(rn_ix);
  
  // fill in the histograms:
  key.descriptor_.assign(rows * cols * bins, 0.0);
  
  typedef itk::ImageRegionConstIteratorWithIndex<gradient_image_t> itex_t;
  itex_t itex(gL, rn);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    const image_t::IndexType & ix = itex.GetIndex();
    
    // calculate the gaussian weight:
    double dx = ix[0] - key.extrema_->pixel_coords_[0];
    double dy = ix[1] - key.extrema_->pixel_coords_[1];
    double r2 = dx * dx + dy * dy;
    if (r2 > max_r2) continue;
    
    // calculate parametric cartesian coordinates:
    double angle = calc_angle(dx, dy, key.local_orientation_);
    double radius = sqrt(r2);
    
    double tx = (radius * cos(angle) / r_quadrant + 1.0) / 2.0;
    double ty = (radius * sin(angle) / r_quadrant + 1.0) / 2.0;
    
    // the point falls outside the descriptor window:
    if (tx < 0.0 || tx > 1.0 || ty < 0.0 || ty > 1.0) continue;
    
    // the overall scaling is not important because the entire feature
    // vector will be normalized, therefore I will omit the proper
    // normalization of the gaussian:
    double gaussian_weight = exp(- r2 / two_sigma_sqrd);
    
    // determine the column contributions:
    calc_histogram_donations(cols, 0.0, 1.0, tx, col_addr, col_w);
    
    // determine the row contributions:
    calc_histogram_donations(rows, 0.0, 1.0, ty, row_addr, row_w);
    
    // lookup the pixel gradient:
    const gradient_t & vec = itex.Get();
    double orientation = clamp_angle(vec[0] + TWO_PI - key.local_orientation_);
    
    for (unsigned int i = 0; i < 3; i++)
    {
      for (unsigned int j = 0; j < 3; j++)
      {
        double w = vec[1] * gaussian_weight * row_w[i] * col_w[j];
        if (w == 0.0) continue;
        
        unsigned int address = bins * (row_addr[i] * cols + col_addr[j]);
        update_orientation_histogram(&(key.descriptor_[address]),
                                     bins,
                                     orientation,
                                     w);
      }
    }
  }
}

//----------------------------------------------------------------
// generate_feature_vector_v1
// 
static void
generate_feature_vector_v1(// gradient image:
                           const gradient_image_t * gL,
                           
                           // sampling window radius:
                           const double & R,
                           
                           // the feature key:
                           descriptor_t & key)
{
  static const unsigned int bins = 8;
  static const unsigned int rows = static_cast<unsigned int>(sqrt(static_cast<double>(KEY_SIZE / bins)));
  static const unsigned int cols = rows;
  
  static const double sqrt_two = sqrt(2.0);
  
  // contrinution buffers:
  static double weight[9];
  
  // weight sigma:
  const double weight_sigma = R / 3.0;
  const double two_sigma_sqrd = 2.0 * weight_sigma * weight_sigma;
  const double max_r2 = double(R * R);
  const double r_quadrant = R / sqrt_two;
  
  // find the region falling under the descriptor:
  const gradient_image_t::RegionType::SizeType sz =
  gL->GetLargestPossibleRegion().GetSize();
  
  unsigned int x0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[0] - R), 0);
  unsigned int y0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[1] - R), 0);
  unsigned int x1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[0] + R), static_cast<int>(sz[0] - 1));
  unsigned int y1 = std::min(int(key.extrema_->pixel_coords_[1] + R), static_cast<int>(sz[1] - 1));
  
  image_t::RegionType rn;
  
  image_t::RegionType::SizeType rn_sz;
  rn_sz[0] = x1 - x0 + 1;
  rn_sz[1] = y1 - y0 + 1;
  rn.SetSize(rn_sz);
  
  image_t::IndexType rn_ix;
  rn_ix[0] = x0;
  rn_ix[1] = y0;
  rn.SetIndex(rn_ix);
  
  // fill in the histograms:
  key.descriptor_.assign(rows * cols * bins, 0.0);
  
  typedef itk::ImageRegionConstIteratorWithIndex<gradient_image_t> itex_t;
  itex_t itex(gL, rn);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    const image_t::IndexType & ix = itex.GetIndex();
    
    // calculate the gaussian weight:
    double dx = ix[0] - key.extrema_->pixel_coords_[0];
    double dy = ix[1] - key.extrema_->pixel_coords_[1];
    double r2 = dx * dx + dy * dy;
    if (r2 > max_r2) continue;
    
    // calculate parametric cartesian coordinates:
    double angle = calc_angle(dx, dy, key.local_orientation_);
    double radius = sqrt(r2);
    
    double tx = (radius * cos(angle) / r_quadrant + 1.0) / 2.0;
    double ty = (radius * sin(angle) / r_quadrant + 1.0) / 2.0;
    
    // the point falls outside the descriptor window:
    if (tx < 0.0 || tx > 1.0 || ty < 0.0 || ty > 1.0) continue;
    
    // the overall scaling is not important because the entire feature
    // vector will be normalized, therefore I will omit the proper
    // normalization of the gaussian:
    double gaussian_weight = exp(- r2 / two_sigma_sqrd);
    
    // determine the quadrant coordinates:
    double tcol = tx * double(cols);
    double trow = ty * double(rows);
    
    unsigned int col = std::min(cols - 1, static_cast<unsigned int>(floor(tcol)));
    unsigned int row = std::min(rows - 1, static_cast<unsigned int>(floor(trow)));
    
//#if 0
//    cout << "quadrant: "
//    << tcol << ' ' << trow << " -> "
//    << (unsigned int)(floor(tcol)) << ' '
//    << (unsigned int)(floor(trow)) << " -> "
//    << col << ' ' << row << std::endl;
//#endif
    
    // The contribution should be distributed across adjacent quadrants.
    // For each neighbor, determine the distance from the point to the
    // center of the quadrant.
    double total_weight = 0.0;
    for (unsigned int i = 0; i < 9; i++)
    {
      int r = row + i / 3 - 1;
      int c = col + i % 3 - 1;
      if (r < 0 || static_cast<unsigned int>(r) >= rows ||
          c < 0 || static_cast<unsigned int>(c) >= cols)
      {
        weight[i] = 0.0;
        continue;
      }
      
      // calculate the weight:
      double center_x = static_cast<double>(c) + 0.5;
      double center_y = static_cast<double>(r) + 0.5;
      
      double dist_x = std::min(1.0, fabs(tcol - center_x));
      double dist_y = std::min(1.0, fabs(trow - center_y));
      double dist = std::max(dist_x, dist_y);
      weight[i] = 1.0 - dist;
      
      total_weight += weight[i];
    }
    
    // calculate the pixel gradients:
    const gradient_t & vec = itex.Get();
    double orientation = clamp_angle(vec[0] + TWO_PI - key.local_orientation_);
    
    // distribute the orientation across the neighbors:
    for (unsigned int i = 0; i < 9; i++)
    {
      double w = vec[1] * gaussian_weight * weight[i] / total_weight;
      if (weight[i] == 0.0) continue;
      int r = row + i / 3 - 1;
      int c = col + i % 3 - 1;
      
      unsigned int address = bins * (r * cols + c);
      update_orientation_histogram(&(key.descriptor_[address]),
                                   bins,
                                   orientation,
                                   w);
    }
  }
}

//----------------------------------------------------------------
// generate_feature_vector_v2
// 
static void
generate_feature_vector_v2(// minima and maxima of a given octave scale:
                           const image_t * min,
                           const image_t * max,
                           
                           // sampling window radius:
                           const double & R,
                           
                           // the feature key:
                           descriptor_t & key)
{
  // the number of annulai in the sampling window:
  static const unsigned int annulai = 4;
  
  // The number of sections in each annulus is allocated such that the
  // area of a section is the same across all annulai. The formula is
  //   n = 8 * i + 4
  // where i is the index of the annulus:
  static const unsigned int sections[] = { 4, 12, 20, 28, 36, 44 };
  
  // section addresses in the feature vector:
  static const unsigned int address[] = { 0, 4, 16, 36, 64, 100 };
  
  // indeces of the first and last annulus of the sampling window:
  static const unsigned int a0 = 0;
  static const unsigned int a1 = annulai - 1;
  
  // total number of sections:
  static const unsigned int num_sections = sections[a1] + address[a1];
  assert(2 * num_sections == KEY_SIZE);
  
  // find the region falling under the sampling window:
  image_t::RegionType rn;
  {
    const gradient_image_t::RegionType::SizeType sz =
    min->GetLargestPossibleRegion().GetSize();
    
    unsigned int x0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[0] - R), 0);
    unsigned int y0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[1] - R), 0);
    unsigned int x1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[0] + R), static_cast<int>(sz[0] - 1));
    unsigned int y1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[1] + R), static_cast<int>(sz[1] - 1));
    
    image_t::RegionType::SizeType rn_sz;
    rn_sz[0] = x1 - x0 + 1;
    rn_sz[1] = y1 - y0 + 1;
    rn.SetSize(rn_sz);
    
    image_t::IndexType rn_ix;
    rn_ix[0] = x0;
    rn_ix[1] = y0;
    rn.SetIndex(rn_ix);
  }
  
  // fill in the histograms:
  key.descriptor_.assign(KEY_SIZE, 0.0);
  
  // iterator over the minima image:
  typedef itk::ImageRegionConstIteratorWithIndex<image_t> itex_t;
  itex_t itex(min, rn);
  
  // shortcut, used to discard pixels outside the sampling window:
  const double max_r2 = R * R;
  
  // iterate through the sampling window:
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure the pixel is within the sampling window:
    const image_t::IndexType & ix = itex.GetIndex();
    double dx = ix[0] - key.extrema_->pixel_coords_[0];
    double dy = ix[1] - key.extrema_->pixel_coords_[1];
    double r2 = dx * dx + dy * dy;
    if (r2 > max_r2) continue;
    
    // polar coordinates of the sample point expressed in the coordinate
    // system of the feature key:
    double angle = calc_angle(dx, dy, key.local_orientation_);
    double radius = sqrt(r2);
    
    // determine the annulus coordinates:
    double t_radius = radius / R;
    double ta = t_radius * static_cast<double>(annulai);
    unsigned int a = std::min(annulai - 1, static_cast<unsigned int>(floor(ta)));
    
    // distribute this pixel across adjacent annulai:
    double w0 = a > a0 ? std::max(0.0, 0.5 - (ta - a)) : 0.0;
    double w2 = a < a1 ? std::max(0.0, (ta - a) - 0.5) : 0.0;
    double w1 = 1.0 - (w0 + w2);
    assert(w0 <= 0.5 && w0 >= 0.0);
    assert(w1 <= 1.0 && w1 >= 0.0);
    assert(w2 <= 0.5 && w2 >= 0.0);
    
    double mass_min = min->GetPixel(ix);
    double mass_max = max->GetPixel(ix);
    if (mass_min == 0.0 && mass_max == 0.0) continue;
    
    // update the target annulus:
    // min:
    update_orientation_histogram(&(key.descriptor_[address[a]]),
                                 sections[a],
                                 angle,
                                 mass_min * w1);
    // max:
    update_orientation_histogram(&(key.descriptor_[address[a] +
                                                   num_sections]),
                                 sections[a],
                                 angle,
                                 mass_max * w1);
    
    // update the preceding annulus:
    if (w0 > 0.0)
    {
      // min:
      update_orientation_histogram(&(key.descriptor_[address[a - 1]]),
                                   sections[a - 1],
                                   angle,
                                   mass_min * w0);
      // min:
      update_orientation_histogram(&(key.descriptor_[address[a - 1] +
                                                     num_sections]),
                                   sections[a - 1],
                                   angle,
                                   mass_max * w0);
    }
    
    // update the following annulus:
    if (w2 > 0.0)
    {
      // min:
      update_orientation_histogram(&(key.descriptor_[address[a + 1]]),
                                   sections[a + 1],
                                   angle,
                                   mass_min * w2);
      // min:
      update_orientation_histogram(&(key.descriptor_[address[a + 1] +
                                                     num_sections]),
                                   sections[a + 1],
                                   angle,
                                   mass_max * w2);
    }
  }
}

//----------------------------------------------------------------
// generate_feature_vector_v3
// 
static void
generate_feature_vector_v3(// gradient image:
                           const gradient_image_t * gL,
                           
                           // sampling window radius:
                           const double & R,
                           
                           // the feature key:
                           descriptor_t & key)
{
  // the number of annulai in the sampling window:
  static const unsigned int num_annulai = 4;
  
  // The number of sections in each annulus is allocated such that the
  // area of a section is the same across all annulai. The formula is
  //   n = 8 * i + 4
  // where i is the index of the annulus:
  static const unsigned int sections[] = { 4, 12, 20, 28, 36, 44 };
  
  // section addresses in the feature vector:
  static const unsigned int address[] = { 0, 4, 16, 36, 64, 100 };
  
  // index of the last annulus of the sampling window:
  static const unsigned int a1 = num_annulai - 1;
  
  // total number of sections:
  static const unsigned int num_sections = sections[a1] + address[a1];
  assert(KEY_SIZE == 2 * num_sections);
  
  // each section has a histogram that will be used to determine
  // the dominant gradient vector for that section:
  static const unsigned section_bins = 12;
  static std::vector<double> section_histogram;
  section_histogram.assign(num_sections * section_bins, 0.0);
  
  // find the region falling under the sampling window:
  image_t::RegionType rn;
  {
    const gradient_image_t::RegionType::SizeType sz =
    gL->GetLargestPossibleRegion().GetSize();
    
    unsigned int x0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[0] - R), 0);
    unsigned int y0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[1] - R), 0);
    unsigned int x1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[0] + R), static_cast<int>(sz[0] - 1));
    unsigned int y1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[1] + R), static_cast<int>(sz[1] - 1));
    
    image_t::RegionType::SizeType rn_sz;
    rn_sz[0] = x1 - x0 + 1;
    rn_sz[1] = y1 - y0 + 1;
    rn.SetSize(rn_sz);
    
    image_t::IndexType rn_ix;
    rn_ix[0] = x0;
    rn_ix[1] = y0;
    rn.SetIndex(rn_ix);
  }
  
  // iterator over the minima image:
  typedef itk::ImageRegionConstIteratorWithIndex<gradient_image_t> itex_t;
  itex_t itex(gL, rn);
  
  // shortcut, used to discard pixels outside the sampling window:
  const double max_r2 = R * R;
  
  // iterate through the sampling window:
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure the pixel is within the sampling window:
    const image_t::IndexType & ix = itex.GetIndex();
    double dx = ix[0] - key.extrema_->pixel_coords_[0];
    double dy = ix[1] - key.extrema_->pixel_coords_[1];
    double r2 = dx * dx + dy * dy;
    if (r2 > max_r2) continue;
    
    // polar coordinates of the sample point expressed in the coordinate
    // system of the feature key:
    double angle = calc_angle(dx, dy, key.local_orientation_);
    double radius = sqrt(r2);
    
    // shortcuts:
    gradient_t vec = gL->GetPixel(ix);
    vec[0] = clamp_angle(vec[0] - key.local_orientation_);
    
    // FIXME: verify that this is working correctly: 2006/03/07:
    unsigned int bin_addr[3];
    double bin_w[3];
    unsigned int a =
    calc_histogram_donations(num_annulai, 0, R, radius, bin_addr, bin_w);
    const double & w0 = bin_w[0];
    const double & w1 = bin_w[1];
    const double & w2 = bin_w[2];
    
    static unsigned int s[3];
    static double       w[3];
    
    // update the target annulus:
    {
      calc_orientation_histogram_donations(sections[a], angle, s, w);
      for (unsigned int i = 0; i < 3; i++)
      {
        unsigned int addr = section_bins * (s[i] + address[a]);
        update_orientation_histogram(&section_histogram[addr],
                                     section_bins,
                                     vec[0],
                                     vec[1] * w1 * w[i]);
      }
    }
    
    // update the preceding annulus:
    if (w0 > 0.0)
    {
      calc_orientation_histogram_donations(sections[a - 1], angle, s, w);
      for (unsigned int i = 0; i < 3; i++)
      {
        unsigned int addr = section_bins * (s[i] + address[a - 1]);
        update_orientation_histogram(&section_histogram[addr],
                                     section_bins,
                                     vec[0],
                                     vec[1] * w0 * w[i]);
      }
    }
    
    // update the following annulus:
    if (w2 > 0.0)
    {
      calc_orientation_histogram_donations(sections[a + 1], angle, s, w);
      for (unsigned int i = 0; i < 3; i++)
      {
        unsigned int addr = section_bins * (s[i] + address[a + 1]);
        update_orientation_histogram(&section_histogram[addr],
                                     section_bins,
                                     vec[0],
                                     vec[1] * w2 * w[i]);
      }
    }
  }
  
  // assemble the feature vector:
  key.descriptor_.assign(KEY_SIZE, 0.0);
  
  for (unsigned int a = 0; a < num_annulai; a++)
  {
    for (unsigned int s = 0; s < sections[a]; s++)
    {
      // identify the dominant gradient vector for this section:
      unsigned int addr = section_bins * (s + address[a]);
      double * hist = &section_histogram[addr];
      
      // FIXME: is this really necessary?
      // smoothout_orientation_histogram(hist, section_bins, 3);
      
      std::list<gradient_t> dm;
      if (vectors_from_orientation_histogram(dm, hist, section_bins))
      {
        gradient_t acc;
        acc[0] = 0.0;
        acc[1] = 0.0;
        for (std::list<gradient_t>::const_iterator i = dm.begin();
             i != dm.end(); ++i)
        {
          const double & angle     = (*i)[0];
          const double & magnitude = (*i)[1];
          
          gradient_t v;
          v[0] = cos(angle) * magnitude;
          v[1] = sin(angle) * magnitude;
          
          acc += v;
        }
        
        key.descriptor_[address[a] + s] = calc_angle(acc[0], acc[1]);
        key.descriptor_[address[a] + s + num_sections] =
        sqrt(acc[0] * acc[0] + acc[1] * acc[1]);
      }
    }
  }
}

//----------------------------------------------------------------
// generate_feature_vector_v4
// 
static void
generate_feature_vector_v4(// gradient image:
                           const gradient_image_t * gL,
                           
                           // sampling window radius:
                           const double & R,
                           
                           // the feature key:
                           descriptor_t & key)
{
  // the number of annulai in the sampling window:
  static const unsigned int num_annulai = 2;
  
  // The number of sections in each annulus is allocated such that the
  // area of a section is the same across all annulai. The formula is
  //   n = 8 * i + 4
  // where i is the index of the annulus:
  static const unsigned int sections[] = { 4, 12, 20, 28, 36, 44 };
  
  // section addresses in the feature vector:
  static const unsigned int address[] = { 0, 4, 16, 36, 64, 100 };
  
  // total number of sections:
  static const unsigned section_bins = 8;
  
#ifndef NDEBUG
  // index of the last annulus of the sampling window:
  static const unsigned int a1 = num_annulai - 1;
  static const unsigned int num_sections = sections[a1] + address[a1];
  assert(KEY_SIZE == section_bins * num_sections);
#endif
  
  // weight sigma:
  const double weight_sigma = R / 3.0;
  const double two_sigma_sqrd = 2.0 * weight_sigma * weight_sigma;
  
  // each section contains a histogram:
  key.descriptor_.assign(KEY_SIZE, 0.0);
  
  // find the region falling under the sampling window:
  image_t::RegionType rn;
  {
    const gradient_image_t::RegionType::SizeType sz =
    gL->GetLargestPossibleRegion().GetSize();
    
    unsigned int x0 = std::max(int(key.extrema_->pixel_coords_[0] - R), 0);
    unsigned int y0 = std::max(int(key.extrema_->pixel_coords_[1] - R), 0);
    unsigned int x1 = std::min(int(key.extrema_->pixel_coords_[0] + R),
                               int(sz[0] - 1));
    unsigned int y1 = std::min(int(key.extrema_->pixel_coords_[1] + R),
                               int(sz[1] - 1));
    
    image_t::RegionType::SizeType rn_sz;
    rn_sz[0] = x1 - x0 + 1;
    rn_sz[1] = y1 - y0 + 1;
    rn.SetSize(rn_sz);
    
    image_t::IndexType rn_ix;
    rn_ix[0] = x0;
    rn_ix[1] = y0;
    rn.SetIndex(rn_ix);
  }
  
  // iterator over the minima image:
  typedef itk::ImageRegionConstIteratorWithIndex<gradient_image_t> itex_t;
  itex_t itex(gL, rn);
  
  // shortcut, used to discard pixels outside the sampling window:
  const double max_r2 = R * R;
  
  // iterate through the sampling window:
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure the pixel is within the sampling window:
    const image_t::IndexType & ix = itex.GetIndex();
    double dx = ix[0] - key.extrema_->pixel_coords_[0];
    double dy = ix[1] - key.extrema_->pixel_coords_[1];
    double r2 = dx * dx + dy * dy;
    if (r2 > max_r2) continue;
    
    // polar coordinates of the sample point expressed in the coordinate
    // system of the feature key:
    double angle = calc_angle(dx, dy, key.local_orientation_);
    double radius = sqrt(r2);
    
    // FIXME: verify that this is working correctly: 2006/03/07:
    unsigned int bin_addr[3];
    double bin_w[3];
    unsigned int a =
    calc_histogram_donations(num_annulai, 0, R, radius, bin_addr, bin_w);
    const double & w0 = bin_w[0];
    const double & w1 = bin_w[1];
    const double & w2 = bin_w[2];
    
    // shortcuts:
    gradient_t vec = gL->GetPixel(ix);
    vec[0] = clamp_angle(vec[0] - key.local_orientation_);
    
    static unsigned int s[3];
    static double       w[3];
    
    // the overall scaling is not important because the entire feature
    // vector will be normalized, therefore I will omit the proper
    // normalization of the gaussian:
    // double gaussian_weight = 1.0;
    // double gaussian_weight = 2.0 - radius / r;
    // double gaussian_weight = 2.0 - integer_power<double>(radius / r, 2);
    double gaussian_weight = exp(- r2 / two_sigma_sqrd);
    
    // update the target annulus:
    {
      calc_orientation_histogram_donations(sections[a], angle, s, w);
      for (unsigned int i = 0; i < 3; i++)
      {
        unsigned int addr = section_bins * (s[i] + address[a]);
        update_orientation_histogram(&(key.descriptor_[addr]),
                                     section_bins,
                                     vec[0],
                                     vec[1] * w1 * w[i] * gaussian_weight);
      }
    }
    
    // update the preceding annulus:
    if (w0 > 0.0)
    {
      calc_orientation_histogram_donations(sections[a - 1], angle, s, w);
      for (unsigned int i = 0; i < 3; i++)
      {
        unsigned int addr = section_bins * (s[i] + address[a - 1]);
        update_orientation_histogram(&(key.descriptor_[addr]),
                                     section_bins,
                                     vec[0],
                                     vec[1] * w0 * w[i] * gaussian_weight);
      }
    }
    
    // update the following annulus:
    if (w2 > 0.0)
    {
      calc_orientation_histogram_donations(sections[a + 1], angle, s, w);
      for (unsigned int i = 0; i < 3; i++)
      {
        unsigned int addr = section_bins * (s[i] + address[a + 1]);
        update_orientation_histogram(&(key.descriptor_[addr]),
                                     section_bins,
                                     vec[0],
                                     vec[1] * w2 * w[i] * gaussian_weight);
      }
    }
  }
  
//#if 0
//  // FIXME: experimental: enhance the histograms:
//  for (unsigned int a = 0; a < num_annulai; a++)
//  {
//    for (unsigned int s = 0; s < sections[a]; s++)
//    {
//      unsigned int addr = section_bins * (s + address[a]);
//      double * histogram = &(key.descriptor_[addr]);
//      isolate_orientation_histogram_peaks(histogram, section_bins);
//      
//      // FIXME: experimental: melt the peaks to aid in matching:
//      smoothout_orientation_histogram(histogram, section_bins, 1);
//    }
//  }
//#endif
}

//----------------------------------------------------------------
// generate_feature_vector_v5
// 
static void
generate_feature_vector_v5(// gradient image:
                           const gradient_image_t * gL,
                           
                           // sampling window radius:
                           const double & R,
                           
                           // the feature key:
                           descriptor_t & key)
{
  static const unsigned int bins = 8;
  static const unsigned int rows = (unsigned int)(sqrt(double(KEY_SIZE /
                                                              bins)));
  static const unsigned int cols = rows;
  
  static const double sqrt_two = sqrt(2.0);
  
  // contrinution buffers:
  static unsigned int col_addr[3];
  static unsigned int row_addr[3];
  static double col_w[3];
  static double row_w[3];
  
  // weight sigma:
  const double weight_sigma = R / 3.0;
  const double two_sigma_sqrd = 2.0 * weight_sigma * weight_sigma;
  const double max_r2 = static_cast<double>(R * R);
  const double r_quadrant = R / sqrt_two;
  
  // find the region falling under the descriptor:
  const gradient_image_t::RegionType::SizeType sz =
  gL->GetLargestPossibleRegion().GetSize();
  
  unsigned int x0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[0] - R), 0);
  unsigned int y0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[1] - R), 0);
  unsigned int x1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[0] + R), static_cast<int>(sz[0] - 1));
  unsigned int y1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[1] + R), static_cast<int>(sz[1] - 1));
  
  image_t::RegionType rn;
  
  image_t::RegionType::SizeType rn_sz;
  rn_sz[0] = x1 - x0 + 1;
  rn_sz[1] = y1 - y0 + 1;
  rn.SetSize(rn_sz);
  
  image_t::IndexType rn_ix;
  rn_ix[0] = x0;
  rn_ix[1] = y0;
  rn.SetIndex(rn_ix);
  
  // fill in the histograms:
  key.descriptor_.assign(rows * cols * bins, 0.0);
  
  typedef itk::ImageRegionConstIteratorWithIndex<gradient_image_t> itex_t;
  itex_t itex(gL, rn);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    const image_t::IndexType & ix = itex.GetIndex();
    
    // calculate the gaussian weight:
    double dx = ix[0] - key.extrema_->pixel_coords_[0];
    double dy = ix[1] - key.extrema_->pixel_coords_[1];
    double r2 = dx * dx + dy * dy;
    if (r2 > max_r2) continue;
    
    // calculate parametric cartesian coordinates:
    double angle = calc_angle(dx, dy, key.local_orientation_);
    double radius = sqrt(r2);
    
    double tx = (radius * cos(angle) / r_quadrant + 1.0) / 2.0;
    double ty = (radius * sin(angle) / r_quadrant + 1.0) / 2.0;
    tx = std::max(0.0, std::min(1.0, tx));
    ty = std::max(0.0, std::min(1.0, ty));
    
    // the overall scaling is not important because the entire feature
    // vector will be normalized, therefore I will omit the proper
    // normalization of the gaussian:
    double gaussian_weight = exp(- r2 / two_sigma_sqrd);
    
    // determine the column contributions:
    calc_histogram_donations(cols, 0.0, 1.0, tx, col_addr, col_w);
    
    // determine the row contributions:
    calc_histogram_donations(rows, 0.0, 1.0, ty, row_addr, row_w);
    
    // lookup the pixel gradient:
    const gradient_t & vec = itex.Get();
    double orientation = clamp_angle(vec[0] + TWO_PI - key.local_orientation_);
    
    for (unsigned int i = 0; i < 3; i++)
    {
      for (unsigned int j = 0; j < 3; j++)
      {
        double w = vec[1] * gaussian_weight * row_w[i] * col_w[j];
        if (w == 0.0) continue;
        
        unsigned int address = bins * (row_addr[i] * cols + col_addr[j]);
        update_orientation_histogram(&(key.descriptor_[address]),
                                     bins,
                                     orientation,
                                     w);
      }
    }
  }
}

//----------------------------------------------------------------
// generate_feature_vector_v6
// 
static void
generate_feature_vector_v6(// gradient image:
                           const gradient_image_t * gL,
                           
                           // sampling window radius:
                           const double & R,
                           
                           // the feature key:
                           descriptor_t & key)
{
  static const unsigned int bins = 8;
  static const unsigned int rows = static_cast<unsigned int>(sqrt(static_cast<double>(KEY_SIZE / bins)));
  static const unsigned int cols = rows;
  
  // contrinution buffers:
  static unsigned int col_addr[3];
  static unsigned int row_addr[3];
  static double col_w[3];
  static double row_w[3];
  
  // weight sigma:
  const double weight_sigma = R / 3.0;
  const double two_sigma_sqrd = 2.0 * weight_sigma * weight_sigma;
  const double R2 = double(R * R);
  
  // find the region falling under the descriptor:
  const gradient_image_t::RegionType::SizeType sz =
  gL->GetLargestPossibleRegion().GetSize();
  
  unsigned int x0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[0] - R), 0);
  unsigned int y0 = std::max(static_cast<int>(key.extrema_->pixel_coords_[1] - R), 0);
  unsigned int x1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[0] + R), static_cast<int>(sz[0] - 1));
  unsigned int y1 = std::min(static_cast<int>(key.extrema_->pixel_coords_[1] + R), static_cast<int>(sz[1] - 1));
  
  image_t::RegionType rn;
  
  image_t::RegionType::SizeType rn_sz;
  rn_sz[0] = x1 - x0 + 1;
  rn_sz[1] = y1 - y0 + 1;
  rn.SetSize(rn_sz);
  
  image_t::IndexType rn_ix;
  rn_ix[0] = x0;
  rn_ix[1] = y0;
  rn.SetIndex(rn_ix);
  
  // fill in the histograms:
  key.descriptor_.assign(rows * cols * bins, 0.0);
  
  typedef itk::ImageRegionConstIteratorWithIndex<gradient_image_t> itex_t;
  itex_t itex(gL, rn);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    const image_t::IndexType & ix = itex.GetIndex();
    
    // calculate the gaussian weight:
    double dx = ix[0] - key.extrema_->pixel_coords_[0];
    double dy = ix[1] - key.extrema_->pixel_coords_[1];
    double r2 = dx * dx + dy * dy;
    if (r2 > R2) continue;
    
    // calculate parametric cartesian coordinates:
    double angle = calc_angle(dx, dy, key.local_orientation_);
    double r = sqrt(r2);
    unsigned int k = static_cast<unsigned int>((angle + M_PI / 4.0) / (M_PI / 2.0)) % 4;
    double rp = r / (R * cos(angle - static_cast<double>(k) * (M_PI / 2.0)));
    
    double tx = (rp * cos(angle) + 1.0) / 2.0;
    double ty = (rp * sin(angle) + 1.0) / 2.0;
    tx = std::max(0.0, std::min(1.0, tx));
    ty = std::max(0.0, std::min(1.0, ty));
    
    // the overall scaling is not important because the entire feature
    // vector will be normalized, therefore I will omit the proper
    // normalization of the gaussian:
    double gaussian_weight = exp(- r2 / two_sigma_sqrd);
    
    // determine the column contributions:
    calc_histogram_donations(cols, 0.0, 1.0, tx, col_addr, col_w);
    
    // determine the row contributions:
    calc_histogram_donations(rows, 0.0, 1.0, ty, row_addr, row_w);
    
    // lookup the pixel gradient:
    const gradient_t & vec = itex.Get();
    double orientation = clamp_angle(vec[0] + TWO_PI - key.local_orientation_);
    
    for (unsigned int i = 0; i < 3; i++)
    {
      for (unsigned int j = 0; j < 3; j++)
      {
        double w = vec[1] * gaussian_weight * row_w[i] * col_w[j];
        if (w == 0.0) continue;
        
        unsigned int address = bins * (row_addr[i] * cols + col_addr[j]);
        update_orientation_histogram(&(key.descriptor_[address]),
                                     bins,
                                     orientation,
                                     w);
      }
    }
  }
}


//----------------------------------------------------------------
// collect_extrema
// 
static void
collect_extrema(// the extrema points:
                std::list<extrema_t> & extrema,
                
                // pyramid index:
                const unsigned int pyramid,
                
                // octave index:
                const unsigned int octave,
                
                // scale index:
                const unsigned int scale,
                
                // difference of gaussians image:
                const image_t * D1,
                
                // image mask:
                const mask_t * mask,
                
                // points of interest:
                const std::list<centerofmass_t<2> > & poi,
                
                // principal curvature ratio:
                const double principal_curvature_ratio)
{
  typedef image_t::IndexType index_t;
  
  const double r1 = principal_curvature_ratio + 1.0;
  const double ratio_threshold = r1 * r1 / principal_curvature_ratio;
  const image_t::RegionType::SizeType sz =
  D1->GetLargestPossibleRegion().GetSize();
  
  index_t i00;
  i00[0] = 0;
  i00[1] = 0;
  
  image_t::PointType origin;
  D1->TransformIndexToPhysicalPoint(i00, origin);
  
  index_t i11;
  i11[0] = 1;
  i11[1] = 1;
  
  image_t::PointType spacing;
  D1->TransformIndexToPhysicalPoint(i11, spacing);
  spacing[0] -= origin[0];
  spacing[1] -= origin[1];
  
  extrema_t ext;
  ext.pyramid_ = pyramid;
  ext.octave_ = octave;
  ext.scale_ = scale;
  
  for (std::list<centerofmass_t<2> >::const_iterator i = poi.begin();
       i != poi.end(); ++i)
  {
    const centerofmass_t<2> & cm = *i;
    
    image_t::IndexType index;
    index[0] = static_cast<unsigned int>(floor(cm[0] + 0.5));
    index[1] = static_cast<unsigned int>(floor(cm[1] + 0.5));
    
    // stay at least 1 pixel away from the image boundaries:
    if (index[0] == 0 ||
        index[1] == 0 ||
        image_size_value_t(index[0] + 1) >= sz[0] ||
        image_size_value_t(index[1] + 1) >= sz[1])
    {
      continue;
    }
    
    double curvature_ratio = calculate_curvature_ratio(D1, index);
    
    // FIXME: David G. Lowe says that hyperbolic points are not extrema points:
    // if (curvature_ratio < 0.0) continue;
    
    // reject keys that lay on an edge:
    if (fabs(curvature_ratio) > ratio_threshold) continue;
    
    // FIXME: store the mass of the extrema point:
    ext.mass_ = cm[2];
    
    // store the local pixel coordinates:
    ext.pixel_coords_[0] = cm[0];
    ext.pixel_coords_[1] = cm[1];
    
    // store the local physical coordinates:
    ext.local_coords_[0] = origin[0] + spacing[0] * cm[0];
    ext.local_coords_[1] = origin[1] + spacing[1] * cm[1];
    
    // initialize the target space coordinates with local physical coordinates:
    ext.target_coords_ = ext.local_coords_;
    
    // add it to the list:
    extrema.push_back(ext);
  }
}

//----------------------------------------------------------------
// generate_descriptor
// 
void
generate_descriptor(// which version of the descriptor should be used:
                    const unsigned int & descriptor_version,
                    
                    // the descriptor:
                    descriptor_t & key,
                    
                    // descriptor local orientation:
                    const double & local_orientation,
                    
                    // gradient image:
                    const gradient_image_t * gL,
                    
                    // sigma of the gaussian weighting function:
                    const double & keypoint_sigma,
                    
                    // sampling window radius:
                    const double & window_radius,
                    
                    // minima and maxima of a given octave scale:
                    const image_t * min,
                    const image_t * max)
{  
  // key orientation (angle of rotation of the local coordinate system):
  key.local_orientation_ = fmod(local_orientation, TWO_PI);
  key.target_orientation_ = key.local_orientation_;
  
  // generate the feature vector:
  switch (descriptor_version)
  {
    case 0:
      // original Lowe descrtiptor (a better implementation):
      // generate_feature_vector_v0(gL, 2.0 * keypoint_sigma, key);
      generate_feature_vector_v0(gL, window_radius, key);
      break;
      
    case 1:
      // original Lowe descrtiptor (a poor implementation):
      // generate_feature_vector_v1(gL, 2.0 * keypoint_sigma, key);
      generate_feature_vector_v1(gL, window_radius, key);
      break;
      
    case 2:
      // radially symmetric descriptor storing the min/max peak landscape
      // within the sections of the descriptor window:
      // generate_feature_vector_v2(min, max, 2.0 * keypoint_sigma, key);
      generate_feature_vector_v2(min, max, window_radius, key);
      break;
      
    case 3:
      // radially symmetric descriptor storing the dominant gradient
      // vectors within the sections of the descriptor window:
      generate_feature_vector_v3(gL, window_radius, key);
      break;
      
    case 4:
      // radially symmetric descriptor storing coarse histograms of gradient
      // vector orientations within the sections of the descriptor window:
      // generate_feature_vector_v4(gL, 2.0 * keypoint_sigma, key);
      generate_feature_vector_v4(gL, window_radius, key);
      break;
      
    case 5:
      // original Lowe descrtiptor (a better implementation):
      // generate_feature_vector_v5(gL, 2.0 * keypoint_sigma, key);
      generate_feature_vector_v5(gL, window_radius, key);
      break;
      
    case 6:
      // original Lowe descrtiptor (a better implementation):
      // generate_feature_vector_v6(gL, 2.0 * keypoint_sigma, key);
      generate_feature_vector_v6(gL, window_radius, key);
      break;
      
    default:
      assert(0);
  }
  
  if (descriptor_version != 3)
  {
    // normalize the feature vector:
    normalize(key.descriptor_);
    
//#if 1
    // FIXME: the following two steps are of questionable utility:
    
    // threshold the individual elements of the feature vector:
    threshold(key.descriptor_, 0.2);
    
    // re-normalize:
    normalize(key.descriptor_);
//#endif
  }
}

//----------------------------------------------------------------
// generate_orientations
// 
static void
generate_orientations(// the feature keys:
                      std::list<descriptor_t> & keys,
                      
                      // points of interest:
                      std::list<extrema_t> & extrema,
                      
                      // gradient image:
                      const gradient_image_t * gL,
                      
                      // sigma of the gaussian weighting function:
                      const double & keypoint_sigma)
{
  for (std::list<extrema_t>::iterator iter = extrema.begin();
       iter != extrema.end(); ++iter)
  {
    extrema_t & ext = *iter;
    
    // find potential key point orientations:
    std::list<gradient_t> direction_magnitude;
    if (!identify_orientations(gL,
                               ext.pixel_coords_[0],
                               ext.pixel_coords_[1],
                               1.5 * keypoint_sigma,
                               36,
                               2,
                               direction_magnitude))
    {
      continue;
    }
    
    descriptor_t key(&ext);
    for (std::list<gradient_t>::iterator jter = direction_magnitude.begin();
         jter != direction_magnitude.end(); ++jter)
    {
      const gradient_t & dm = *jter;
      key.local_orientation_ = dm[0];
      key.target_orientation_ = key.local_orientation_;
      
      keys.push_back(key);
      
      // FIXME: take the highest peak:
      break;
    }
  }
}

//----------------------------------------------------------------
// erode
// 
mask_t::Pointer
erode(const mask_t * mask, const int radius)
{
  mask_t::Pointer out = cast<mask_t, mask_t>(mask);
  
  // FIXME:
//#if 1
  mask_t::RegionType rn = mask->GetLargestPossibleRegion();
  mask_t::RegionType::SizeType sz = rn.GetSize();
  
  typedef itk::ImageRegionConstIteratorWithIndex<mask_t> itex_t;
  itex_t itex(mask, rn);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    mask_t::IndexType ix0 = itex.GetIndex();
    
    unsigned int sum = 0;
    
    // horizontal:
    for (int j = -radius; j <= radius; j++)
    {
      mask_t::IndexType ix = ix0;
      ix[0] = int(ix0[0]) + j;
      if (ix[0] >= 0 && image_size_value_t(ix[0]) < sz[0])
      {
        sum += mask->GetPixel(ix);
      }
    }
    
    for (int j = -radius; j <= radius; j++)
    {
      // vertical:
      mask_t::IndexType ix = ix0;
      ix[1] = int(ix0[1]) + j;
      if (ix[1] >= 0 && image_size_value_t(ix[1]) < sz[1])
      {
        sum += mask->GetPixel(ix);
      }
      
      // diagonal:
      ix[0] = static_cast<int>(ix0[0]) + j;
      ix[1] = static_cast<int>(ix0[1]) + j;
      if (ix[0] >= 0 && image_size_value_t(ix[0]) < sz[0] &&
          ix[1] >= 0 && image_size_value_t(ix[1]) < sz[1])
      {
        sum += mask->GetPixel(ix);
      }
      
      // diagonal:
      ix[1] = static_cast<int>(ix0[1]) - j;
      if (ix[0] >= 0 && image_size_value_t(ix[0]) < sz[0] &&
          ix[1] >= 0 && image_size_value_t(ix[1]) < sz[1])
      {
        sum += mask->GetPixel(ix);
      }
    }
    
    // if (sum != 0) cout << 2 + 4 * radius << '\t' << sum << std::endl;
    bool ok = (sum == static_cast<unsigned int>(4 + 8 * radius));
    out->SetPixel(ix0, ok);
  }
//#endif
  
  return out;
}


//----------------------------------------------------------------
// octave_t::r0_
// 
const double octave_t::r0_ = 6.0;

//----------------------------------------------------------------
// octave_t::r1_
// 
const double octave_t::r1_ = 32.0;

//----------------------------------------------------------------
// octave_t::setup
// 
void
octave_t::setup(const unsigned int & octave,
                const image_t * L0,
                const mask_t * mask,
                const double & s0,
                const double & k,
                const unsigned int s,
                const bool & make_keys)
{
  sigma_.resize(s + 3);
  L_.resize(s + 3);
  D_.resize(s + 2);
  gL_.resize(s);
  extrema_min_.resize(s);
  extrema_max_.resize(s);
  keys_min_.resize(s);
  keys_max_.resize(s);
  
  // deep copy the initial image:
  L_[0] = cast<image_t, image_t>(L0);
  sigma_[0] = s0;
  
  mask_ = mask;
  
  mask_eroded_ = mask;
  if (mask != NULL)
  {
    mask_eroded_ = erode(mask_, static_cast<int>(r0_));
  }
  
  // FIXME:
//#if 0
//  {
//    static unsigned int counter = 0;
//    save<native_image_t>(remap_min_max<native_image_t>
//                         (cast<mask_t, native_image_t>(mask_)),
//                         the_text_t("mask-") + the_text_t::number(counter) +
//                         "-v0.tif");
//    save<native_image_t>(remap_min_max<native_image_t>
//                         (cast<mask_t, native_image_t>(mask_eroded_)),
//                         the_text_t("mask-") + the_text_t::number(counter) +
//                         "-v1.tif");
//    counter++;
//  }
//#endif
  
  // calculate the DoG images:
  double sigma_scale = s0 * sqrt(k * k - 1.0);
  for (unsigned int p = 1; p < s + 3; p++)
  {
    sigma_[p] = s0 * integer_power<double>(k, p);
    double sp = s0 * integer_power<double>(k, p - 1) * sigma_scale;
    L_[p] = smooth<image_t>(L_[p - 1], sp);
    
    // skip calculating DoG if the weren't requested:
    if (!make_keys) continue;
    
    D_[p - 1] = subtract<image_t>(L_[p], L_[p - 1]);
  }
  
  // skip calculating the keys if they weren't requested:
  if (!make_keys) return;
  
  // calculate the gradient direction and magnitude:
  for (unsigned int p = 0; p < s; p++)
  {
    // first, calculate dL/dx, dL/dy:
    gradient_filter_t::Pointer gradient_filter = gradient_filter_t::New();
    gradient_filter->SetInput(L_[p]);
    gradient_filter->SetUseImageSpacing(true);
    
    try { gradient_filter->Update(); }
    catch (itk::ExceptionObject & exception)
    {
      // oops:
      std::cerr << "gradient filter threw an exception:" << std::endl
      << exception << std::endl;
      throw exception;
    }
    
    gL_[p] = gradient_filter->GetOutput();
    
    // convert the gradient vector into polar coordinates (angle, magnitude):
    typedef itk::ImageRegionIterator<gradient_image_t> iter_t;
    iter_t iter(gL_[p], gL_[p]->GetLargestPossibleRegion());
    for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
    {
      const gradient_t & Lxy = iter.Get();
      const image_t::PixelType & Lx = Lxy[0];
      const image_t::PixelType & Ly = Lxy[1];
      
      gradient_t angle_magnitude;
      angle_magnitude[0] = calc_angle(Lx, Ly);
      angle_magnitude[1] = sqrt(Lx * Lx + Ly * Ly);
      
      iter.Set(angle_magnitude);
    }
  }
}

//----------------------------------------------------------------
// octave_t::detect_extrema
// 
// for each scale, find the minima and maxima points of the DoF images:
void
octave_t::detect_extrema(const unsigned int & pyramid,
                         const unsigned int & octave,
                         const double percent_below_threshold,
                         const bool threshold_by_area,
                         const std::string & fn_prefix)
{
  raw_min_.resize(scales());
  raw_max_.resize(scales());
  
  for (unsigned int i = 0; i < scales(); i++)
  {
    const image_t * D0 = D_[i + 0];
    const image_t * D1 = D_[i + 1];
    const image_t * D2 = D_[i + 2];
    
//#if 1
    image_t::Pointer min_a = subtract<image_t>(D0, D1);
    image_t::Pointer min_b = subtract<image_t>(D2, D1);
    image_t::Pointer max_a = subtract<image_t>(D1, D0);
    image_t::Pointer max_b = subtract<image_t>(D1, D2);
    
//#if 1
    min_a = smooth<image_t>(min_a, 1.0);
    min_b = smooth<image_t>(min_b, 1.0);
    max_a = smooth<image_t>(max_a, 1.0);
    max_b = smooth<image_t>(max_b, 1.0);
//#endif
    
    min_a = clip_min_max<image_t>(min_a, 0.0, 255.0);
    min_b = clip_min_max<image_t>(min_b, 0.0, 255.0);
    max_a = clip_min_max<image_t>(max_a, 0.0, 255.0);
    max_b = clip_min_max<image_t>(max_b, 0.0, 255.0);
    
    // these images will be processed in place:
    image_t::Pointer min = multiply<image_t>(min_a, min_b);
    image_t::Pointer max = multiply<image_t>(max_a, max_b);
    
    // keep a copy of the un-thresholded min/max images around, so that
    // they can be used to build the feature keys:
    raw_min_[i] = cast<image_t, image_t>(min);
    raw_max_[i] = cast<image_t, image_t>(max);
    
    if (threshold_by_area)
    {
      threshold_by_area_inplace<image_t>(min, percent_below_threshold);
      threshold_by_area_inplace<image_t>(max, percent_below_threshold);
    }
    else
    {
      threshold_by_intensity_inplace<image_t>(min, percent_below_threshold);
      threshold_by_intensity_inplace<image_t>(max, percent_below_threshold);
    }
    
    remap_min_max_inplace<image_t>(min, 0.0, 1.0);
    remap_min_max_inplace<image_t>(max, 0.0, 1.0);
    
//    // FIXME:
//#if 0
//    if (fn_prefix.size() != 0)
//    {
//      save<native_image_t>(cast<image_t, native_image_t>
//                           (remap_min_max<image_t>(min, 0.0, 255.0)),
//                           fn_prefix + the_text_t::number(i) + "-min.tif");
//      
//      save<native_image_t>(cast<image_t, native_image_t>
//                           (remap_min_max<image_t>(max, 0.0, 255.0)),
//                           fn_prefix + the_text_t::number(i) + "-max.tif");
//      
//      image_t::Pointer minmax = subtract<image_t>(raw_max_[i], raw_min_[i]);
//      save<native_image_t>(cast<image_t, native_image_t>
//                           (remap_min_max<image_t>(minmax, 0.0, 255.0)),
//                           fn_prefix + "s" + the_text_t::number(i) +
//                           "-both.tif");
//    }
//#endif
    
    // extract clusters from the thresholded images:
    the_dynamic_array_t<cluster_t> clusters_min;
    the_dynamic_array_t<cluster_t> clusters_max;
    assemble_clusters<image_t>(min, mask_eroded_, 0.0, 8, clusters_min);
    assemble_clusters<image_t>(max, mask_eroded_, 0.0, 8, clusters_max);
    
    // identify the center of mass of each cluster:
    std::list<centerofmass_t<2> > cm_min;
    std::list<centerofmass_t<2> > cm_max;
    identify_clusters_cm(min, clusters_min, cm_min);
    identify_clusters_cm(max, clusters_max, cm_max);
    
    // generate extrema points:
    collect_extrema(extrema_min_[i],
                    pyramid,		// pyramid index
                    octave,		// octave index
                    i,			// scale index
                    D1,			// DoG image
                    mask_eroded_,
                    cm_min,		// centers of mass of maxima peaks
                    1.1);		// principal curvature ratio
    
    collect_extrema(extrema_max_[i],
                    pyramid,		// pyramid index
                    octave,		// octave index
                    i,			// scale index
                    D1,			// DoG image
                    mask_eroded_,
                    cm_max,		// centers of mass of maxima peaks
                    1.1);		// principal curvature ratio
//#else
//    // try to isolate flat regions in the image:
//    image_t::RegionType rn = L_[i]->GetLargestPossibleRegion();
//    image_t::RegionType::SizeType sz = rn.GetSize();
//    
//    cout << "enhancing the image with CLAHE" << std::endl;
//    image_t::Pointer L = // cast<image_t, image_t>(D_[1 + i]);
//    cast<image_t, image_t>(L_[i]);
//    /*
//     CLAHE<image_t>(L_[i], sz[0] / 16, sz[1] / 16, 1.1, 256, 0, 255, mask_eroded_);
//     */
//    
//    // typedef itk::CurvatureAnisotropicDiffusionImageFilter<image_t, image_t> anisodiff_t;
//    typedef itk::GradientAnisotropicDiffusionImageFilter<image_t, image_t> anisodiff_t;
//    anisodiff_t::Pointer adf = anisodiff_t::New();
//    adf->SetInput(L);
//    adf->SetInPlace(true);
//    adf->SetNumberOfIterations(500);
//    adf->SetTimeStep(0.125);
//    adf->SetConductanceParameter(0.25);
//    adf->UseImageSpacingOff();
//    adf->Update();
//    L = adf->GetOutput();
//    
//    save<native_image_t>(cast<image_t, native_image_t>
//                         (remap_min_max<image_t>(L)),
//                         fn_prefix + "s" + the_text_t::number(i) +
//                         "-clahe.tif");
//    
//    image_t::Pointer flat = cast<image_t, image_t>(L);
//    
//    typedef itk::ImageRegionConstIteratorWithIndex<image_t> itex_t;
//    itex_t itex(L, rn);
//    
//    double min = std::numeric_limits<double>::max();
//    double max = -min;
//    double avg = 0.0;
//    double wgt = 0.0;
//    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
//    {
//      double px = itex.Get();
//      image_t::IndexType ix = itex.GetIndex();
//      if (mask_eroded_.GetPointer() &&
//          mask_eroded_->GetPixel(ix) == 0)
//      {
//        continue;
//      }
//      
//      min = std::min(min, px);
//      max = std::max(max, px);
//      avg += px;
//      wgt += 1.0;
//    }
//    
//    if (wgt != 0.0)
//    {
//      avg /= wgt;
//    }
//    double rng = max - min;
//    
//    cout << "pyramid: " << pyramid << std::endl
//    << "octave: " << octave << std::endl
//    << "scale: " << i << std::endl
//    << "min: " << min << std::endl
//    << "max: " << max << std::endl
//    << "avg: " << avg << std::endl
//    << "rng: " << rng << std::endl
//    << std::endl;
//    
//    const unsigned int bins = 80;
//    std::vector<double> PDF(bins, 0.0);
//    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
//    {
//      image_t::PixelType px = itex.Get();
//      image_t::IndexType ix = itex.GetIndex();
//      if (mask_eroded_.GetPointer() &&
//          mask_eroded_->GetPixel(ix) == 0) continue;
//      
//      double t = (px - min) / rng;
//      unsigned index = int(t * double(bins - 1));
//      PDF[index] += 1.0;
//    }
//    /*
//     // find the mean:
//     double mean = 0.0;
//     double mass = 0.0;
//     for (unsigned int j = 0; j < bins; j++)
//     {
//     if (PDF[j] == 0.0) continue;
//     mean += PDF[j];
//     mass += 1.0;
//     }
//     
//     if (mass != 0.0)
//     {
//     mean /= mass;
//     }
//     */
//    
//    // find the center of mass:
//    double sumt = 0.0;
//    double sumw = 0.0;
//    double sumb = 0.0;
//    for (unsigned int j = 0; j < bins; j++)
//    {
//      if (PDF[j] == 0.0) continue;
//      
//      sumw += PDF[j];
//      sumt += PDF[j] * double(j);
//      sumb += 1.0;
//    }
//    
//    double cm = sumt / sumw;
//    double mean = sumw / sumb;
//    
//    double donation[3] = { 0.0 };
//    unsigned int donation_bin[3] = { 0 };
//    calc_histogram_donations(bins, 0.0, double(bins - 1), cm,
//                             donation_bin, donation);
//    double peak =
//    PDF[donation_bin[0]] * donation[0] +
//    PDF[donation_bin[1]] * donation[1] +
//    PDF[donation_bin[2]] * donation[2];
//    
//    // find the variance:
//    double variance = 0.0;
//    for (unsigned int j = 0; j < bins; j++)
//    {
//      if (PDF[j] == 0.0) continue;
//      
//      double d = PDF[j] - mean;
//      variance += d * d;
//    }
//    variance /= sumb;
//    
//    // standard deviation:
//    double stddev = sqrt(variance);
//    
//    // find the index of the central bin:
//    int center = int(cm);
//    int radius = int(stddev);
//    
//    // threshold the PDF:
//    dump(cout, &PDF[0], PDF.size(), 40, true);
//    /*
//     for (unsigned int j = 0; j < bins; j++)
//     {
//     if (PDF[j] < mean)
//     {
//     PDF[j] = 0.0;
//     }
//     }
//     */
//    cout << "cm:       " << cm << std::endl
//    << "mean:     " << mean << std::endl
//    << "peak:     " << peak << std::endl
//    << "variance: " << variance << std::endl
//    << "stddev:   " << stddev << std::endl
//    << "center:   " << center << std::endl
//    << "radius:   " << radius << std::endl;
//    
//    for (unsigned int j = 0; j < bins; j++)
//    {
//      /*
//       double r = PDF[j] / peak;
//       if (r < 0.1)
//       {
//       PDF[j] = 0.0;
//       }
//       */
//      
//      double d = double(j) - cm;
//      double t = (d < 0.0) ? -d / cm : d / (double(bins - 1) - cm);
//      if (t > 0.5)
//      {
//        PDF[j] = 0.0;
//      }
//    }
//    dump(cout, &PDF[0], PDF.size(), 40, true);
//    
//    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
//    {
//      image_t::PixelType px = itex.Get();
//      image_t::IndexType ix = itex.GetIndex();
//      double t = (px - min) / rng;
//      int index = int(t * double(bins - 1));
//      
//      if ((mask_eroded_.GetPointer() &&
//           mask_eroded_->GetPixel(ix) == 0) ||
//          PDF[index] != 0.0)
//      {
//        px = avg;
//      }
//      
//      flat->SetPixel(ix, px);
//    }
//    
//    save<native_image_t>(cast<image_t, native_image_t>
//                         (remap_min_max<image_t>(flat)),
//                         fn_prefix + "s" + the_text_t::number(i) +
//                         "-flat.tif");
//    
//#endif
  }
}

//----------------------------------------------------------------
// octave_t::generate_keys
// 
void
octave_t::generate_keys()
{
  for (unsigned int i = 0; i < scales(); i++)
  {
    // generate feature keys:
    ::generate_orientations(keys_min_[i],
                            extrema_min_[i],
                            gL_[i],		// gradient vector image
                            sigma_[i]);		// keypoint sigma
    
    ::generate_orientations(keys_max_[i],
                            extrema_max_[i],
                            gL_[i],		// gradient vector image
                            sigma_[i]);		// keypoint sigma
  }
}

//----------------------------------------------------------------
// octave_t::generate_descriptors
// 
void
octave_t::generate_descriptors(const unsigned int & descriptor_version)
{
  const unsigned int num_scales = scales();
  for (unsigned int i = 0; i < num_scales; i++)
  {
    // generate feature keys:
    const double r = r0_ + static_cast<double>(i) / static_cast<double>(num_scales) * (r1_ - r0_);
    
    std::list<descriptor_t>::iterator iter;
    for (iter = keys_min_[i].begin(); iter != keys_min_[i].end(); ++iter)
    {
      descriptor_t & key = *iter;
      generate_descriptor(descriptor_version,
                          key,
                          key.local_orientation_,
                          gL_[i],
                          sigma_[i],
                          r,
                          raw_min_[i],
                          raw_max_[i]);
    }
    
    for (iter = keys_max_[i].begin(); iter != keys_max_[i].end(); ++iter)
    {
      descriptor_t & key = *iter;
      generate_descriptor(descriptor_version,
                          key,
                          key.local_orientation_,
                          gL_[i],
                          sigma_[i],
                          r,
                          raw_min_[i],
                          raw_max_[i]);
    }
  }
}

//----------------------------------------------------------------
// octave_t::count_extrema
// 
// count the number of extrema points detected within this octave:
unsigned int
octave_t::count_extrema() const
{
  unsigned int total = 0;
  for (unsigned int i = 0; i < scales(); i++)
  {
    total += extrema_min_[i].size() + extrema_max_[i].size();
  }
  
  return total;
}

//----------------------------------------------------------------
// octave_t::count_keys
// 
// count the number of keys detected within this octave:
unsigned int
octave_t::count_keys() const
{
  unsigned int total = 0;
  for (unsigned int i = 0; i < scales(); i++)
  {
    total += keys_min_[i].size() + keys_max_[i].size();
  }
  
  return total;
}


//----------------------------------------------------------------
// pyramid_t::setup
// 
void
pyramid_t::setup(const image_t * initial_image,
                 const mask_t * mask,
                 const double &  initial_sigma,
                 const unsigned int s,
                 const double min_edge,
                 const bool & make_keys)
{
  typedef image_t::RegionType::SizeType sz_t;
  sz_t max_sz = initial_image->GetLargestPossibleRegion().GetSize();
  
  double adjusted_min_edge = min_edge;
  if ( adjusted_min_edge > max_sz[0] )
    adjusted_min_edge = max_sz[0];
  if ( adjusted_min_edge > max_sz[1] )
    adjusted_min_edge = max_sz[1];
  unsigned int num_octaves =
  1 + int(floor(std::min(log(static_cast<double>(max_sz[0]) / adjusted_min_edge),
                         log(static_cast<double>(max_sz[1]) / adjusted_min_edge)) / LOG_2));
  
  // setup the octaves:
  octave_.resize(num_octaves);
  
  static const double n = 2.0;
  const double k = pow(n, 1.0 / static_cast<double>(s));
  
  image_t::Pointer L0 = cast<image_t, image_t>(initial_image);
  mask_t::Pointer M0;
  if (mask != 0) M0 = cast<mask_t, mask_t>(mask);
  
  for (unsigned int i = 0; i < num_octaves; i++)
  {
    // initialize the octave:
    octave_[i].setup(i, L0, M0, initial_sigma, k, s, make_keys);
    
    // downsample the image:
    L0 = resize<image_t>(octave_[i].L_[s], 0.5);
    if (mask != NULL)
    {
      M0 = cast<image_t, mask_t>(resize<image_t>
                                 (cast<mask_t, image_t>(M0), 0.5));
    }
    
    // update the pixel spacing of the downsampled image:
    image_t::SpacingType spacing = octave_[i].L_[s]->GetSpacing();
    spacing[0] *= 2.0;
    spacing[1] *= 2.0;
    L0->SetSpacing(spacing);
    
    if (mask != NULL)
    {
      M0->SetSpacing(spacing);
    }
  }
}

//----------------------------------------------------------------
// pyramid_t::remove_lowest_resolution_octave
// 
// helper functions:
bool
pyramid_t::remove_lowest_resolution_octave(const unsigned int how_many)
{
  if (octaves() == 0) return false;
  
  std::vector<octave_t>::iterator start = octave_.begin();
  std::vector<octave_t>::iterator end = octave_.end();
  for (unsigned int i = 0; i < how_many && start != end; i++, --end);
  
  std::vector<octave_t> tmp(start, end);
  octave_ = tmp;
  return true;
}

//----------------------------------------------------------------
// pyramid_t::remove_highest_resolution_octave
// 
bool
pyramid_t::remove_highest_resolution_octave(const unsigned int how_many)
{
  if (octaves() == 0) return false;
  
  std::vector<octave_t>::iterator start = octave_.begin();
  std::vector<octave_t>::iterator end = octave_.end();
  for (unsigned int i = 0; i < how_many && start != end; i++, ++start);
  
  std::vector<octave_t> tmp(start, end);
  octave_ = tmp;
  return true;
}

//----------------------------------------------------------------
// pyramid_t::detect_extrema
// 
// generate key points from each octave:
// 
void
pyramid_t::detect_extrema(const unsigned int & pyramid,
                          const double percent_below_threshold,
                          const bool threshold_by_area,
                          const std::string & fn_prefix)
{
  for (unsigned int i = 0; i < octaves(); i++)
  {
    std::ostringstream prefix;
    prefix << fn_prefix;
    if (fn_prefix.size() != 0)
    {
      prefix << "o" << the_text_t::number(i) << "-";
    }
    
    octave_[i].detect_extrema(pyramid,	// pyramid id
                              i,	// octave id
                              percent_below_threshold,
                              threshold_by_area,
                              prefix.str());
  }
}

//----------------------------------------------------------------
// pyramid_t::generate_keys
// 
void
pyramid_t::generate_keys()
{
  for (unsigned int i = 0; i < octaves(); i++)
  {
    octave_[i].generate_keys();
  }
}

//----------------------------------------------------------------
// pyramid_t::generate_descriptors
// 
void
pyramid_t::generate_descriptors(const unsigned int & descriptor_version)
{
  for (unsigned int i = 0; i < octaves(); i++)
  {
    octave_[i].generate_descriptors(descriptor_version);
  }
}

//----------------------------------------------------------------
// pyramid_t::count_extrema
// 
// count the number of extrema points detected within this pyramid:
// 
unsigned int
pyramid_t::count_extrema() const
{
  unsigned int total = 0;
  for (unsigned int i = 0; i < octaves(); i++)
  {
    total += octave_[i].count_extrema();
  }
  
  return total;
}

//----------------------------------------------------------------
// pyramid_t::count_keys
// 
// count the number of keys detected within this pyramid:
// 
unsigned int
pyramid_t::count_keys() const
{
  unsigned int total = 0;
  for (unsigned int i = 0; i < octaves(); i++)
  {
    total += octave_[i].count_keys();
  }
  
  return total;
}

//----------------------------------------------------------------
// pyramid_t::debug
// 
// save a bunch of images with the keys marked on them in color:
// 
void
pyramid_t::debug(const std::string & fn_prefix) const
{
//#if 1
  // RGB:
  native_image_t::Pointer tmp[3];
  to_rgb<image_t>(octave_[0].L_[0], tmp);
  
  static const double offset = 3.0;
  static const double radius = 16.0;
  static const xyz_t zebra[] = {
    xyz(0.0, 127.0, 255.0),
    xyz(255.0, 127.0, 0.0)
  };
  
  const double sx = octave_[0].L_[0]->GetSpacing()[0];
  
  for (unsigned int i = 0; i < octaves(); i++)
  {
    const unsigned int s = octave_[i].scales();
    for (unsigned int j = 0; j < s && octave_[i].gL_[j].GetPointer(); j++)
    {
      double r0 =
      sx * (offset + radius * (static_cast<double>(i) + static_cast<double>(j) / static_cast<double>(s)));
      
      double r1 =
      sx * (offset + radius * (static_cast<double>(i) + static_cast<double>(j + 1) / static_cast<double>(s)));
      
      draw_keys(tmp, octave_[i].keys_min_[j], zebra[0], r0, r1, i);
      draw_keys(tmp, octave_[i].keys_max_[j], zebra[1], r0, r1, i);
      
      // FIXME:
      native_image_t::Pointer tmp2[3];
      to_rgb<image_t>(octave_[i].D_[j + 1], tmp2);
      
      r0 = sx * (offset + radius * static_cast<double>(j) / static_cast<double>(s));
      r1 = sx * (offset + radius * static_cast<double>(j + 1) / static_cast<double>(s));
      draw_keys(tmp2, octave_[i].keys_min_[j], zebra[0], r0, r1, 0);
      draw_keys(tmp2, octave_[i].keys_max_[j], zebra[1], r0, r1, 0);
      std::ostringstream fn;
      fn << fn_prefix << "." << the_text_t::number(i) << "." << the_text_t::number(j) << "-debug.tif";
      save_rgb<native_image_t::Pointer>(tmp2, fn.str());
    }
  }

  std::ostringstream oss;
  oss << fn_prefix << "-debug.tif";
  save_rgb<native_image_t::Pointer>(tmp, oss.str());
//#endif
}

//----------------------------------------------------------------
// save_image
// 
template <class T>
void
save_image(std::ostream & fout,
           const std::string & fn,
           const T * data)
{
  // save the filename:
  fout << fn << std::endl;
  
  // save pixel spacing:
  typename T::SpacingType sp = data->GetSpacing();
  fout << sp[0] << '\t' << sp[1] << std::endl;
  
  // save the image:
  ::save<T>(data, fn);
}

//----------------------------------------------------------------
// load_image
// 
template <class T>
void
load_image(const bfs::path & dir,
           std::istream & f_in,
           typename T::Pointer & data)
{
  std::string fn;
  f_in >> fn;
  
  typename T::SpacingType sp;
  f_in >> sp[0] >> sp[1];

  bfs::path data_fn = dir / fn;
  data = ::load<T>(data_fn.string());
  data->SetSpacing(sp);
}

//----------------------------------------------------------------
// save_images
// 
template <class T>
void
save_images(std::ostream & fout,
            const std::string & pfx,
            const std::string & sfx,
            const std::vector<T> & data)
{
  typedef typename T::ObjectType Ti;
  
  fout << data.size() << std::endl;
  for (unsigned int j = 0; j < data.size(); j++)
  {
    std::ostringstream fn;
    fn << pfx << ":" << the_text_t::number(j) << sfx;
    save_image<Ti>(fout, fn.str(), data[j]);
  }
}

//----------------------------------------------------------------
// load_images
// 
template <class T>
void
load_images(const std::string & dir,
            std::istream & f_in,
            std::vector<T> & data)
{
  typedef typename T::ObjectType Ti;
  
  unsigned int num = 0;
  f_in >> num;
  data.resize(num);
  
  for (unsigned int j = 0; j < data.size(); j++)
  {
    load_image<Ti>(dir, f_in, data[j]);
  }
}

//----------------------------------------------------------------
// save_extrema
// 
static void
save_extrema(std::ostream & fout, const extrema_t & ex)
{
  fout << ex.pixel_coords_[0] << ' ' << ex.pixel_coords_[1] << '\t'
  << ex.local_coords_[0] << ' ' << ex.local_coords_[1] << '\t'
  << ex.target_coords_[0] << ' ' << ex.target_coords_[1] << '\t'
  << ex.mass_ << ' '
  << ex.pyramid_ << ' '
  << ex.octave_ << ' '
  << ex.scale_ << std::endl;
}

//----------------------------------------------------------------
// load_extrema
// 
static void
load_extrema(std::istream & f_in, extrema_t & ex)
{
  f_in >> ex.pixel_coords_[0] >> ex.pixel_coords_[1]
  >> ex.local_coords_[0] >> ex.local_coords_[1]
  >> ex.target_coords_[0] >> ex.target_coords_[1]
  >> ex.mass_
  >> ex.pyramid_
  >> ex.octave_
  >> ex.scale_;
}

//----------------------------------------------------------------
// pyramid_t::save
// 
bool
pyramid_t::save(const std::string & fn_save) const
{
  std::fstream fout;
  fout.open(fn_save.c_str(), std::ios::out);
  if (!fout.is_open()) return false;
  
  std::ios::fmtflags old_flags = fout.setf(std::ios::scientific);
  int old_precision = fout.precision();
  fout.precision(12);
  
  // save the pyramid id and number of octaves:
  fout << fn_data_ << std::endl << octave_.size() << std::endl;
  
  // save each octave:
  for (unsigned int i = 0; i < octaves(); i++)
  {
    std::ostringstream fn_oct;
    fn_oct << fn_save << ":" << the_text_t::number(i, 2, '0');
    const octave_t & o = octave_[i];
    
    // save the masks:
    {
      std::ostringstream fn_mask;
      fn_mask << fn_oct << ":mask.nrrd";
      save_image<mask_t>(fout, fn_mask.str(), o.mask_);
      
      std::ostringstream fn_mask_eroded;
      fn_mask_eroded << fn_oct << ":mask_eroded.nrrd";
      save_image<mask_t>(fout, fn_mask_eroded.str(), o.mask_eroded_);
    }
    
    // save the blurred images:
    save_images<image_t::Pointer>(fout, fn_oct.str(), ":L.nrrd", o.L_);
    
    // save the Difference of Gaussian images:
    save_images<image_t::Pointer>(fout, fn_oct.str(), ":D.nrrd", o.D_);
    
    // save the gradient images:
    save_images<gradient_image_t::Pointer>(fout, fn_oct.str(), ":gL.nrrd", o.gL_);
    
    // save raw_min images:
    save_images<image_t::Pointer>(fout, fn_oct.str(), ":raw_min.nrrd", o.raw_min_);
    
    // save raw_max images:
    save_images<image_t::Pointer>(fout, fn_oct.str(), ":raw_max.nrrd", o.raw_max_);
    
    // save sigma:
    fout << o.sigma_.size() << std::endl;
    for (unsigned int j = 0; j < o.sigma_.size(); j++)
    {
      fout << o.sigma_[j] << std::endl;
    }
    
    // save extrema_min;
    fout << o.extrema_min_.size() << std::endl;
    for (unsigned int j = 0; j < o.extrema_min_.size(); j++)
    {
      fout << o.extrema_min_[j].size() << std::endl;
      for (std::list<extrema_t>::const_iterator
           iter = o.extrema_min_[j].begin();
           iter != o.extrema_min_[j].end(); ++iter)
      {
        save_extrema(fout, *iter);
      }
    }
    
    // save extrema_max;
    fout << o.extrema_max_.size() << std::endl;
    for (unsigned int j = 0; j < o.extrema_max_.size(); j++)
    {
      fout << o.extrema_max_[j].size() << std::endl;
      for (std::list<extrema_t>::const_iterator
           iter = o.extrema_max_[j].begin();
           iter != o.extrema_max_[j].end(); ++iter)
      {
        save_extrema(fout, *iter);
      }
    }
  }
  
  fout.setf(old_flags);
  fout.precision(old_precision);
  
  return true;
}

//----------------------------------------------------------------
// pyramid_t::load
// 
bool
pyramid_t::load(const std::string & fn_load)
{
  std::fstream f_in;
  f_in.open(fn_load.c_str(), std::ios::in);
  if (!f_in.is_open()) return false;
  
  // extract the directory portion from the filename:
//  std::string dir = fn_load.reverse().cut('/', 1, ~0).reverse();
  bfs::path fn_path(fn_load);
  bfs::path dir = fn_path.parent_path();
  if (dir.empty())
  {
    CORE_LOG_WARNING("Missing directory for loading pyramid file " + fn_load);
  }
  
//  if (!dir.is_empty() && !dir.match_tail("/"))
//    dir += "/";
  
  // load the pyramid id and number of octaves:
  f_in >> fn_data_;
  
  unsigned int num_octaves = 0;
  f_in >> num_octaves;
  octave_.resize(num_octaves);
  
  // load each octave:
  for (unsigned int i = 0; i < octaves(); i++)
  {
    octave_t & o = octave_[i];
    
    // load the masks:
    {
      mask_t::Pointer mask;
      load_image<mask_t>(dir.string(), f_in, mask);
      o.mask_ = mask;
      
      mask_t::Pointer mask_eroded;
      load_image<mask_t>(dir.string(), f_in, mask_eroded);
      o.mask_eroded_ = mask_eroded;
    }
    
    // load the blurred images:
    load_images<image_t::Pointer>(dir.string(), f_in, o.L_);
    
    // load the Difference of Gaussian images:
    load_images<image_t::Pointer>(dir.string(), f_in, o.D_);
    
    // load the gradient images:
    load_images<gradient_image_t::Pointer>(dir.string(), f_in, o.gL_);
    
    // load raw_min images:
    load_images<image_t::Pointer>(dir.string(), f_in, o.raw_min_);
    
    // load raw_max images:
    load_images<image_t::Pointer>(dir.string(), f_in, o.raw_max_);
    
    // load sigma:
    unsigned int num_sigma = 0;
    f_in >> num_sigma;
    o.sigma_.resize(num_sigma);
    
    for (unsigned int j = 0; j < o.sigma_.size(); j++)
    {
      f_in >> o.sigma_[j];
    }
    
    // load extrema_min:
    unsigned int num_ext_min = 0;
    f_in >> num_ext_min;
    o.extrema_min_.resize(num_ext_min);
    o.keys_min_.resize(num_ext_min);
    for (unsigned int j = 0; j < o.extrema_min_.size(); j++)
    {
      unsigned int num = 0;
      f_in >> num;
      for (unsigned int k = 0; k < num; k++)
      {
        extrema_t ex;
        load_extrema(f_in, ex);
        o.extrema_min_[j].push_back(ex);
      }
    }
    
    // load extrema_max:
    unsigned int num_ext_max = 0;
    f_in >> num_ext_max;
    o.extrema_max_.resize(num_ext_max);
    o.keys_max_.resize(num_ext_max);
    for (unsigned int j = 0; j < o.extrema_max_.size(); j++)
    {
      unsigned int num = 0;
      f_in >> num;
      for (unsigned int k = 0; k < num; k++)
      {
        extrema_t ex;
        load_extrema(f_in, ex);
        o.extrema_max_[j].push_back(ex);
      }
    }
  }
  
  return true;
}

//----------------------------------------------------------------
// load_pyramid
// 
void
load_pyramid(const std::string & fn_load,
             pyramid_t & pyramid,
             image_t::Pointer & mosaic,
             mask_t::Pointer & mosaic_mask)
{
  pyramid.load(fn_load);
  mosaic = pyramid.octave_[0].L_[0];
  mosaic_mask = const_cast<mask_t *>(pyramid.octave_[0].mask_.GetPointer());
}
