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

// File         : stos.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/06/23 13:40
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Slice to slice registration container class.

#ifndef STOS_HXX_
#define STOS_HXX_

// system includes:
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

// boost:
#include <boost/filesystem.hpp>

// local includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/the_utils.hxx>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

namespace bfs=boost::filesystem;

//----------------------------------------------------------------
// stos_t
//
template <typename TImage = image_t>
class stos_t
{
public:
  stos_t() {}
  
  stos_t(const bfs::path & fn_s0,
         const bfs::path & fn_s1,
         const bfs::path & fn_mask_s0,
         const bfs::path & fn_mask_s1,
         const bool & flipped_0,
         const bool & flipped_1,
         const TImage * s0,
         const TImage * s1,
         const unsigned int & shrink_factor,
         const base_transform_t * t01) : t01_(t01)
  {
    fn_[0] = fn_s0;
    fn_[1] = fn_s1;
    
    fn_mask_[0] = fn_mask_s0;
    fn_mask_[1] = fn_mask_s1;
    
    flipped_[0] = flipped_0;
    flipped_[1] = flipped_1;
    
    sp_[0] = s0->GetSpacing();
    sp_[1] = s1->GetSpacing();
    
    sz_[0] = s0->GetLargestPossibleRegion().GetSize();
    sz_[1] = s1->GetLargestPossibleRegion().GetSize();
    
    sp_[0][0] /= shrink_factor;
    sp_[0][1] /= shrink_factor;
    sp_[1][0] /= shrink_factor;
    sp_[1][1] /= shrink_factor;
    
    sz_[0][0] *= shrink_factor;
    sz_[0][1] *= shrink_factor;
    sz_[1][0] *= shrink_factor;
    sz_[1][1] *= shrink_factor;
  }
  
  stos_t(const bfs::path & fn_s0,
         const bfs::path & fn_s1,
         const bfs::path & fn_mask_s0,
         const bfs::path & fn_mask_s1,
         const bool & flipped_0,
         const bool & flipped_1,
         const double spacing_x0,
         const double spacing_y0,
         const double spacing_x1,
         const double spacing_y1,
         const unsigned int size_x0,
         const unsigned int size_y0,
         const unsigned int size_x1,
         const unsigned int size_y1,
         const base_transform_t * t01) : t01_(t01)
  {
    fn_[0] = fn_s0;
    fn_[1] = fn_s1;
    
    fn_mask_[0] = fn_mask_s0;
    fn_mask_[1] = fn_mask_s1;
    
    flipped_[0] = flipped_0;
    flipped_[1] = flipped_1;
    
    sp_[0][0] = spacing_x0;
    sp_[0][1] = spacing_y0;
    sp_[1][0] = spacing_x1;
    sp_[1][1] = spacing_y1;
    
    sz_[0][0] = size_x0;
    sz_[0][1] = size_y0;
    sz_[1][0] = size_x1;
    sz_[1][1] = size_y1;
  }
  
  stos_t(const bfs::path & fn_load,
         const bfs::path & override_slice0_path = "",
         const bfs::path & override_slice1_path = "")
  {
    if (! load(fn_load, override_slice0_path, override_slice1_path) )
    {
      CORE_THROW_EXCEPTION("Loading stos file failed.");
    }
  }
  
  // load the filenames of the slices and the corresponding transform:
  bool load(const bfs::path & fn_load,
            const bfs::path & slice0_path = "",
            const bfs::path & slice1_path = "",
            const bool blab = true)
  {
    std::fstream si;
    the::open_utf8(si, fn_load.c_str(), std::ios::in);
    if (!si.is_open())
    {
      std::ostringstream oss;
      oss << "Could not open " << fn_load << " for reading, skipping....";
      CORE_LOG_WARNING(oss.str());
      return false;
    }
    
    fn_load_ = fn_load;
    if (blab) std::cout << "loading " << fn_load_.string() << std::endl;
    
    bool ok = load(si, slice0_path, slice1_path);
    si.close();
    
    return ok;
  }
  
  bool load(std::istream & si,
            const bfs::path & override_slice0_path = "",
            const bfs::path & override_slice1_path = "")
  {
    std::string f[2];
    std::getline(si, f[0]);
    std::getline(si, f[1]);
    
    // Setup slice paths...
    bfs::path corrected_path[2];
    corrected_path[0] = override_slice0_path;
    corrected_path[1] = override_slice1_path;
    
    // Override if needed.
    for (int i = 0; i < 2; i++)
    {
      if (! corrected_path[i].empty() )
      {
        bfs::path mosaicPath(f[i]);
        bfs::path p = corrected_path[i] / mosaicPath.filename();
        if ( bfs::exists(p) )
        {
          fn_[i] = p;
        }
        else
        {
          std::ostringstream oss;
          oss << "Overridden file path " << p << " does not exist. Ignoring path override.";
          CORE_LOG_WARNING(oss.str());

          fn_[i] = f[i];
        }
      }
    }
    
    int sz[2][2];
    si >> flipped_[0] >> flipped_[1]
    >> sp_[0][0] >> sp_[0][1] >> sz[0][0] >> sz[0][1]
    >> sp_[1][0] >> sp_[1][1] >> sz[1][0] >> sz[1][1];
    
    sz_[0][0] = sz[0][0];
    sz_[0][1] = sz[0][1];
    sz_[1][0] = sz[1][0];
    sz_[1][1] = sz[1][1];
    
    itk::TransformBase::Pointer tmp = load_transform(si);
    
    t01_ = dynamic_cast<base_transform_t *>(tmp.GetPointer());
    bool ok = t01_.GetPointer() != NULL;
    
    fn_mask_[0].clear();
    fn_mask_[1].clear();
    
    // skip to the next line (by skipping white space):
    si >> std::ws;
    
    std::string optional;
    std::getline(si, optional);
    if (optional == "two_user_supplied_masks:")
    {
      std::string tmp;
      std::getline(si, tmp);
      if ( bfs::exists(bfs::path(tmp)) )
      {
        fn_mask_[0] = tmp;
      }
      tmp.clear();
      std::getline(si, tmp);
      if ( bfs::exists(bfs::path(tmp)) )
      {
        fn_mask_[1] = tmp;
      }
    }
    
    return ok;
  }
  
  // save this slice-to-slice registration:
  bool save(const bfs::path & fn_save, const bool blab = true) const
  {
    std::fstream so;
    the::open_utf8(so, fn_save.c_str(), std::ios::out);
    if (!so.is_open())
    {
      std::ostringstream oss;
      oss << "Could not open " << fn_save << " for writing, skipping....";
      CORE_LOG_WARNING(oss.str());
      return false;
    }
    if (blab) std::cout << "saving " << fn_save << std::endl;
    
    save(so);
    so.close();
    
    // FIXME: not sure if this is useful/necessary:
    fn_load_ = fn_save;
    
    return true;
  }
  
  void save(std::ostream & so) const
  {
    std::ios::fmtflags old_flags = so.setf(std::ios::scientific);
    int old_precision = so.precision();
    so.precision(12);
    
    so << fn_[0].string() << std::endl
    << fn_[1].string() << std::endl
    << flipped_[0] << std::endl
    << flipped_[1] << std::endl
    << sp_[0][0] << '\t'
    << sp_[0][1] << '\t'
    << static_cast<int>(sz_[0][0]) << '\t'
    << static_cast<int>(sz_[0][1]) << std::endl
    << sp_[1][0] << '\t'
    << sp_[1][1] << '\t'
    << static_cast<int>(sz_[1][0]) << '\t'
    << static_cast<int>(sz_[1][1]) << std::endl;
    
    save_transform(so, t01_);
    so << std::endl;
    
    if (!fn_mask_[0].empty() && !fn_mask_[1].empty())
    {
      so << "two_user_supplied_masks:" << std::endl
      << fn_mask_[0].string() << std::endl
      << fn_mask_[1].string() << std::endl;
    }
    
    so.setf(old_flags);
    so.precision(old_precision);
  }
  
  // convenience functions:
  inline pnt2d_t tile_min(unsigned int /* i */) const
  { return pnt2d(0, 0); }
  
  inline pnt2d_t tile_max(unsigned int i) const
  {
    return pnt2d(sp_[i][0] * static_cast<double>(sz_[i][0]),
                 sp_[i][1] * static_cast<double>(sz_[i][1]));
  }
  
  inline pnt2d_t tile_center(unsigned int i) const
  {
    pnt2d_t min = tile_min(i);
    pnt2d_t max = tile_max(i);
    return min + (max - min) * 0.5;
  }
  
  // datamembers:
  mutable bfs::path fn_load_;
  bfs::path fn_[2]; // filenames of the fixed/moving slices
  bfs::path image_dirs_[2];
  bfs::path fn_mask_[2]; // filenames of slice masks, optional
  bool flipped_[2];
  typename TImage::SpacingType sp_[2];
  typename TImage::RegionType::SizeType sz_[2];
  base_transform_t::ConstPointer t01_;
};

//----------------------------------------------------------------
// operator <<
// 
template <typename TImage>
inline static std::ostream &
operator << (std::ostream & so, const stos_t<TImage> & stos)
{
  stos.save(so);
  return so;
}

//----------------------------------------------------------------
// save_stos
//
template <typename TImage>
inline static bool
save_stos(const bfs::path & fn_save,
          const bfs::path & fn_s0,
          const bfs::path & fn_s1,
          const bfs::path & fn_mask_s0,
          const bfs::path & fn_mask_s1,
          const bool & flipped_0,
          const bool & flipped_1,
          const TImage * s0,
          const TImage * s1,
          const unsigned int & shrink_factor,
          const base_transform_t * transform,
          const bool blab = true)
{
  stos_t<TImage> stos(fn_s0,
                      fn_s1,
                      fn_mask_s0,
                      fn_mask_s1,
                      flipped_0,
                      flipped_1,
                      s0,
                      s1,
                      shrink_factor,
                      transform);
  return stos.save(fn_save, blab);
}


#endif // STOS_HXX_
