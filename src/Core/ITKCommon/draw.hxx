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

// File         : draw.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/01/04 11:26
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Functions for drawing anti-aliased lines using
//                Xiaolin Wu's line algorithm taken from the wikipedia article
//                http://en.wikipedia.org/wiki/Xiaolin_Wu's_line_algorithm

#ifndef DRAW_HXX_
#define DRAW_HXX_


// system includes:
#include <algorithm>

// ITK includes:
#include <itkImage.h>


namespace wu
{
  //----------------------------------------------------------------
  // ipart
  // 
  // return integer part of x
  inline double ipart(const double & x)
  {
    return (x == x) ? x - fmod(x, 1.0) : x;
  }
  
  //----------------------------------------------------------------
  // round
  // 
  inline double round(const double & x)
  {
    return ipart(x + 0.5);
  }
  
  //----------------------------------------------------------------
  // fpart
  // 
  // return fractional part of x
  inline double fpart(const double & x)
  {
    return x - ipart(x);
  }
  
  //----------------------------------------------------------------
  // rfpart
  // 
  inline double rfpart(const double & x)
  {
    return 1.0 - fpart(x);
  }
  
  //----------------------------------------------------------------
  // plot_pixel
  // 
  template <class image_t>
  void
  plot_pixel(typename image_t::Pointer & image,
       const unsigned int & width,
       const unsigned int & height,
       const typename image_t::IndexType & index,
       const double & w,
       const double & c)
  {
    if ((unsigned int)(index[0]) >= width ||
  (unsigned int)(index[1]) >= height)
    {
      return;
    }
    
    typedef typename image_t::PixelType color_t;
    double o = double(image->GetPixel(index));
    double n = (1.0 - w) * o + w * c;
    n = std::max(0.0, std::min(255.0, n));
    image->SetPixel(index, color_t(n));
  }
  
  //----------------------------------------------------------------
  // plot_normal
  // 
  template <class image_t>
  void
  plot_normal(typename image_t::Pointer & image,
        const unsigned int & width,
        const unsigned int & height,
        const double & x,
        const double & y,
        const double & w,
        const double & c)
  {
    typename image_t::IndexType index;
    index[0] = (unsigned int)(x);
    index[1] = (unsigned int)(y);
    plot_pixel<image_t>(image, width, height, index, w, c);
  }
  
  //----------------------------------------------------------------
  // plot_swapped
  // 
  template <class image_t>
  void
  plot_swapped(typename image_t::Pointer & image,
         const unsigned int & width,
         const unsigned int & height,
         const double & x,
         const double & y,
         const double & w,
         const double & c)
  {
    typename image_t::IndexType index;
    index[0] = (unsigned int)(y);
    index[1] = (unsigned int)(x);
    plot_pixel<image_t>(image, width, height, index, w, c);
  }
  
  //----------------------------------------------------------------
  // draw_nearly_horizontal_line
  // 
  // Xiaolin Wu's line algorithm taken from the wikipedia article
  // http://en.wikipedia.org/wiki/Xiaolin_Wu's_line_algorithm
  // 
  template <class image_t>
  void
  draw_nearly_horizontal_line
  (typename image_t::Pointer & image,
   const double & c,
   double x1,
   double y1,
   double x2,
   double y2,
   void(*draw_pixel)(typename image_t::Pointer &, // image
         const unsigned int &,    // width
         const unsigned int &,    // height
         const double &,      // x
         const double &,      // y
         const double &,      // weight
         const double &))     // paint color
  {
    typename image_t::SizeType sz =
      image->GetLargestPossibleRegion().GetSize();
    
    // check that x1 < x2
    if (x2 < x1)
    {
      std::swap(x1, x2);
      std::swap(y1, y2);
    }
    
    double dx = x2 - x1;
    double dy = y2 - y1;
    double gradient = dy / dx;
    
    // handle first endpoint
    double xend = round(x1);
    double yend = y1 + gradient * (xend - x1);
    double xgap = rfpart(x1 + 0.5);
    double xpxl1 = xend;  // this will be used in the main loop
    double ypxl1 = ipart(yend);
    draw_pixel(image, sz[0], sz[1], xpxl1, ypxl1, rfpart(yend) * xgap, c);
    draw_pixel(image, sz[0], sz[1], xpxl1, ypxl1 + 1, fpart(yend) * xgap, c);
    double intery = yend + gradient; // first y-intersection for the main loop
    
    // handle second endpoint
    xend = round(x2);
    yend = y2 + gradient * (xend - x2);
    xgap = rfpart(x2 - 0.5);
    double xpxl2 = xend;  // this will be used in the main loop
    double ypxl2 = ipart(yend);
    draw_pixel(image, sz[0], sz[1], xpxl2, ypxl2, rfpart(yend) * xgap, c);
    draw_pixel(image, sz[0], sz[1], xpxl2, ypxl2 + 1, fpart(yend) * xgap, c);
    
    // main loop
    for (double x = xpxl1 + 1.0; x <= xpxl2 - 1.0; x += 1.0)
    {
      draw_pixel(image, sz[0], sz[1], x, ipart(intery), rfpart(intery), c);
      draw_pixel(image, sz[0], sz[1], x, ipart(intery) + 1, fpart(intery), c);
      intery = intery + gradient;
    }
  }
}

//----------------------------------------------------------------
// draw_line
//
template <class image_t>
void
draw_line(typename image_t::Pointer & image,
    const typename image_t::PixelType color,
    double x1,
    double y1,
    double x2,
    double y2)
{
  if (false)
  {
    // MSVC++ will not instantiate plot_normal and plot_swapped
    // unless it sees them called explicitly:
    wu::plot_normal<image_t>(image, 0, 0, 0.0, 0.0, 0.0, 0.0);
    wu::plot_swapped<image_t>(image, 0, 0, 0.0, 0.0, 0.0, 0.0);
  }
  
  double dx = fabs(x2 - x1);
  double dy = fabs(y2 - y1);
  
  typename image_t::PointType origin = image->GetOrigin();
  typename image_t::SpacingType spacing = image->GetSpacing();
  
  x1 /= spacing[0];
  x2 /= spacing[0];
  
  y1 /= spacing[1];
  y2 /= spacing[1];
  
  if (dx > dy)
  {
    // horizontal line:
    wu::draw_nearly_horizontal_line<image_t>(image,
               double(color),
               x1, y1, x2, y2,
               wu::plot_normal<image_t>);
  }
  else if (dy != 0.0)
  {
    // vertical line:
    wu::draw_nearly_horizontal_line<image_t>(image,
               double(color),
               y1, x1, y2, x2,
               wu::plot_swapped<image_t>);
  }
}

//----------------------------------------------------------------
// draw_line_rotated
// 
template <class image_t>
void
draw_line_rotated(typename image_t::Pointer & image,
      const typename image_t::PixelType color,
      const double & x1,
      const double & y1,
      const double & x2,
      const double & y2,
      const double & radians,
      const double & ox = 0.0,
      const double & oy = 0.0)
{
  const double ct = ::cos(radians);
  const double st = ::sin(radians);
  
  const double u1 = ox + ct * (x1 - ox) - st * (y1 - oy);
  const double v1 = oy + ct * (y1 - oy) + st * (x1 - ox);
  
  const double u2 = ox + ct * (x2 - ox) - st * (y2 - oy);
  const double v2 = oy + ct * (y2 - oy) + st * (x2 - ox);
  
  draw_line<image_t>(image, color, u1, v1, u2, v2);
}


#endif // DRAW_HXX_
