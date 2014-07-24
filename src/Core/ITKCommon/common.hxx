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

// File         : common.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2005/03/24 16:53
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for mosaicing, image warping,
//                image preprocessing, convenience wrappers for
//                ITK file and ITK filters.

#ifndef COMMON_HXX_
#define COMMON_HXX_

// local includes:
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/ThreadUtils/itk_terminator.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>
#include <Core/ITKCommon/ThreadUtils/the_transaction.hxx>
#include <Core/ITKCommon/ThreadUtils/the_thread_pool.hxx>
#include <Core/ITKCommon/Filtering/itkNormalizeImageFilterWithMask.h>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>

// boost:
#include <boost/filesystem.hpp>

// system includes:
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <list>
#include <limits>
#include <string>


// common:
#include <itkSimpleFilterWatcher.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkShrinkImageFilter.h>
#include <itkMedianImageFilter.h>
#include <itkPasteImageFilter.h>

// image metrics:
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>

// image interpolators:
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>

// registration:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>

// transforms:
#include <itkTransformBase.h>
#include <itkTranslationTransform.h>
#include <itkScaleTransform.h>

// mosaic:
#include <itkNumericTraits.h>
#include <itkAddImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkDivideImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkRGBPixel.h>
#include <itkComposeRGBImageFilter.h>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

namespace bfs=boost::filesystem;

//----------------------------------------------------------------
// image_size_value_t
// 
typedef itk::Size<2>::SizeValueType image_size_value_t;

//----------------------------------------------------------------
// array2d
// 
#define array2d( T ) std::vector<std::vector<T> >

//----------------------------------------------------------------
// mask_t
// 
// 2D image with unsigned char pixel type.
// 
typedef itk::Image<unsigned char, 2> mask_t;

//----------------------------------------------------------------
// mask_so_t
// 
// Spatial object for mask_t, used to specify a mask to ITK image
// filters.
// 
typedef itk::ImageMaskSpatialObject<2> mask_so_t;

//----------------------------------------------------------------
// mask_so
// 
// Convenience function for setting up a mask spatial object for
// a give mask image.
// 
inline mask_so_t::Pointer
mask_so(const mask_t * mask)
{
  mask_so_t::Pointer so = mask_so_t::New();
  so->SetImage(mask);
  return so;
}

//----------------------------------------------------------------
// BREAK
// 
// This is for debugging. Insert a call to BREAK somewhere in
// your code, recompile. Then load into a debugger and put a break
// point in BREAK. This is to be used in those cases when you know
// exactly when you want to hit the breakpoint.
// 
extern int BREAK(unsigned int i);

//----------------------------------------------------------------
// native_pixel_t
// 
// Native refers to the usual 8 bit pixels here. These are native
// (standard) in the real world, but may look odd in the medical
// imaging world where float or short int are more common.
// 
typedef unsigned char native_pixel_t;

//----------------------------------------------------------------
// native_image_t
//
// 8-bit grayscale image.
// 
typedef itk::Image<native_pixel_t, 2> native_image_t;

//----------------------------------------------------------------
// pixel_t
//
// All-encompasing pixel type -- float.
// Works everywhere, takes up 4 times as much memory as 8-bit pixels.
// 
typedef float pixel_t;

//----------------------------------------------------------------
// std_tile
// 
// Load a mosaic tile image, reset the origin to zero, set pixel
// spacing as specified. Downscale the image according to the
// shrink factor.
// 
template <typename T>
typename T::Pointer
std_tile(const bfs::path& fn_image,
         const unsigned int & shrink_factor,
         const double & pixel_spacing,
         bool blab = true);

//----------------------------------------------------------------
// image_t
// 
// float grayscale image.
// 
typedef itk::Image<pixel_t, 2> image_t;

//----------------------------------------------------------------
// base_transform_t
// 
// Shorthand for abstract 2D tranforms.
// 
typedef itk::Transform<double, 2, 2> base_transform_t;

//----------------------------------------------------------------
// identity_transform_t
// 
// Shorthand for 2D identity ITK transform.
// 
typedef itk::IdentityTransform<double, 2> identity_transform_t;

//----------------------------------------------------------------
// translate_transform_t
// 
// Shorthand for 2D rigid translation ITK transform.
// 
typedef itk::TranslationTransform<double, 2> translate_transform_t;

//----------------------------------------------------------------
// pnt2d_t
// 
// Shorthand for 2D points.
// 
typedef itk::Point<double, 2> pnt2d_t;

//----------------------------------------------------------------
// vec2d_t
// 
// Shorthand for 2D vectors.
// 
typedef itk::Vector<double, 2> vec2d_t;

//----------------------------------------------------------------
// xyz_t
//
// Shorthand for 3D points. This is typically used to represent RGB
// or HSV colors.
// 
typedef itk::Vector<double, 3> xyz_t;

//----------------------------------------------------------------
// xyz
//
// Constructor function for xyz_t.
// 
inline static xyz_t
xyz(const double & r, const double & g, const double & b)
{
  xyz_t rgb;
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
  return rgb;
}

//----------------------------------------------------------------
// hsv_to_rgb
// 
// Convenience function for converting between HSV/RGB color
// spaces. This is used for colormapping.
// 
inline static xyz_t
hsv_to_rgb(const xyz_t & HSV)
{
  double H = HSV[0];
  double S = HSV[1];
  double V = HSV[2];
  
  xyz_t RGB;
  double & R = RGB[0];
  double & G = RGB[1];
  double & B = RGB[2];
  
  if (S == 0.0)
  {
    // monochromatic:
    R = V;
    G = V;
    B = V;
    return RGB;
  }
  
  H *= 6.0;
  double i = floor(H);
  double f = H - i;
  
  double p = V * (1.0 - S);
  double q = V * (1.0 - S * f);
  double t = V * (1.0 - S * (1 - f));
  
  if (i == 0.0)
  {
    R = V;
    G = t;
    B = p;
  }
  else if (i == 1.0)
  {
    R = q;
    G = V;
    B = p;
  }
  else if (i == 2.0)
  {
    R = p;
    G = V;
    B = t;
  }
  else if (i == 3.0)
  {
    R = p;
    G = q;
    B = V;
  }
  else if (i == 4.0)
  {
    R = t;
    G = p;
    B = V;
  }
  else
  { 
    // i == 5.0
    R = V;
    G = p;
    B = q;
  }
  
  return RGB;
}

//----------------------------------------------------------------
// rgb_to_hsv
// 
// Convenience function for converting between RGB/HSV color
// spaces. This is used for colormapping.
// 
inline static xyz_t
rgb_to_hsv(const xyz_t & RGB)
{
  double R = RGB[0];
  double G = RGB[1];
  double B = RGB[2];
  
  xyz_t HSV;
  double & H = HSV[0];
  double & S = HSV[1];
  double & V = HSV[2];
  
  double min = std::min(R, std::min(G, B));
  double max = std::max(R, std::max(G, B));
  V = max;
  
  double delta = max - min;
  if (max == 0)
  { 
    S = 0;
    H = -1;
  }
  else
  {
    S = delta / max;
    
    if (delta == 0)
    {
      delta = 1;
    }
    
    if (R == max)
    {
      // between yellow & magenta
      H = (G - B) / delta;
    }
    else if (G == max)
    { 
      // between cyan & yellow
      H = (B - R) / delta + 2;
    }
    else
    {
      // between magenta & cyan
      H = (R - G) / delta + 4;
    }
    
    H /= 6.0;
    
    if (H < 0.0)
    { 
      H = H + 1.0;
    }
  }
  
  return HSV;
}

//----------------------------------------------------------------
// pnt2d
// 
// Constructor function for pnt2d_t.
// 
inline static const pnt2d_t
pnt2d(const double & x, const double & y)
{
  pnt2d_t pt;
  pt[0] = x;
  pt[1] = y;
  return pt;
}

//----------------------------------------------------------------
// vec2d
// 
// Constructor function for vec2d_t.
// 
inline static const vec2d_t
vec2d(const double & x, const double & y)
{
  vec2d_t vc;
  vc[0] = x;
  vc[1] = y;
  return vc;
}

//----------------------------------------------------------------
// add
// 
// Arithmetics helper function.
// 
inline static const pnt2d_t
add(const pnt2d_t & pt, const vec2d_t & vc)
{
  pnt2d_t out;
  out[0] = pt[0] + vc[0];
  out[1] = pt[1] + vc[1];
  return out;
}

//----------------------------------------------------------------
// add
// 
// Arithmetics helper function.
// 
inline static const vec2d_t
add(const vec2d_t & a, const vec2d_t & b)
{
  vec2d_t out;
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  return out;
}

//----------------------------------------------------------------
// sub
//
// return (a - b)
//
inline static const vec2d_t
sub(const pnt2d_t & a, const pnt2d_t & b)
{
  vec2d_t out;
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  return out;
}

//----------------------------------------------------------------
// mul
// 
// Arithmetics helper function.
// 
inline static const vec2d_t
mul(const double & s, const vec2d_t & vc)
{
  vec2d_t out;
  out[0] = s * vc[0];
  out[1] = s * vc[1];
  return out;
}

//----------------------------------------------------------------
// neg
// 
// Arithmetics helper function.
// 
inline static const vec2d_t
neg(const vec2d_t & v)
{
  return vec2d(-v[0], -v[1]);
}

//----------------------------------------------------------------
// is_empty_bbox
// 
// Test whether a bounding box is empty (min > max)
// 
extern bool
is_empty_bbox(const pnt2d_t & min,
              const pnt2d_t & max);

//----------------------------------------------------------------
// is_singular_bbox
// 
// Test whether a bounding box is singular (min == max)
// 
extern bool
is_singular_bbox(const pnt2d_t & min,
                 const pnt2d_t & max);


//----------------------------------------------------------------
// calc_tile_mosaic_bbox
//
// Calculate the warped image bounding box in the mosaic space.
//
extern bool
calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
                      
                      // image space bounding boxes of the tile:
                      const pnt2d_t & tile_min,
                      const pnt2d_t & tile_max,
                      
                      // mosaic space bounding boxes of the tile:
                      pnt2d_t & mosaic_min,
                      pnt2d_t & mosaic_max,
                      
                      // sample points along the image edges:
                      const unsigned int np = 15);



//----------------------------------------------------------------
// calc_tile_mosaic_bbox
//
// Calculate the warped image bounding box in the mosaic space.
//
template <typename T>
bool
calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
                      const T * tile,
                      
                      // mosaic space bounding boxes of the tile:
                      pnt2d_t & mosaic_min,
                      pnt2d_t & mosaic_max,
                      
                      // sample points along the image edges:
                      const unsigned int np = 15)
{
  typename T::SizeType sz = tile->GetLargestPossibleRegion().GetSize();
  typename T::SpacingType sp = tile->GetSpacing();
  
  pnt2d_t tile_min = tile->GetOrigin();
  pnt2d_t tile_max;
  tile_max[0] = tile_min[0] + sp[0] * static_cast<double>(sz[0]);
  tile_max[1] = tile_min[1] + sp[1] * static_cast<double>(sz[1]);
  
  return calc_tile_mosaic_bbox(mosaic_to_tile,
                               tile_min,
                               tile_max,
                               mosaic_min,
                               mosaic_max,
                               np);
}


//----------------------------------------------------------------
// calc_image_bbox
//
// Calculate the bounding box for a given image,
// expressed in image space.
//
template <typename T>
void
calc_image_bbox(const T * image, pnt2d_t & bbox_min, pnt2d_t & bbox_max)
{
  typename T::SizeType sz = image->GetLargestPossibleRegion().GetSize();
  typename T::SpacingType sp = image->GetSpacing();
  bbox_min = image->GetOrigin();
  bbox_max[0] = bbox_min[0] + sp[0] * static_cast<double>(sz[0]);
  bbox_max[1] = bbox_min[1] + sp[1] * static_cast<double>(sz[1]);
}


//----------------------------------------------------------------
// clamp_bbox
// 
// Restrict a bounding box to be within given limits.
// 
extern void
clamp_bbox(const pnt2d_t & confines_min,
           const pnt2d_t & confines_max,
           pnt2d_t & min,
           pnt2d_t & max);

//----------------------------------------------------------------
// update_bbox
// 
// Expand the bounding box to include a given point.
// 
inline static void
update_bbox(pnt2d_t & min, pnt2d_t & max, const pnt2d_t & pt)
{
  if (min[0] > pt[0]) min[0] = pt[0];
  if (min[1] > pt[1]) min[1] = pt[1];
  if (max[0] < pt[0]) max[0] = pt[0];
  if (max[1] < pt[1]) max[1] = pt[1];
}


//----------------------------------------------------------------
// suspend_itk_multithreading_t
// 
class suspend_itk_multithreading_t
{
public:
  suspend_itk_multithreading_t():
  itk_threads_(itk::MultiThreader::GetGlobalDefaultNumberOfThreads()),
  max_threads_(itk::MultiThreader::GetGlobalMaximumNumberOfThreads())
  {
    // turn off ITK multithreading:
    itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1);
    itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1);
  }
  
  ~suspend_itk_multithreading_t()
  {
    // restore ITK multithreading:
    itk::MultiThreader::SetGlobalMaximumNumberOfThreads(max_threads_);
    itk::MultiThreader::SetGlobalDefaultNumberOfThreads(itk_threads_);
  }
  
private:
  int itk_threads_;
  int max_threads_;
};


//----------------------------------------------------------------
// cast
// 
// Return a copy of image T0 cast to T1.
// 
template <class T0, class T1>
typename T1::Pointer
cast(const T0 * a)
{
  typedef typename itk::CastImageFilter<T0, T1> cast_t;
  typename cast_t::Pointer filter = cast_t::New();
  filter->SetInput(a);
  
  // put a terminator on the filter:
  WRAP(terminator_t<cast_t> terminator(filter));
  filter->Update();
  return filter->GetOutput();
}

//----------------------------------------------------------------
// make_image
// 
// make an image of the requested size, filled with some
// value (by default zero):
// 
template <class image_t>
typename image_t::Pointer
make_image(const typename image_t::RegionType::SizeType & sz,
           const typename image_t::PixelType & fill_value =
           itk::NumericTraits<typename image_t::PixelType>::Zero)
{
  typename image_t::Pointer image = image_t::New();
  image->SetRegions(sz);
  image->Allocate();
  image->FillBuffer(fill_value);
  return image;
}

//----------------------------------------------------------------
// make_image
//
// make an image of the requested size, filled with some
// value (by default zero):
// 
template <class image_t>
typename image_t::Pointer
make_image(const unsigned int width,
           const unsigned int height,
           const double spacing,
           typename image_t::PixelType fill_value =
           itk::NumericTraits<typename image_t::PixelType>::Zero)
{
  typename image_t::SizeType sz;
  sz[0] = width;
  sz[1] = height;
  
  typename image_t::Pointer image = make_image<image_t>(sz, fill_value);
  typename image_t::SpacingType sp;
  sp[0] = spacing;
  sp[1] = spacing;
  image->SetSpacing(sp);
  
  return image;
}

//----------------------------------------------------------------
// make_image
// 
// make an image of the requested size, filled with some
// value (by default zero):
// 
template <class image_t>
typename image_t::Pointer
make_image(const unsigned int width,
           const unsigned int height,
           const double spacing,
           typename image_t::PointType origin,
           typename image_t::PixelType fill_value =
           itk::NumericTraits<typename image_t::PixelType>::Zero)
{
  typename image_t::Pointer image =
  make_image<image_t>(width, height, spacing, fill_value);
  image->SetOrigin(origin);
  return image;
}

//----------------------------------------------------------------
// make_image
// 
// make an image of the requested size, filled with some
// value (by default zero):
// 
template <class image_t>
typename image_t::Pointer
make_image(const typename image_t::SpacingType & sp,
           const typename image_t::RegionType::SizeType & sz,
           const typename image_t::PixelType & fill_value =
           itk::NumericTraits<typename image_t::PixelType>::Zero)
{
  typename image_t::Pointer image = image_t::New();
  image->SetRegions(sz);
  image->SetSpacing(sp);
  image->Allocate();
  image->FillBuffer(fill_value);
  return image;
}

//----------------------------------------------------------------
// make_image
// 
// make an image of the requested size, filled with some
// value (by default zero):
// 
template <class image_t>
typename image_t::Pointer
make_image(const typename image_t::PointType & origin,
           const typename image_t::SpacingType & sp,
           const typename image_t::RegionType::SizeType & sz,
           const typename image_t::PixelType & fill_value =
           itk::NumericTraits<typename image_t::PixelType>::Zero)
{
  typename image_t::Pointer image = image_t::New();
  image->SetOrigin(origin);
  image->SetRegions(sz);
  image->SetSpacing(sp);
  image->Allocate();
  image->FillBuffer(fill_value);
  return image;
}

//----------------------------------------------------------------
// add
//
// image arithmetic -- add two images together:
// 
template <class image_t>
typename image_t::Pointer
add(const image_t * a, const image_t * b)
{
  typedef typename itk::AddImageFilter<image_t, image_t, image_t> sum_t;
  
  typename sum_t::Pointer sum = sum_t::New();
  sum->SetInput1(a);
  sum->SetInput2(b);
  
  WRAP(terminator_t<sum_t> terminator(sum));
  sum->Update();
  return sum->GetOutput();
}

//----------------------------------------------------------------
// subtract
//
// image arithmetic -- subtract two images: c = a - b
// 
template <class image_t>
typename image_t::Pointer
subtract(const image_t * a, const image_t * b)
{
  typedef typename itk::SubtractImageFilter<image_t, image_t, image_t> dif_t;
  
  typename dif_t::Pointer dif = dif_t::New();
  dif->SetInput1(a);
  dif->SetInput2(b);
  
  WRAP(terminator_t<dif_t> terminator(dif));
  dif->Update();
  return dif->GetOutput();
}

//----------------------------------------------------------------
// multiply
// 
// image arithmetic -- return an image resulting from
// pixel-wise division a/b:
// 
template <class image_t>
typename image_t::Pointer
multiply(const image_t * a, const image_t * b)
{
  typedef typename itk::MultiplyImageFilter<image_t, image_t, image_t> mul_t;
  
  typename mul_t::Pointer mul = mul_t::New();
  mul->SetInput1(a);
  mul->SetInput2(b);
  
  WRAP(terminator_t<mul_t> terminator(mul));
  mul->Update();
  return mul->GetOutput();
}

//----------------------------------------------------------------
// divide
// 
// image arithmetic -- return an image resulting from
// pixel-wise division a/b:
// 
template <class image_t, class mask_t>
typename image_t::Pointer
divide(const image_t * a, const mask_t * b)
{
  typedef typename itk::DivideImageFilter<image_t, mask_t, image_t> div_t;
  
  typename div_t::Pointer div = div_t::New();
  div->SetInput1(a);
  div->SetInput2(b);
  
  WRAP(terminator_t<div_t> terminator(div));
  div->Update();
  return div->GetOutput();
}

//----------------------------------------------------------------
// pixel_in_mask
// 
// Check whether a given pixel falls inside a mask.
// 
// NOTE: the mask is assumed to be at higher resolution
//       than the image.
// 
template <class image_t>
inline static bool
pixel_in_mask(const mask_t * mask,
              const mask_t::SizeType & mask_size,
              const typename image_t::IndexType & index,
              const unsigned int spacing_scale)
{
  mask_t::IndexType mask_index(index);
  mask_index[0] *= spacing_scale;
  mask_index[1] *= spacing_scale;
  
  if (mask_index[0] >= 0 &&
      mask_index[1] >= 0 &&
      image_size_value_t(mask_index[0]) < mask_size[0] &&
      image_size_value_t(mask_index[1]) < mask_size[1])
  {
    // check the mask:
    return (mask == NULL) ? true : (mask->GetPixel(mask_index) != 0);
  }
  
  // pixel falls outside the mask image:
  return false;
}

//----------------------------------------------------------------
// pixel_in_mask
// 
// Check whether a given physical point falls inside a mask.
// 
template <typename T>
inline static bool
pixel_in_mask(const T * mask,
              const typename T::PointType & physical_point)
{
  if (mask == NULL) return true;
  
  typename T::IndexType mask_pixel_index;
  if (mask->TransformPhysicalPointToIndex(physical_point, mask_pixel_index))
  {
    // let the mask decide:
    return mask->GetPixel(mask_pixel_index) != 0;
  }
  
  // point falls outside the mask image:
  return false;
}

//----------------------------------------------------------------
// pixel_in_mask
// 
// Check whether a given pixel falls inside a mask.
// 
template <typename T>
inline static bool
pixel_in_mask(const T * image,
              const itk::Image<unsigned char, T::ImageDimension> * mask,
              const typename T::IndexType & image_pixel_index)
{
  if (mask == NULL)
  {
    return true;
  }
  
  typename T::PointType physical_point;
  image->TransformIndexToPhysicalPoint(image_pixel_index, physical_point);
  
  typedef itk::Image<unsigned char, T::ImageDimension> mask_t;
  return pixel_in_mask<mask_t>(mask, physical_point);
}

//----------------------------------------------------------------
// image_min_max
// 
// Find min/max pixel values, return mean pixel value (average):
// 
template <typename T>
double
image_min_max(const T * a,
              typename T::PixelType & min,
              typename T::PixelType & max,
              const itk::Image<unsigned char, T::ImageDimension> * mask = NULL)
{
  WRAP(itk_terminator_t terminator("image_min_max"));
  
  typedef typename T::PixelType pixel_t;
  typedef typename itk::ImageRegionConstIteratorWithIndex<T> iter_t;
  
  min = std::numeric_limits<pixel_t>::max();
  max = -min;
  
  double mean = 0.0;
  double counter = 0.0;
  iter_t iter(a, a->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    if (!pixel_in_mask<T>(a, mask, iter.GetIndex()))
    {
      continue;
    }
    
    pixel_t v = iter.Get();
    min = std::min(min, v);
    max = std::max(max, v);
    
    mean += static_cast<double>(v);
    counter += 1.0;
  }
  
  mean /= counter;
  return mean;
}

//----------------------------------------------------------------
// remap_min_max_inplace
//
// Rescale image intensities in-place
// 
template <class image_t>
bool
remap_min_max_inplace(image_t * a,
                      double min,
                      double max,
                      double new_min,
                      double new_max)
{
  WRAP(itk_terminator_t terminator("remap_min_max_inplace"));
  
  typedef typename image_t::PixelType pixel_t;
  typedef typename itk::ImageRegionIterator<image_t> iter_t;
  
  // rescale the intensities:
  double rng = max - min;
  if (rng == 0.0) return false;
  
  double new_rng = new_max - new_min;
  
  iter_t iter(a, a->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    double t = (iter.Get() - min) / rng;
    iter.Set(pixel_t(new_min + t * new_rng));
  }
  
  return true;
}

//----------------------------------------------------------------
// remap_min_max_inplace
//
// Rescale image intensities in-place
// 
template <class image_t>
bool
remap_min_max_inplace(image_t * a,
                      typename image_t::PixelType new_min = 0,
                      typename image_t::PixelType new_max = 255)
{
  WRAP(itk_terminator_t terminator("remap_min_max_inplace"));
  
  typedef typename image_t::PixelType pixel_t;
  typedef typename itk::ImageRegionIterator<image_t> iter_t;
  
  double min = std::numeric_limits<pixel_t>::max();
  double max = -min;
  
  iter_t iter(a, a->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    double v = iter.Get();
    min = std::min(min, v);
    max = std::max(max, v);
  }
  
  return remap_min_max_inplace<image_t>(a, min, max, new_min, new_max);
}

//----------------------------------------------------------------
// remap_min_max
//
// Return a copy of the image with rescaled pixel intensities
// 
template <typename T>
typename T::Pointer
remap_min_max(const T * a,
              typename T::PixelType new_min = 0,
              typename T::PixelType new_max = 255)
{
  WRAP(itk_terminator_t terminator("remap_min_max"));
  
  typedef typename T::RegionType rn_t;
  typedef typename T::SizeType sz_t;
  rn_t rn = a->GetLargestPossibleRegion();
  sz_t sz = rn.GetSize();
  
  typename T::Pointer b = T::New();
  b->SetRegions(sz);
  b->SetSpacing(a->GetSpacing());
  b->Allocate();
  
  double min = std::numeric_limits<pixel_t>::max();
  double max = -min;
  
  typename T::IndexType ix_end;
  ix_end[0] = sz[0];
  ix_end[1] = sz[1];
  
  typename T::IndexType ix;
  for (ix[1] = 0; ix[1] < ix_end[1]; ++ix[1])
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    for (ix[0] = 0; ix[0] < ix_end[0]; ++ix[0])
    {
      double v = a->GetPixel(ix);
      min = std::min(min, v);
      max = std::max(max, v);
    }
  }
  
  // rescale the intensities:
  double rng = max - min;
  if (rng == 0.0)
  {
    b->FillBuffer(itk::NumericTraits<typename T::PixelType>::Zero);
    return b;
  }
  
  double new_rng = new_max - new_min;
  
  for (ix[1] = 0; ix[1] < ix_end[1]; ++ix[1])
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    for (ix[0] = 0; ix[0] < ix_end[0]; ++ix[0])
    {
      double v = a->GetPixel(ix);
      double t = (v - min) / rng;
      b->SetPixel(ix, typename T::PixelType(new_min + t * new_rng));
    }
  }
  
  return b;
}

//----------------------------------------------------------------
// invert
//
// Invert pixel intensities in-place.
// 
template <class T>
void
invert(typename T::Pointer & a)
{
  WRAP(itk_terminator_t terminator("invert"));
  
  typedef typename T::PixelType pixel_t;
  typedef typename itk::ImageRegionIterator<T> iter_t;
  
  pixel_t min = std::numeric_limits<pixel_t>::max();
  pixel_t max = -min;
  
  iter_t iter(a, a->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    pixel_t v = iter.Get();
    min = std::min(min, v);
    max = std::max(max, v);
  }
  
  pixel_t rng = max - min;
  if (rng == pixel_t(0)) return;
  
  // rescale the value:
  pixel_t new_min = max;
  pixel_t new_max = min;
  pixel_t new_rng = new_max - new_min;
  
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    pixel_t t = (iter.Get() - min) / rng;
    iter.Set(new_min + t * new_rng);
  }
}

//----------------------------------------------------------------
// threshold
//
// Return a copy of the image with pixels above and below
// a given threshold remapped to new values.
// 
template <typename T>
typename T::Pointer
threshold(const T * a,
          typename T::PixelType min,
          typename T::PixelType max,
          typename T::PixelType new_min,
          typename T::PixelType new_max)
{
  WRAP(itk_terminator_t terminator("threshold"));
  
  typename T::RegionType::SizeType sz = a->GetLargestPossibleRegion().GetSize();
  typename T::Pointer b = T::New();
  b->SetRegions(sz);
  b->SetSpacing(a->GetSpacing());
  b->Allocate();
  
  typedef typename itk::ImageRegionConstIteratorWithIndex<T> itex_t;
  itex_t itex(a, sz);
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    typename T::PixelType v = itex.Get();
    if (v < min) v = new_min;
    else if (v > max) v = new_max;
    
    b->SetPixel(itex.GetIndex(), v);
  }
  
  return b;
}

//----------------------------------------------------------------
// clip_min_max
// 
// Clip the pixel intensities to the specified min/max limits.
// 
template <typename T>
typename T::Pointer
clip_min_max(const T * a,
             typename T::PixelType min = 0,
             typename T::PixelType max = 255)
{ return threshold<T>(a, min, max, min, max); }

//----------------------------------------------------------------
// calc_padding
// 
// Given two 2D images, return the pixel bounding box encompasing both.
// 
template <typename T>
typename T::SizeType
calc_padding(const T * a, const T * b)
{
  typedef typename T::SizeType sz_t;
  sz_t sa = a->GetLargestPossibleRegion().GetSize();
  const sz_t sb = b->GetLargestPossibleRegion().GetSize();
  
  const unsigned int d = T::GetImageDimension();
  for (unsigned int i = 0; i < d; i++)
  {
    sa[i] = std::max(sa[i], sb[i]);
  }
  
  return sa;
}

//----------------------------------------------------------------
// pad
// 
// Pad an image to a given size (in pixels), return the padded image.
// The image is padded with zeros.
// 
template <typename T>
typename T::Pointer
pad(const T * a,
    const typename T::SizeType & size)
{
  typedef typename T::RegionType rn_t;
  typedef typename T::SizeType sz_t;
  rn_t r = a->GetLargestPossibleRegion();
  sz_t z = r.GetSize();
  
  WRAP(itk_terminator_t terminator("pad"));
  
  typename T::Pointer b = T::New();
  b->SetRegions(size);
  b->SetSpacing(a->GetSpacing());
  b->Allocate();
  
  typename T::PixelType zero = itk::NumericTraits<typename T::PixelType>::Zero;
  typename T::IndexType ix;
  for (ix[1] = 0; (image_size_value_t)(ix[1]) < z[1]; ++ix[1])
  {
    for (ix[0] = 0; (image_size_value_t)(ix[0]) < z[0]; ++ix[0])
    {
      b->SetPixel(ix, a->GetPixel(ix));
    }
    
    for (ix[0] = z[0]; (image_size_value_t)(ix[0]) < size[0]; ++ix[0])
    {
      b->SetPixel(ix, zero);
    }
  }
  
  for (ix[1] = z[1]; (image_size_value_t)(ix[1]) < size[1]; ++ix[1])
  {
    for (ix[0] = 0; (image_size_value_t)(ix[0]) < size[0]; ++ix[0])
    {
      b->SetPixel(ix, zero);
    }
  }
  
  return b;
}

//----------------------------------------------------------------
// crop
// 
// Return a copy of the cropped image region
// 
template <typename T>
typename T::Pointer
crop(const T * image,
     const typename T::IndexType & min,
     const typename T::IndexType & max)
{
  WRAP(itk_terminator_t terminator("crop"));
  typedef typename T::RegionType::SizeType sz_t;
  
  sz_t img_sz = image->GetLargestPossibleRegion().GetSize();
  sz_t reg_sz;
  const unsigned int d = T::GetImageDimension();
  for (unsigned int i = 0; i < d; i++)
  {
    reg_sz[i] = std::min(static_cast<unsigned int>(max[i] - min[i] + 1),
                         static_cast<unsigned int>(img_sz[i] - min[i]));
  }
  
  typename T::RegionType region;
  region.SetIndex(min);
  region.SetSize(reg_sz);
  
  typename T::Pointer out = make_image<T>(reg_sz);
  
  typedef itk::ImageRegionIteratorWithIndex<T> itex_t;
  itex_t itex(out, out->GetLargestPossibleRegion());
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    typename T::IndexType ix1 = itex.GetIndex();
    typename T::IndexType ix0;
    for (unsigned int i = 0; i < d; i++)
    {
      ix0[i] = ix1[i] + min[i];
    }
    
    out->SetPixel(ix1, image->GetPixel(ix0));
  }
  
  // FIXME: do I really want this?
  {
    typename T::PointType origin;
    image->TransformIndexToPhysicalPoint(region.GetIndex(), origin);
    out->SetOrigin(origin);
  }
  out->SetSpacing(image->GetSpacing());
  
  return out;
}

//----------------------------------------------------------------
// smooth
// 
// Return a copy of the image smothed with a Gaussian kernel.
// 
template <typename T>
typename T::Pointer
smooth(const T * in,
       const double sigma,
       const double maximum_error = 0.1)
{
  typedef typename itk::DiscreteGaussianImageFilter<T, T> smoother_t;
  typename smoother_t::Pointer smoother = smoother_t::New();
  
  // FIXME: itk::SimpleFilterWatcher w(smoother.GetPointer(), "smoother");
  
  smoother->SetInput(in);
  smoother->SetUseImageSpacing(false);
  smoother->SetVariance(sigma * sigma);
  smoother->SetMaximumError(maximum_error);
  
  WRAP(terminator_t<smoother_t> terminator(smoother));
  smoother->Update();
  return smoother->GetOutput();
}

//----------------------------------------------------------------
// shrink
// 
// Return a scaled down copy of the image.
// 
template <typename T>
typename T::Pointer
shrink(const T * in,
       const unsigned int & shrink_factor,
       const double maximum_error = 0.1,
       const bool & antialias = true)
{
  // Create caster, smoother and shrinker filters
  typedef itk::Image<float, T::ImageDimension> flt_img_t;
  typename flt_img_t::Pointer image = cast<T, flt_img_t>(in);
  
  if (antialias)
  {
    double variance = static_cast<double>(shrink_factor) / 2.0;
    double sigma = sqrt(variance);
    image = ::smooth<flt_img_t>(image, sigma, maximum_error);
  }
  
  typedef itk::ShrinkImageFilter<flt_img_t, flt_img_t> shrinker_t;
  typename shrinker_t::Pointer shrinker = shrinker_t::New();
  
  // FIXME: itk::SimpleFilterWatcher w(shrinker.GetPointer(), "shrinker");
  
  shrinker->SetInput(image);
  shrinker->SetShrinkFactors(shrink_factor);
  
  WRAP(terminator_t<shrinker_t> terminator(shrinker));
  shrinker->Update();
  image = shrinker->GetOutput();
  return cast<flt_img_t, T>(image);
}

//----------------------------------------------------------------
// load
// 
// Convenience functions for loading an ITK image.
// 
template <typename T>
typename T::Pointer
load(const bfs::path& filename, bool blab = true)
{
  typedef typename itk::ImageFileReader<T> reader_t;
  typename reader_t::Pointer reader = reader_t::New();
  
  // FIXME: itk::SimpleFilterWatcher w(reader.GetPointer(), "reader");
  
  reader->SetFileName(filename.c_str());
  if (blab) std::cout << "loading " << filename.string() << std::endl;
  
//  WRAP(terminator_t<reader_t> terminator(reader));
  
  reader->Update();
  return reader->GetOutput();
}

//----------------------------------------------------------------
// save
// 
// Convenience functions for saving an ITK image.
// 
template <typename T>
void
save(const T * image, const bfs::path & filename, bool blab = true)
{
  typedef typename itk::ImageFileWriter<T> writer_t;
  typename writer_t::Pointer writer = writer_t::New();
  writer->SetInput(image);
  
  if (blab) std::cout << "saving " << filename << std::endl;
  writer->SetFileName(filename.string().c_str());
  writer->Update();
}

//----------------------------------------------------------------
// save_image_tile
// 
// Convenience functions for saving out a section of an ITK image.
// 
template <typename T>
void
save_image_tile(T * image, 
                const bfs::path & filename, 
                const unsigned int x, 
                const unsigned int y, 
                const unsigned int source_width, 
                const unsigned int source_height, 
                const unsigned int tile_width, 
                const unsigned int tile_height,
                bool blab = true)
{
  typename itk::PasteImageFilter<T>::Pointer pasteImageFilter =
  itk::PasteImageFilter<T>::New();
  
  pasteImageFilter->SetSourceImage( image );
  
  typename T::RegionType mosaicRegion = image->GetBufferedRegion();
  typename T::SizeType mosaicSize = mosaicRegion.GetSize();
  
  // Setup the region we're interested in pasting to a new image.
  typename T::IndexType sourceIndex;
  sourceIndex[0] = x;
  sourceIndex[1] = y;
  
  typename T::SizeType sourceSize;
  sourceSize[0] = source_width;
  sourceSize[1] = source_height;
  
  typename T::RegionType sourceRegion;
  sourceRegion.SetIndex( sourceIndex );
  sourceRegion.SetSize( sourceSize );
  pasteImageFilter->SetSourceRegion( sourceRegion );
  
  // Create a new image to paste the section onto.
  typename T::Pointer destImage = T::New();
  pasteImageFilter->SetDestinationImage( destImage );
  
  typename T::IndexType destIndex;
  destIndex[0] = 0;
  destIndex[1] = 0;
  
  typename T::SizeType destSize;
  destSize[0] = tile_width;
  destSize[1] = tile_height;
  
  typename T::RegionType sectionRegion;
  sectionRegion.SetIndex( destIndex );
  sectionRegion.SetSize( destSize );
  destImage->SetRegions( sectionRegion );
  destImage->Allocate();
  destImage->FillBuffer(0);
  
  pasteImageFilter->SetDestinationIndex( destIndex );
  
  typename T::Pointer partialImage = pasteImageFilter->GetOutput();
  
  typedef typename itk::ImageFileWriter<T> writer_t;
  typename writer_t::Pointer writer = writer_t::New();
  writer->SetInput(partialImage);
  
  if (blab)
  {
    std::cout << "saving " << filename << std::endl;
  }
  
  writer->SetFileName(filename.c_str());
  writer->Update();
}

//----------------------------------------------------------------
// save_as_tiles
// 
// Convenience functions for saving out series of tiles as ITK images.  Also
// writes out an xml file explaining the position of these files.
// 
template <typename T>
bool
save_as_tiles(T * image, 
              const bfs::path & prefix, 
              const bfs::path & extension, 
              unsigned int w, 
              unsigned int h,
              const double downsample,
              bool save_image = true,
              bool blab = true)
{
  //  the_text_t fileName = prefix;
//  the_text_t xmlFileName = fileName;
//  xmlFileName += ".xml";
//  bfs::path fileName(prefix);
  bfs::path xmlFileName(prefix);
  xmlFileName.replace_extension("xml");
  std::ofstream xmlOut(xmlFileName.string().c_str());
  
  if (! xmlOut.is_open()) 
  {
    std::cout << "Error opening xml file for writing: " << xmlFileName << std::endl;
    return false;
  }
  
  typename T::RegionType mosaicRegion = image->GetBufferedRegion();
  typename T::SizeType mosaicSize = mosaicRegion.GetSize();
  if (w == std::numeric_limits<unsigned int>::max())
  {
    w = mosaicSize[0];
  }
  
  if (h == std::numeric_limits<unsigned int>::max())
  {
    h = mosaicSize[1];
  }
  
  unsigned int numTilesWide = (mosaicSize[0] + (w - 1)) / w;
  unsigned int numTilesTall = (mosaicSize[1] + (h - 1)) / h;
  
  // extract just the name portion
//  size_t num_forward = fileName.contains('/');
//  size_t num_backward = fileName.contains('\\');
//  char slash = ( num_forward > num_backward ) ? '/' : '\\';
//  the_text_t name_part = fileName.reverse().cut(slash, 0, 0).reverse() + "_";
//  bfs::path name_part = fileName.stem() + "_";
  std::string name_part(prefix.stem().string() + "_");
  
  xmlOut << "<?xml version=\"1.0\"?>" << std::endl;
  xmlOut << "<Level GridDimX=\"" << numTilesWide
  << "\" GridDimY=\"" << numTilesTall
  << "\" " << "TileXDim=\"" << w
  << "\" " << "TileYDim=\"" << h
  << "\" " << "Downsample=\"" << downsample
  << "\" " << "FilePrefix=\"" << name_part
  << "\" " << "FilePostfix=\"" << extension
  << "\"/>" << std::endl;
  
  for (unsigned int x = 0, xid = 0; x < mosaicSize[0]; x += w, xid++)
  {
    for (unsigned int y = 0, yid = 0; y < mosaicSize[1]; y += h, yid++)
    {
//      the_text_t fn_partialSave = prefix;
      bfs::path fn_partialSave(prefix);
      fn_partialSave += "_X";
      fn_partialSave += the_text_t::number(xid, 3, '0');
      fn_partialSave += "_Y";
      fn_partialSave += the_text_t::number(yid, 3, '0');
      fn_partialSave += extension;
      
      unsigned int sectionWidth = std::min<unsigned int>(w, mosaicSize[0] - x);
      unsigned int sectionHeight = std::min<unsigned int>(h, mosaicSize[1] - y);
      if ( save_image )
      {
        save_image_tile<T>(image,
                           fn_partialSave,
                           x,
                           y,
                           sectionWidth,
                           sectionHeight,
                           w,
                           h);
      }
    }
  }
  
  xmlOut.close();
  return true;
}

//----------------------------------------------------------------
// save_tile_xml
// 
// Exports the same xml file as save_as_tiles.  But without actually
// saving the tiles out.
// 
template <typename T>
bool
save_tile_xml(const bfs::path & prefix, 
              const bfs::path & extension, 
              unsigned int w, 
              unsigned int h,
              unsigned int full_width,
              unsigned int full_height,
              const double downsample,
              bool save_image = true,
              bool blab = true)
{
  //  the_text_t fileName = prefix;
//  the_text_t xmlFileName = fileName;
//  xmlFileName += ".xml";
//  bfs::path fileName(prefix);
  bfs::path xmlFileName(prefix);
  xmlFileName.replace_extension("xml");
  std::ofstream xmlOut(xmlFileName.c_str());
  
  if (!xmlOut.is_open()) 
  {
    std::cout << "Error opening xml file for writing: " << xmlFileName << std::endl;
    return false;
  }
  
  if (w == std::numeric_limits<unsigned int>::max())
  {
    w = full_width;
  }
  
  if (h == std::numeric_limits<unsigned int>::max())
  {
    h = full_height;
  }
  
  unsigned int numTilesWide = (full_width + (w - 1)) / w;
  unsigned int numTilesTall = (full_height + (h - 1)) / h;
  
  // extract just the name portion
//  size_t num_forward = fileName.contains('/');
//  size_t num_backward = fileName.contains('\\');
//  char slash = ( num_forward > num_backward ) ? '/' : '\\';
//  the_text_t name_part = fileName.reverse().cut(slash, 0, 0).reverse() + "_";
  std::string name_part(prefix.stem().string() + "_");
  
  xmlOut << "<?xml version=\"1.0\"?>" << std::endl;
  xmlOut << "<Level GridDimX=\"" << numTilesWide
  << "\" GridDimY=\"" << numTilesTall
  << "\" " << "TileXDim=\"" << w
  << "\" " << "TileYDim=\"" << h
  << "\" " << "Downsample=\"" << downsample
  << "\" " << "FilePrefix=\"" << name_part
  << "\" " << "FilePostfix=\"" << extension
  << "\"/>" << std::endl;
  
  for (unsigned int x = 0, xid = 0; x < full_width; x += w, xid++)
  {
    for (unsigned int y = 0, yid = 0; y < full_height; y += h, yid++)
    {
//      the_text_t fn_partialSave = prefix;
      bfs::path fn_partialSave = prefix;
      fn_partialSave += "_X";
      fn_partialSave += the_text_t::number(xid, 3, '0');
      fn_partialSave += "_Y";
      fn_partialSave += the_text_t::number(yid, 3, '0');
      fn_partialSave += extension;
    }
  }
  
  xmlOut.close();
  return true;
}

//----------------------------------------------------------------
// calc_area
// 
// Calculate the area covered by a given number of pixels
// at the specified pixels spacing.
// 
inline double
calc_area(const itk::Vector<double, 2> & spacing,
          const unsigned long int pixels)
{
  double pixel_area = spacing[0] * spacing[1];
  double area = pixel_area * static_cast<double>(pixels);
  return area;
}

//----------------------------------------------------------------
// calc_area
// 
// Calculate image area under the mask.
// 
template <typename T>
double
calc_area(const T * image, const mask_t * mask)
{
  WRAP(itk_terminator_t terminator("calc_area"));
  unsigned long int pixels = 0;
  
  typedef itk::ImageRegionConstIteratorWithIndex<T> itex_t;
  itex_t itex(image, image->GetLargestPossibleRegion());
  
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    if (pixel_in_mask<T>(image, mask, itex.GetIndex()))
    {
      pixels++;
    }
  }
  
  return calc_area(image->GetSpacing(), pixels);
}

//----------------------------------------------------------------
// get_area
// 
template <typename T>
double
get_area(const T * image)
{
  const typename T::RegionType::SizeType & sz =
  image->GetLargestPossibleRegion().GetSize();
  const unsigned int num_pixels = sz[0] * sz[1];
  return calc_area(image->GetSpacing(), num_pixels);
}

//----------------------------------------------------------------
// eval_metric
// 
// A wrapper function for evaluating an image registration metric for
// two images (one fixed, one moving) in the overlap region defined
// by the fixed image to moving image transform and image masks.
// 
template <class metric_t, class interpolator_t>
double
eval_metric(const base_transform_t * fi_to_mi,
            const typename metric_t::FixedImageType * fi,
            const typename metric_t::MovingImageType * mi,
            const mask_t * fi_mask = NULL,
            const mask_t * mi_mask = NULL)
{
  typename metric_t::Pointer metric = metric_t::New();
  metric->SetFixedImage(fi);
  metric->SetMovingImage(mi);
  metric->SetTransform(const_cast<base_transform_t *>(fi_to_mi));
  metric->SetInterpolator(interpolator_t::New());
  
  // Instead of iterating over the whole fixed image, find the
  // bounding box of the overlapping region of the fixed and
  // moving images, clip it to the fixed image bounding box, and iterate
  // over that -- this will produce a dramatic speedup in situations
  // where the fixed image is large and the moving image is small:
  typedef typename metric_t::FixedImageType fi_image_t;
  typename fi_image_t::RegionType fi_roi = fi->GetLargestPossibleRegion();
  
  // find the bounding box of the moving image in the space of the fixed image:
  pnt2d_t mi_min;
  pnt2d_t mi_max;
  if (calc_tile_mosaic_bbox(fi_to_mi,
                            mi,
                            mi_min,
                            mi_max,
                            16))
  {
    // find the bounding box of the fixed image:
    pnt2d_t fi_min;
    pnt2d_t fi_max;
    calc_image_bbox(fi, fi_min, fi_max);
    
    // clip the bounding box to the bounding box of the fixed image:
    clamp_bbox(fi_min, fi_max, mi_min, mi_max);
    
    // reset the region of interest:
    typename fi_image_t::IndexType roi_index[2];
    if (fi->TransformPhysicalPointToIndex(mi_min, roi_index[0]) &&
        fi->TransformPhysicalPointToIndex(mi_max, roi_index[1]))
    {
      typename fi_image_t::SizeType roi_size;
      roi_size[0] = roi_index[1][0] - roi_index[0][0] + 1;
      roi_size[1] = roi_index[1][1] - roi_index[0][1] + 1;
      fi_roi.SetIndex(roi_index[0]);
      fi_roi.SetSize(roi_size);
    }
  }
  
  metric->SetFixedImageRegion(fi_roi);
  
  if (fi_mask != NULL)
  {
    metric->SetFixedImageMask(mask_so(fi_mask));
  }
  
  if (mi_mask != NULL)
  {
    metric->SetMovingImageMask(mask_so(mi_mask));
  }
  
  metric->Initialize();
  
  double measure;
  try
  {
    measure = metric->GetValue(fi_to_mi->GetParameters());
  }
  catch (itk::ExceptionObject & exception)
  {
    std::cerr << "image metric threw an exception:" << std::endl
    << exception << std::endl;
    measure = std::numeric_limits<double>::max();
  }
  
  return measure;
}

//----------------------------------------------------------------
// my_metric
// 
// Calculate a normalized cross correlation metric between two
// masked images, calculate the area of overlap.
// 
template <typename TImage, typename TInterpolator>
double
my_metric(double & area,
          
          const TImage * fi,
          const TImage * mi,
          
          const base_transform_t * fi_to_mi,
          const mask_t * fi_mask,
          const mask_t * mi_mask,
          
          const TInterpolator * mi_interpolator)
{
  WRAP(itk_terminator_t terminator("my_metric"));
  typedef typename itk::ImageRegionConstIteratorWithIndex<TImage> itex_t;
  typedef typename TImage::IndexType index_t;
  typedef typename TImage::PointType point_t;
  typedef typename TImage::PixelType pixel_t;
  
  // Instead of iterating over the whole fixed image, find the
  // bounding box of the overlapping region of the fixed and
  // moving images, clip it to the fixed image bounding box, and iterate
  // over that -- this will produce a dramatic speedup in situations
  // where the fixed image is large and the moving image is small:
  typename TImage::RegionType fi_roi = fi->GetLargestPossibleRegion();
  
  // find the bounding box of the moving image in the space of the fixed image:
  pnt2d_t mi_min;
  pnt2d_t mi_max;
  if (calc_tile_mosaic_bbox(fi_to_mi,
                            mi,
                            mi_min,
                            mi_max,
                            16))
  {
    // find the bounding box of the fixed image:
    pnt2d_t fi_min;
    pnt2d_t fi_max;
    calc_image_bbox(fi, fi_min, fi_max);
    
    // clip the bounding box to the bounding box of the fixed image:
    clamp_bbox(fi_min, fi_max, mi_min, mi_max);
    if (is_singular_bbox(mi_min, mi_max))
    {
      return std::numeric_limits<double>::max();
    }
    
    // reset the region of interest:
    index_t roi_index[2];
    if (fi->TransformPhysicalPointToIndex(mi_min, roi_index[0]) &&
        fi->TransformPhysicalPointToIndex(mi_max, roi_index[1]))
    {
      typename TImage::SizeType roi_size;
      roi_size[0] = roi_index[1][0] - roi_index[0][0] + 1;
      roi_size[1] = roi_index[1][1] - roi_index[0][1] + 1;
      fi_roi.SetIndex(roi_index[0]);
      fi_roi.SetSize(roi_size);
    }
  }
  
  // performance shortcuts:
  mask_t::SizeType fi_mask_size = fi->GetLargestPossibleRegion().GetSize();
  unsigned int fi_spacing_scale = 1;
  if (fi_mask)
  {
    mask_t::SizeType sz = fi_mask->GetLargestPossibleRegion().GetSize();
    fi_spacing_scale = sz[0] / fi_mask_size[0];
    fi_mask_size = sz;
  }
  
  // counters:
  double ab = 0.0;
  double aa = 0.0;
  double bb = 0.0;
  double sa = 0.0;
  double sb = 0.0;
  unsigned long int pixels = 0;
  
  // iterate over the image:
  itex_t itex(fi, fi_roi);
  index_t fi_ix;
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    fi_ix = itex.GetIndex();
    if (fi_mask && !pixel_in_mask<TImage>(fi_mask,
                                          fi_mask_size,
                                          fi_ix,
                                          fi_spacing_scale))
    {
      continue;
    }
    
    // point coordinates in the fixed image:
    point_t xy;
    fi->TransformIndexToPhysicalPoint(fi_ix, xy);
    
    // corresponding coordinates in the moving image:
    const point_t uv = fi_to_mi->TransformPoint(xy);
    
    // check whether the point is within the moving image:
    if (!pixel_in_mask<mask_t>(mi_mask, uv) ||
        !mi_interpolator->IsInsideBuffer(uv))
    {
      continue;
    }
    
    pixel_t m = pixel_t(mi_interpolator->Evaluate(uv));
    pixel_t f = itex.Get();
    
    double A = f;
    double B = m;
    
    ab += A * B;
    aa += A * A;
    bb += B * B;
    sa += A;
    sb += B;
    
    pixels++;
  }
  
  area = calc_area(fi->GetSpacing(), pixels);
  if (area == 0)
  {
    return std::numeric_limits<double>::max();
  }
  
  aa = aa - ((sa * sa) / static_cast<double>(pixels));
  bb = bb - ((sb * sb) / static_cast<double>(pixels));
  ab = ab - ((sa * sb) / static_cast<double>(pixels));
  
  double result = -ab / sqrt(aa * bb);
  return result;
}

//----------------------------------------------------------------
// my_metric
// 
// Calculate the normalized cross correlation metric between two
// masked images and the ratio of area of overlap to the area of the
// smaller image. The metric is restricted by the min/max overlap
// ratio thresholds.
// 
template <typename TImage>
double
my_metric(double & overlap,
          const TImage * fi,
          const TImage * mi,
          const base_transform_t * fi_to_mi,
          const mask_t * fi_mask = NULL,
          const mask_t * mi_mask = NULL,
          const double min_overlap = 0.05,
          const double max_overlap = 1.0)
{
  overlap = 0;
  
  typedef itk::NearestNeighborInterpolateImageFunction
  <TImage, double> interpolator_t;
  typename interpolator_t::Pointer mi_interpolator = interpolator_t::New();
  mi_interpolator->SetInputImage(mi);
  
  double overlap_area = 0;
  double m = my_metric<TImage, interpolator_t>(overlap_area,
                                               fi,
                                               mi,
                                               fi_to_mi,
                                               fi_mask,
                                               mi_mask,
                                               mi_interpolator);
  if (overlap_area != 0)
  {
//#if 0
//    // this is very slow, but accurate:
//    double area_a = calc_area(fi, fi_mask);
//    double area_b = calc_area(mi, mi_mask);
//#else
    // this is fast, but approximate -- the mask is ingored,
    // only the image dimensions and pixel spacing are used:
    double area_a = get_area<TImage>(fi);
    double area_b = get_area<TImage>(mi);
//#endif
    
    double smaller_area = std::min(area_a, area_b);
    overlap = overlap_area / smaller_area;
    
    if (overlap < min_overlap || overlap > max_overlap)
    {
      return std::numeric_limits<double>::max();
    }
  }
  
  return m;
}

//----------------------------------------------------------------
// my_metric
// 
// Calculate the normalized cross correlation metric between two
// masked images and the ratio of area of overlap to the area of the
// smaller image. The metric is restricted by the min/max overlap
// ratio thresholds.
// 
template <typename TImage>
double
my_metric(const TImage * fi,
          const TImage * mi,
          const base_transform_t * fi_to_mi,
          const mask_t * fi_mask = NULL,
          const mask_t * mi_mask = NULL,
          const double min_overlap = 0.0,
          const double max_overlap = 1.0)
{
  double overlap = 0.0;
  return my_metric<TImage>(overlap,
                           fi,
                           mi,
                           fi_to_mi,
                           fi_mask,
                           mi_mask,
                           min_overlap,
                           max_overlap);
}

//----------------------------------------------------------------
// calc_image_bbox
// 
// Calculate the bounding box for a given image,
// expressed in image space.
// 
//template <typename T>
//void
//calc_image_bbox(const T * image, pnt2d_t & bbox_min, pnt2d_t & bbox_max)
//{
//  typename T::SizeType sz = image->GetLargestPossibleRegion().GetSize();
//  typename T::SpacingType sp = image->GetSpacing();
//  bbox_min = image->GetOrigin();
//  bbox_max[0] = bbox_min[0] + sp[0] * static_cast<double>(sz[0]);
//  bbox_max[1] = bbox_min[1] + sp[1] * static_cast<double>(sz[1]);
//}

//----------------------------------------------------------------
// calc_image_bboxes
// 
// Calculate the bounding boxes for a set of given images,
// expressed in image space.
// 
template <class image_pointer_t>
void
calc_image_bboxes(const std::vector<image_pointer_t> & image,
                  
                  // image space bounding boxes (for feathering):
                  std::vector<pnt2d_t> & image_min,
                  std::vector<pnt2d_t> & image_max)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType T;
  typedef typename T::RegionType::SizeType imagesz_t;
  typedef typename T::SpacingType spacing_t;
  
  const unsigned int num_images = image.size();
  image_min.resize(num_images);
  image_max.resize(num_images);
  
  // calculate the bounding boxes:
  for (unsigned int i = 0; i < num_images; i++)
  {
    // initialize an empty bounding box:
    image_min[i][0] = std::numeric_limits<double>::max();
    image_min[i][1] = image_min[i][0];
    image_max[i][0] = -image_min[i][0];
    image_max[i][1] = -image_min[i][0];
    
    // it happens:
    if (image[i].GetPointer() == NULL) continue;
    
    // bounding box in the image space:
    calc_image_bbox<T>(image[i], image_min[i], image_max[i]);
  }
}

//----------------------------------------------------------------
// calc_image_bboxes_load_images
// 
// Calculate the bounding boxes for a set of images,
// expressed in image space.  These must be loaded, and are loaded
// one by one to save memory on large images.
// 
template <class image_pointer_t>
void
calc_image_bboxes_load_images(const std::list<bfs::path> & in,
                              
                              // image space bounding boxes (for feathering):
                              std::vector<pnt2d_t> & image_min,
                              std::vector<pnt2d_t> & image_max,
                              
                              const unsigned int shrink_factor,
                              const double pixel_spacing)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType T;
  typedef typename T::RegionType::SizeType imagesz_t;
  typedef typename T::SpacingType spacing_t;
  
  const unsigned int num_images = in.size();
  image_min.resize(num_images);
  image_max.resize(num_images);
  
  // calculate the bounding boxes:
  unsigned int i = 0;
  for (std::list<bfs::path>::const_iterator iter = in.begin();
       iter != in.end(); ++iter, i++)
  {
    // initialize an empty bounding box:
    image_min[i][0] = std::numeric_limits<double>::max();
    image_min[i][1] = image_min[i][0];
    image_max[i][0] = -image_min[i][0];
    image_max[i][1] = -image_min[i][0];
    
    // Read in just the OutputInformation for speed.
    typedef typename itk::ImageFileReader<T> reader_t;
    typename reader_t::Pointer reader = reader_t::New();
    
    reader->SetFileName((*iter).c_str());
    std::cout << "loading region from " << (*iter) << std::endl;
    
    WRAP(terminator_t<reader_t> terminator(reader));
    
    // Load up the OutputInformation, faster than the whole image
    reader->UpdateOutputInformation();
    typename T::Pointer image = reader->GetOutput();
    
    // bounding box in the image space:
    calc_image_bbox<T>(image, image_min[i], image_max[i]);
    
    // Unload the image.
    image = image_t::Pointer(NULL);
  }
}

//----------------------------------------------------------------
// calc_tile_mosaic_bbox
// 
// Calculate the warped image bounding box in the mosaic space.
// 
//extern bool
//calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
//                      
//                      // image space bounding boxes of the tile:
//                      const pnt2d_t & tile_min,
//                      const pnt2d_t & tile_max,
//                      
//                      // mosaic space bounding boxes of the tile:
//                      pnt2d_t & mosaic_min,
//                      pnt2d_t & mosaic_max,
//                      
//                      // sample points along the image edges:
//                      const unsigned int np = 15);

//----------------------------------------------------------------
// calc_tile_mosaic_bbox
// 
// Calculate the warped image bounding box in the mosaic space.
// 
//template <typename T>
//bool
//calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
//                      const T * tile,
//                      
//                      // mosaic space bounding boxes of the tile:
//                      pnt2d_t & mosaic_min,
//                      pnt2d_t & mosaic_max,
//                      
//                      // sample points along the image edges:
//                      const unsigned int np = 15)
//{
//  typename T::SizeType sz = tile->GetLargestPossibleRegion().GetSize();
//  typename T::SpacingType sp = tile->GetSpacing();
//  
//  pnt2d_t tile_min = tile->GetOrigin();
//  pnt2d_t tile_max;
//  tile_max[0] = tile_min[0] + sp[0] * static_cast<double>(sz[0]);
//  tile_max[1] = tile_min[1] + sp[1] * static_cast<double>(sz[1]);
//  
//  return calc_tile_mosaic_bbox(mosaic_to_tile,
//                               tile_min,
//                               tile_max,
//                               mosaic_min,
//                               mosaic_max,
//                               np);
//}

//----------------------------------------------------------------
// calc_mosaic_bboxes
// 
// Calculate the warped image bounding boxes in the mosaic space.
// 
template <class point_t, class transform_pointer_t>
bool
calc_mosaic_bboxes(const std::vector<transform_pointer_t> & xform,
                   
                   // image space bounding boxes of mosaic tiles::
                   const std::vector<point_t> & image_min,
                   const std::vector<point_t> & image_max,
                   
                   // mosaic space bounding boxes of individual tiles:
                   std::vector<point_t> & mosaic_min,
                   std::vector<point_t> & mosaic_max,
                   
                   // sample points along the image edges:
                   const unsigned int np = 15)
{
  const unsigned int num_images = xform.size();
  mosaic_min.resize(num_images);
  mosaic_max.resize(num_images);
  
  point_t image_point;
  point_t mosaic_point;
  
  // calculate the bounding boxes
  bool ok = true;
  for (unsigned int i = 0; i < num_images; i++)
  {
    ok = ok & calc_tile_mosaic_bbox(xform[i],
                                    image_min[i],
                                    image_max[i],
                                    mosaic_min[i],
                                    mosaic_max[i],
                                    np);
  }
  
  return ok;
}


//----------------------------------------------------------------
// calc_mosaic_bbox
// 
// Calculate the mosaic bounding box (a union of mosaic space
// bounding boxes of the warped tiles).
// 
template <class point_t>
void
calc_mosaic_bbox(// mosaic space bounding boxes of individual mosaic tiles:
                 const std::vector<point_t> & mosaic_min,
                 const std::vector<point_t> & mosaic_max,
                 
                 // mosiac bounding box:
                 point_t & min,
                 point_t & max)
{
  // initialize an empty bounding box:
  min[0] = std::numeric_limits<double>::max();
  min[1] = min[0];
  max[0] = -min[0];
  max[1] = -min[0];
  
  // calculate the bounding box:
  const unsigned int num_images = mosaic_min.size();
  for (unsigned int i = 0; i < num_images; i++)
  {
    // it happens:
    if (mosaic_min[i][0] == std::numeric_limits<double>::max()) continue;
    
    // update the mosaic bounding box:
    update_bbox(min, max, mosaic_min[i]);
    update_bbox(min, max, mosaic_max[i]);
  }
}

//----------------------------------------------------------------
// calc_mosaic_bbox
// 
// Calculate the mosaic bounding box (a union of mosaic space
// bounding boxes of the warped tiles).
// 
template <class image_pointer_t, class transform_pointer_t>
void
calc_mosaic_bbox(const std::vector<transform_pointer_t> & transform,
                 const std::vector<image_pointer_t> & image,
                 
                 // mosaic bounding box:
                 typename image_pointer_t::ObjectType::PointType & min,
                 typename image_pointer_t::ObjectType::PointType & max,
                 
                 // points along the image edges:
                 const unsigned int np = 15)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType image_t;
  typedef typename image_t::PointType point_t;
  
  // image space bounding boxes:
  std::vector<point_t> image_min;
  std::vector<point_t> image_max;
  calc_image_bboxes<image_pointer_t>(image, image_min, image_max);
  
  // mosaic space bounding boxes:
  std::vector<point_t> mosaic_min;
  std::vector<point_t> mosaic_max;
  
  // FIXME:
  calc_mosaic_bboxes<point_t, transform_pointer_t>(transform,
                                                   image_min,
                                                   image_max,
                                                   mosaic_min,
                                                   mosaic_max,
                                                   np);
  
  // mosiac bounding box:
  calc_mosaic_bbox<point_t>(mosaic_min, mosaic_max, min, max);
}

//----------------------------------------------------------------
// calc_mosaic_bbox_load_images
// 
// Calculate the mosaic bounding box (a union of mosaic space
// bounding boxes of the warped tiles).
// 
template <class image_pointer_t, class transform_pointer_t>
void
calc_mosaic_bbox_load_images(const std::vector<transform_pointer_t> & transform,
                             const std::list<bfs::path> & fn_image,
                             
                             // mosaic bounding box:
                             typename image_pointer_t::ObjectType::PointType & min,
                             typename image_pointer_t::ObjectType::PointType & max,
                             std::vector<typename image_pointer_t::ObjectType::PointType> & mosaic_min,
                             std::vector<typename image_pointer_t::ObjectType::PointType> & mosaic_max,
                             const unsigned int shrink_factor,
                             const double pixel_spacing,
                             
                             // points along the image edges:
                             const unsigned int np = 15)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType image_t;
  typedef typename image_t::PointType point_t;
  
  // image space bounding boxes:
  std::vector<point_t> image_min;
  std::vector<point_t> image_max;
  
  calc_image_bboxes_load_images<image_pointer_t>(fn_image, 
                                                 image_min, 
                                                 image_max,
                                                 shrink_factor,
                                                 pixel_spacing);
  
  // mosaic space bounding boxes:
  // FIXME:
  calc_mosaic_bboxes<point_t, transform_pointer_t>(transform,
                                                   image_min,
                                                   image_max,
                                                   mosaic_min,
                                                   mosaic_max,
                                                   np);
  
  // mosiac bounding box:
  calc_mosaic_bbox<point_t>(mosaic_min, mosaic_max, min, max);
}


//----------------------------------------------------------------
// bbox_overlap
// 
// Test whether two bounding boxes overlap.
// 
inline bool
bbox_overlap(const pnt2d_t & min_box1, 
             const pnt2d_t & max_box1, 
             const pnt2d_t & min_box2,
             const pnt2d_t & max_box2)
{
  return
  max_box1[0] > min_box2[0] && 
  min_box1[0] < max_box2[0] &&
  max_box1[1] > min_box2[1] && 
  min_box1[1] < max_box2[1];
}

//----------------------------------------------------------------
// inside_bbox
// 
// Test whether a given point is inside the bounding box.
// 
inline bool
inside_bbox(const pnt2d_t & min, const pnt2d_t & max, const pnt2d_t & pt)
{
  return
  min[0] <= pt[0] &&
  pt[0] <= max[0] &&
  min[1] <= pt[1] &&
  pt[1] <= max[1];
}

//----------------------------------------------------------------
// calc_pixel_weight
// 
// Calculate a pixel feathering weight used to blend mosaic tiles.
// 
inline static double
calc_pixel_weight(const pnt2d_t & bbox_min,
                  const pnt2d_t & bbox_max,
                  const pnt2d_t & pt)
{
  double w = bbox_max[0] - bbox_min[0];
  double h = bbox_max[1] - bbox_min[1];
  double r = 0.5 * ((w > h) ? h : w);
  double sx = std::min(pt[0] - bbox_min[0], bbox_max[0] - pt[0]);
  double sy = std::min(pt[1] - bbox_min[1], bbox_max[1] - pt[1]);
  double s = (1.0 + std::min(sx, sy)) / (r + 1.0);
  return s * s * s * s;
}


//----------------------------------------------------------------
// feathering_t
//
// Supported pixel feathering modes
// 
typedef enum {
  FEATHER_NONE_E,
  FEATHER_BLEND_E,
  FEATHER_BINARY_E
} feathering_t;

//----------------------------------------------------------------
// make_mosaic_st
// 
// Assemble a portion of the mosaic positioned at mosaic_min.
// Each tile may be individually tinted with a grayscale color.
// Individual tiles may be omitted.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic_st
(bool assemble_mosaic_mask,
 mask_t::Pointer & mosaic_mask,
 const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const typename image_pointer_t::ObjectType::PointType & mosaic_min,
 const typename image_pointer_t::ObjectType::SizeType & mosaic_sz,
 const unsigned int num_images,
 const std::vector<bool> & omit,
 const std::vector<double> & tint,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E,
 double background = 255.0,
 
 bool dont_allocate = false)
{
  WRAP(itk_terminator_t terminator("make_mosaic_st"));
  
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType image_t;
  typedef typename image_t::IndexType index_t;
  typedef typename image_t::PointType point_t;
  typedef typename image_t::PixelType pixel_t;
  typedef typename image_t::SpacingType spacing_t;
  typedef typename image_t::RegionType::SizeType imagesz_t;
  typedef typename itk::ImageRegionIteratorWithIndex<image_t> itex_t;
  
  // setup the image interpolators:
  typedef typename itk::LinearInterpolateImageFunction
  <image_t, double> img_interpolator_t;
  std::vector<typename img_interpolator_t::Pointer> img(num_images);
  
  for (unsigned int i = 0; i < num_images; i++)
  {
    img[i] = img_interpolator_t::New();
    img[i]->SetInputImage(image[i]);
  }
  
  // setup the image mask interpolators:
  typedef itk::NearestNeighborInterpolateImageFunction
  <mask_t, double> msk_interpolator_t;
  std::vector<msk_interpolator_t::Pointer> msk(num_images);
  msk.assign(num_images, msk_interpolator_t::Pointer(NULL));
  
  for (unsigned int i = 0; i < image_mask.size() && i < num_images; i++)
  {
    if (image_mask[i].GetPointer() == NULL) continue;
    
    msk[i] = msk_interpolator_t::New();
    msk[i]->SetInputImage(image_mask[i]);
  }
  
  // image space bounding boxes (for feathering):
  std::vector<point_t> bbox_min(num_images);
  std::vector<point_t> bbox_max(num_images);
  calc_image_bboxes<image_pointer_t>(image, bbox_min, bbox_max);
  
  // mosaic space bounding boxes:
  std::vector<point_t> min(num_images);
  std::vector<point_t> max(num_images);
  
  calc_mosaic_bboxes<point_t, transform_pointer_t>(transform,
                                                   bbox_min,
                                                   bbox_max,
                                                   min,
                                                   max);
  
  // setup the mosaic image:
  typename image_t::Pointer mosaic = image_t::New();
  mosaic->SetOrigin(mosaic_min);
  mosaic->SetRegions(mosaic_sz);
  mosaic->SetSpacing(mosaic_sp);
  
  mosaic_mask = NULL;
  if (assemble_mosaic_mask)
  {
    mosaic_mask = mask_t::New();
    mosaic_mask->SetOrigin(mosaic_min);
    mosaic_mask->SetRegions(mosaic_sz);
    mosaic_mask->SetSpacing(mosaic_sp);
  }
  
  if (dont_allocate)
  {
    // this is useful for estimating the mosaic size:
    return mosaic;
  }
  
  // make sure there hasn't been an interrupt:
  WRAP(terminator.terminate_on_request());
  
  mosaic->Allocate();
  
  if (assemble_mosaic_mask)
  {
    mosaic_mask->Allocate();
  }
  
  // this is needed in order to prevent holes in the mask mosaic:
  bool integer_pixel = std::numeric_limits<pixel_t>::is_integer;
  double pixel_max = static_cast<double>(std::numeric_limits<pixel_t>::max());
  double pixel_min = integer_pixel ?
  static_cast<double>(std::numeric_limits<pixel_t>::min()) : -pixel_max;
  
  itex_t itex(mosaic, mosaic->GetLargestPossibleRegion());
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    point_t point;
    mosaic->TransformIndexToPhysicalPoint(itex.GetIndex(), point);
    
    double pixel = 0.0;
    double weight = 0.0;
    unsigned int num_pixels = 0;
    for (unsigned int k = 0; k < num_images; k++)
    {
      // don't try to add missing or omitted images to the mosaic:
      if (omit[k] || (image[k].GetPointer() == NULL)) continue;
      
      // avoid undesirable distortion artifacts:
      if (!inside_bbox(min[k], max[k], point)) continue;
      
      const transform_pointer_t & t = transform[k];
      point_t pt_k = t->TransformPoint(point);
      
      // make sure the pixel maps into the image:
      if (!img[k]->IsInsideBuffer(pt_k)) continue;
      
      // make sure the pixel maps into the image mask:
      double alpha = 1.0;
      if ((msk[k].GetPointer() != NULL) &&
          (alpha = msk[k]->Evaluate(pt_k)) < 1.0) continue;
      
      // feather out the edges by giving them a tiny weight:
      num_pixels++;
      double wp = tint[k];
      double p = img[k]->Evaluate(pt_k) * wp;
      double wa = ((alpha == 1.0) ? 1e-0 : 1e-6) * wp;
      
      if (feathering == FEATHER_NONE_E)
      {
        pixel += p * wa;
        weight += wa;
      }
      else
      {
        double pixel_weight =
        calc_pixel_weight(bbox_min[k], bbox_max[k], pt_k);
        
        if (feathering == FEATHER_BLEND_E)
        {
          pixel += pixel_weight * p * wa;
          weight += pixel_weight * wa;
        }
        else // FEATHER_BINARY_E
        {
          if (pixel_weight > weight)
          {
            pixel = pixel_weight * p * wa;
            weight = pixel_weight * wa;
          }
        }
      }
    }
    
    // calculate the final pixel value:
    if (weight > 0.0)
    {
      pixel /= weight;
      if (integer_pixel)
      {
        pixel = floor(pixel + 0.5);
        
        // make sure we don't exceed the intensity range:
        pixel = std::max(pixel_min, std::min(pixel_max, pixel));
      }
      
      pixel_t tmp = pixel_t(pixel);
      itex.Set(tmp);
    }
    else
    {
      itex.Set(num_pixels > 0 ? 0 : pixel_t(background));
    }
    
    if (mosaic_mask.GetPointer())
    {
      mask_t::PixelType mask_pixel = (weight > 0.0) ? 1 : 0;
      mosaic_mask->SetPixel(itex.GetIndex(), mask_pixel);
    }
  }
  
  return mosaic;
}


//----------------------------------------------------------------
// assemble_mosaic_t
// 
// Parallelized mosaic assembly mechanism
// 
template <typename image_pointer_t, typename transform_pointer_t>
class assemble_mosaic_t : public the_transaction_t
{
public:
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType tile_t;
  typedef typename image_t::PointType pnt_t;
  typedef typename image_t::PixelType pxl_t;
  typedef typename image_t::RegionType rn_t;
  typedef typename image_t::RegionType::SizeType sz_t;
  typedef typename image_t::RegionType::IndexType ix_t;
  typedef typename itk::LinearInterpolateImageFunction<tile_t, double> itile_t;
  typedef itk::NearestNeighborInterpolateImageFunction<mask_t, double> imask_t;
  
  assemble_mosaic_t(// thread index and number of threads, used to avoid
                    // concurrent access to the same point in the mosaic:
                    unsigned int thread_offset,
                    unsigned int thread_stride,
                    
                    // mosaic being assembled:
                    typename tile_t::Pointer & mosaic,
                    mask_t * mosaic_mask,
                    
                    // number of tiles to use for this mosaic (may be fewer
                    // then the number of tiles available):
                    unsigned int num_tiles,
                    
                    // omitted tile flags:
                    const std::vector<bool> & omit,
                    
                    // tile tint:
                    const std::vector<double> & tint,
                    
                    // tile trasforms:
                    const std::vector<transform_pointer_t> & transform,
                    
                    // tiles and tile masks
                    const std::vector<image_pointer_t> & tile,
                    const std::vector<mask_t::ConstPointer> & mask,
                    
                    // tile and tile mask interpolators:
                    const std::vector<typename itile_t::Pointer> & itile,
                    const std::vector<typename imask_t::Pointer> & imask,
                    
                    // image space tile bounding boxes (for feathering):
                    const std::vector<pnt_t> & bbox_min,
                    const std::vector<pnt_t> & bbox_max,
                    
                    // mosaic space tile bounding boxes:
                    const std::vector<pnt_t> & min,
                    const std::vector<pnt_t> & max,
                    
                    // overlap region feathering method:
                    feathering_t feathering,
                    
                    // default mosaic pixel value:
                    double background):
  
  thread_offset_(thread_offset),
  thread_stride_(thread_stride),
  mosaic_(mosaic),
  mosaic_mask_(mosaic_mask),
  num_tiles_(num_tiles),
  omit_(omit),
  tint_(tint),
  transform_(transform),
  tile_(tile),
  mask_(mask),
  itile_(itile),
  imask_(imask),
  bbox_min_(bbox_min),
  bbox_max_(bbox_max),
  min_(min),
  max_(max),
  feathering_(feathering),
  background_(background)
  {}
  
  void execute(the_thread_interface_t * thread)
  {
    WRAP(itk_terminator_t terminator("assemble_mosaic_t::execute"));
    
    // this is needed in order to prevent holes in the mask mosaic:
    const bool integer_pixel = std::numeric_limits<pxl_t>::is_integer;
    const double pixel_max = static_cast<double>(std::numeric_limits<pxl_t>::max());
    const double pixel_min = integer_pixel ?
    static_cast<double>(std::numeric_limits<pxl_t>::min()) : -pixel_max;
    
    rn_t region = mosaic_->GetLargestPossibleRegion();
    ix_t origin = region.GetIndex();
    sz_t extent = region.GetSize();
    ix_t::IndexValueType x_end = origin[0] + extent[0];
    ix_t::IndexValueType y_end = origin[1] + extent[1];
    
    ix_t ix = origin;
    for (ix[1] = origin[1]; ix[1] < y_end; ++ix[1])
    {
      for (ix[0] = origin[0] + thread_offset_;
           ix[0] < x_end;
           ix[0] += thread_stride_)
      {
        // check whether termination was requested:
        WRAP(terminator.terminate_on_request());
        
        pnt_t point;
        mosaic_->TransformIndexToPhysicalPoint(ix, point);
        
        double pixel = 0.0;
        double weight = 0.0;
        unsigned int num_pixels = 0;
        
        for (unsigned int k = 0; k < num_tiles_; k++)
        {
          // don't try to add missing or omitted images to the mosaic:
          if (omit_[k] || (tile_[k].GetPointer() == NULL))
          {
            continue;
          }
          
          // avoid undesirable distortion artifacts:
          if (!inside_bbox(min_[k], max_[k], point))
          {
            continue;
          }
          
          const transform_pointer_t & t = transform_[k];
          pnt_t pt_k = t->TransformPoint(point);
          
          // make sure the pixel maps into the image:
          if (!itile_[k]->IsInsideBuffer(pt_k))
          {
            continue;
          }
          
          // make sure the pixel maps into the image mask:
          double alpha = 1.0;
          if ((imask_[k].GetPointer() != NULL) &&
              (alpha = imask_[k]->Evaluate(pt_k)) < 1.0)
          {
            continue;
          }
          
          // feather out the edges by giving them a tiny weight:
          num_pixels++;
          double wp = tint_[k];
          double p = itile_[k]->Evaluate(pt_k) * wp;
          double wa = ((alpha == 1.0) ? 1e-0 : 1e-6) * wp;
          
          if (feathering_ == FEATHER_NONE_E)
          {
            pixel += p * wa;
            weight += wa;
          }
          else
          {
            double pixel_weight = calc_pixel_weight(bbox_min_[k],
                                                    bbox_max_[k],
                                                    pt_k);
            
            if (feathering_ == FEATHER_BLEND_E)
            {
              pixel += pixel_weight * p * wa;
              weight += pixel_weight * wa;
            }
            else // FEATHER_BINARY_E
            {
              if (pixel_weight > weight)
              {
                pixel = pixel_weight * p * wa;
                weight = pixel_weight * wa;
              }
            }
          }
        }
        
        // calculate the final pixel value:
        if (weight > 0.0)
        {
          pixel /= weight;
          if (integer_pixel)
          {
            pixel = floor(pixel + 0.5);
            
            // make sure we don't exceed the intensity range:
            pixel = std::max(pixel_min, std::min(pixel_max, pixel));
          }
          
          pxl_t output = pxl_t(pixel);
          mosaic_->SetPixel(ix, output);
        }
        else
        {
          pxl_t output = num_pixels > 0 ? 0 : pxl_t(background_);
          mosaic_->SetPixel(ix, output);
        }
        
        if (mosaic_mask_)
        {
          mask_t::PixelType mask_pixel = (weight > 0.0) ? 1 : 0;
          mosaic_mask_->SetPixel(ix, mask_pixel);
        }
      }
    }
  }
  
  // data members facilitating concurrent mosaic assembly:
  unsigned int thread_offset_;
  unsigned int thread_stride_;
  
  // output mosaic:
  typename tile_t::Pointer & mosaic_;
  mask_t * mosaic_mask_;
  
  // number of tiles used for this mosaic (may be fewer than
  // what's available from the tile vector):
  unsigned int num_tiles_;
  
  // omitted tile flags:
  const std::vector<bool> & omit_;
  
  // tile tint:
  const std::vector<double> & tint_;
  
  // tile trasforms:
  const std::vector<transform_pointer_t> & transform_;
  
  // tiles and tile masks
  const std::vector<image_pointer_t> & tile_;
  const std::vector<mask_t::ConstPointer> & mask_;
  
  
  // tile and tile mask interpolators:
  const std::vector<typename itile_t::Pointer> & itile_;
  const std::vector<typename imask_t::Pointer> & imask_;
  
  // image space tile bounding boxes (for feathering):
  const std::vector<pnt_t> & bbox_min_;
  const std::vector<pnt_t> & bbox_max_;
  
  // mosaic space tile bounding boxes:
  const std::vector<pnt_t> & min_;
  const std::vector<pnt_t> & max_;
  
  // overlap region feathering method:
  feathering_t feathering_;
  
  // default mosaic pixel value:
  double background_;
};

//----------------------------------------------------------------
// make_mosaic_mt
// 
// Assemble a portion of the mosaic positioned at mosaic_min.
// Each tile may be individually tinted with a grayscale color.
// Individual tiles may be omitted.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic_mt
(unsigned int num_threads,
 bool assemble_mosaic_mask,
 mask_t::Pointer & mosaic_mask,
 const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const typename image_pointer_t::ObjectType::PointType & mosaic_min,
 const typename image_pointer_t::ObjectType::SizeType & mosaic_sz,
 const unsigned int num_images,
 const std::vector<bool> & omit,
 const std::vector<double> & tint,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E,
 double background = 255.0,
 
 bool dont_allocate = false)
{
  if (num_threads == 1)
  {
    // use the old single-threaded code:
    return
    make_mosaic_st<image_pointer_t, transform_pointer_t>
    (assemble_mosaic_mask,
     mosaic_mask,
     mosaic_sp,
     mosaic_min,
     mosaic_sz,
     num_images,
     omit,
     tint,
     transform,
     image,
     image_mask,
     feathering,
     background,
     dont_allocate);
  }
  
  WRAP(itk_terminator_t terminator("make_mosaic_mt"));
  
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType img_t;
  typedef typename img_t::PointType pnt_t;
  
  // setup the image interpolators:
  typedef typename itk::LinearInterpolateImageFunction
  <img_t, double> img_interpolator_t;
  std::vector<typename img_interpolator_t::Pointer> img(num_images);
  
  for (unsigned int i = 0; i < num_images; i++)
  {
    img[i] = img_interpolator_t::New();
    img[i]->SetInputImage(image[i]);
  }
  
  // setup the image mask interpolators:
  typedef itk::NearestNeighborInterpolateImageFunction
  <mask_t, double> msk_interpolator_t;
  std::vector<msk_interpolator_t::Pointer> msk(num_images);
  msk.assign(num_images, msk_interpolator_t::Pointer(NULL));
  
  for (unsigned int i = 0; i < image_mask.size() && i < num_images; i++)
  {
    if (image_mask[i].GetPointer() == NULL) continue;
    
    msk[i] = msk_interpolator_t::New();
    msk[i]->SetInputImage(image_mask[i]);
  }
  
  // image space bounding boxes (for feathering):
  std::vector<pnt_t> bbox_min(num_images);
  std::vector<pnt_t> bbox_max(num_images);
  calc_image_bboxes<image_pointer_t>(image, bbox_min, bbox_max);
  
  // mosaic space bounding boxes:
  std::vector<pnt_t> min(num_images);
  std::vector<pnt_t> max(num_images);
  calc_mosaic_bboxes<pnt_t, transform_pointer_t>(transform,
                                                 bbox_min,
                                                 bbox_max,
                                                 min,
                                                 max);
  
  // setup the mosaic image:
  typename img_t::Pointer mosaic = img_t::New();
  mosaic->SetOrigin(mosaic_min);
  mosaic->SetRegions(mosaic_sz);
  mosaic->SetSpacing(mosaic_sp);
  
  mosaic_mask = NULL;
  if (assemble_mosaic_mask)
  {
    mosaic_mask = mask_t::New();
    mosaic_mask->SetOrigin(mosaic_min);
    mosaic_mask->SetRegions(mosaic_sz);
    mosaic_mask->SetSpacing(mosaic_sp);
  }
  
  if (dont_allocate)
  {
    // this is useful for estimating the mosaic size:
    return mosaic;
  }
  
  // allocate the mosaic:
  mosaic->Allocate();
  
  if (assemble_mosaic_mask)
  {
    mosaic_mask->Allocate();
  }
  
  // setup transactions for multi-threaded mosaic assembly:
  std::list<the_transaction_t *> schedule;
  for (unsigned int i = 0; i < num_threads; i++)
  {
    assemble_mosaic_t<image_pointer_t, transform_pointer_t> * t =
    new assemble_mosaic_t<image_pointer_t, transform_pointer_t>
    (i,
     num_threads,
     mosaic,
     mosaic_mask.GetPointer(),
     num_images,
     omit,
     tint,
     transform,
     image,
     image_mask,
     img,
     msk,
     bbox_min,
     bbox_max,
     min,
     max,
     feathering,
     background);
    
    schedule.push_back(t);
  }
  
  // setup the thread pool:
  the_thread_pool_t thread_pool(num_threads);
  thread_pool.set_idle_sleep_duration(50); // 50 usec
  thread_pool.push_back(schedule);
  thread_pool.pre_distribute_work();
  
  // execute mosaic assembly transactions:
  suspend_itk_multithreading_t suspend_itk_mt;
  thread_pool.start();
  thread_pool.wait();
  
  // done:
  return mosaic;
}

//----------------------------------------------------------------
// make_mosaic
// 
// Assemble a portion of the mosaic positioned at mosaic_min.
// Each tile may be individually tinted with a grayscale color.
// Individual tiles may be omitted.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
//
// Use all possible processors/cores available for multi-threading:
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic
(const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const typename image_pointer_t::ObjectType::PointType & mosaic_min,
 const typename image_pointer_t::ObjectType::SizeType & mosaic_sz,
 const unsigned int num_images,
 const std::vector<bool> & omit,
 const std::vector<double> & tint,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E,
 double background = 255.0,
 
 bool dont_allocate = false)
{
  // use as many threads as supported by hardware:
  unsigned int num_threads = boost::thread::hardware_concurrency();
  
  // NOTE: for backwards compatibility we do not assemble the mosaic mask:
  const bool assemble_mosaic_mask = false;
  mask_t::Pointer mosaic_mask;
  
  return
  make_mosaic_mt<image_pointer_t, transform_pointer_t>
  (num_threads,
   assemble_mosaic_mask,
   mosaic_mask,
   mosaic_sp,
   mosaic_min,
   mosaic_sz,
   num_images,
   omit,
   tint,
   transform,
   image,
   image_mask,
   feathering,
   background,
   dont_allocate);
}


//----------------------------------------------------------------
// make_mosaic
// 
// Assemble a portion of the mosaic positioned at mosaic_min.
// Tiles are not tinted.
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic
(const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const typename image_pointer_t::ObjectType::PointType & mosaic_min,
 const typename image_pointer_t::ObjectType::SizeType & mosaic_sz,
 const unsigned int num_images,
 const std::vector<bool> & omit,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E,
 double background = 255.0,
 
 bool dont_allocate = false)
{
  const std::vector<double> tint(num_images, 1.0);
  return make_mosaic<image_pointer_t, transform_pointer_t>
  (mosaic_sp,
   mosaic_min,
   mosaic_sz,
   num_images,
   omit,
   tint,
   transform,
   image,
   image_mask,
   feathering,
   background,
   dont_allocate);
}

//----------------------------------------------------------------
// make_mosaic
// 
// Assemble a portion of the mosaic bounded by
// mosaic_min and mosaic_max.
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic
(const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const typename image_pointer_t::ObjectType::PointType & mosaic_min,
 const typename image_pointer_t::ObjectType::PointType & mosaic_max,
 const unsigned int num_images,
 const std::vector<bool> & omit,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E,
 
 double background = 255.0,
 
 bool dont_allocate = false)
{
  typename image_pointer_t::ObjectType::SizeType mosaic_sz;
  mosaic_sz[0] = static_cast<unsigned int>((mosaic_max[0] - mosaic_min[0]) /
                                mosaic_sp[0]);
  mosaic_sz[1] = static_cast<unsigned int>((mosaic_max[1] - mosaic_min[1]) /
                                mosaic_sp[1]);
  
  return make_mosaic<image_pointer_t, transform_pointer_t>
  (mosaic_sp,
   mosaic_min,
   mosaic_sz,
   num_images,
   omit,
   transform,
   image,
   image_mask,
   feathering,
   background,
   dont_allocate);
}

//----------------------------------------------------------------
// make_mosaic
// 
// Assemble a portion of the mosaic bounded by
// mosaic_min and mosaic_max.
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic
(const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const unsigned int num_images,
 const std::vector<bool> & omit,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E,
 
 double background = 255.0,
 
 bool dont_allocate = false)
{
  // mosaic bounding box:
  typename image_pointer_t::ObjectType::PointType mosaic_min;
  typename image_pointer_t::ObjectType::PointType mosaic_max;
  calc_mosaic_bbox<image_pointer_t, transform_pointer_t>
  (transform,
   image,
   mosaic_min,
   mosaic_max);
  
  return make_mosaic<image_pointer_t, transform_pointer_t>
  (mosaic_sp,
   mosaic_min,
   mosaic_max,
   num_images,
   omit,
   transform,
   image,
   image_mask,
   feathering,
   background,
   dont_allocate);
}

//----------------------------------------------------------------
// make_mosaic
// 
// Assemble the entire mosaic from a set of tiles and transforms.
// Individual tiles may be omitted.
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic(const std::vector<image_pointer_t> & image,
            const std::vector<transform_pointer_t> & transform,
            const feathering_t feathering = FEATHER_NONE_E,
            const unsigned int omit = std::numeric_limits<unsigned int>::max(),
            unsigned int num_images = std::numeric_limits<unsigned int>::max(),
            // the masks are optional:
            const std::vector<mask_t::ConstPointer> & image_mask =
            std::vector<mask_t::ConstPointer>(0))
{
  if (num_images == std::numeric_limits<unsigned int>::max())
  {
    num_images = image.size();
  }
  
  std::vector<bool> omit_vec(num_images);
  omit_vec.assign(num_images, false);
  if (omit != std::numeric_limits<unsigned int>::max())
  {
    omit_vec[omit] = true;
  }
  
  return
  make_mosaic<image_pointer_t, transform_pointer_t>
  (image[0]->GetSpacing(),
   num_images,
   omit_vec,
   transform,
   image,
   image_mask,
   feathering,
   0.0); // background
}

//----------------------------------------------------------------
// update_mosaic
// 
// Expand the mosaic by adding another tile to it. This may be
// used to assemble the mosaic incrementally.
// 
template <typename T>
typename T::Pointer
update_mosaic(const T * mosaic,
              const T * tile,
              const base_transform_t * tile_transform,
              const mask_t * mask_mosaic = NULL,
              const mask_t * mask_tile = NULL)
{
  std::vector<typename T::ConstPointer> image(2);
  image[0] = mosaic;
  image[1] = tile;
  
  std::vector<base_transform_t::ConstPointer> transform(2);
  transform[0] = identity_transform_t::New();
  transform[1] = tile_transform;
  
  std::vector<mask_t::ConstPointer> image_mask(2);
  image_mask[0] = mask_mosaic;
  image_mask[1] = mask_tile;
  
  std::vector<bool> omit(2);
  omit.assign(2, false);
  
  return
  make_mosaic<typename T::ConstPointer, base_transform_t::ConstPointer>
  (mosaic->GetSpacing(),
   2,
   omit,
   transform,
   image,
   image_mask,
   FEATHER_NONE_E);
}

//----------------------------------------------------------------
// make_mask_st
//
// Assemble a mask for the entire mosaic.
// Individual tiles may be omitted.
// 
template <class transform_pointer_t>
mask_t::Pointer
make_mask_st(const mask_t::SpacingType & mosaic_sp,
             const unsigned int num_images,
             const std::vector<bool> & omit,
             const std::vector<transform_pointer_t> & transform,
             const std::vector<mask_t::ConstPointer> & image_mask)
{
  WRAP(itk_terminator_t terminator("make_mask_st"));
  
  typedef mask_t::IndexType index_t;
  typedef mask_t::PointType point_t;
  typedef mask_t::PixelType pixel_t;
  typedef mask_t::SpacingType spacing_t;
  typedef mask_t::RegionType::SizeType imagesz_t;
  typedef itk::ImageRegionIteratorWithIndex<mask_t> itex_t;
  
  // setup the image mask interpolators:
  typedef itk::NearestNeighborInterpolateImageFunction
  <mask_t, double> msk_interpolator_t;
  std::vector<msk_interpolator_t::Pointer> msk(num_images);
  msk.assign(num_images, msk_interpolator_t::Pointer(NULL));
  
  for (unsigned int i = 0; i < image_mask.size() && i < num_images; i++)
  {
    if (image_mask[i].GetPointer() == NULL) continue;
    
    msk[i] = msk_interpolator_t::New();
    msk[i]->SetInputImage(image_mask[i]);
  }
  
  // image space bounding boxes (for feathering):
  std::vector<point_t> bbox_min(num_images);
  std::vector<point_t> bbox_max(num_images);
  calc_image_bboxes<mask_t::ConstPointer>(image_mask, bbox_min, bbox_max);
  
  // mosaic space bounding boxes:
  std::vector<point_t> min(num_images);
  std::vector<point_t> max(num_images);
  calc_mosaic_bboxes<point_t, transform_pointer_t>(transform,
                                                   bbox_min,
                                                   bbox_max,
                                                   min,
                                                   max);
  
  // mosiac bounding box:
  point_t mosaic_min;
  point_t mosaic_max;
  calc_mosaic_bbox<point_t>(min, max, mosaic_min, mosaic_max);
  
  // setup the mosaic image:
  mask_t::Pointer mosaic = mask_t::New();
  imagesz_t mosaic_sz;
  
  mosaic_sz[0] = static_cast<unsigned int>((mosaic_max[0] - mosaic_min[0]) /
                                mosaic_sp[0]);
  mosaic_sz[1] = static_cast<unsigned int>((mosaic_max[1] - mosaic_min[1]) /
                                mosaic_sp[1]);
  mosaic->SetOrigin(pnt2d(mosaic_min[0], mosaic_min[1]));
  mosaic->SetRegions(mosaic_sz);
  mosaic->SetSpacing(mosaic_sp);
  
  // make sure there hasn't been an interrupt:
  WRAP(terminator.terminate_on_request());
  
  mosaic->Allocate();
  mosaic->FillBuffer(itk::NumericTraits<pixel_t>::Zero);
  
  itex_t itex(mosaic, mosaic->GetLargestPossibleRegion());
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    point_t point;
    mosaic->TransformIndexToPhysicalPoint(itex.GetIndex(), point);
    
    for (unsigned int k = 0; k < num_images; k++)
    {
      // don't try to add missing or omitted images to the mosaic:
      if (omit[k] || (image_mask[k].GetPointer() == NULL)) continue;
      
      // avoid undesirable radial distortion artifacts for R >> Rmax:
      if (!inside_bbox(min[k], max[k], point)) continue;
      
      const transform_pointer_t & t = transform[k];
      point_t pt_k = t->TransformPoint(point);
      
      // make sure the pixel maps into the image:
      if (!msk[k]->IsInsideBuffer(pt_k)) continue;
      
      // make sure the pixel maps into the image mask:
      if (msk[k]->Evaluate(pt_k) == 0) continue;
      
      itex.Set(1);
      break;
    }
  }
  
  return mosaic;
}



//----------------------------------------------------------------
// assemble_mask_t
// 
// Parallelized mosaic mask assembly mechanism
// 
template <typename transform_pointer_t>
class assemble_mask_t : public the_transaction_t
{
public:
  typedef mask_t::PointType pnt_t;
  typedef mask_t::PixelType pxl_t;
  typedef mask_t::RegionType rn_t;
  typedef mask_t::RegionType::SizeType sz_t;
  typedef mask_t::RegionType::IndexType ix_t;
  typedef itk::NearestNeighborInterpolateImageFunction<mask_t, double> imask_t;
  
  assemble_mask_t(// thread index and number of threads, used to avoid
                  // concurrent access to the same point in the mosaic:
                  unsigned int thread_offset,
                  unsigned int thread_stride,
                  
                  // mosaic being assembled:
                  mask_t::Pointer & mosaic,
                  
                  // number of tiles to use for this mosaic (may be fewer
                  // then the number of tiles available):
                  unsigned int num_tiles,
                  
                  // omitted tile flags:
                  const std::vector<bool> & omit,
                  
                  // tile trasforms:
                  const std::vector<transform_pointer_t> & transform,
                  
                  // tiles and tile masks
                  const std::vector<mask_t::ConstPointer> & mask,
                  
                  // tile and tile mask interpolators:
                  const std::vector<typename imask_t::Pointer> & imask,
                  
                  // mosaic space tile bounding boxes:
                  const std::vector<pnt_t> & min,
                  const std::vector<pnt_t> & max):
  
  thread_offset_(thread_offset),
  thread_stride_(thread_stride),
  mosaic_(mosaic),
  num_tiles_(num_tiles),
  omit_(omit),
  transform_(transform),
  mask_(mask),
  imask_(imask),
  min_(min),
  max_(max)
  {}
  
  void execute(the_thread_interface_t * thread)
  {
    WRAP(itk_terminator_t terminator("assemble_mask_t::execute"));
    
    rn_t region = mosaic_->GetLargestPossibleRegion();
    ix_t origin = region.GetIndex();
    sz_t extent = region.GetSize();
    ix_t::IndexValueType x_end = origin[0] + extent[0];
    ix_t::IndexValueType y_end = origin[1] + extent[1];
    
    ix_t ix = origin;
    for (ix[1] = origin[1]; ix[1] < y_end; ++ix[1])
    {
      for (ix[0] = origin[0] + thread_offset_;
           ix[0] < x_end;
           ix[0] += thread_stride_)
      {
        // check whether termination was requested:
        WRAP(terminator.terminate_on_request());
        
        pnt_t point;
        mosaic_->TransformIndexToPhysicalPoint(ix, point);
        
        mask_t::PixelType mask_pixel = 0;
        for (unsigned int k = 0; k < num_tiles_; k++)
        {
          // don't try to add missing or omitted images to the mosaic:
          if (omit_[k] || (mask_[k].GetPointer() == NULL))
          {
            continue;
          }
          
          // avoid undesirable distortion artifacts:
          if (!inside_bbox(min_[k], max_[k], point))
          {
            continue;
          }
          
          const transform_pointer_t & t = transform_[k];
          pnt_t pt_k = t->TransformPoint(point);
          
          // make sure the pixel maps into the image:
          if (!imask_[k]->IsInsideBuffer(pt_k))
          {
            continue;
          }
          
          // make sure the pixel maps into the image mask:
          if (imask_[k]->Evaluate(pt_k) == 0)
          {
            continue;
          }
          
          mask_pixel = 1;
          break;
        }
        
        mosaic_->SetPixel(ix, mask_pixel);
      }
    }
  }
  
  // data members facilitating concurrent mosaic assembly:
  unsigned int thread_offset_;
  unsigned int thread_stride_;
  
  // output mosaic:
  mask_t::Pointer & mosaic_;
  
  // number of tiles used for this mosaic (may be fewer than
  // what's available from the tile vector):
  unsigned int num_tiles_;
  
  // omitted tile flags:
  const std::vector<bool> & omit_;
  
  // tile trasforms:
  const std::vector<transform_pointer_t> & transform_;
  
  // tile masks
  const std::vector<mask_t::ConstPointer> & mask_;
  
  // tile mask interpolators:
  const std::vector<typename imask_t::Pointer> & imask_;
  
  // mosaic space tile bounding boxes:
  const std::vector<pnt_t> & min_;
  const std::vector<pnt_t> & max_;
};


//----------------------------------------------------------------
// make_mask_mt
//
// Assemble a mask for the entire mosaic.
// Individual tiles may be omitted.
// 
template <class transform_pointer_t>
mask_t::Pointer
make_mask_mt(unsigned int num_threads,
             const mask_t::SpacingType & mosaic_sp,
             const unsigned int num_images,
             const std::vector<bool> & omit,
             const std::vector<transform_pointer_t> & transform,
             const std::vector<mask_t::ConstPointer> & image_mask)
{
  if (num_threads == 1)
  {
    // use the old single-threaded code:
    return make_mask_st<transform_pointer_t>(mosaic_sp,
                                             num_images,
                                             omit,
                                             transform,
                                             image_mask);
  }
  
  // FIXME: make it multi-threaded:
  WRAP(itk_terminator_t terminator("make_mask_mt"));
  
  // setup the image mask interpolators:
  typedef itk::NearestNeighborInterpolateImageFunction<mask_t, double> imask_t;
  std::vector<imask_t::Pointer> imask(num_images);
  imask.assign(num_images, imask_t::Pointer(NULL));
  
  for (unsigned int i = 0; i < image_mask.size() && i < num_images; i++)
  {
    if (image_mask[i].GetPointer() == NULL) continue;
    
    imask[i] = imask_t::New();
    imask[i]->SetInputImage(image_mask[i]);
  }
  
  // image space bounding boxes:
  typedef mask_t::PointType pnt_t;
  std::vector<pnt_t> bbox_min(num_images);
  std::vector<pnt_t> bbox_max(num_images);
  calc_image_bboxes<mask_t::ConstPointer>(image_mask, bbox_min, bbox_max);
  
  // mosaic space bounding boxes:
  std::vector<pnt_t> min(num_images);
  std::vector<pnt_t> max(num_images);
  calc_mosaic_bboxes<pnt_t, transform_pointer_t>(transform,
                                                 bbox_min,
                                                 bbox_max,
                                                 min,
                                                 max);
  
  // mosiac bounding box:
  pnt_t mosaic_min;
  pnt_t mosaic_max;
  calc_mosaic_bbox<pnt_t>(min, max, mosaic_min, mosaic_max);
  
  // setup the mosaic image:
  mask_t::Pointer mosaic = mask_t::New();
  mask_t::RegionType::SizeType mosaic_sz;
  mosaic_sz[0] = static_cast<unsigned int>((mosaic_max[0] - mosaic_min[0]) /
                                mosaic_sp[0]);
  mosaic_sz[1] = static_cast<unsigned int>((mosaic_max[1] - mosaic_min[1]) /
                                mosaic_sp[1]);
  mosaic->SetOrigin(pnt2d(mosaic_min[0], mosaic_min[1]));
  mosaic->SetRegions(mosaic_sz);
  mosaic->SetSpacing(mosaic_sp);
  mosaic->Allocate();
  
  // setup transactions for multi-threaded mosaic assembly:
  std::list<the_transaction_t *> schedule;
  for (unsigned int i = 0; i < num_threads; i++)
  {
    assemble_mask_t<transform_pointer_t> * t =
    new assemble_mask_t<transform_pointer_t>(i,
                                             num_threads,
                                             mosaic,
                                             num_images,
                                             omit,
                                             transform,
                                             image_mask,
                                             imask,
                                             min,
                                             max);
    
    schedule.push_back(t);
  }
  
  // setup the thread pool:
  the_thread_pool_t thread_pool(num_threads);
  thread_pool.set_idle_sleep_duration(50); // 50 usec
  thread_pool.push_back(schedule);
  thread_pool.pre_distribute_work();
  
  // execute mosaic assembly transactions:
  suspend_itk_multithreading_t suspend_itk_mt;
  thread_pool.start();
  thread_pool.wait();
  
  // done:
  return mosaic;
}

//----------------------------------------------------------------
// make_mask
// 
template <class transform_pointer_t>
mask_t::Pointer
make_mask(const mask_t::SpacingType & mosaic_sp,
          const unsigned int num_images,
          const std::vector<bool> & omit,
          const std::vector<transform_pointer_t> & transform,
          const std::vector<mask_t::ConstPointer> & image_mask)
{
  // use as many threads as supported by hardware:
  unsigned int num_threads = boost::thread::hardware_concurrency();
  
  return make_mask_mt<transform_pointer_t>(num_threads,
                                           mosaic_sp,
                                           num_images,
                                           omit,
                                           transform,
                                           image_mask);
}

//----------------------------------------------------------------
// make_mask
// 
// Assemble a mask for the entire mosaic.
// 
template <class transform_pointer_t>
mask_t::Pointer
make_mask(const std::vector<transform_pointer_t> & transform,
          const std::vector<mask_t::ConstPointer> & image_mask)
{
  const unsigned int num_images = transform.size();
  return make_mask<transform_pointer_t>(image_mask[0]->GetSpacing(),
                                        num_images,
                                        std::vector<bool>(num_images, false),
                                        transform,
                                        image_mask);
}

//----------------------------------------------------------------
// make_mosaic
// 
// Assemble the entire mosaic.
// 
template <class image_pointer_t, class transform_pointer_t>
typename image_pointer_t::ObjectType::Pointer
make_mosaic(const feathering_t feathering,
            const std::vector<transform_pointer_t> & transform,
            const std::vector<image_pointer_t> & image,
            const std::vector<mask_t::ConstPointer> & image_mask =
            std::vector<mask_t::ConstPointer>(0),
            bool dont_allocate = false)
{
  const unsigned int num_images = transform.size();
  return make_mosaic<image_pointer_t, transform_pointer_t>
  (image[0]->GetSpacing(),
   num_images,
   std::vector<bool>(num_images, false), // omit
   transform,
   image,
   
   // optional image masks:
   image_mask,
   
   // feathering to reduce image blurring is optional:
   feathering,
   0.0,
   dont_allocate);
}

//----------------------------------------------------------------
// warp
//
// Return a copy of a given image warped according to
// a given transform.
// 
template <typename T>
typename T::Pointer
warp(const T * a,
     const base_transform_t * t)
{
  std::vector<typename T::ConstPointer> image(1);
  image[0] = a;
  
  std::vector<base_transform_t::ConstPointer> transform(1);
  transform[0] = t;
  
  return
  make_mosaic<typename T::ConstPointer, base_transform_t::ConstPointer>
  (image,
   transform,
   FEATHER_NONE_E);
}

//----------------------------------------------------------------
// resize
//
// scale = 0.5 ---> reduce image in half along each dimension
// scale = 2.0 ---> double the image size along each dimension
// 
template <typename T>
typename T::Pointer
resize(const T * in, double scale)
{
  typedef itk::ScaleTransform<double, 2> scale_t;
  scale_t::Pointer transform = scale_t::New();
  scale_t::ScaleType s = transform->GetScale();
  s[0] = 1.0 / scale;
  s[1] = 1.0 / scale;
  transform->SetScale(s);
  return warp<T>(in, transform);
}

//----------------------------------------------------------------
// resize
//
// Calculate a scale factor for a given image, such that
// the largest dimension of the image is equal to max_edge_size.
// Return a scaled copy of the image.
// 
template <typename T>
typename T::Pointer
resize(const T * in, unsigned int max_edge_size, double & scale)
{
  typename T::RegionType::SizeType sz =
  in->GetLargestPossibleRegion().GetSize();
  
  double xScale = static_cast<double>(max_edge_size) / static_cast<double>(sz[0]);
  double yScale = static_cast<double>(max_edge_size) / static_cast<double>(sz[1]);
  scale = xScale < yScale ? xScale : yScale;
  return resize<T>(in, scale);
}

//----------------------------------------------------------------
// resize
// 
// Scale a given image such that the largest dimension
// of the image is equal to max_edge_size. 
// Return a scaled copy of the image.
// 
template <typename T>
typename T::Pointer
resize(const T * in, unsigned int max_edge_size)
{
  double scale;
  return resize<T>(in, max_edge_size, scale);
}


//----------------------------------------------------------------
// save_mosaic
//
// Assemble the mosaic and save the mosaic image
// Individual mosaic tiles may be omitted.
// 
template <class image_pointer_t, class transform_pointer_t>
void
save_mosaic(const std::vector<image_pointer_t> & image,
            const std::vector<transform_pointer_t> & transform,
            const bfs::path & filename,
            const unsigned int omit = std::numeric_limits<unsigned int>::max(),
            unsigned int num_images = std::numeric_limits<unsigned int>::max())
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType image_t;
  
  typename image_t::Pointer mosaic =
  make_mosaic<image_pointer_t, transform_pointer_t>(image,
                                                    transform,
                                                    false,
                                                    omit,
                                                    num_images);
  save<image_t>(mosaic, filename);
}

//----------------------------------------------------------------
// align_fi_mi
// 
// Warp images fi and mi. Both warped images will have the same
// dimensions as a mosaic assembled from fi and mi.
// 
template <typename T>
void
align_fi_mi(const T * fi,
            const T * mi,
            const base_transform_t * mi_transform,
            typename T::Pointer & fi_aligned,
            typename T::Pointer & mi_aligned)
{
  std::vector<typename T::ConstPointer> image(2);
  image[0] = fi;
  image[1] = mi;
  
  std::vector<base_transform_t::ConstPointer> transform(2);
  transform[0] = identity_transform_t::New();
  transform[1] = mi_transform;
  
  fi_aligned =
  make_mosaic<typename T::ConstPointer, base_transform_t::ConstPointer>
  (image,
   transform,
   FEATHER_NONE_E,
   1);
  
  mi_aligned =
  make_mosaic<typename T::ConstPointer, base_transform_t::ConstPointer>
  (image,
   transform,
   FEATHER_NONE_E,
   0);
}

//----------------------------------------------------------------
// save_composite
//
// Save an RGB image of images fi and mi registered via
// a given transform. The mi image will be saved in the red channel,
// and fi will be saved in the green and blue channels.
// 
template <typename T>
void
save_composite(const bfs::path & filename,
               const T * fi,
               const T * mi,
               const base_transform_t * xform,
               bool blab = true)
{
  typename T::Pointer fi_aligned;
  typename T::Pointer mi_aligned;
  align_fi_mi(fi, mi, xform, fi_aligned, mi_aligned);
  
  typedef itk::CastImageFilter<T, native_image_t> cast_to_native_t;
  typename cast_to_native_t::Pointer fi_native = cast_to_native_t::New();
  typename cast_to_native_t::Pointer mi_native = cast_to_native_t::New();
  fi_native->SetInput(fi_aligned);
  mi_native->SetInput(mi_aligned);
  
  typedef itk::RGBPixel<native_pixel_t> composite_pixel_t;
  typedef itk::Image<composite_pixel_t, 2> composite_image_t;
  typedef itk::ComposeRGBImageFilter<native_image_t, composite_image_t>
  compose_filter_t;
  
  compose_filter_t::Pointer composer = compose_filter_t::New();
  composer->SetInput1(mi_native->GetOutput());
  composer->SetInput2(fi_native->GetOutput());
  composer->SetInput3(fi_native->GetOutput());
  
  typedef itk::ImageFileWriter<composite_image_t> composite_writer_t;
  composite_writer_t::Pointer writer = composite_writer_t::New();
  writer->SetFileName(filename.string().c_str());
  writer->SetInput(composer->GetOutput());
  
  if (blab) std::cout << "saving " << filename << std::endl;
  writer->Update();
}

//----------------------------------------------------------------
// make_mosaic_rgb
// 
// Assemble a portion of the mosaic positioned at mosaic_min.
// Each tile may be individually tinted with an RGB color.
// Individual tiles may be omitted.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
// 
template <class image_pointer_t, class transform_pointer_t>
void
make_mosaic_rgb
(typename image_pointer_t::ObjectType::Pointer * mosaic,
 const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const typename image_pointer_t::ObjectType::PointType & mosaic_min,
 const typename image_pointer_t::ObjectType::SizeType & mosaic_sz,
 const xyz_t & background_color,
 const std::vector<xyz_t> & color,
 const std::vector<bool> & omit,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E)
{
  WRAP(itk_terminator_t terminator("make_mosaic_rgb"));
  unsigned int num_images = image.size();
  
  for (unsigned int i = 0; i < 3; i++)
  {
    std::vector<double> tint(num_images);
    for (unsigned int j = 0; j < num_images; j++)
    {
      tint[j] = color[j][i] / 255.0;
    }
    
    mosaic[i] =
    make_mosaic<image_pointer_t, transform_pointer_t>
    (mosaic_sp,
     mosaic_min,
     mosaic_sz,
     num_images,
     omit,
     tint,
     transform,
     image,
     image_mask,
     feathering,
     background_color[i]);
  }
}

//----------------------------------------------------------------
// make_mosaic_rgb
// 
// Assemble a portion of the mosaic bounded by
// mosaic_min and mosaic_max.
// 
// Each tile may be individually tinted with an RGB color.
// Individual tiles may be omitted.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
// 
template <class image_pointer_t, class transform_pointer_t>
void
make_mosaic_rgb
(typename image_pointer_t::ObjectType::Pointer * mosaic,
 const typename image_pointer_t::ObjectType::SpacingType & mosaic_sp,
 const typename image_pointer_t::ObjectType::PointType & mosaic_min,
 const typename image_pointer_t::ObjectType::PointType & mosaic_max,
 const xyz_t & background_color,
 const std::vector<xyz_t> & color,
 const std::vector<bool> & omit,
 const std::vector<transform_pointer_t> & transform,
 const std::vector<image_pointer_t> & image,
 
 // optional image masks:
 const std::vector<mask_t::ConstPointer> & image_mask =
 std::vector<mask_t::ConstPointer>(0),
 
 // feathering to reduce image blurring is optional:
 const feathering_t feathering = FEATHER_NONE_E)
{
  typename image_pointer_t::ObjectType::SizeType mosaic_sz;
  mosaic_sz[0] = static_cast<unsigned int>((mosaic_max[0] - mosaic_min[0]) /
                                mosaic_sp[0]);
  mosaic_sz[1] = static_cast<unsigned int>((mosaic_max[1] - mosaic_min[1]) /
                                mosaic_sp[1]);
  
  make_mosaic_rgb<image_pointer_t, transform_pointer_t>
  (mosaic,
   mosaic_sp,
   mosaic_min,
   mosaic_sz,
   background_color,
   color,
   omit,
   transform,
   image,
   image_mask,
   feathering);
}

//----------------------------------------------------------------
// make_mosaic_rgb
// 
// Assemble the entire mosaic.
// Each tile may be individually tinted with an RGB color.
// Individual tiles may be omitted.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
// 
template <class image_pointer_t, class transform_pointer_t>
void
make_mosaic_rgb(typename image_pointer_t::ObjectType::Pointer * mosaic,
                const xyz_t & background_color,
                const std::vector<xyz_t> & color,
                const std::vector<bool> & omit,
                const std::vector<transform_pointer_t> & transform,
                const std::vector<image_pointer_t> & image,
                
                // optional image masks:
                const std::vector<mask_t::ConstPointer> & image_mask =
                std::vector<mask_t::ConstPointer>(0),
                
                // feathering to reduce image blurring is optional:
                const feathering_t feathering = FEATHER_NONE_E)
{
  // mosaic pixel spacing will be the same as for the first mosaic tile:
  typename image_pointer_t::ObjectType::SpacingType mosaic_sp =
  image[0]->GetSpacing();
  
  // mosaic bounding box:
  typename image_pointer_t::ObjectType::PointType mosaic_min;
  typename image_pointer_t::ObjectType::PointType mosaic_max;
  calc_mosaic_bbox<image_pointer_t, transform_pointer_t>
  (transform,
   image,
   mosaic_min,
   mosaic_max);
  
  make_mosaic_rgb<image_pointer_t, transform_pointer_t>
  (mosaic,
   mosaic_sp,
   mosaic_min,
   mosaic_max,
   background_color,
   color,
   omit,
   transform,
   image,
   image_mask,
   feathering);
}

//----------------------------------------------------------------
// make_colors
//
// Generate a scrambled rainbow colormap.
// 
extern void
make_colors(const unsigned int & num_color, std::vector<xyz_t> & color);

//----------------------------------------------------------------
// make_mosaic_rgb
// 
// Assemble the entire mosaic.
// Each tile will be tinted with an RGB color
// from a scrambled rainbox colormap.
// Individual tiles may be omitted.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
// 
template <class image_pointer_t, class transform_pointer_t>
void
make_mosaic_rgb(typename image_pointer_t::ObjectType::Pointer * mosaic,
                const std::vector<bool> & omit,
                const std::vector<transform_pointer_t> & transform,
                const std::vector<image_pointer_t> & image,
                
                // optional image masks:
                const std::vector<mask_t::ConstPointer> & image_mask =
                std::vector<mask_t::ConstPointer>(0),
                
                // feathering to reduce image blurring is optional:
                const feathering_t feathering = FEATHER_NONE_E,
                
                // background color:
                double background = 255.0,
                
                // should the first tile be white?
                bool first_tile_white = false)
{
  static const xyz_t EAST  = xyz(1, 0, 0);
  static const xyz_t NORTH = xyz(0, 1, 0);
  static const xyz_t WEST  = xyz(0, 0, 1);
  static const xyz_t SOUTH = xyz(0, 0, 0);
  
  unsigned int num_images = image.size();
  xyz_t background_color = xyz(background, background, background);
  std::vector<xyz_t> color;
  
  if (first_tile_white)
  {
    std::vector<xyz_t> tmp;
    make_colors(num_images - 1, tmp);
    color.resize(num_images);
    color[0] = xyz(255, 255, 255);
    for (unsigned int i = 1; i < num_images; i++)
    {
      color[i] = tmp[i - 1];
    }
  }
  else
  {
    make_colors(num_images, color);
  }
  
  make_mosaic_rgb<image_pointer_t, transform_pointer_t>
  (mosaic,
   background_color,
   color,
   omit,
   transform,
   image,
   image_mask,
   feathering);
}

//----------------------------------------------------------------
// make_mosaic_rgb
// 
// Assemble the entire mosaic.
// Each tile will be tinted with an RGB color
// from a scrambled rainbox colormap.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
// 
template <class image_pointer_t, class transform_pointer_t>
void
make_mosaic_rgb(typename image_pointer_t::ObjectType::Pointer * mosaic,
                const feathering_t feathering,
                const std::vector<transform_pointer_t> & transform,
                const std::vector<image_pointer_t> & image,
                const std::vector<mask_t::ConstPointer> & image_mask =
                std::vector<mask_t::ConstPointer>(0),
                const double background = 255.0,
                bool first_tile_white = false)
{
  make_mosaic_rgb<image_pointer_t, transform_pointer_t>
  (mosaic,
   std::vector<bool>(transform.size(), false),
   transform,
   image,
   image_mask,
   feathering,
   background,
   first_tile_white);
}

//----------------------------------------------------------------
// to_rgb
//
// Make an RGB image from a given grayscale image
// 
template <typename T>
void
to_rgb(const T * image, native_image_t::Pointer * rgb)
{
  rgb[0] =
  cast<T, native_image_t>(remap_min_max<T>(image, 0.0, 255.0));
  rgb[1] = cast<native_image_t, native_image_t>(rgb[0]);
  rgb[2] = cast<native_image_t, native_image_t>(rgb[0]);
}

//----------------------------------------------------------------
// save_rgb
//
// Save and RGB image specified by the individual color channel images.
// 
template <class image_ptr_t>
void
save_rgb(const image_ptr_t * image, const bfs::path & filename, bool blab = true)
{
  typedef typename image_ptr_t::ObjectType::Pointer::ObjectType image_t;
  
  typedef itk::RGBPixel<native_pixel_t> composite_pixel_t;
  typedef itk::Image<composite_pixel_t, 2> composite_image_t;
  typedef itk::ComposeRGBImageFilter<image_t, composite_image_t>
  compose_filter_t;
  
  typename compose_filter_t::Pointer composer = compose_filter_t::New();
  composer->SetInput1(image[0]);
  composer->SetInput2(image[1]);
  composer->SetInput3(image[2]);
  
  typedef itk::ImageFileWriter<composite_image_t> composite_writer_t;
  typename composite_writer_t::Pointer writer = composite_writer_t::New();
  writer->SetFileName(filename.string().c_str());
  writer->SetInput(composer->GetOutput());
  
  if (blab) std::cout << "saving " << filename << std::endl;
  writer->Update();
}

//----------------------------------------------------------------
// save_mosaic_rgb
// 
// Assemble the entire mosaic.
// Each tile will be tinted with an RGB color
// from a scrambled rainbox colormap.
// Background color (outside the mask) may be specified.
// Tile masks are optional and may be NULL.
//
// Save the resulting mosaic
// 
template <class image_pointer_t, class transform_pointer_t>
void
save_mosaic_rgb(const bfs::path & filename,
                const feathering_t feathering,
                const std::vector<transform_pointer_t> & transform,
                const std::vector<image_pointer_t> & image,
                const std::vector<mask_t::ConstPointer> & image_mask =
                std::vector<mask_t::ConstPointer>(0),
                const double background = 255.0,
                bool first_tile_white = false,
                bool blab = true)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType image_t;
  typename image_t::Pointer mosaic[3];
  make_mosaic_rgb<image_pointer_t, transform_pointer_t>
  (mosaic,
   feathering,
   transform,
   image,
   image_mask,
   background,
   first_tile_white);
  save_rgb<typename image_t::Pointer>(mosaic, filename, blab);
}

//----------------------------------------------------------------
// save_rgb
// 
// Save an RGB image of images fi and mi registered via
// a given transform.
// 
template <typename T>
void
save_rgb(const bfs::path & fn_save,
         const T * fi,
         const T * mi,
         const base_transform_t * xform,
         const mask_t * fi_mask = 0,
         const mask_t * mi_mask = 0,
         bool blab = true)
{
  std::vector<typename T::ConstPointer> image(2);
  std::vector<mask_t::ConstPointer> mask(2);
  std::vector<base_transform_t::ConstPointer> transform(2);
  
  image[0] = fi;
  image[1] = mi;
  
  mask[0] = fi_mask;
  mask[1] = mi_mask;
  
  transform[0] = identity_transform_t::New();
  transform[1] = xform;
  
  typename T::Pointer mosaic[3];
  make_mosaic_rgb(mosaic,
                  std::vector<bool>(2, false),
                  transform,
                  image,
                  mask,
                  FEATHER_NONE_E,
                  255.0,
                  false);
  
  save_rgb<typename T::Pointer>(mosaic, fn_save, blab);
}


//----------------------------------------------------------------
// inverse
// 
// Return an inverse of a given translation transform.
// 
inline static translate_transform_t::Pointer
inverse(const translate_transform_t * transform)
{
  translate_transform_t::Pointer inv = NULL;
  if (transform != NULL)
  {
    inv = translate_transform_t::New();
    inv->SetOffset(neg(transform->GetOffset()));
  }
  
  return inv;
}


//----------------------------------------------------------------
// remap
// 
// Re-arrange a data vector according to a given mapping
// 
template <class data_t>
void
remap(const std::list<unsigned int> & mapping,
      const std::vector<data_t> & in,
      std::vector<data_t> & out)
{
  unsigned int size = mapping.size();
  out.resize(size);
  
  typename std::list<unsigned int>::const_iterator iter = mapping.begin();
  for (unsigned int i = 0; i < size; i++, ++iter)
  {
    out[i] = in[*iter];
  }
}

//----------------------------------------------------------------
// remap
// 
// Re-arrange a data list according to a given mapping
// 
template <class data_t>
void
remap(const std::list<unsigned int> & mapping,
      const std::list<data_t> & in,
      std::list<data_t> & out)
{
  unsigned int size = in.size();
  
  // setup rapid access to the list elements:
  std::vector<const data_t *> rapid(size);
  {
    typename std::list<data_t>::const_iterator iter = in.begin();
    for (unsigned int i = 0; i < size; i++, ++iter)
    {
      rapid[i] = &(*iter);
    }
  }
  
  // swap the list elements:
  out.clear();
  typename std::list<unsigned int>::const_iterator iter = mapping.begin();
  for (; iter != mapping.end(); ++iter)
  {
    out.push_back(*(rapid[*iter]));
  }
}

//----------------------------------------------------------------
// remap
// 
// Return a re-arranged data container according to a given mapping.
// 
template <class container_t>
container_t
remap(const std::list<unsigned int> & mapping,
      const container_t & container)
{
  container_t out;
  remap(mapping, container, out);
  return out;
}


//----------------------------------------------------------------
// mark
// 
// Draw a cross mark in the given image.
// 
template <typename T>
void
mark(typename T::Pointer & image,
     const typename T::IndexType & index,
     const typename T::PixelType & mark_value,
     const int arm_length = 2,
     const char symbol = '+')
{
  typedef typename T::SpacingType spacing_t;
  typedef typename T::RegionType::SizeType image_size_t;
  typedef typename T::IndexType index_t;
  
  image_size_t sz = image->GetLargestPossibleRegion().GetSize();
  index_t xy;
  
  for (int j = -arm_length; j <= arm_length; j++)
  {
    int x = index[0] + j;
    int y = index[1] + j;
    
    if (symbol == '+')
    {
      // draw a cross:
      xy[0] = x;
      xy[1] = index[1];
      if (xy[0] >= 0 && image_size_value_t(xy[0]) < sz[0] &&
          xy[1] >= 0 && image_size_value_t(xy[1]) < sz[1])
      {
        image->SetPixel(xy, mark_value);
      }
      
      xy[0] = index[0];
      xy[1] = y;
      if (xy[0] >= 0 && image_size_value_t(xy[0]) < sz[0] &&
          xy[1] >= 0 && image_size_value_t(xy[1]) < sz[1])
      {
        image->SetPixel(xy, mark_value);
      }
    }
    else
    {
      // draw a diagonal cross:
      xy[0] = x;
      xy[1] = y;
      if (xy[0] >= 0 && image_size_value_t(xy[0]) < sz[0] &&
          xy[1] >= 0 && image_size_value_t(xy[1]) < sz[1])
      {
        image->SetPixel(xy, mark_value);
      }
      
      xy[1] = index[1] - j;
      if (xy[0] >= 0 && image_size_value_t(xy[0]) < sz[0] &&
          xy[1] >= 0 && image_size_value_t(xy[1]) < sz[1])
      {
        image->SetPixel(xy, mark_value);
      }
    }
  }
}

//----------------------------------------------------------------
// mark
// 
// Draw a cross mark in the given image.
// 
template <typename T>
void
mark(typename T::Pointer & image,
     const pnt2d_t & mark_coords,
     const typename T::PixelType & mark_value,
     const int arm_length = 2,
     const char symbol = '+')
{
  typedef typename T::IndexType index_t;
  
  index_t index;
  if (!image->TransformPhysicalPointToIndex(mark_coords, index))
  {
    // the mark lays outside of the image:
    return;
  }
  
  mark<T>(image, index, mark_value, arm_length, symbol);
}

//----------------------------------------------------------------
// fill
// 
// Fill a portion of the image with a constant value.
// 
template <typename T>
void
fill(typename T::Pointer & a,
     unsigned int x,
     unsigned int y,
     unsigned int w,
     unsigned int h,
     const typename T::PixelType & fill_value =
     itk::NumericTraits<typename T::PixelType>::Zero)
{
  WRAP(itk_terminator_t terminator("fill"));
  
  // shortcuts:
  typedef typename T::IndexType  ix_t;
  typedef typename T::RegionType rn_t;
  typedef typename T::SizeType   sz_t;
  
  sz_t sz = a->GetLargestPossibleRegion().GetSize();
  
  ix_t origin;
  origin[0] = x;
  origin[1] = y;
  
  sz_t extent;
  extent[0] = w;
  extent[1] = h;
  
  rn_t region;
  region.SetIndex(origin);
  region.SetSize(extent);
  
  typedef typename itk::ImageRegionIterator<T> iter_t;
  iter_t iter(a, region);
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    ix_t ix = iter.GetIndex();
    if (image_size_value_t(ix[0]) >= sz[0] ||
        image_size_value_t(ix[1]) >= sz[1])
    {
      continue;
    }
    
    iter.Set(fill_value);
  }
}

//----------------------------------------------------------------
// save_transform
// 
// Save an ITK transform to a stream.
// 
extern void
save_transform(std::ostream & so, const itk::TransformBase * t);

//----------------------------------------------------------------
// load_transform
// 
// Load an ITK transform of specified type from a stream.
// 
extern itk::TransformBase::Pointer
load_transform(std::istream & si, const bfs::path & transform_type);

//----------------------------------------------------------------
// load_transform
//
// Load transform type from the stream, then load the transform
// of that type.
// 
extern itk::TransformBase::Pointer
load_transform(std::istream & si);


//----------------------------------------------------------------
// save_mosaic
// 
// Save image filenames and associated ITK transforms to a stream.
// 
extern void
save_mosaic(std::ostream & so,
            const unsigned int & num_images,
            const double & pixel_spacing,
            const bool & use_std_mask,
            const std::vector<bfs::path> & images,
            const std::vector<const itk::TransformBase *>& transform);

//----------------------------------------------------------------
// load_mosaic
//
// Load image filenames and associated ITK transforms from a stream.
// 
extern void
load_mosaic(std::istream & si,
            double & pixel_spacing,
            bool & use_std_mask,
            std::vector<bfs::path> & image,
            std::vector<itk::TransformBase::Pointer> & transform);


//----------------------------------------------------------------
// save_mosaic
// 
// Save tile image names and associated ITK transforms to a stream.
// 
template <class transform_t>
void
save_mosaic(std::ostream & so,
            const double & pixel_spacing,
            const bool & use_std_mask,
            const std::list<bfs::path> & images,
            const std::vector<typename transform_t::Pointer> & transforms)
{
  unsigned int num_images = transforms.size();
  std::vector<bfs::path> image(num_images);
  std::vector<const itk::TransformBase *> tbase(num_images);
  
  std::vector<typename transform_t::Pointer> & ttmp =
  const_cast<std::vector<typename transform_t::Pointer> &>(transforms);
  
  std::list<bfs::path>::const_iterator iter = images.begin();
  for (unsigned int i = 0; i < images.size(); i++, ++iter)
  {
    // TODO: copy to new container needed?
    image[i] = *iter;
    tbase[i] = ttmp[i].GetPointer();
  }
  
  save_mosaic(so,
              image.size(),
              pixel_spacing,
              use_std_mask,
              image,
              tbase);
}

//----------------------------------------------------------------
// load_mosaic
// 
// Load tile image names and associated ITK transforms from a stream.
// 
template <class transform_t>
void
load_mosaic(std::istream & si,
            double & pixel_spacing,
            bool & use_std_mask,
            std::list<bfs::path> & images,
            std::vector<typename transform_t::Pointer> & transforms,
            const bfs::path & image_path)
{
  std::vector<bfs::path> image;
  std::vector<itk::TransformBase::Pointer> tbase;
  
  load_mosaic(si, pixel_spacing, use_std_mask, image, tbase);
  transforms.resize(tbase.size());
  for (unsigned int i = 0; i < tbase.size(); i++)
  {
    images.push_back(image[i]);
    transforms[i] = dynamic_cast<transform_t *>(tbase[i].GetPointer());
  }
  
  if ( image_path.empty() )
    return;
  
  // Replace global paths for backwards compatibility.
  for (std::list<bfs::path>::iterator iter = images.begin();
				 iter != images.end(); ++iter)
  {
    (*iter) = image_path / iter->filename();
std::cerr << (*iter) << std::endl;
//    IRPath::CleanSlashes( (*iter) );
//    (*iter) = IRPath::CleanPath( image_path ) + 
//              IRPath::FilenameFromPath( *iter );
  }
}

//----------------------------------------------------------------
// histogram_equalization
// 
// Histogram Equalization.
// Transformed image will be mapped into the new min/max pixel
// range. If the new range is not specified, pixels will be
// remapped into the original range.
//
template <typename T>
typename T::Pointer
histogram_equalization(const T * in,
                       const unsigned int bins = 256,
                       typename T::PixelType new_min =
                       std::numeric_limits<typename T::PixelType>::max(),
                       typename T::PixelType new_max =
                       -std::numeric_limits<typename T::PixelType>::max(),
                       const mask_t * mask = NULL)
{
  // local typedefs:
  typedef typename T::RegionType rn_t;
  typedef typename T::IndexType  ix_t;
  typedef typename T::SizeType   sz_t;
  typedef typename T::PixelType  pixel_t;
  
  // range of the image intensities:
  pixel_t p_min;
  pixel_t p_max;
  image_min_max<T>(in, p_min, p_max, mask);
  pixel_t p_rng = p_max - p_min;
  
  typename T::Pointer image = cast<T, T>(in);
  
  if (p_rng == pixel_t(0))
  {
    // nothing to do:
    return image;
  }
  
  if (new_min == std::numeric_limits<pixel_t>::max())  new_min = p_min;
  if (new_max == -std::numeric_limits<pixel_t>::max()) new_max = p_max;
  pixel_t new_rng = new_max - new_min;
  
  sz_t sz = in->GetLargestPossibleRegion().GetSize();
  
  // initialize the histogram:
  std::vector<unsigned int> pdf(bins);
  std::vector<double> clipped_pdf(bins);
  std::vector<double> cdf(bins);
  
  for (unsigned int i = 0; i < bins; i++)
  {
    pdf[i] = 0;
    clipped_pdf[i] = 0.0;
    cdf[i] = 0.0;
  }
  
  typedef typename itk::ImageRegionConstIteratorWithIndex<T> iter_t;
  iter_t iter(in, in->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    ix_t ix = iter.GetIndex();
    
    if (pixel_in_mask<T>(in, mask, ix))
    {
      pixel_t p = iter.Get();
      unsigned int bin =
      static_cast<unsigned int>(static_cast<double>(p - p_min) / static_cast<double>(p_rng) *
                     static_cast<double>(bins - 1));
      pdf[bin]++;
    }
  }
  
  // build a cumulative distribution function (CDF):
  cdf[0] = static_cast<double>(pdf[0]);
  for (unsigned int i = 1; i < bins; i++)
  {
    cdf[i] = cdf[i - 1] + static_cast<double>(pdf[i]);
  }
  
  // update the image:
  iter = iter_t(in, in->GetLargestPossibleRegion());
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    ix_t ix = iter.GetIndex();
    
    // generate the output pixel:
    pixel_t p_out = new_min;
    if (pixel_in_mask<T>(in, mask, ix))
    {
      pixel_t p_in = iter.Get();
      unsigned int bin = static_cast<unsigned int>((static_cast<double>(p_in - p_min) /
                                         static_cast<double>(p_rng)) *
                                        static_cast<double>(bins - 1));
      p_out = pixel_t(static_cast<double>(new_min) + static_cast<double>(new_rng) *
                      static_cast<double>(cdf[bin]) / static_cast<double>(cdf[bins - 1]));
    }
    
    image->SetPixel(ix, p_out);
  }
  
  return image;
}


//----------------------------------------------------------------
// clip_histogram
// 
// This is used by CLAHE to limit the contrast ratio slope.
// 
extern void
clip_histogram(const double & clipping_height,
               const unsigned int & pdf_size,
               const unsigned int * pdf,
               double * clipped_pdf);

//----------------------------------------------------------------
// CLAHE
// 
// Perform Contrast-Limited Adaptive Histogram Equalization
// Transformed image will be mapped into the new min/max pixel
// range. If the new range is not specified, pixels will be
// remapped into the original range.
//
template <typename T>
typename T::Pointer
CLAHE(const T * in,
      int nx,
      int ny,
      double max_slope,
      const unsigned int bins = 256,
      typename T::PixelType new_min =
      std::numeric_limits<typename T::PixelType>::max(),
      typename T::PixelType new_max =
      -std::numeric_limits<typename T::PixelType>::max(),
      const mask_t * mask = NULL)
{
  // local typedefs:
  typedef typename T::RegionType rn_t;
  typedef typename T::IndexType  ix_t;
  typedef typename T::SizeType   sz_t;
  typedef typename T::PixelType  pixel_t;
  
  // sanity check:
//  assert(nx > 1 && ny > 1);
  if ( nx <= 1 && ny <= 1 )
  {
    CORE_THROW_EXCEPTION("Window dimensions (nx, ny) fail sanity check");
  }

  //  assert(max_slope >= 1.0 || max_slope == 0.0);
  if ( max_slope < 1.0 && max_slope != 0 ) // TODO: comparison tolerance?
  {
    CORE_THROW_EXCEPTION("Max_slope fails sanity check");
  }

  if (in == 0)
  {
    CORE_THROW_EXCEPTION("Null input image");
  }

  // range of the image intensities:
  pixel_t p_min;
  pixel_t p_max;
  image_min_max<T>(in, p_min, p_max, mask);
  pixel_t p_rng = p_max - p_min;
  
  typename T::Pointer image = cast<T, T>(in);
  
  if (p_rng == pixel_t(0))
  {
    // nothing to do:
    return image;
  }
  
  if (new_min == std::numeric_limits<pixel_t>::max())  new_min = p_min;
  if (new_max == -std::numeric_limits<pixel_t>::max()) new_max = p_max;
  pixel_t new_rng = new_max - new_min;
  
  sz_t sz = in->GetLargestPossibleRegion().GetSize();
  const int image_w = sz[0];
  const int image_h = sz[1];
  
  // make sure the histogram window doesn't exceed the image:
  nx = std::min(nx, image_w);
  ny = std::min(ny, image_h);
  
  // calculate the histogram window center:
  const int cx = nx / 2;
  const int cy = ny / 2;
  
  // initialize the histogram:
  std::vector<unsigned int> pdf(bins);
  std::vector<double> clipped_pdf(bins);
  std::vector<double> cdf(bins);
  
  for (unsigned int i = 0; i < bins; i++)
  {
    pdf[i] = 0;
    clipped_pdf[i] = 0.0;
    cdf[i] = 0.0;
  }
  
  for (int x = 0; x < nx; x++)
  {
    for (int y = 0; y < ny; y++)
    {
      ix_t ix;
      ix[0] = x;
      ix[1] = y;
      
      if (pixel_in_mask<T>(in, mask, ix))
      {
        pixel_t p = in->GetPixel(ix);
        unsigned int bin =
        static_cast<unsigned int>(static_cast<double>(p - p_min) / static_cast<double>(p_rng) *
                       static_cast<double>(bins - 1));
        pdf[bin]++;
      }
    }
  }
  
  // clip the histogram:
  if (max_slope != 0.0)
  {
    double clipping_height =
    (static_cast<double>(nx * ny) / static_cast<double>(bins)) * max_slope;
    clip_histogram(clipping_height, bins, &(pdf[0]), &(clipped_pdf[0]));
  }
  else
  {
    for (unsigned int i = 0; i < bins; i++)
    {
      clipped_pdf[i] = static_cast<double>(pdf[i]);
    }
  }
  
  // build a cumulative distribution function (CDF):
  cdf[0] = static_cast<double>(clipped_pdf[0]);
  for (unsigned int i = 1; i < bins; i++)
  {
    cdf[i] = cdf[i - 1] + static_cast<double>(clipped_pdf[i]);
  }
  
  // start at the origin:
  int hist_x = cx;
  int hist_y = cy;
//#if 1
  for (int x = 0; x < image_w; x++)
  {
    // alternate the direction of the scanline traversal:
    int y0 = (x % 2 == 0) ? 0 : image_h - 1;
    int y1 = (x % 2 == 0) ? image_h - 1 : 0;
    int dy = (x % 2 == 0) ? 1 : -1;
    
    for (int y = y0; y != (y1 + dy); y += dy)
    {
//#else
//      /*
//       for (int y = 0; y < image_h; y++)
//       {
//       // alternate the direction of the scanline traversal:
//       int x0 = (y % 2 == 0) ? 0 : image_w - 1;
//       int x1 = (y % 2 == 0) ? image_w - 1 : 0;
//       int dx = (y % 2 == 0) ? 1 : -1;
//       
//       for (int x = x0; x != (x1 + dx); x += dx)
//       {
//       */
//#endif
      int spill_x0 = std::max(0, cx - x);
      int spill_y0 = std::max(0, cy - y);
      int spill_x1 = std::max(0, x - (image_w - nx + cx));
      int spill_y1 = std::max(0, y - (image_h - ny + cy));
      
      int new_hx = x + spill_x0 - spill_x1;
      int new_hy = y + spill_y0 - spill_y1;
      
      int shift_x = new_hx - hist_x;
      int shift_y = new_hy - hist_y;
      
      if (shift_x != 0)
      {
        // update the histogram (add-remove columns):
//        assert(shift_x == 1 || shift_x == -1);
        if ( shift_x != 1 && shift_x != -1 )
        {
          CORE_THROW_EXCEPTION("bad histogram shift");
        }
        
        for (int ty = 0; ty < ny; ty++)
        {
          ix_t ix;
          ix[1] = hist_y + ty - cy;
          
          // add:
          ix[0] = (shift_x > 0) ? new_hx - cx + nx - 1 : new_hx - cx;
          
          if (pixel_in_mask<T>(in, mask, ix))
          {
            pixel_t a = in->GetPixel(ix);
            unsigned int bin = static_cast<unsigned int>(
              (static_cast<double>(a - p_min) / static_cast<double>(p_rng)) * static_cast<double>(bins - 1));
            pdf[bin]++;
          }
          
          // remove:
          ix[0] = (shift_x > 0) ? hist_x - cx : hist_x - cx + nx - 1;
          if (pixel_in_mask<T>(in, mask, ix))
          {
            pixel_t d = in->GetPixel(ix);
            unsigned int bin = static_cast<unsigned int>(
              (static_cast<double>(d - p_min) / static_cast<double>(p_rng)) * static_cast<double>(bins - 1));
            pdf[bin]--;
          }
        }
        
        hist_x = new_hx;
      }
      
      if (shift_y != 0)
      {
        // update the histogram (add-remove rows):
//        assert(shift_y == 1 || shift_y == -1);
        if ( shift_y != 1 && shift_y != -1 )
        {
          CORE_THROW_EXCEPTION("bad histogram shift");
        }
        
        for (int tx = 0; tx < nx; tx++)
        {
          ix_t ix;
          ix[0] = hist_x + tx - cx;
          
          // add:
          ix[1] = (shift_y > 0) ? new_hy - cy + ny - 1 : new_hy - cy;
          if (pixel_in_mask<T>(in, mask, ix))
          {
            pixel_t a = in->GetPixel(ix);
            unsigned int bin = static_cast<unsigned int>(
              (static_cast<double>(a - p_min) / static_cast<double>(p_rng)) * static_cast<double>(bins - 1));
            pdf[bin]++;
          }
          
          // remove:
          ix[1] = (shift_y > 0) ? hist_y - cy : hist_y - cy + ny - 1;
          if (pixel_in_mask<T>(in, mask, ix))
          {
            pixel_t d = in->GetPixel(ix);
            unsigned int bin = static_cast<unsigned int>(
              (static_cast<double>(d - p_min) / static_cast<double>(p_rng)) * static_cast<double>(bins - 1));
            pdf[bin]--;
          }
        }
        
        hist_y = new_hy;
      }
      
      if (shift_x != 0 || shift_y != 0)
      {
        // clip the histogram:
        if (max_slope != 0.0)
        {
          double clipping_height =
          (static_cast<double>(nx * ny) / static_cast<double>(bins)) * max_slope;
          clip_histogram(clipping_height, bins, &(pdf[0]), &(clipped_pdf[0]));
        }
        else
        {
          for (unsigned int i = 0; i < bins; i++)
          {
            clipped_pdf[i] = static_cast<double>(pdf[i]);
          }
        }
        
        // build a cumulative distribution function (CDF):
        cdf[0] = static_cast<double>(clipped_pdf[0]);
        for (unsigned int i = 1; i < bins; i++)
        {
          cdf[i] = cdf[i - 1] + static_cast<double>(clipped_pdf[i]);
        }
      }
      
      // generate the output pixel:
      ix_t ix;
      ix[0] = x;
      ix[1] = y;
      
      pixel_t p_out = new_min;
      if (pixel_in_mask<T>(in, mask, ix))
      {
        pixel_t p_in = in->GetPixel(ix);
        unsigned int bin = static_cast<unsigned int>(
          (static_cast<double>(p_in - p_min) / static_cast<double>(p_rng)) * static_cast<double>(bins - 1));
        p_out = pixel_t(static_cast<double>(new_min) + static_cast<double>(new_rng) *
                        static_cast<double>(cdf[bin]) / static_cast<double>(cdf[bins - 1]));
      }
      
      image->SetPixel(ix, p_out);
    }
  }
  
  return image;
}


//----------------------------------------------------------------
// median
// 
// Return a copy of a given image de-noised with a median filter.
// 
template <typename T>
typename T::Pointer
median(const T * a, const  unsigned int & median_radius)
{
  typedef itk::MedianImageFilter<T, T> filter_t;
  typename filter_t::Pointer filter = filter_t::New();
  typename filter_t::InputSizeType radius;
  radius[0] = median_radius;
  radius[1] = median_radius;
  filter->SetInput(a);
  filter->SetRadius(radius);
  
  // FIXME:
  // itk::SimpleFilterWatcher w(filter.GetPointer(), "median");
  
  WRAP(terminator_t<filter_t> terminator(filter));
  filter->Update();
  return filter->GetOutput();
}


//----------------------------------------------------------------
// normalize
// 
// Return a copy of the image normalized (pixels shifted and
// rescaled) using the 
// 
template <typename T>
typename T::Pointer
normalize(const T * a, const mask_t * ma)
{
  typedef itk::NormalizeImageFilterWithMask<T, T> filter_t;
  typename filter_t::Pointer filter = filter_t::New();
  filter->SetInput(a);
  filter->SetImageMask(mask_so(ma));
  
  WRAP(terminator_t<filter_t> terminator(filter));
  filter->Update();
  return filter->GetOutput();
}

//----------------------------------------------------------------
// calc_pixel_weight
//
// Calculate a pixel feathering weight used to blend mosaic tiles.
// 
inline double
pixel_weight(const image_t::IndexType & min,
             const image_t::IndexType & max,
             const image_t::IndexType & ix)
{
  double w = static_cast<double>(max[0]) - static_cast<double>(min[0]);
  double h = static_cast<double>(max[1]) - static_cast<double>(min[1]);
  double r = 0.5 * ((w > h) ? h : w);
  double sx = std::min(static_cast<double>(ix[0]) - static_cast<double>(min[0]),
                       static_cast<double>(max[0]) - static_cast<double>(ix[0]));
  double sy = std::min(static_cast<double>(ix[1]) - static_cast<double>(min[1]),
                       static_cast<double>(max[1]) - static_cast<double>(ix[1]));
  double s = (1.0 + std::min(sx, sy)) / (r + 1.0);
  return s;
}

//----------------------------------------------------------------
// normalize
// 
// Tiled image normalization -- mean shift and rescale
// pixel intensities to match a normal distribution.
// Tiles overlap and the overlap regions are blended together.
// Clip the pixels to the [-3, 3] range
// and remap into the new [min, max] range.
// 
template <typename T>
typename T::Pointer
normalize(const T * image,
          const unsigned int cols,
          const unsigned int rows,
          typename T::PixelType new_min =
          std::numeric_limits<typename T::PixelType>::max(),
          typename T::PixelType new_max =
          -std::numeric_limits<typename T::PixelType>::max(),
          const mask_t * mask = NULL)
{
  WRAP(itk_terminator_t terminator("normalize"));
  
  if (new_min == std::numeric_limits<typename T::PixelType>::max() ||
      new_max == -std::numeric_limits<typename T::PixelType>::max())
  {
    // range of the image intensities:
    typename T::PixelType p_min;
    typename T::PixelType p_max;
    image_min_max<T>(image, p_min, p_max, mask);
    
    if (new_min == std::numeric_limits<typename T::PixelType>::max())
    {
      new_min = p_min;
    }
    
    if (new_max == -std::numeric_limits<typename T::PixelType>::max())
    {
      new_max = p_max;
    }
  }
  
  // split the image into a set of overlapping tiles:
  typename T::SizeType sz = image->GetLargestPossibleRegion().GetSize();
  
  double half_tw = static_cast<double>(sz[0]) / static_cast<double>(cols + 1);
  double half_th = static_cast<double>(sz[1]) / static_cast<double>(rows + 1);
  
  typename T::Pointer out = cast<T, T>(image);
  
  array2d(typename T::Pointer) tile;
  resize<typename T::Pointer>(tile, cols, rows);
  
  array2d(mask_t::Pointer) tile_mask;
  resize<mask_t::Pointer>(tile_mask, cols, rows);
  
  array2d(typename T::IndexType) tile_min;
  resize<typename T::IndexType>(tile_min, cols, rows);
  
  array2d(typename T::IndexType) tile_max;
  resize<typename T::IndexType>(tile_max, cols, rows);
  
  for (unsigned int i = 0; i < cols; i++)
  {
    double x0 = static_cast<double>(i) * half_tw;
    unsigned int ix0 = static_cast<unsigned int>(floor(x0));
    double x1 = static_cast<double>(ix0) + (x0 - static_cast<double>(ix0)) + 2.0 * half_tw;
    unsigned int ix1 = std::min(static_cast<unsigned int>(sz[0] - 1),
                                static_cast<unsigned int>(ceil(x1)));
    
    for (unsigned int j = 0; j < rows; j++)
    {
      double y0 = static_cast<double>(j) * half_th;
      unsigned int iy0 = static_cast<unsigned int>(floor(y0));
      double y1 = static_cast<double>(iy0) + (y0 - static_cast<double>(iy0)) + 2.0 * half_th;
      unsigned int iy1 = std::min(static_cast<unsigned int>(sz[1] - 1),
                                  static_cast<unsigned int>(ceil(y1)));
      
      tile_min[i][j][0] = ix0;
      tile_min[i][j][1] = iy0;
      tile_max[i][j][0] = ix1;
      tile_max[i][j][1] = iy1;
      
      tile[i][j] = crop<T>(image, tile_min[i][j], tile_max[i][j]);
      tile_mask[i][j] = crop<mask_t>(mask, tile_min[i][j], tile_max[i][j]);
      
      typename T::Pointer v1 =
      normalize<T>(tile[i][j], tile_mask[i][j]);
      
      typename T::Pointer v2 =
      threshold<T>(v1, -3, 3, -3, 3);
      
      tile[i][j] = v2;
    }
  }
  
  // blend the tiles together:
  typedef itk::ImageRegionIteratorWithIndex<T> itex_t;
  itex_t itex(out, out->GetLargestPossibleRegion());
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    // make sure there hasn't been an interrupt:
    WRAP(terminator.terminate_on_request());
    
    typename T::IndexType ix = itex.GetIndex();
    
    double wsum = 0.0;
    double mass = 0.0;
    for (unsigned int i = 0; i < cols; i++)
    {
      for (unsigned int j = 0; j < rows; j++)
      {
        if (ix[0] >= tile_min[i][j][0] &&
            ix[0] <= tile_max[i][j][0] &&
            ix[1] >= tile_min[i][j][1] &&
            ix[1] <= tile_max[i][j][1])
        {
          typename T::IndexType index;
          index[0] = ix[0] - tile_min[i][j][0];
          index[1] = ix[1] - tile_min[i][j][1];
          
          double weight = pixel_weight(tile_min[i][j], tile_max[i][j], ix);
          wsum += weight * tile[i][j]->GetPixel(index);
          mass += weight;
        }
      }
    }
    
    if (mass != 0.0) wsum /= mass;
    out->SetPixel(ix, wsum);
  }  
  
  remap_min_max_inplace<T>(out, new_min, new_max);
  return out;
}


//----------------------------------------------------------------
// forward_transform
// 
// Cascaded forward transform. This function assumes
// that it is given a vector of forward transforms.
// 
template <typename T, class transform_vec_t>
typename T::PointType
forward_transform(const transform_vec_t & t,
                  const unsigned int t_size,
                  const typename T::PointType & in)
{
  typename T::PointType out = in;
  for (unsigned int i = 0; i < t_size; i++)
  {
    out = t[i]->TransformPoint(out);
  }
  return out;
}

//----------------------------------------------------------------
// inverse_transform
// 
// Cascaded inverse transform. This function assumes
// that it is given a vector of inverse transforms t_inverse,
// it will not call GetInverseTransform on any of them.
// 
template <typename T, class transform_vec_t>
typename T::PointType
inverse_transform(const transform_vec_t & t_inverse,
                  const unsigned int t_size,
                  const typename T::PointType & in)
{
  typename T::PointType out = in;
  for (int i = t_size - 1; i >= 0; i--)
  {
    out = t_inverse[i]->TransformPoint(out);
  }
  return out;
}


//----------------------------------------------------------------
// flip
// 
// Flip an image around vertical and/or horizontal axis.
// 
// flip_x -- flip around the vertical axis
// flip_y -- flip around the horizontal axis.
// 
template <typename T>
typename T::Pointer
flip(const T * in,
     const bool flip_x = true,
     const bool flip_y = false)
{
  WRAP(itk_terminator_t terminator("flip"));
  
  // local typedefs:
  typedef typename T::RegionType rn_t;
  typedef typename T::IndexType  ix_t;
  typedef typename T::SizeType   sz_t;
  
  typename T::Pointer out = cast<T, T>(in);
  
  if (flip_x || flip_y)
  {
    rn_t rn = in->GetLargestPossibleRegion();
    sz_t sz = rn.GetSize();
    
    typedef itk::ImageRegionIteratorWithIndex<T> itex_t;
    itex_t itex(out, rn);
    
    for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
    {
      // make sure there hasn't been an interrupt:
      WRAP(terminator.terminate_on_request());
      
      ix_t ix = itex.GetIndex();
      if (flip_x) ix[0] = sz[0] - ix[0] - 1;
      if (flip_y) ix[1] = sz[1] - ix[1] - 1;
      
      itex.Set(in->GetPixel(ix));
    }
  }
  
  return out;
}


//----------------------------------------------------------------
// overlap_ratio
// 
// Calculate the ratio of the overlap region area to the area
// of the smaller image.
// 
template <typename T>
double
overlap_ratio(const T * fi,
              const T * mi,
              const base_transform_t * fi_to_mi)
{
  if (fi_to_mi == NULL) return 0.0;
  
  typename T::PointType fi_min = fi->GetOrigin();
  typename T::PointType mi_min = mi->GetOrigin();
  
  typename T::SpacingType fi_sp = fi->GetSpacing();
  typename T::SpacingType mi_sp = mi->GetSpacing();
  
  typename T::SizeType fi_sz = fi->GetLargestPossibleRegion().GetSize();
  typename T::SizeType mi_sz = mi->GetLargestPossibleRegion().GetSize();
  
  typename T::PointType fi_max =
  fi_min + vec2d((fi_sz[0]) * fi_sp[0],
                 (fi_sz[1]) * fi_sp[1]);
  
  typename T::PointType mi_max =
  mi_min + vec2d((mi_sz[0]) * mi_sp[0],
                 (mi_sz[1]) * mi_sp[1]);
  
  const double w0 = fi_max[0] - fi_min[0];
  const double h0 = fi_max[1] - fi_min[1];
  
  const double w1 = mi_max[0] - mi_min[0];
  const double h1 = mi_max[1] - mi_min[1];
  
  const double smaller_area = std::min(w0, w1) * std::min(h0, h1);
  
  typename T::PointType ul = fi_to_mi->TransformPoint(fi_min);
  ul[0] = std::max(0.0, std::min(w1, ul[0] - mi_min[0]));
  ul[1] = std::max(0.0, std::min(h1, ul[1] - mi_min[1]));
  
  typename T::PointType lr = fi_to_mi->TransformPoint(fi_max);
  lr[0] = std::max(0.0, std::min(w1, lr[0] - mi_min[0]));
  lr[1] = std::max(0.0, std::min(h1, lr[1] - mi_min[1]));
  
  const double dx = lr[0] - ul[0];
  const double dy = lr[1] - ul[1];
  const double area_ratio = (dx * dy) / smaller_area;
  
  return area_ratio;
}

//----------------------------------------------------------------
// find_inverse
// 
// Given a forward transform and image space coordinates, 
// find mosaic space coordinates.
// 
extern bool
find_inverse(const pnt2d_t & tile_min,        // tile space
             const pnt2d_t & tile_max,        // tile space
             const base_transform_t * mosaic_to_tile, // forward transform
             const pnt2d_t & xy,        // tile space
             pnt2d_t & uv,                // mosaic space
             const unsigned int max_iterations = 100,
             const double min_step_scale = 1e-12,
             const double min_error_sqrd = 1e-16,
             const unsigned int pick_up_pace_steps = 5);

//----------------------------------------------------------------
// find_inverse
// 
// Given a forward transform, an approximate inverse transform,
// and image space coordinates, find mosaic space coordinates.
// 
extern bool
find_inverse(const pnt2d_t & tile_min,        // tile space
             const pnt2d_t & tile_max,        // tile space
             const base_transform_t * mosaic_to_tile, // forward transform
             const base_transform_t * tile_to_mosaic, // inverse transform
             const pnt2d_t & xy,        // tile space
             pnt2d_t & uv,                // mosaic space
             const unsigned int max_iterations = 100,
             const double min_step_scale = 1e-12,
             const double min_error_sqrd = 1e-16,
             const unsigned int pick_up_pace_steps = 5);

//----------------------------------------------------------------
// find_inverse
// 
// Given a forward transform, an approximate inverse transform,
// and image space coordinates, find mosaic space coordinates.
// 
extern bool
find_inverse(const base_transform_t * mosaic_to_tile, // forward transform
             const base_transform_t * tile_to_mosaic, // inverse transform
             const pnt2d_t & xy,        // tile space
             pnt2d_t & uv,                // mosaic space
             const unsigned int max_iterations = 100,
             const double min_step_scale = 1e-12,
             const double min_error_sqrd = 1e-16,
             const unsigned int pick_up_pace_steps = 5);

//----------------------------------------------------------------
// generate_landmarks
// 
// Given a forward transform, generate a set of image space
// coordinates and find their matching mosaic space coordinates.
// 
// version 1 -- uniform jittered sampling over the tile
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
extern bool
generate_landmarks(const pnt2d_t & tile_min,
                   const pnt2d_t & tile_max,
                   const mask_t * tile_mask,
                   const base_transform_t * mosaic_to_tile,
                   const unsigned int samples,
                   std::vector<pnt2d_t> & xy,
                   std::vector<pnt2d_t> & uv,
                   const unsigned int version,
                   const bool refine);

//----------------------------------------------------------------
// generate_landmarks
// 
// Given a forward transform, generate a set of image space
// coordinates and find their matching mosaic space coordinates.
//
// version 1 -- uniform jittered sampling over the tile
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
extern bool
generate_landmarks(const image_t * tile,
                   const mask_t * mask,
                   const base_transform_t * mosaic_to_tile,
                   const unsigned int samples,
                   std::vector<pnt2d_t> & xy,
                   std::vector<pnt2d_t> & uv,
                   const unsigned int version = 1,
                   const bool refine = false);

//----------------------------------------------------------------
// approx_transform
// 
// Given a forward Legendre polynomial transform, solve for
// transform parameters of the inverse Legendre polynomial transform.
// 
// version 1 -- uniform jittered sampling over the tile
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
template <class legendre_transform_t>
typename legendre_transform_t::Pointer
approx_transform(const pnt2d_t & tile_min,
                 const pnt2d_t & tile_max,
                 const mask_t * tile_mask,
                 const base_transform_t * forward,
                 const unsigned int samples = 16,
                 const unsigned int version = 1,
                 const bool refine = false)
{
  base_transform_t::Pointer inverse = forward->GetInverseTransform();
//  assert(inverse.GetPointer() != NULL);
  if (inverse.GetPointer() == NULL)
  {
    CORE_THROW_EXCEPTION("Null inverse transform");
  }
  
  // calculate the shift:
  pnt2d_t center = pnt2d(tile_min[0] + (tile_max[0] - tile_min[0]) / 2.0,
                         tile_min[1] + (tile_max[1] - tile_min[1]) / 2.0);
  vec2d_t shift = center - inverse->TransformPoint(center);
  
  typename legendre_transform_t::Pointer approx =
  setup_transform<legendre_transform_t>(tile_min, tile_max);
  approx->setup_translation(shift[0], shift[1]);
  
  std::vector<pnt2d_t> xy;
  std::vector<pnt2d_t> uv;
  generate_landmarks(tile_min,
                     tile_max,
                     tile_mask,
                     forward,
                     samples,
                     xy,
                     uv,
                     version,
                     refine);
  
  unsigned int order = legendre_transform_t::Degree + 1;
  unsigned int loworder = std::min(2u, order);
  
//#if 1
  approx->solve_for_parameters(0, loworder, uv, xy);
  approx->solve_for_parameters(loworder, order - loworder, uv, xy);
//#else
//  approx->solve_for_parameters(0, order, uv, xy);
//#endif
  
  return approx;
}

//----------------------------------------------------------------
// solve_for_transform
// 
// Given a forward transform and a gross translation vector for
// the inverse mapping, solve for transform parameters of the
// inverse Legendre polynomial transform.
// 
// version 1 -- uniform jittered sampling over the tile
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
template <class legendre_transform_t>
void
solve_for_transform(const pnt2d_t & tile_min,
                    const pnt2d_t & tile_max,
                    const mask_t * tile_mask,
                    const base_transform_t * mosaic_to_tile_0,
                    typename legendre_transform_t::Pointer & mosaic_to_tile_1,
                    const vec2d_t & shift,
                    const unsigned int samples = 16,
                    const unsigned int version = 1,
                    const bool refine = false)
{
  mosaic_to_tile_1->setup_translation(shift[0], shift[1]);
  
//#if 0
//  // FIXME:
//  cerr << "SHIFT:      " << shift << endl
//       << "ROUGH:      " << mosaic_to_tile_1->GetParameters() << endl;
//#endif
  
  std::vector<pnt2d_t> xy;
  std::vector<pnt2d_t> uv;
  generate_landmarks(tile_min,
                     tile_max,
                     tile_mask,
                     mosaic_to_tile_0,
                     samples,
                     xy,
                     uv,
                     version,
                     refine);
  
  unsigned int order = legendre_transform_t::Degree + 1;
  unsigned int loworder = std::min(2u, order);
  
//#if 1
  mosaic_to_tile_1->solve_for_parameters(0, loworder, uv, xy);
  mosaic_to_tile_1->solve_for_parameters(loworder, order - loworder, uv, xy);
//#else
//  mosaic_to_tile_1->solve_for_parameters(0, order, uv, xy);
//#endif
  
//#if 0
//  // FIXME:
//  cerr << "FINAL: " << mosaic_to_tile_1->GetParameters() << endl;
//#endif
}

//----------------------------------------------------------------
// solve_for_transform
// 
// Given a forward transform and a gross translation vector for
// the inverse mapping, solve for transform parameters of the
// inverse Legendre polynomial transform.
// 
// version 1 -- uniform jittered sampling over the tile
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
template <typename T, class legendre_transform_t>
void
solve_for_transform(const T * tile,
                    const mask_t * mask,
                    const base_transform_t * mosaic_to_tile_0,
                    typename legendre_transform_t::Pointer & mosaic_to_tile_1,
                    const vec2d_t & shift,
                    const unsigned int samples = 16,
                    const unsigned int version = 1,
                    const bool refine = false)
{
  typename T::SpacingType sp = tile->GetSpacing();
  typename T::SizeType sz = tile->GetLargestPossibleRegion().GetSize();
  const pnt2d_t min = tile->GetOrigin();
  const pnt2d_t max = min + vec2d(sp[0] * static_cast<double>(sz[0]),
                                  sp[1] * static_cast<double>(sz[1]));
  
  solve_for_transform<legendre_transform_t>(min,
                                            max,
                                            mask,
                                            mosaic_to_tile_0,
                                            mosaic_to_tile_1,
                                            shift,
                                            samples,
                                            version,
                                            refine);
}

//----------------------------------------------------------------
// solve_for_transform
// 
// Given a forward transform, solve for transform parameters of
// the inverse Legendre polynomial transform.
// 
// version 1 -- uniform jittered sampling over the tile
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
template <class legendre_transform_t>
void
solve_for_transform(const pnt2d_t & tile_min,
                    const pnt2d_t & tile_max,
                    const mask_t * tile_mask,
                    const base_transform_t * mosaic_to_tile_0,
                    typename legendre_transform_t::Pointer & mosaic_to_tile_1,
                    const unsigned int samples = 16,
                    const unsigned int version = 1,
                    const bool refine = false)
{
  base_transform_t::Pointer tile_to_mosaic = mosaic_to_tile_0->GetInverseTransform();
//  assert(tile_to_mosaic.GetPointer() != NULL);
  if (tile_to_mosaic.GetPointer() == NULL)
  {
    CORE_THROW_EXCEPTION("Null inverse transform");
  }
  
  // calculate the shift:
  pnt2d_t center = pnt2d(tile_min[0] + (tile_max[0] - tile_min[0]) / 2.0,
                         tile_min[1] + (tile_max[1] - tile_min[1]) / 2.0);
  vec2d_t shift = center - tile_to_mosaic->TransformPoint(center);
  
  solve_for_transform<legendre_transform_t>(tile_min,
                                            tile_max,
                                            tile_mask,
                                            mosaic_to_tile_0,
                                            mosaic_to_tile_1,
                                            shift,
                                            samples,
                                            version,
                                            refine);
}

//----------------------------------------------------------------
// solve_for_transform
//
// Given a forward transform, solve for transform parameters of
// the inverse Legendre polynomial transform.
// 
// version 1 -- uniform jittered sampling over the tile
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
template <typename T, class legendre_transform_t>
void
solve_for_transform(const T * tile,
                    const mask_t * mask,
                    const base_transform_t * mosaic_to_tile_0,
                    typename legendre_transform_t::Pointer & mosaic_to_tile_1,
                    const unsigned int samples = 16,
                    const unsigned int version = 1,
                    const bool refine = false)
{
  base_transform_t::Pointer tile_to_mosaic = mosaic_to_tile_0->GetInverseTransform();
//  assert(tile_to_mosaic.GetPointer() != NULL);
  if (tile_to_mosaic.GetPointer() == NULL)
  {
    CORE_THROW_EXCEPTION("Null inverse transform");
  }
  
  typename T::SizeType sz = tile->GetLargestPossibleRegion().GetSize();
  typename T::SpacingType sp = tile->GetSpacing();
  pnt2d_t tile_min = tile->GetOrigin();
  pnt2d_t tile_max;
  tile_max[0] = tile_min[0] + sp[0] * static_cast<double>(sz[0]);
  tile_max[1] = tile_min[1] + sp[1] * static_cast<double>(sz[1]);
  
  return solve_for_transform<legendre_transform_t>(tile_min,
                                                   tile_max,
                                                   mask,
                                                   mosaic_to_tile_0,
                                                   mosaic_to_tile_1,
                                                   samples,
                                                   version,
                                                   refine);
}


//----------------------------------------------------------------
// std_mask
// 
// Given image dimensions, generate a mask image.
// use_std_mask -- the standard mask for the Robert Marc Lab data.
// 
extern mask_t::Pointer
std_mask(const itk::Point<double, 2> & origin,
         const itk::Vector<double, 2> & spacing,
         const itk::Size<2> & sz,
         bool use_std_mask = true);

//----------------------------------------------------------------
// std_mask
// 
// Given an image, generate a matching mask image.
// use_std_mask -- the standard mask for the Robert Marc Lab data.
// 
template <typename T>
mask_t::Pointer
std_mask(const T * tile, bool use_std_mask = true)
{
  return std_mask(tile->GetOrigin(),
                  tile->GetSpacing(),
                  tile->GetLargestPossibleRegion().GetSize(),
                  use_std_mask);
}

//----------------------------------------------------------------
// std_tile
//
// Load a mosaic tile image, reset the origin to zero, set pixel
// spacing as specified. Downscale the image according to the
// shrink factor. If clahe_slope is greater than 1 then process
// the image with the CLAHE algorithm.
// 
template <typename T>
typename T::Pointer
std_tile(const bfs::path& fn_image,
         const unsigned int & shrink_factor,
         const double & pixel_spacing,
         const double & clahe_slope,
         const unsigned int & clahe_window,
         const bool & blab)
{
  typename T::Pointer image = load<T>(fn_image, blab);
  
  // reset the tile image origin and spacing:
  typename T::PointType origin = image->GetOrigin();
  origin[0] = 0;
  origin[1] = 0;
  image->SetOrigin(origin);
  
  typename T::SpacingType spacing = image->GetSpacing();
  spacing[0] = 1;
  spacing[1] = 1;
  image->SetSpacing(spacing);
  
  // don't blur the images unnecessarily:
  if (shrink_factor > 1)
  {
    image = shrink<T>(image, shrink_factor);
  }
  
  typename T::SpacingType sp = image->GetSpacing();
  
  sp[0] *= pixel_spacing;
  sp[1] *= pixel_spacing;
  image->SetSpacing(sp);
  
  if (clahe_slope > 1.0)
  {
    image = CLAHE<T>(image,
                     clahe_window,
                     clahe_window,
                     clahe_slope,
                     256,        // histogram bins
                     0,                // new min
                     1);        // new max
  }
  
  return image;
}

//----------------------------------------------------------------
// std_tile
// 
// Load a mosaic tile image, reset the origin to zero, set pixel
// spacing as specified. Downscale the image according to the
// shrink factor.
// 
template <typename T>
typename T::Pointer
std_tile(const bfs::path& fn_image,
         const unsigned int & shrink_factor,
         const double & pixel_spacing,
         bool blab)
{
  return std_tile<T>(fn_image,
                     shrink_factor,
                     pixel_spacing,
                     1.0, // clahe slope
                     0,   // clahe window size
                     blab);
}

//----------------------------------------------------------------
// save_volume_slice
//
// Save a volume slice transform. This is used by ir-stom-approx
// to generate the .stom files.
// 
template <class transform_t>
void
save_volume_slice(std::ostream & so,
                  const unsigned int & index,
                  const bfs::path & image,
                  const double & sp_x,
                  const double & sp_y,
                  const transform_t * transform,
                  const pnt2d_t & overall_min,
                  const pnt2d_t & overall_max,
                  const pnt2d_t & slice_min,
                  const pnt2d_t & slice_max,
                  const bool & flip)
{
  std::ios::fmtflags old_flags = so.setf(std::ios::scientific);
  int old_precision = so.precision();
  so.precision(12);
  
  so << "index: " << index << std::endl
  << "overall_min: " << overall_min[0] << ' ' << overall_min[1] << std::endl
  << "overall_max: " << overall_max[0] << ' ' << overall_max[1] << std::endl
  << "slice_min: " << slice_min[0] << ' ' << slice_min[1] << std::endl
  << "slice_max: " << slice_max[0] << ' ' << slice_max[1] << std::endl
  << "flip: " << flip << std::endl
  << "sp: " << sp_x << ' ' << sp_y << std::endl
  << "image:" << std::endl
  << image << std::endl;
  save_transform(so, transform);
  so << std::endl;
  
  so.setf(old_flags);
  so.precision(old_precision);
}

//----------------------------------------------------------------
// load_volume_slice
// 
// Load a volume slice transform. This is used by ir-slice
// to assemble a volume slice image from a given .stom file.
// 
template <class transform_t>
void
load_volume_slice(std::istream & si,
                  unsigned int & index,
                  bfs::path & image,
                  double & sp_x,
                  double & sp_y,
                  typename transform_t::Pointer & transform,
                  pnt2d_t & overall_min,
                  pnt2d_t & overall_max,
                  pnt2d_t & slice_min,
                  pnt2d_t & slice_max,
                  bool & flip)
{
  while (true)
  {
    std::string token;
    si >> token;
    if (si.eof() && token.size() == 0) break;
    
    if (token == "index:")
    {
      si >> index;
    }
    else if (token == "overall_min:")
    {
      si >> overall_min[0] >> overall_min[1];
    }
    else if (token == "overall_max:")
    {
      si >> overall_max[0] >> overall_max[1];
    }
    else if (token == "slice_min:")
    {
      si >> slice_min[0] >> slice_min[1];
    }
    else if (token == "slice_max:")
    {
      si >> slice_max[0] >> slice_max[1];
    }
    else if (token == "flip:")
    {
      si >> flip;
    }
    else if (token == "sp:")
    {
      si >> sp_x >> sp_y;
    }
    else if (token == "image:")
    {
      image.clear();
      while (image.empty())
      {
        std::string line;
        std::getline(si, line);
        image = line;
        if (! bfs::exists(image))
        {
          std::ostringstream oss;
          oss << "load_volume_slice cannot find " << image.string();
          CORE_LOG_WARNING(oss.str());
        }
      }
      
      itk::TransformBase::Pointer t_base = load_transform(si);
      transform = dynamic_cast<transform_t *>(t_base.GetPointer());
//      assert(transform.GetPointer() != NULL);
      if (transform.GetPointer() == NULL)
      {
        CORE_THROW_EXCEPTION("Null transform");
      }
    }
    else
    {
      std::cerr << "WARNING: unknown token: '" << token << "', ignoring ..."
      << std::endl;
    }
  }
}

//----------------------------------------------------------------
// calc_size
// 
// Given a bounding box expressed in the image space and
// pixel spacing, calculate corresponding image size.
// 
extern image_t::SizeType
calc_size(const pnt2d_t & min,
          const pnt2d_t & max,
          const double & spacing);

//----------------------------------------------------------------
// track_progress
// 
// Sets the current progress for determining the overall percentage
// 
inline static void
track_progress(bool track)
{
  if ( track )
    std::cout << "Tracking progress..." << std::endl;
  else
    std::cout << "Progress tracking disabled..." << std::endl;
}

//----------------------------------------------------------------
// set_major_progress
// 
// Sets the current progress for determining the overall percentage
// 
inline static void
set_major_progress(double major_percent)
{
  // TODO: Draw out a nifty bar instead.  This would be easier to read, and
  // better understood.
  std::cout << "Tool Percentage: " << major_percent << std::endl;
}

//----------------------------------------------------------------
// set_minor_progress
// 
// Sets the current progress for determining the percentage of this
// particular task.
// 
inline static void
set_minor_progress(double minor_percent, double major_percent)
{
  // TODO: Draw out a nifty bar instead.  This would be easier to read, and
  // better understood.
  std::cout << "Task Percentage: " << minor_percent << " until " << major_percent << std::endl;
}

#endif // COMMON_HXX_
