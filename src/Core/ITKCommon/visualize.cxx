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

// File         : visualize.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/05 11:16
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for visualizing unmatched
//                and matched SIFT keys and descriptors.

// local includes:
#include <Core/ITKCommon/visualize.hxx>
#include <Core/ITKCommon/the_utils.hxx>

// system includes:
#include <sstream>
#include <vector>


//----------------------------------------------------------------
// dump
// 
void
dump(std::ostream & so,
     const double * data,
     const unsigned int & size,
     const unsigned int & height,
     const bool & normalize)
{
  std::vector<std::string> plot(height);
  for (unsigned int i = 0; i < height; i++)
  {
    plot[i].assign(size, ' ');
  }
  
  double peak = 1.0;
  if (normalize)
  {
    peak = -std::numeric_limits<double>::max();
    for (unsigned int i = 0; i < size; i++)
    {
      peak = std::max(peak, data[i]);
    }
  }
  
  for (unsigned int i = 0; i < size; i++)
  {
    unsigned int h = (unsigned int)ceil((data[i] / peak) * double(height));
    for (unsigned int j = 0; j < h; j++)
    {
      plot[height - j - 1][i] = '#';
    }
  }
  
  so << '^' << std::endl;
  for (unsigned int i = 0; i < height; i++)
  {
    so << "| " << plot[i] << std::endl;
  }
  
  so << '+';
  for (unsigned int i = 0; i <= size; i++)
  {
    so << '-';
  }
  so << ">" << std::endl << std::endl;
}


//----------------------------------------------------------------
// dump
// 
void
dump(std::ostream & so,
     const double * data_a,
     const double * data_b,
     const unsigned int & size,
     const unsigned int & height,
     const bool & normalize)
{
  std::vector<std::string> plot(height);
  for (unsigned int i = 0; i < height; i++)
  {
    plot[i].assign(size, ' ');
  }
  
  double peak = 1.0;
  if (normalize)
  {
    peak = -std::numeric_limits<double>::max();
    for (unsigned int i = 0; i < size; i++)
    {
      peak = std::max(peak, std::max(data_a[i], data_b[i]));
    }
  }
  
  for (unsigned int i = 0; i < size; i++)
  {
    unsigned int h = static_cast<unsigned int>( ceil((data_a[i] / peak) * static_cast<double>(height)) );
    for (unsigned int j = 0; j < h; j++)
    {
      unsigned int index = height - j - 1;
      plot[index][i] = '=';
    }
    
    h = static_cast<unsigned int>( ceil((data_b[i] / peak) * static_cast<double>(height)) );
    for (unsigned int j = 0; j < h; j++)
    {
      unsigned int index = height - j - 1;
      if (plot[index][i] == '=')
      {
        plot[index][i] = '#';
      }
      else
      {
        plot[index][i] = '+';
      }
    }
  }
  
  so << '^' << std::endl;
  for (unsigned int i = 0; i < height; i++)
  {
    so << "| " << plot[i] << std::endl;
  }
  
  so << '+';
  for (unsigned int i = 0; i <= size; i++)
  {
    so << '-';
  }
  so << ">" << std::endl << std::endl;
}


//----------------------------------------------------------------
// operator
// 
template <typename data_t>
std::ostream &
operator << (std::ostream & so, const std::vector<data_t> & container)
{
  bool first = true;
  
  for (typename std::vector<data_t>::const_iterator i = container.begin();
       i != container.end(); ++i)
  {
    if (!first) so << ' ';
    else first = false;
    so << *i;
  }
  
  return so;
}


//----------------------------------------------------------------
// draw_feature_dot
// 
void
draw_feature_dot(native_image_t::Pointer * image,
                 const double & x0,
                 const double & y0,
                 const double & direction,
                 const double & R,
                 const xyz_t & color)
{
  native_image_t::IndexType index;
  native_image_t::PointType pt;
  pt[0] = x0;
  pt[1] = y0;
  image[0]->TransformPhysicalPointToIndex(pt, index);
  
  for (unsigned int ii = 0; ii < 3; ii++)
    mark<native_image_t>(image[ii], index, static_cast<unsigned char>(color[ii]), 1);
  
  const int thickness = static_cast<int>(floor((0.5 + R) / 0.75));
  for (int j = 0; j <= thickness; j++)
  {
    double t = static_cast<double>(j) / static_cast<double>(thickness);
    double radius = R * static_cast<double>(image[0]->GetSpacing()[0]) * t;
    double x1 = x0 + radius * cos(direction);
    double y1 = y0 + radius * sin(direction);
    
    double tc = t * t;
    double cmax = std::max(color[0], std::max(color[1], color[2]));
    xyz_t c = color * (255.0 * (1.0 - tc / 2.0) / cmax);
    
    unsigned int segments = 180;
    for (unsigned int i = 1; i <= segments; i++)
    {
      double t = static_cast<double>(i) / static_cast<double>(segments);
      double a = direction + TWO_PI * t;
      double x2 = x0 + radius * cos(a);
      double y2 = y0 + radius * sin(a);
      
      for (unsigned int ii = 0; ii < 3; ii++)
        draw_line<native_image_t>
        (image[ii], static_cast<unsigned char>(c[ii]), x1, y1, x2, y2);
      
      x1 = x2;
      y1 = y2;
    }
  }
}

//----------------------------------------------------------------
// draw_feature_key
// 
void
draw_feature_key(native_image_t::Pointer * image,
                 const double & x0,
                 const double & y0,
                 const double & direction,
                 const double & R,
                 const xyz_t & color)
{
  native_image_t::IndexType index;
  native_image_t::PointType pt;
  pt[0] = x0;
  pt[1] = y0;
  image[0]->TransformPhysicalPointToIndex(pt, index);
  
  for (unsigned int ii = 0; ii < 3; ii++)
    mark<native_image_t>(image[ii], index, static_cast<unsigned char>(color[ii]), 1);
  
  const int thickness = int(4.0 * image[0]->GetSpacing()[0]);
  for (int j = 0; j < thickness; j++)
  {
    double radius = R + static_cast<double>(j);// * 0.75;
    double x1 = x0 + radius * cos(direction);
    double y1 = y0 + radius * sin(direction);
    
    for (unsigned int ii = 0; ii < 3; ii++)
      draw_line<native_image_t>
      (image[ii], static_cast<unsigned char>(color[ii]), x0, y0, x1, y1);
    
    xyz_t ca(color);
    xyz_t cb = xyz(fabs(128.0 - ca[0]),
                   fabs(128.0 - ca[1]),
                   fabs(128.0 - ca[2]));
    
    unsigned int segments = 180;
    for (unsigned int i = 1; i <= segments; i++)
    {
      double t = static_cast<double>(i) / static_cast<double>(segments);
      double a = direction + TWO_PI * t;
      double x2 = x0 + radius * cos(a);
      double y2 = y0 + radius * sin(a);
      
      xyz_t c = ca * t + cb * (1.0 - t);
      
      for (unsigned int ii = 0; ii < 3; ii++)
        draw_line<native_image_t>
        (image[ii], static_cast<unsigned char>(c[ii]), x1, y1, x2, y2);
      
      x1 = x2;
      y1 = y2;
    }
  }
}

//----------------------------------------------------------------
// draw_feature_vector
// 
// x0, y0 are expressed in physical coordinates:
// 
void
draw_feature_vector(native_image_t::Pointer * image,
                    double x0,
                    double y0,
                    const double & direction,
                    const xyz_t & color,
                    const std::vector<double> & feature,
                    double scale)
{
  const unsigned int num_features = feature.size();
  
  xyz_t ca(color);
  xyz_t cb = xyz(fabs(128.0 - ca[0]),
                 fabs(128.0 - ca[1]),
                 fabs(128.0 - ca[2]));
  
  // find the range of the vector entries:
  double min = std::numeric_limits<double>::max();
  double max = -min;
  for (unsigned int i = 0; i < num_features; i++)
  {
    const double f = feature[i];
    min = std::min(min, f);
    max = std::max(max, f);
  }
  
  double rng = max - min;
  // TODO: replace cerr with logging
  if (rng == 0.0)
  {
    std::cerr << "bad feature key:";
    for (unsigned int i = 0; i < num_features; i++)
    {
      std::cerr << ' ' << feature[i];
    }
    std::cerr << ", skipping..." << std::endl;
    return;
  }
  
  unsigned int height =
//#if 0
//  16
//#elif 0
//  4
//#elif 1
  8
//#else
//  32
//#endif
  ;
  
  height = static_cast<int>(static_cast<double>(height) * scale + 0.5);
  
  const double sx = image[0]->GetSpacing()[0];
  const double H = static_cast<double>(height) * sx;
  const double W = 4.0 * H;
  
  xyz_t cc = cb + (ca - cb) * 0.5;
  
  for (unsigned int ii = 0; ii < 3; ii++)
    draw_line_rotated<native_image_t>
    (image[ii], static_cast<unsigned char>(cc[ii]),
     x0, y0, x0 + W, y0, direction, x0, y0);
  
  for (unsigned int ii = 0; ii < 3; ii++)
    draw_line_rotated<native_image_t>
    (image[ii], static_cast<unsigned char>(cc[ii]),
     x0, y0 - H, x0 + W, y0 - H, direction, x0, y0);
  
  for (unsigned int ii = 0; ii < 3; ii++)
    draw_line_rotated<native_image_t>
    (image[ii], static_cast<unsigned char>(cc[ii]),
     x0, y0, x0, y0 - H, direction, x0, y0);
  
  for (unsigned int ii = 0; ii < 3; ii++)
    draw_line_rotated<native_image_t>
    (image[ii], static_cast<unsigned char>(cc[ii]),
     x0 + W, y0, x0 + W, y0 - H, direction, x0, y0);
  
  for (unsigned int i = 0; i < num_features; i++)
  {
    const double f = feature[i];
    const double t = static_cast<double>(i) / static_cast<double>(num_features - 1);
    
    double x1 = x0 + t * W;
    double h0 = H * f / max;
    double y1 = y0 - h0;
    
    for (unsigned int ii = 0; ii < 3; ii++)
      draw_line_rotated<native_image_t>
      (image[ii], static_cast<unsigned char>(color[ii]),
       x1, y0, x1, y1, direction, x0, y0);
  }
  
  native_image_t::IndexType index;
  native_image_t::PointType pt;
  pt[0] = x0;
  pt[1] = y0;
  image[0]->TransformPhysicalPointToIndex(pt, index);
  for (unsigned int ii = 0; ii < 3; ii++)
    mark<native_image_t>(image[ii], index, static_cast<unsigned char>(color[ii]), 3);
}

//----------------------------------------------------------------
// draw_keys
// 
void
draw_keys(native_image_t::Pointer * image,
          const std::list<descriptor_t> & keys,
          const xyz_t & rgb,
          const double & r0,
          const double & r1,
          const unsigned int octave)
{
  static const xyz_t EAST  = xyz(1, 0, 0);
  static const xyz_t NORTH = xyz(0, 1, 0);
  static const xyz_t WEST  = xyz(0, 0, 1);
  static const xyz_t SOUTH = xyz(0, 0, 0);
  
  // find min/max range of the keys:
  double key_min = std::numeric_limits<double>::max();
  double key_max = -key_min;
  
  for (std::list<descriptor_t>::const_iterator i = keys.begin();
       i != keys.end(); ++i)
  {
    const descriptor_t & d = *i;
    if (d.extrema_ == NULL) continue;
    
    const double & v = d.extrema_->mass_;
    key_min = std::min(key_min, v);
    key_max = std::max(key_max, v);
  }
  
  double key_rng = key_max - key_min;
  if (key_rng == 0.0) key_rng = 1.0;
  
  // draw the keys:
  const unsigned int num_keys = keys.size();
  std::list<descriptor_t>::const_iterator iter = keys.begin();
  for (unsigned int i = 0; i < num_keys; i++, ++iter)
  {
    const descriptor_t & key = *iter;
    if (key.extrema_ == NULL) continue;
    
//#if 0
//    const double t = (key.extrema_->mass_ - key_min) / key_rng;
//    const double radius = r0 + t * (r1 - r0);
//    draw_feature_key(image,
//                     key.extrema_->local_coords_[0],
//                     key.extrema_->local_coords_[1],
//                     key.local_orientation_,
//                     radius,
//                     rgb);
//#else
    
    // calculate the key color:
    xyz_t color;
    {
      double t = double(i) / double(num_keys);
      double angle = ((2.0 * t) - 1.0) * M_PI;
      double x = cos(angle);
      double y = sin(angle);
      
      double u = (x + 1.0) / 2.0;
      double v = (y + 1.0) / 2.0;
      
      xyz_t cu = WEST  + (EAST  - WEST)  * u;
      xyz_t cv = SOUTH + (NORTH - SOUTH) * v;
      color = (cu + cv) * 255.0;
    }
    
    draw_feature_vector(image,
                        key.extrema_->local_coords_[0],
                        key.extrema_->local_coords_[1],
                        key.local_orientation_,
                        color,
                        key.descriptor_,
                        static_cast<double>(key.extrema_->octave_ + 1));
//#endif
  }
}

//----------------------------------------------------------------
// visualize_matches
// 
// visualize the matching keys:
// 
void
visualize_matches(const pyramid_t & a,
                  const pyramid_t & b,
                  const unsigned int & io, // octave index
                  const unsigned int & is, // scale index
                  const std::list<match_t> & ab,
                  const bfs::path & fn_prefix,
                  unsigned int num_keys)
{
  static const xyz_t EAST  = xyz(1, 0, 0);
  static const xyz_t NORTH = xyz(0, 1, 0);
  static const xyz_t WEST  = xyz(0, 0, 1);
  static const xyz_t SOUTH = xyz(0, 0, 0);
  
  native_image_t::Pointer a_rgb[3]; 
  to_rgb<image_t>(a.octave_[io].D_[is], a_rgb);
  
  native_image_t::Pointer b_rgb[3];
  to_rgb<image_t>(b.octave_[io].D_[is], b_rgb);
  
  if (num_keys == ~0u)
  {
    num_keys = ab.size();
  }
  else
  {
    num_keys = std::min(num_keys, static_cast<unsigned int>(ab.size()));
  }
  
  // draw the matching keys:
  {
    unsigned int index = 0;
    for (std::list<match_t>::const_iterator i = ab.begin();
         i != ab.end() && index < num_keys;
         i++, index++)
    {
      const match_t & rec = *i;
      
      const descriptor_t & a_key = *(rec.a_);
      const descriptor_t & b_key = *(rec.b_);
      
      // calculate the key color:
      xyz_t color;
      {
        double t = static_cast<double>(index) / static_cast<double>(num_keys);
        double angle = ((2.0 * t) - 1.0) * M_PI;
        double x = cos(angle);
        double y = sin(angle);
        
        double u = (x + 1.0) / 2.0;
        double v = (y + 1.0) / 2.0;
        
        xyz_t cu = WEST  + (EAST  - WEST)  * u;
        xyz_t cv = SOUTH + (NORTH - SOUTH) * v;
        color = (cu + cv) * 255.0;
      }
      
//#if 0
//      // calculate the key radius:
//      double t = fmod(double(index % 3) / 3.0 +
//                      double(index) / double(num_keys - 1), 1.0);
//      double ra = r0 + t * (r1 - r0);
//      double rb = ra;
//      
//      draw_feature_key(a_rgb,
//                       a_key.extrema_->local_coords_[0],
//                       a_key.extrema_->local_coords_[1],
//                       a_key.local_orientation_,
//                       ra,
//                       color);
//      
//      draw_feature_key(b_rgb,
//                       b_key.extrema_->local_coords_[0],
//                       b_key.extrema_->local_coords_[1],
//                       b_key.local_orientation_,
//                       rb,
//                       color);
//#else
      draw_feature_vector(a_rgb,
                          a_key.extrema_->local_coords_[0],
                          a_key.extrema_->local_coords_[1],
                          a_key.local_orientation_,
                          color,
                          a_key.descriptor_,
                          static_cast<double>(a_key.extrema_->octave_ + 1));
      
      draw_feature_vector(b_rgb,
                          b_key.extrema_->local_coords_[0],
                          b_key.extrema_->local_coords_[1],
                          b_key.local_orientation_,
                          color,
                          b_key.descriptor_,
                          static_cast<double>(b_key.extrema_->octave_ + 1));
//#endif
    }
  }
  
  std::ostringstream fn;
  fn << fn_prefix << the_text_t::number(io) << ":" << the_text_t::number(is);
  
  save_rgb<native_image_t::Pointer>(a_rgb, fn.str() + "-a.tif");
  save_rgb<native_image_t::Pointer>(b_rgb, fn.str() + "-b.tif");
}

//----------------------------------------------------------------
// visualize_best_fit
// 
void
visualize_best_fit(const bfs::path & fn_prefix,
                   const image_t * a_img,
                   const image_t * b_img,
                   const base_transform_t * t_ab,
                   const std::vector<const match_t *> & ab,
                   const std::list<unsigned int> & inliers,
                   const mask_t * a_mask,
                   const mask_t * b_mask)
{
  static const xyz_t EAST  = xyz(1, 0, 0);
  static const xyz_t NORTH = xyz(0, 1, 0);
  static const xyz_t WEST  = xyz(0, 0, 1);
  static const xyz_t SOUTH = xyz(0, 0, 0);
  
  native_image_t::Pointer a_rgb[3];
  to_rgb<image_t>(a_img, a_rgb);
  
  native_image_t::Pointer b_rgb[3];
  to_rgb<image_t>(b_img, b_rgb);
  
  // FIXME:
  const unsigned int num_inliers = inliers.size();
  const unsigned int step_size =
  std::max(1u, static_cast<unsigned int>(floor(0.5 + static_cast<double>(num_inliers) / 20.0)));
  
  std::list<unsigned int>::const_iterator iter = inliers.begin();
  for (unsigned int i = 0; i < num_inliers; i++, ++iter)
  {
    // FIXME: if ((i % step_size) != 0) continue;
    
    const match_t * match = ab[*iter];
    const descriptor_t & a = *(match->a_);
    const descriptor_t & b = *(match->b_);
    
    // calculate the key color:
    xyz_t color;
    {
      // double t = double(i) / double(num_inliers);
      double t = fmod(static_cast<double>(i % 3) / 3.0 +
                      static_cast<double>(i) / static_cast<double>(num_inliers - 1), 1.0);
      
      double angle = ((2.0 * t) - 1.0) * M_PI;
      double x = cos(angle);
      double y = sin(angle);
      
      double u = (x + 1.0) / 2.0;
      double v = (y + 1.0) / 2.0;
      
      xyz_t cu = WEST  + (EAST  - WEST)  * u;
      xyz_t cv = SOUTH + (NORTH - SOUTH) * v;
      color = (cu + cv) * 255.0;
    }
    
    // draw ski poles:
    /*
     {
     // FIXME:
     #if 0
     dump(std::cout, &(a.descriptor_[0]), a.descriptor_.size(), 10, true);
     dump(std::cout, &(b.descriptor_[0]), b.descriptor_.size(), 10, true);
     std::cout << "a: " << a.descriptor_ << std::endl
	   << "b: " << b.descriptor_ << std::endl
	   << std::endl;
     #endif
     
     draw_feature_dot(a_rgb,
     a.extrema_->local_coords_[0],
     a.extrema_->local_coords_[1],
     a.local_orientation_,
     4.0,
     color);
     
     draw_feature_dot(b_rgb,
     b.extrema_->local_coords_[0],
     b.extrema_->local_coords_[1],
     b.local_orientation_,
     4.0,
     color);
     }
     */
    
    // draw descriptors:
    {
      draw_feature_vector(a_rgb,
                          a.extrema_->local_coords_[0],
                          a.extrema_->local_coords_[1],
                          a.local_orientation_,
                          color,
                          a.descriptor_,
                          double(a.extrema_->octave_ + 1));
      
      draw_feature_vector(b_rgb,
                          b.extrema_->local_coords_[0],
                          b.extrema_->local_coords_[1],
                          b.local_orientation_,
                          color,
                          b.descriptor_,
                          double(b.extrema_->octave_ + 1));
    }
    
// FIXME:
//#if 0
//    std::cout << fn_prefix
//	 << " a" << a.octave_ << '.' << a.scale_
//	 << " b" << b.octave_ << '.' << b.scale_
//	 << std::endl;
//#endif
  }
  
  std::cout << "num inliers: " << num_inliers << std::endl
  << "step size:   " << step_size << std::endl;
  
  // save the keys:
  save_rgb<native_image_t::Pointer>(a_rgb, fn_prefix.string() + "a.tif");
  save_rgb<native_image_t::Pointer>(b_rgb, fn_prefix.string() + "b.tif");
  
  // save a mosaic:
  save_rgb<image_t>(fn_prefix.string() + "mosaic.tif",
                    remap_min_max<image_t>(a_img),
                    remap_min_max<image_t>(b_img),
                    t_ab,
                    a_mask,
                    b_mask);
  
  base_transform_t::ConstPointer tmp(t_ab);
  std::cout << "t_ab: " << tmp << std::endl;
}

//----------------------------------------------------------------
// visualize_matches_v2
// 
// visualize the matching keys:
// 
void
visualize_matches_v2(const pyramid_t & a,
                     const pyramid_t & b,
                     const std::list<const match_t *> & ab,
                     const bfs::path & fn_prefix,
                     unsigned int num_keys)
{
  static const xyz_t EAST  = xyz(1, 0, 0);
  static const xyz_t NORTH = xyz(0, 1, 0);
  static const xyz_t WEST  = xyz(0, 0, 1);
  static const xyz_t SOUTH = xyz(0, 0, 0);
  
  native_image_t::Pointer a_rgb[3]; 
  to_rgb<image_t>(a.octave_[0].L_[0], a_rgb);
  
  native_image_t::Pointer b_rgb[3];
  to_rgb<image_t>(b.octave_[0].L_[0], b_rgb);
  
  if (num_keys == ~0u)
  {
    num_keys = ab.size();
  }
  else
  {
    num_keys = std::min(num_keys, static_cast<unsigned int>(ab.size()));
  }
  
  // draw the matching keys:
  std::list<const match_t *>::const_iterator iter = ab.begin();
  for (unsigned int i = 0; i < num_keys; i++, ++iter)
  {
    const match_t * rec = *iter;
    
    const descriptor_t & a_key = *(rec->a_);
    const descriptor_t & b_key = *(rec->b_);
    
    // calculate the key color:
    xyz_t color;
    {
      double t = static_cast<double>(i) / static_cast<double>(num_keys);
      double angle = ((2.0 * t) - 1.0) * M_PI;
      double x = cos(angle);
      double y = sin(angle);
      
      double u = (x + 1.0) / 2.0;
      double v = (y + 1.0) / 2.0;
      
      xyz_t cu = WEST  + (EAST  - WEST)  * u;
      xyz_t cv = SOUTH + (NORTH - SOUTH) * v;
      color = (cu + cv) * 255.0;
    }
//#if 1
    draw_feature_vector(a_rgb,
                        a_key.extrema_->local_coords_[0],
                        a_key.extrema_->local_coords_[1],
                        a_key.local_orientation_,
                        color,
                        a_key.descriptor_,
                        static_cast<double>(a_key.extrema_->octave_ + 1));
    
    draw_feature_vector(b_rgb,
                        b_key.extrema_->local_coords_[0],
                        b_key.extrema_->local_coords_[1],
                        b_key.local_orientation_,
                        color,
                        b_key.descriptor_,
                        static_cast<double>(b_key.extrema_->octave_ + 1));
//#else
//    draw_feature_dot(a_rgb,
//                     a_key.extrema_->local_coords_[0],
//                     a_key.extrema_->local_coords_[1],
//                     a_key.local_orientation_,
//                     4.0,
//                     color);
//    
//    draw_feature_dot(b_rgb,
//                     b_key.extrema_->local_coords_[0],
//                     b_key.extrema_->local_coords_[1],
//                     b_key.local_orientation_,
//                     4.0,
//                     color);
//#endif
  }
  
  std::ostringstream fn;
  fn << fn_prefix << "matched";
  save_rgb<native_image_t::Pointer>(a_rgb, fn.str() + "-a.tif");
  save_rgb<native_image_t::Pointer>(b_rgb, fn.str() + "-b.tif");
}

//----------------------------------------------------------------
// visualize_nn
// 
void
visualize_nn(const bfs::path & fn_prefix,
             const pyramid_t & a,
             const pyramid_t & b,
             const ext_wrapper_t & b_key_wrapper,
             const std::vector<ext_wrapper_t> & a_key_wrappers,
             const double & window_radius)
{
  static const xyz_t EAST  = xyz(1, 0, 0);
  static const xyz_t NORTH = xyz(0, 1, 0);
  static const xyz_t WEST  = xyz(0, 0, 1);
  static const xyz_t SOUTH = xyz(0, 0, 0);
  
  static const double offset = 3.0;
  static const double radius = 21.0;
  
  const double r0 = offset;
  const double r1 = offset + radius;
  
  native_image_t::Pointer a_rgb[3]; 
  to_rgb<image_t>(a.octave_[0].L_[0], a_rgb);
  
  native_image_t::Pointer b_rgb[3];
  to_rgb<image_t>(b.octave_[0].L_[0], b_rgb);
  
  draw_feature_key(b_rgb,
                   b_key_wrapper.key_->extrema_->local_coords_[0],
                   b_key_wrapper.key_->extrema_->local_coords_[1],
                   b_key_wrapper.key_->local_orientation_,
                   window_radius + 1.0,
                   xyz(0, 0, 0));
  
  const unsigned int num_keys = a_key_wrappers.size();
  
  // draw the matching keys:
  for (unsigned int i = 0; i < num_keys; i++)
  {
    const ext_wrapper_t & a_key_wrapper = a_key_wrappers[i];
    const descriptor_t & a_key = *(a_key_wrapper.key_);
    
    double distance = 0.0;
    for (unsigned int j = 0; j < 2; j++)
    {
      double d = a_key_wrapper[j] - b_key_wrapper[j];
      distance += d * d;
    }
    distance = sqrt(distance);
    
    // calculate the key color:
    xyz_t color;
    {
      double t = static_cast<double>(i) / static_cast<double>(num_keys);
      double angle = ((2.0 * t) - 1.0) * M_PI;
      double x = cos(angle);
      double y = sin(angle);
      
      double u = (x + 1.0) / 2.0;
      double v = (y + 1.0) / 2.0;
      
      xyz_t cu = WEST  + (EAST  - WEST)  * u;
      xyz_t cv = SOUTH + (NORTH - SOUTH) * v;
      color = (cu + cv) * 255.0;
    }
    
    // calculate the key radius:
    double t = fmod(static_cast<double>(i % 3) / 3.0 +
                    static_cast<double>(i) / static_cast<double>(num_keys - 1), 1.0);
    double ra = r0 + t * (r1 - r0);
    double rb = ra;
    
//#if 1
    if (distance > window_radius)
    {
      draw_feature_key(a_rgb,
                       a_key.extrema_->local_coords_[0],
                       a_key.extrema_->local_coords_[1],
                       a_key.local_orientation_,
                       0.0,
                       color);
      
      draw_feature_key(b_rgb,
                       a_key_wrapper[0],
                       a_key_wrapper[1],
                       a_key_wrapper.key_->target_orientation_,
                       0.0,
                       color);
    }
    else
//#endif
    {
      t = 1.0 - distance / window_radius;
      ra = window_radius * t + 1.0;
      rb = ra;
      
      draw_feature_key(a_rgb,
                       a_key.extrema_->local_coords_[0],
                       a_key.extrema_->local_coords_[1],
                       a_key.local_orientation_,
                       ra,
                       color);
      
      draw_feature_key(b_rgb,
                       a_key_wrapper[0],
                       a_key_wrapper[1],
                       a_key_wrapper.key_->target_orientation_,
                       ra,
                       color);
    }
  }
  
  std::ostringstream fn;
  fn << fn_prefix << "nn";
  save_rgb<native_image_t::Pointer>(a_rgb, fn.str() + "-a.tif");
  save_rgb<native_image_t::Pointer>(b_rgb, fn.str() + "-b.tif");
}
