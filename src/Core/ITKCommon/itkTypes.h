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

#ifndef CORE_ITKCOMMON_ITKTYPES_H
#define CORE_ITKCOMMON_ITKTYPES_H

#include <itkImage.h>
#include <itkPoint.h>
#include <itkTransform.h>
#include <itkIdentityTransform.h>
#include <itkTranslationTransform.h>
#include <itkVector.h>

//TODO: namespace!!!

//----------------------------------------------------------------
// pixel_t
//
// All-encompasing pixel type -- float.
// Works everywhere, takes up 4 times as much memory as 8-bit pixels.
// 
typedef float pixel_t;

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

#endif