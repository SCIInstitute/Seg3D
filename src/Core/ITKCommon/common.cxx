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

// File         : common.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2005/03/24 16:53
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for mosaicing, image warping,
//                image preprocessing, convenience wrappers for
//                ITK file and ITK filters.

// local includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Transform/itkGridTransform.h>
#include <Core/ITKCommon/Transform/itkMeshTransform.h>
#include <Core/ITKCommon/Transform/itkRBFTransform.h>
#include <Core/ITKCommon/Transform/itkRadialDistortionTransform.h>

// ITK includes:
#include <itkTransformFactoryBase.h>
#include <itkTransformFactory.h>
#include <itkFixedCenterOfRotationAffineTransform.h>

#include <Core/Utils/Exception.h>

// system includes:
#include <iomanip>


//----------------------------------------------------------------
// BREAK
//
int BREAK(unsigned int i)
{
  printf("\n\n\nBREAK BREAK BREAK BREAK BREAK BREAK BREAK: %i\n\n\n\n", i);
  return 0;
}

//----------------------------------------------------------------
// register_transforms
//
static void
register_transforms()
{
  // try to avoid re-registering the same transforms over and over again:
  static bool transforms_registered = false;
  if (transforms_registered) return;
  
  // make sure the transforms I care about are known to the object factory:
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  
  itk::TransformFactory<itk::IdentityTransform<double, 2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::TranslationTransform<double, 2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::FixedCenterOfRotationAffineTransform<double,2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 1> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 3> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 4> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 5> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::GridTransform>::
  RegisterTransform();
  
  itk::TransformFactory<itk::MeshTransform>::
  RegisterTransform();
  
  itk::TransformFactory< itk::RBFTransform>::
  RegisterTransform();
  
  itk::TransformFactory< itk::RadialDistortionTransform<double, 2> >::
  RegisterTransform();
  
  transforms_registered = true;
}

//----------------------------------------------------------------
// save_transform
//
void
save_transform(std::ostream & so, const itk::TransformBase * t)
{
  typedef itk::TransformBase::ParametersType params_t;
  
  // make sure the transforms I care about are known to the object factory:
  register_transforms();
  
  // save the transform type:
  so << t->GetTransformTypeAsString();
  
  // get the stream precision:
  int prec = so.precision();
  
  // save the variable parameters:
  params_t vp = t->GetParameters();
  const unsigned int num_vp = vp.size();
  so << " vp " << num_vp;
  for (unsigned int i = 0; i < num_vp; i++)
  {
    so << ' ' << std::setw(prec + 7) << vp[i];
  }
  
  // save the fixed parameters:
  try
  {
    params_t fp = t->GetFixedParameters();
    
    const unsigned int num_fp = fp.size();
    so << " fp " << num_fp;
    for (unsigned int i = 0; i < num_fp; i++)
    {
      so << ' ' << std::setw(prec + 7) << fp[i];
    }
  }
  catch (itk::ExceptionObject &)
  {
    so << " no_fp";
  }
}

//----------------------------------------------------------------
// load_transform
//
// Load an ITK transform of specified type from a stream.
// 
itk::TransformBase::Pointer
load_transform(std::istream & si, const std::string & transform_type)
{
  typedef itk::TransformBase::ParametersType params_t;
  
  // make sure the transforms I care about are known to the object factory:
  register_transforms();
  
  //  // FIXME:
  //#if 0
  //  std::list<std::string> names =
  //  itk::TransformFactoryBase::GetFactory()->GetClassOverrideWithNames();
  //  for (std::list<std::string>::iterator it = names.begin();
  //       it != names.end();
  //       ++it)
  //  {
  //    std::cerr << "FIXME: " << *it << std::endl;
  //  }
  //#endif
  
  itk::LightObject::Pointer tmp =
  itk::ObjectFactoryBase::CreateInstance(transform_type.c_str());
  
  itk::TransformBase::Pointer t =
  dynamic_cast<itk::TransformBase *>(tmp.GetPointer());
  
  if (t.GetPointer() == NULL)
  {
    std::cerr << "could not instantiate " << transform_type
    << ", giving up ..." << std::endl;
    return t;
  }
  t->Register();
  
  // load the variable parameters:
  params_t vp;
  std::string vp_token;
  si >> vp_token;
  if (vp_token != "vp")
  {
    std::ostringstream oss;
    oss << "expected vp, received '" << vp_token << "', aborting ..." ;
    CORE_LOG_ERROR(oss.str());
    //    assert(false);
    CORE_THROW_EXCEPTION(oss.str());
  }
  
  unsigned int num_vp = 0;
  si >> num_vp;
  vp.SetSize(num_vp);
  
  for (unsigned int i = 0; i < num_vp; i++)
  {
#ifdef __APPLE__
    // NOTE: OSX std::iostream is broken -- it can write out doubles
    // so small that it can't read them back in.  The workaround is
    // to read them in as long doubles, and then cast them down to
    // a double:
    long double tmp = 0;
    si >> tmp;
    if (si.fail()) si.clear();
    vp[i] = static_cast<double>(tmp);
#else
    si >> vp[i];
#endif
  }
  
  // load the fixed parameters:
  params_t fp;
  std::string fp_token;
  si >> fp_token;
  if (fp_token == "fp")
  {
    fp = t->GetFixedParameters();
    unsigned int num_fp = 0;
    si >> num_fp;
    
    if (num_fp != fp.size())
    {
      // some transforms (RBF) have variable number of fixed parameters:
      fp.SetSize(num_fp);
    }
    
    for (unsigned int i = 0; i < num_fp; i++)
    {
#ifdef __APPLE__
      // NOTE: OSX std::iostream is broken -- it can write out doubles
      // so small that it can't read them back in.  The workaround is
      // to read them in as long doubles, and then cast them down to
      // a double:
      long double tmp = 0;
      si >> tmp;
      if (si.fail()) si.clear();
      fp[i] = static_cast<double>(tmp);
#else
      si >> fp[i];
#endif
    }
  }
  else if (fp_token != "no_fp")
  {
    std::ostringstream oss;
    oss << "unexpected token: '" << fp_token << "', aborting load transform...";
    CORE_LOG_ERROR(oss.str());
    //    assert(false);
    CORE_THROW_EXCEPTION(oss.str());
  }
  
  // set the fixed parameters first:
  try
  {
    t->SetFixedParameters(fp);
  }
  catch (itk::ExceptionObject &)
  {}
  
  // set the variable parameters:
  t->SetParametersByValue(vp);
  
  return t;
}

//----------------------------------------------------------------
// load_transform
// 
itk::TransformBase::Pointer
load_transform(std::istream & si)
{
  // load the transform type:
  std::string transform_type;
  si >> transform_type;
  
  return load_transform(si, transform_type);
}


//----------------------------------------------------------------
// save_mosaic
//
// Save image filenames and associated ITK transforms to a stream.
// 
void
save_mosaic(std::ostream & so,
            const unsigned int & num_images,
            const double & pixel_spacing,
            const bool & use_std_mask,
            const std::vector<bfs::path> & images,
            const std::vector<const itk::TransformBase *>& transform)
{
  std::ios::fmtflags old_flags = so.setf(std::ios::scientific);
  int old_precision = so.precision();
  so.precision(12);
  
  so << "format_version_number: " << 1 << std::endl
  << "number_of_images: " << num_images << std::endl
  << "pixel_spacing: " << pixel_spacing << std::endl
  << "use_std_mask: " << use_std_mask << std::endl;
  
  for (unsigned int i = 0; i < num_images; i++)
  {
    // Find just the filename to save out to the mosaic.
    so << "image:" << std::endl << images[i] << std::endl;
    save_transform(so, transform[i]);
    so << std::endl;
  }
  
  so.setf(old_flags);
  so.precision(old_precision);
}

//----------------------------------------------------------------
// load_mosaic
// 
// Load image filenames and associated ITK transforms from a stream.
// 
void
load_mosaic(std::istream & si,
            double & pixel_spacing,
            bool & use_std_mask,
            std::vector<bfs::path> & image,
            std::vector<itk::TransformBase::Pointer> & transform)
{
  // make sure the transforms I care about are known to the object factory:
  register_transforms();
  
  unsigned int num_images = ~0;
  unsigned int i = 0;
  unsigned int version = 0;
  
  // for backwards compatibility assume the standard mask is used:
  use_std_mask = true;
  
  while (si.eof() == false && num_images != i)
  {
    std::string token;
    si >> token;
    
    if (token == "format_version_number:")
    {
      si >> version;
    }
    else if (token == "number_of_images:")
    {
      si >> num_images;
      image.resize(num_images);
      transform.resize(num_images);
    }
    else if (token == "pixel_spacing:")
    {
      si >> pixel_spacing;
    }
    else if (token == "use_std_mask:")
    {
      si >> use_std_mask;
    }
    else if (token == "image:")
    {
      if (version == 0)
      {
        si >> image[i];
      }
      else
      {
        si >> std::ws;
        std::string line;
        std::getline(si, line);
        image[i] = line;
        if (! bfs::exists(image[i]))
        {
          std::ostringstream oss;
          oss << "load_volume_slice cannot find " << image[i].string();
          CORE_LOG_WARNING(oss.str());
        }
      }
      
      // the next token should be the transform type string:
      std::string next_token;
      si >> next_token;
      
      if (version == 0)
      {
        // Original .mosaic file format kept the filename
        // and the transform type string on the same line,
        // which made filenames with spaces difficult to handle:
        
        while (si.eof() == false)
        {
          itk::LightObject::Pointer tmp =
          itk::ObjectFactoryBase::CreateInstance(next_token.c_str());
          
          if (tmp.GetPointer())
          {
            break;
          }
          
          // NOTE: this is a dirty hack to work around filenames with
          // spaces problem in the original file format.
          
          // assume the string that was just read in is a part of filename:
//          image[i] += the_text_t(" ");
//          image[i] += the_text_t(next_token.c_str());
          image[i] += next_token;
          si >> next_token;
        }
      }
      
      transform[i] = load_transform(si, next_token);
      i++;
    }
    else
    {
      std::cerr << "WARNING: unknown token: '" << token << "', ignoring ..." << std::endl;
    }
  }
}

//----------------------------------------------------------------
// is_empty_bbox
// 
// Test whether a bounding box is empty (min > max)
// 
bool
is_empty_bbox(const pnt2d_t & min,
              const pnt2d_t & max)
{
  return min[0] > max[0] || min[1] > max[1];
}

//----------------------------------------------------------------
// is_singular_bbox
// 
// Test whether a bounding box is singular (min == max)
// 
bool
is_singular_bbox(const pnt2d_t & min,
                 const pnt2d_t & max)
{
  return min == max;
}

//----------------------------------------------------------------
// clamp_bbox
// 
// Restrict a bounding box to be within given limits.
// 
void
clamp_bbox(const pnt2d_t & confines_min,
           const pnt2d_t & confines_max,
           pnt2d_t & min,
           pnt2d_t & max)
{
  if (!is_empty_bbox(confines_min, confines_max))
  {
    min[0] = std::min(confines_max[0], std::max(confines_min[0], min[0]));
    min[1] = std::min(confines_max[1], std::max(confines_min[1], min[1]));
    
    max[0] = std::min(confines_max[0], std::max(confines_min[0], max[0]));
    max[1] = std::min(confines_max[1], std::max(confines_min[1], max[1]));
  }
}


//----------------------------------------------------------------
// clip_histogram
// 
// This is used by CLAHE to limit the contrast ratio slope.
// 
void
clip_histogram(const double & clipping_height,
               const unsigned int & pdf_size,
               const unsigned int * pdf,
               double * clipped_pdf)
{
  // count the number of pixels exceeding the clipping height:
  double excess = double(0);
  double deficit = double(0);
  for (unsigned int i = 0; i < pdf_size; i++)
  {
    if (double(pdf[i]) <= clipping_height)
    {
      deficit += clipping_height - double(pdf[i]);
      clipped_pdf[i] = double(pdf[i]);
    }
    else
    {
      excess += double(pdf[i]) - clipping_height;
      clipped_pdf[i] = clipping_height;
    }
  }
  
  const double height_increment = excess / deficit;
  for (unsigned int i = 0; i < pdf_size; i++)
  {
    if (clipped_pdf[i] >= clipping_height) continue;
    
    double d = clipping_height - clipped_pdf[i];
    clipped_pdf[i] += d * height_increment;
  }
}

//----------------------------------------------------------------
// std_mask
// 
// Given image dimensions, generate a mask image.
// use_std_mask -- the standard mask for the Robert Marc Lab data.
// 
mask_t::Pointer
std_mask(const itk::Point<double, 2> & origin,
         const itk::Vector<double, 2> & spacing,
         const itk::Size<2> & sz,
         bool use_std_mask)
{
  // setup the mask:
  mask_t::Pointer mask = make_image<mask_t>(spacing, sz, 255);
  mask->SetOrigin(origin);
  
  if (use_std_mask)
  {
    unsigned int roi_x = 0;
    unsigned int roi_y = (unsigned int)(0.97 * sz[1]);
    unsigned int roi_w = (unsigned int)(0.3025 * sz[0]);
    unsigned int roi_h = sz[1] - roi_y;
    ::fill<mask_t>(mask, roi_x, roi_y, roi_w, roi_h, 0);
    
    /*
     roi_y = (unsigned int)(0.98 * sz[1]);
     roi_w = sz[0];
     roi_h = sz[1] - roi_y;
     fill<mask_t>(mask, roi_x, roi_y, roi_w, roi_h, 0);
     */
  }
  
  return mask;
}


//----------------------------------------------------------------
// to_wcs
//
inline static vec2d_t to_wcs(const vec2d_t & u_axis,
                             const vec2d_t & v_axis,
                             const double & u,
                             const double & v)
{
  return u_axis * u + v_axis * v;
}


//----------------------------------------------------------------
// to_wcs
//
inline static pnt2d_t to_wcs(const pnt2d_t & origin,
                             const vec2d_t & u_axis,
                             const vec2d_t & v_axis,
                             const double & u,
                             const double & v)
{
  return origin + to_wcs(u_axis, v_axis, u, v);
}

//----------------------------------------------------------------
// calc_tile_mosaic_bbox
//
bool
calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
                      
                      // image space bounding boxes of the tile:
                      const pnt2d_t & tile_min,
                      const pnt2d_t & tile_max,
                      
                      // mosaic space bounding boxes of the tile:
                      pnt2d_t & mosaic_min,
                      pnt2d_t & mosaic_max,
                      
                      // sample points along the image edges:
                      const unsigned int np)
{
  // initialize an empty bounding box:
  mosaic_min[0] = std::numeric_limits<double>::max();
  mosaic_min[1] = mosaic_min[0];
  mosaic_max[0] = -mosaic_min[0];
  mosaic_max[1] = -mosaic_min[0];
  
  // it happens:
  if (tile_min[0] == std::numeric_limits<double>::max() || !mosaic_to_tile)
  {
    return true;
  }
  
  base_transform_t::Pointer tile_to_mosaic = mosaic_to_tile->GetInverseTransform();
  if (tile_to_mosaic.GetPointer() == NULL)
  {
    return false;
  }
  
  double W = tile_max[0] - tile_min[0];
  double H = tile_max[1] - tile_min[1];
  
  // a temporary vector to hold the sample points:
  std::vector<pnt2d_t> xy((np + 1) * 4);
  
  // corner points:
  xy[0] = pnt2d(tile_min[0], tile_min[1]);
  xy[1] = pnt2d(tile_min[0], tile_max[1]);
  xy[2] = pnt2d(tile_max[0], tile_min[1]);
  xy[3] = pnt2d(tile_max[0], tile_max[1]);
  
  // edge points:
  for (unsigned int j = 0; j < np; j++)
  {
    const double t = double(j + 1) / double(np + 1);
    double x = tile_min[0] + t * W;
    double y = tile_min[1] + t * H;
    
    unsigned int offset = (j + 1) * 4;
    xy[offset + 0] = pnt2d(x, tile_min[1]);
    xy[offset + 1] = pnt2d(x, tile_max[1]);
    xy[offset + 2] = pnt2d(tile_min[0], y);
    xy[offset + 3] = pnt2d(tile_max[0], y);
  }
  
  // find the inverse mapping for each point, if possible:
  std::list<pnt2d_t> uv_list;
  for (unsigned int j = 0; j < xy.size(); j++)
  {
    pnt2d_t uv;
    if (find_inverse(tile_min,
                     tile_max,
                     mosaic_to_tile,
                     tile_to_mosaic.GetPointer(),
                     xy[j],
                     uv))
    {
      uv_list.push_back(uv);
    }
  }
  
  // calculate the bounding box of the inverse-mapped points:
  for (std::list<pnt2d_t>::const_iterator iter = uv_list.begin();
       iter != uv_list.end(); ++iter)
  {
    const pnt2d_t & uv = *iter;
    mosaic_min[0] = std::min(mosaic_min[0], uv[0]);
    mosaic_max[0] = std::max(mosaic_max[0], uv[0]);
    mosaic_min[1] = std::min(mosaic_min[1], uv[1]);
    mosaic_max[1] = std::max(mosaic_max[1], uv[1]);
  }
  
  return (! uv_list.empty());
}


//----------------------------------------------------------------
// make_colors
//
// Generate a scrambled rainbow colormap.
// 
void
make_colors(const unsigned int & num_colors, std::vector<xyz_t> & color)
{
  static const xyz_t EAST  = xyz(1, 0, 0);
  static const xyz_t NORTH = xyz(0, 1, 0);
  static const xyz_t WEST  = xyz(0, 0, 1);
  static const xyz_t SOUTH = xyz(0, 0, 0);
  
  color.resize(num_colors);
  
  for (unsigned int i = 0; i < num_colors; i++)
  {
#if 1
    double t = fmod(double(i % 2) / 2.0 +
                    double(i) / double(num_colors - 1), 1.0);
#else
    double t = double(i) / double(num_colors);
#endif
    
    double s = 0.5 + 0.5 * fmod(double((i + 1) % 3) / 3.0 +
                                double(i) / double(num_colors - 1), 1.0);
    color[i] = hsv_to_rgb(xyz(t, s, 1.0)) * 255.0;
  }
}

//----------------------------------------------------------------
// find_inverse
// 
// Given a forward transform and image space coordinates, 
// find mosaic space coordinates.
// 
bool
find_inverse(const pnt2d_t & tile_min,        // tile space
             const pnt2d_t & tile_max,        // tile space
             const base_transform_t * mosaic_to_tile,
             const pnt2d_t & xy,        // tile space
             pnt2d_t & uv,                // mosaic space
             const unsigned int max_iterations,
             const double min_step_scale,
             const double min_error_sqrd,
             const unsigned int pick_up_pace_steps)
{
  base_transform_t::Pointer tile_to_mosaic = mosaic_to_tile->GetInverseTransform();
  return find_inverse(tile_min,
                      tile_max,
                      mosaic_to_tile,
                      tile_to_mosaic.GetPointer(),
                      xy,
                      uv,
                      max_iterations,
                      min_step_scale,
                      min_error_sqrd);
}

//----------------------------------------------------------------
// find_inverse
// 
// Given a forward transform, an approximate inverse transform,
// and image space coordinates, find mosaic space coordinates.
// 
bool
find_inverse(const pnt2d_t & tile_min,        // tile space
             const pnt2d_t & tile_max,        // tile space
             const base_transform_t * mosaic_to_tile,
             const base_transform_t * tile_to_mosaic,
             const pnt2d_t & xy,        // tile space
             pnt2d_t & uv,                // mosaic space
             const unsigned int max_iterations,
             const double min_step_scale,
             const double min_error_sqrd,
             const unsigned int pick_up_pace_steps)
{
  // #define DEBUG_FIND_INVERSE
  if (tile_to_mosaic == NULL)
  {
    return false;
  }
  
  const itk::GridTransform *gridTransform = 
  dynamic_cast<const itk::GridTransform *>(tile_to_mosaic);
  if (gridTransform != NULL)
  {
    // special case for the grid transform -- the inverse is either exact
    // or it doesn't exist (maps to extreme coordinates):
    pnt2d_t xy_plus;
    
    // Convert image space to mosaic space.
    xy_plus[0] = xy[0] + gridTransform->transform_.tile_min_[0];
    xy_plus[1] = xy[1] + gridTransform->transform_.tile_min_[1];
    uv = tile_to_mosaic->TransformPoint(xy_plus);
    return uv[0] != std::numeric_limits<double>::max();
  }
  
  // the local coordinate system, at the center of the tile:
  pnt2d_t p00 = tile_min + (tile_max - tile_min) * 0.5;
  pnt2d_t p10 = p00;
  pnt2d_t p01 = p00;
  
  double x_unit = 1.0;
  double y_unit = 1.0;
  p10[0] += x_unit;
  p01[1] += y_unit;
  
  vec2d_t x_axis = p10 - p00;
  vec2d_t y_axis = p01 - p00;
  
  // FIXME: this may fail, because inverse may be unstable:
  // the same coordinate system in the mosaic space:
  pnt2d_t q00 = tile_to_mosaic->TransformPoint(p00);
  pnt2d_t q10 = tile_to_mosaic->TransformPoint(p10);
  pnt2d_t q01 = tile_to_mosaic->TransformPoint(p01);
  
  vec2d_t u_axis = q10 - q00;
  vec2d_t v_axis = q01 - q00;
  
#ifdef DEBUG_FIND_INVERSE
  std::cerr << std::endl
  << "x_axis: " << x_axis << std::endl
  << "u_axis: " << u_axis << std::endl << std::endl
  << "y_axis: " << y_axis << std::endl
  << "v_axis: " << v_axis << std::endl << std::endl
  << "p00: " << p00 << std::endl
  << "q00: " << q00 << std::endl << std::endl;
#endif
  
  // initial guess:
  uv = to_wcs(q00,
              u_axis,
              v_axis,
              (xy[0] - p00[0]) / x_unit,
              (xy[1] - p00[1]) / y_unit);
  
  // estimate the error:
  const pnt2d_t & p0 = xy;
  const pnt2d_t p1 = mosaic_to_tile->TransformPoint(uv);
  vec2d_t er = p1 - p0;
  
#ifdef DEBUG_FIND_INVERSE
  std::cerr << "initial error: " << er.GetSquaredNorm()
  << ", dx: " << er[0] << ", dy: " << er[1]
  << std::endl;
#endif
  
  const double max_step_scale = 1.0;
  double step_scale = max_step_scale;
  unsigned int iteration = 0;
  unsigned int successful_steps = 0;
  
  double e0_sqrd = er.GetSquaredNorm();
  
  // don't try to improve samples with poor initialization (map to NaN):
  if (e0_sqrd != e0_sqrd) return false;
  
  while (true)
  {
    const vec2d_t uv_correction = to_wcs(u_axis,
                                         v_axis,
                                         -er[0] / x_unit,
                                         -er[1] / y_unit);
    
    pnt2d_t q = uv;
    q[0] += step_scale * uv_correction[0];
    q[1] += step_scale * uv_correction[1];
    
    pnt2d_t p = mosaic_to_tile->TransformPoint(q);
    vec2d_t e = p - p0;
    double e1_sqrd = e.GetSquaredNorm();
    
#ifdef DEBUG_FIND_INVERSE
    std::cerr << std::setw(2) << iteration << ": "
    << e0_sqrd << " vs " << e1_sqrd << " -- ";
#endif
    if (e1_sqrd < e0_sqrd)
    {
#ifdef DEBUG_FIND_INVERSE
      std::cerr << "ok" << std::endl;
#endif
      uv = q;
      er = e;
      e0_sqrd = e1_sqrd;
      successful_steps++;
      
      if (successful_steps % pick_up_pace_steps == 0)
      {
        step_scale = std::min(max_step_scale, step_scale * 2.0);
        
#ifdef DEBUG_FIND_INVERSE
        std::cerr << successful_steps
        << " successful steps -- increasing pace, new step length: "
        << step_scale << std::endl;
#endif
      }
    }
    else
    {
      step_scale /= 2.0;
      successful_steps = 0;
#ifdef DEBUG_FIND_INVERSE
      std::cerr << "relaxing and backtracking, new step length: "
      << step_scale << std::endl;
#endif
    }
    
    iteration++;
    if (iteration >= max_iterations) break;
    if (e0_sqrd < min_error_sqrd) break;
    if (step_scale < min_step_scale) break;
  }
#ifdef DEBUG_FIND_INVERSE
  std::cerr << std::endl;
#endif
  
  return true;
}

//----------------------------------------------------------------
// find_inverse
// 
// Given a forward transform, an approximate inverse transform,
// and image space coordinates, find mosaic space coordinates.
// 
bool
find_inverse(const base_transform_t * mosaic_to_tile,
             const base_transform_t * tile_to_mosaic,
             const pnt2d_t & xy,        // tile space
             pnt2d_t & uv,                // mosaic space
             const unsigned int max_iterations,
             const double min_step_scale,
             const double min_error_sqrd,
             const unsigned int pick_up_pace_steps)
{
  // #define DEBUG_FIND_INVERSE
  if (tile_to_mosaic == NULL)
  {
    return false;
  }
  
  if (dynamic_cast<const itk::GridTransform *>(tile_to_mosaic) != NULL || dynamic_cast<const itk::MeshTransform *>(tile_to_mosaic) != NULL)
  {
    // special case for the grid transform -- the inverse is either exact
    // or it doesn't exist (maps to extreme coordinates):
    uv = tile_to_mosaic->TransformPoint(xy);
    return uv[0] != std::numeric_limits<double>::max();
  }
  
  // initial guess:
  uv = tile_to_mosaic->TransformPoint(xy);
  
  // calculate initial error:
  const pnt2d_t & p0 = xy;
  pnt2d_t p1 = mosaic_to_tile->TransformPoint(uv);
  vec2d_t er = p1 - p0;
  
#ifdef DEBUG_FIND_INVERSE
  std::cerr << "initial error: " << er.GetSquaredNorm()
  << ", dx: " << er[0] << ", dy: " << er[1]
  << std::endl;
#endif
  
  const double max_step_scale = 1.0;
  double step_scale = max_step_scale;
  unsigned int iteration = 0;
  unsigned int successful_steps = 0;
  
  double e0_sqrd = er.GetSquaredNorm();
  
  // don't try to improve samples with poor initialization (map to NaN):
  if (e0_sqrd != e0_sqrd) return false;
  
  while (true)
  {
    pnt2d_t uv1 = tile_to_mosaic->TransformPoint(xy - er);
    const vec2d_t uv_correction = uv - uv1;
    
    pnt2d_t q = uv;
    q[0] += step_scale * uv_correction[0];
    q[1] += step_scale * uv_correction[1];
    
    pnt2d_t p = mosaic_to_tile->TransformPoint(q);
    vec2d_t e = p - p0;
    double e1_sqrd = e.GetSquaredNorm();
    
#ifdef DEBUG_FIND_INVERSE
    std::cerr << std::setw(2) << iteration << ": "
    << e0_sqrd << " vs " << e1_sqrd << " -- ";
#endif
    if (e1_sqrd < e0_sqrd)
    {
#ifdef DEBUG_FIND_INVERSE
      std::cerr << "ok" << std::endl;
#endif
      uv = q;
      er = e;
      e0_sqrd = e1_sqrd;
      successful_steps++;
      
      if (successful_steps % pick_up_pace_steps == 0)
      {
        step_scale = std::min(max_step_scale, step_scale * 2.0);
        
#ifdef DEBUG_FIND_INVERSE
        std::cerr << successful_steps
        << " successful steps -- increasing pace, new step length: "
        << step_scale << std::endl;
#endif
      }
    }
    else
    {
      step_scale /= 2.0;
      successful_steps = 0;
#ifdef DEBUG_FIND_INVERSE
      std::cerr << "relaxing and backtracking, new step length: "
      << step_scale << std::endl;
#endif
    }
    
    iteration++;
    if (iteration >= max_iterations) break;
    if (e0_sqrd < min_error_sqrd) break;
    if (step_scale < min_step_scale) break;
  }
#ifdef DEBUG_FIND_INVERSE
  std::cerr << std::endl;
#endif
  
  return true;
}

//----------------------------------------------------------------
// generate_landmarks_v1
// 
// Given a forward transform, generate a set of image space
// coordinates and find their matching mosaic space coordinates.
// 
// version 1 -- uniform jittered sampling over the tile
// 
bool
generate_landmarks_v1(const pnt2d_t & tile_min,
                      const pnt2d_t & tile_max,
                      const mask_t * tile_mask,
                      const base_transform_t * mosaic_to_tile,
                      const unsigned int samples,
                      std::vector<pnt2d_t> & xy,
                      std::vector<pnt2d_t> & uv,
                      bool refine)
{
  // #define DEBUG_LANDMARKS
  
  base_transform_t::Pointer tile_to_mosaic = mosaic_to_tile->GetInverseTransform();
  if (tile_to_mosaic.GetPointer() == NULL)
  {
    return false;
  }
  
  // the local coordinate system, at the center of the tile:
  pnt2d_t p00 = tile_min + (tile_max - tile_min) * 0.5;
  pnt2d_t p10 = p00;
  pnt2d_t p01 = p00;
  
  double W = tile_max[0] - tile_min[0];
  double H = tile_max[1] - tile_min[1];
  double x_unit = 1.0;
  double y_unit = 1.0;
  p10[0] += x_unit;
  p01[1] += y_unit;
  
  vec2d_t x_axis = p10 - p00;
  vec2d_t y_axis = p01 - p00;
  
  // FIXME: this may fail, because tile_to_mosaic may be unstable:
  // the same coordinate system in the mosaic space:
  pnt2d_t q00 = tile_to_mosaic->TransformPoint(p00);
  pnt2d_t q10 = tile_to_mosaic->TransformPoint(p10);
  pnt2d_t q01 = tile_to_mosaic->TransformPoint(p01);
  
  vec2d_t u_axis = q10 - q00;
  vec2d_t v_axis = q01 - q00;
  
#ifdef DEBUG_LANDMARKS
  std::cerr << std::endl
  << "x_axis: " << x_axis << std::endl
  << "u_axis: " << u_axis << std::endl << endl
  << "y_axis: " << y_axis << std::endl
  << "v_axis: " << v_axis << std::endl << endl
  << "p00: " << p00 << std::endl
  << "q00: " << q00 << std::endl << endl;
#endif
  
  std::list<pnt2d_t> xy_list;
  std::list<pnt2d_t> uv_list;
  std::list<vec2d_t> er_list;
  
  // sample the transform space:
  for (unsigned int i = 0; i < samples; i++)
  {
    for (unsigned int j = 0; j < samples; j++)
    {
      const double s = (double(i) + drand()) / double(samples);
      const double t = (double(j) + drand()) / double(samples);
      
      double x = tile_min[0] + s * W;
      double y = tile_min[1] + t * H;
      pnt2d_t pt_xy = pnt2d(x, y);
      
      // check to make sure this point is actually inside the tile/mask:
      if (!pixel_in_mask<mask_t>(tile_mask, pt_xy)) continue;
      
      // map (approximately) into the mosaic space:
      pnt2d_t pt_uv = to_wcs(q00,
                             u_axis,
                             v_axis,
                             (pt_xy[0] - p00[0]) / x_unit,
                             (pt_xy[1] - p00[1]) / y_unit);
      
      // estimate the error:
      const pnt2d_t & p0 = pt_xy;
      const pnt2d_t   p1 = mosaic_to_tile->TransformPoint(pt_uv);
      vec2d_t pt_er = p1 - p0;
#ifdef DEBUG_LANDMARKS
      std::cerr << "initial error: " << pt_er.GetSquaredNorm()
      << ", dx: " << p1[0] - p0[0] << ", dy: " << p1[1] - p0[1]
      << std::endl;
#endif
      
      xy_list.push_back(pt_xy);
      uv_list.push_back(pt_uv);
      er_list.push_back(pt_er);
    }
  }
  
  // try to refine the boundary:
  xy.assign(xy_list.begin(), xy_list.end());
  uv.assign(uv_list.begin(), uv_list.end());
  std::vector<vec2d_t> er(er_list.begin(), er_list.end());
  
  if (refine)
  {
    for (unsigned int j = 0; j < xy.size(); j++)
    {
      const unsigned int max_iterations = 100;
      const double min_step_scale = 1e-12;
      const double min_error_sqrd = 1e-16;
      const unsigned int pick_up_pace_steps = 5;
      const double max_step_scale = 1.0;
      
      double step_scale = max_step_scale;
      unsigned int iteration = 0;
      unsigned int successful_steps = 0;
      
      const pnt2d_t & p0 = xy[j];
      double e0_sqrd = er[j].GetSquaredNorm();
      
      // don't try to improve samples that fail:
      if (e0_sqrd != e0_sqrd) continue;
      
      while (true)
      {
        const vec2d_t uv_correction = to_wcs(u_axis,
                                             v_axis,
                                             -er[j][0] / x_unit,
                                             -er[j][1] / y_unit);
        
        pnt2d_t q = uv[j];
        q[0] += step_scale * uv_correction[0];
        q[1] += step_scale * uv_correction[1];
        
        pnt2d_t p = mosaic_to_tile->TransformPoint(q);
        vec2d_t e = p - p0;
        double e1_sqrd = e.GetSquaredNorm();
        
#ifdef DEBUG_LANDMARKS
        std::cerr << std::setw(3) << j << " " << std::setw(2) << iteration << ": "
        << e0_sqrd << " vs " << e1_sqrd << " -- ";
#endif
        if (e1_sqrd < e0_sqrd)
        {
#ifdef DEBUG_LANDMARKS
          std::cerr << "ok" << std::endl;
#endif
          uv[j] = q;
          er[j] = e;
          e0_sqrd = e1_sqrd;
          successful_steps++;
          
          if (successful_steps % pick_up_pace_steps == 0)
          {
            step_scale = std::min(max_step_scale, step_scale * 2.0);
            
#ifdef DEBUG_LANDMARKS
            std::cerr << successful_steps
            << " successful steps -- increasing pace, new step length: "
            << step_scale << std::endl;
#endif
          }
        }
        else
        {
          step_scale /= 2.0;
          successful_steps = 0;
#ifdef DEBUG_LANDMARKS
          std::cerr << "relaxing and backtracking, new step length: "
          << step_scale << std::endl;
#endif
        }
        
        iteration++;
        if (iteration >= max_iterations) break;
        if (e0_sqrd < min_error_sqrd) break;
        if (step_scale < min_step_scale) break;
      }
#ifdef DEBUG_LANDMARKS
      std::cerr << std::endl;
#endif
    }
  }
  
  // clean up -- remove failed samples:
  xy_list.clear();
  uv_list.clear();
  for (unsigned int j = 0; j < er.size(); j++)
  {
    if (er[j] != er[j]) continue;
    
    xy_list.push_back(xy[j]);
    uv_list.push_back(uv[j]);
  }
  
  xy.assign(xy_list.begin(), xy_list.end());
  uv.assign(uv_list.begin(), uv_list.end());
  
  return true;
}


//----------------------------------------------------------------
// generate_landmarks_v2
// 
// Given a forward transform, generate a set of image space
// coordinates and find their matching mosaic space coordinates.
// 
// version 2 -- non-uniform sampling skewed towards the center
//              of the tile radially. This may be useful when
//              the transform is ill-behaved at the tile edges.
// 
bool
generate_landmarks_v2(const pnt2d_t & tile_min,
                      const pnt2d_t & tile_max,
                      const mask_t * tile_mask,
                      const base_transform_t * mosaic_to_tile,
                      const unsigned int samples,
                      std::vector<pnt2d_t> & xy,
                      std::vector<pnt2d_t> & uv,
                      bool refine)
{
  // #define DEBUG_LANDMARKS
  
  base_transform_t::Pointer tile_to_mosaic = mosaic_to_tile->GetInverseTransform();
  if (tile_to_mosaic.GetPointer() == NULL)
  {
    return false;
  }
  
  // the local coordinate system, at the center of the tile:
  pnt2d_t p00 = tile_min + (tile_max - tile_min) * 0.5;
  pnt2d_t p10 = p00;
  pnt2d_t p01 = p00;
  
  double W = tile_max[0] - tile_min[0];
  double H = tile_max[1] - tile_min[1];
  double x_unit = 1.0;
  double y_unit = 1.0;
  p10[0] += x_unit;
  p01[1] += y_unit;
  
  vec2d_t x_axis = p10 - p00;
  vec2d_t y_axis = p01 - p00;
  
  // FIXME: this may fail, because tile_to_mosaic may be unstable:
  // the same coordinate system in the mosaic space:
  pnt2d_t q00 = tile_to_mosaic->TransformPoint(p00);
  pnt2d_t q10 = tile_to_mosaic->TransformPoint(p10);
  pnt2d_t q01 = tile_to_mosaic->TransformPoint(p01);
  
  vec2d_t u_axis = q10 - q00;
  vec2d_t v_axis = q01 - q00;
  
#ifdef DEBUG_LANDMARKS
  std::cerr << std::endl
  << "x_axis: " << x_axis << std::endl
  << "u_axis: " << u_axis << std::endl << endl
  << "y_axis: " << y_axis << std::endl
  << "v_axis: " << v_axis << std::endl << endl
  << "p00: " << p00 << std::endl
  << "q00: " << q00 << std::endl << endl;
#endif
  
  std::list<pnt2d_t> xy_list;
  std::list<pnt2d_t> uv_list;
  std::list<vec2d_t> er_list;
  
  // sample the transform space (stay away from the corners):
  for (unsigned int i = 0; i < samples; i++)
  {
    for (unsigned int j = 0; j < samples; j++)
    {
      // non-uniform sampling of the radius:
      const double r = sqrt(drand());
      
      // uniform sampling of the theta angle:
      const double t = TWO_PI * (double(i) + drand()) / double(samples);
      
      // convert the polar coordinates into cartesian coordinates:
      const double x = p00[0] + 0.7 * r * cos(t) * W / 2.0;
      const double y = p00[1] + 0.7 * r * sin(t) * H / 2.0;
      
      pnt2d_t pt_xy = pnt2d(x, y);
      
      // check to make sure this point is actually inside the tile/mask:
      if (!pixel_in_mask<mask_t>(tile_mask, pt_xy)) continue;
      
      // map (approximately) into the mosaic space:
      pnt2d_t pt_uv = to_wcs(q00,
                             u_axis,
                             v_axis,
                             (pt_xy[0] - p00[0]) / x_unit,
                             (pt_xy[1] - p00[1]) / y_unit);
      
      if (refine)
      {
        // estimate the error:
        const pnt2d_t & p0 = pt_xy;
        const pnt2d_t   p1 = mosaic_to_tile->TransformPoint(pt_uv);
        vec2d_t pt_er = p1 - p0;
#ifdef DEBUG_LANDMARKS
        std::cerr << "initial error: " << pt_er.GetSquaredNorm()
        << ", dx: " << p1[0] - p0[0] << ", dy: " << p1[1] - p0[1]
        << std::endl;
#endif
        
        xy_list.push_back(pt_xy);
        uv_list.push_back(pt_uv);
        er_list.push_back(pt_er);
      }
      else
      {
        pt_xy = mosaic_to_tile->TransformPoint(pt_uv);
        xy_list.push_back(pt_xy);
        uv_list.push_back(pt_uv);
      }
    }
  }
  
  // try to refine the boundary:
  xy.assign(xy_list.begin(), xy_list.end());
  uv.assign(uv_list.begin(), uv_list.end());
  
  if (refine)
  {
    std::vector<vec2d_t> er(er_list.begin(), er_list.end());
    
    for (unsigned int j = 0; j < xy.size(); j++)
    {
      const unsigned int max_iterations = 100;
      const double min_step_scale = 1e-12;
      const double min_error_sqrd = 1e-16;
      const unsigned int pick_up_pace_steps = 5;
      const double max_step_scale = 1.0;
      
      double step_scale = max_step_scale;
      unsigned int iteration = 0;
      unsigned int successful_steps = 0;
      
      const pnt2d_t & p0 = xy[j];
      double e0_sqrd = er[j].GetSquaredNorm();
      
      while (true)
      {
        const vec2d_t uv_correction = to_wcs(u_axis,
                                             v_axis,
                                             -er[j][0] / x_unit,
                                             -er[j][1] / y_unit);
        
        pnt2d_t q = uv[j];
        q[0] += step_scale * uv_correction[0];
        q[1] += step_scale * uv_correction[1];
        
        pnt2d_t p = mosaic_to_tile->TransformPoint(q);
        vec2d_t e = p - p0;
        double e1_sqrd = e.GetSquaredNorm();
        
#ifdef DEBUG_LANDMARKS
        std::cerr << std::setw(3) << j << " " << std::setw(2) << iteration << ": "
        << e0_sqrd << " vs " << e1_sqrd << " -- ";
#endif
        if (e1_sqrd < e0_sqrd)
        {
#ifdef DEBUG_LANDMARKS
          std::cerr << "ok" << std::endl;
#endif
          uv[j] = q;
          er[j] = e;
          e0_sqrd = e1_sqrd;
          successful_steps++;
          
          if (successful_steps % pick_up_pace_steps == 0)
          {
            step_scale = std::min(max_step_scale, step_scale * 2.0);
            
#ifdef DEBUG_LANDMARKS
            std::cerr << successful_steps
            << " successful steps -- increasing pace, new step length: "
            << step_scale << std::endl;
#endif
          }
        }
        else
        {
          step_scale /= 2.0;
          successful_steps = 0;
#ifdef DEBUG_LANDMARKS
          std::cerr << "relaxing and backtracking, new step length: "
          << step_scale << std::endl;
#endif
        }
        
        iteration++;
        if (iteration >= max_iterations) break;
        if (e0_sqrd < min_error_sqrd) break;
        if (step_scale < min_step_scale) break;
      }
#ifdef DEBUG_LANDMARKS
      std::cerr << std::endl;
#endif
    }
  }
  
  return true;
}

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
bool
generate_landmarks(const pnt2d_t & tile_min,
                   const pnt2d_t & tile_max,
                   const mask_t * tile_mask,
                   const base_transform_t * mosaic_to_tile,
                   const unsigned int samples,
                   std::vector<pnt2d_t> & xy,
                   std::vector<pnt2d_t> & uv,
                   const unsigned int version,
                   const bool refine)
{
  switch (version)
  {
    case 1:
      return generate_landmarks_v1(tile_min,
                                   tile_max,
                                   tile_mask,
                                   mosaic_to_tile,
                                   samples,
                                   xy,
                                   uv,
                                   refine);
      
    case 2:
      return generate_landmarks_v2(tile_min,
                                   tile_max,
                                   tile_mask,
                                   mosaic_to_tile,
                                   samples,
                                   xy,
                                   uv,
                                   refine);
  }
  
  return false;
}

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
bool
generate_landmarks(const image_t * tile,
                   const mask_t * mask,
                   const base_transform_t * mosaic_to_tile,
                   const unsigned int samples,
                   std::vector<pnt2d_t> & xy,
                   std::vector<pnt2d_t> & uv,
                   const unsigned int version,
                   const bool refine)
{
  image_t::SpacingType sp = tile->GetSpacing();
  image_t::SizeType sz = tile->GetLargestPossibleRegion().GetSize();
  const pnt2d_t min = tile->GetOrigin();
  const pnt2d_t max = min + vec2d(sp[0] * double(sz[0]),
                                  sp[1] * double(sz[1]));
  
  switch (version)
  {
    case 1:
      return generate_landmarks_v1(min,
                                   max,
                                   mask,
                                   mosaic_to_tile,
                                   samples,
                                   xy,
                                   uv,
                                   refine);
      
    case 2:
      return generate_landmarks_v2(min,
                                   max,
                                   mask,
                                   mosaic_to_tile,
                                   samples,
                                   xy,
                                   uv,
                                   refine);
  }
  
  return false;
}

//----------------------------------------------------------------
// calc_size
// 
// Given a bounding box expressed in the image space and
// pixel spacing, calculate corresponding image size.
// 
image_t::SizeType
calc_size(const pnt2d_t & min,
          const pnt2d_t & max,
          const double & spacing)
{
  image_t::SizeType sz;
  sz[0] = (unsigned int)((ceilf(max[0]) - floor(min[0])) / spacing);
  sz[1] = (unsigned int)((ceilf(max[1]) - floor(min[1])) / spacing);
  return sz;
}
