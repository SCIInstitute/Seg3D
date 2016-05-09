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

// File         : cluster.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/05 12:36
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for identifying pixel clusters in
//                thresholded 1D/2D discrete function/image.

#ifndef CLUSTER_HXX_
#define CLUSTER_HXX_

// local includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/the_dynamic_array.hxx>
#include <Core/ITKCommon/the_utils.hxx>

#include <Core/Utils/Exception.h>

// system includes:
#include <list>

// ITK includes:
#include <itkPoint.h>


//----------------------------------------------------------------
// cluster_t
// 
// A list of 2D pixels.
// 
typedef std::list<image_t::IndexType> cluster_t;


//----------------------------------------------------------------
// centerofmass_t
// 
// A wrapper for a physical point and a scalar mass.
// 
template <unsigned int d>
class centerofmass_t
{
public:
  centerofmass_t():
  mass_(0.0)
  {}
  
  // comparison operators:
  inline bool operator < (const centerofmass_t<d> & cm) const
  { return mass_ < cm.mass_; }
  
  inline bool operator > (const centerofmass_t<d> & cm) const
  { return mass_ > cm.mass_; }
  
  // accessors:
  inline const double & operator [] (const unsigned int & index) const
  { return cm_[index]; }
  
  inline double & operator [] (const unsigned int & index)
  { return cm_[index]; }
  
  // the center of mass of the cluster:
  itk::Point<double, d> cm_;
  
  // the mass of the cluster normalized by the size of the cluster:
  double mass_;
};


//----------------------------------------------------------------
// identify_clusters_cm
// 
// Identify clusters in the 1D data (anything that's not 0),
// calculate a center of mass for each cluster.
// 
extern bool
identify_clusters_cm(const double * data,
                     const unsigned int & size,
                     std::list<centerofmass_t<1> > & centerofmass,
                     const bool periodic = true);

//----------------------------------------------------------------
// identify_clusters_cm
// 
// Calculate the centers of mass for a given set of
// 2D pixel clusters.
// 
extern bool
identify_clusters_cm(const image_t * image,
                     const the_dynamic_array_t<cluster_t> & clusters,
                     std::list<centerofmass_t<2> > & centerofmass);

//----------------------------------------------------------------
// assemble_clusters
// 
// Identify 2D pixel clusters (above a given threshold -- the
// background), using either 4 or 8 connectivity stencil.
// 
template <class image_t>
void
assemble_clusters(const image_t * image,
                  const mask_t * mask,
                  typename image_t::PixelType background,
                  const unsigned int connectivity,
                  the_dynamic_array_t<std::list<typename image_t::IndexType> >
                  & clusters)
{
  typedef typename image_t::RegionType region_t;
  typedef typename image_t::PixelType pixel_t;
  typedef typename image_t::IndexType index_t;
  typedef std::list<index_t> cluster_t;
  
  //  assert(connectivity <= 8);
  if (connectivity > 8)
  {
    CORE_THROW_EXCEPTION("cluster connectivity > 8");
  }
  
  // classify the clusters:
  static int stencil[][2] = {
    // 4 connected:
    { 0, -1 },
    {-1,  0 },
    { 0,  1 },
    { 1,  0 },
    
    // 8 connected:
    {-1, -1 },
    { 1,  1 },
    {-1,  1 },
    { 1, -1 }
  };
  
  mask_t::RegionType::SizeType mask_size =
  image->GetLargestPossibleRegion().GetSize();
  unsigned int spacing_scale = 1;
  
  if (mask)
  {
    mask_size = mask->GetLargestPossibleRegion().GetSize();
    spacing_scale =
    static_cast<unsigned int>(image->GetSpacing()[0] / mask->GetSpacing()[0]);
  }
  
  region_t rn = image->GetLargestPossibleRegion();
  typename region_t::SizeType sz = rn.GetSize();
  
  typedef itk::Image<unsigned int> cluster_map_t;
  cluster_map_t::Pointer cluster_map = make_image<cluster_map_t>(sz, ~0u);
  
  typedef itk::ImageRegionConstIteratorWithIndex<image_t> itex_t;
  itex_t itex(image, rn);
  
  for (itex.GoToBegin(); !itex.IsAtEnd(); ++itex)
  {
    const pixel_t v = itex.Get();
    
    // skip over the background:
    if (v <= background) continue;
    
    const index_t index = itex.GetIndex();
    
    // skip over pixels outside the mask:
    if (!pixel_in_mask<image_t>(mask, mask_size, index, spacing_scale))
    {
      continue;
    }
    
    // iterate over the neighborhood, collect the cluster ids of the neighbors:
    std::list<unsigned int> neighbors;
    
    for (unsigned int k = 0; k < connectivity; k++)
    {
      index_t uv = index;
      uv[0] += stencil[k][0];
      uv[1] += stencil[k][1];
      if (!pixel_in_mask<image_t>(mask, mask_size, uv, spacing_scale))
      {
        continue;
      }
      
//      assert(uv[0] >= 0 && image_size_value_t(uv[0]) < sz[0] &&
//             uv[1] >= 0 && image_size_value_t(uv[1]) < sz[1]);
      if (uv[0] < 0 || image_size_value_t(uv[0]) > sz[0] ||
          uv[1] < 0 || image_size_value_t(uv[1]) > sz[1])
      {
        CORE_THROW_EXCEPTION("cluster ids of neighbors out of bounds");
      }
      
      unsigned int cluster_id = cluster_map->GetPixel(uv);
      if (cluster_id != ~0u)
      {
        push_back_unique(neighbors, cluster_id);
      }
    }
    
    if (neighbors.empty())
    {
      // make a new cluster:
      clusters.append(cluster_t());
      
      unsigned int id = clusters.end_index(true);
      cluster_map->SetPixel(index, id);
      clusters[id].push_back(index);
    }
    else
    {
      // add this pixel to the cluster:
      unsigned int id = neighbors.front();
      cluster_map->SetPixel(index, id);
      clusters[id].push_back(index);
      
      if (neighbors.size() > 1)
      {
        // merge the cluster into one (the first one):
        for (std::list<unsigned int>::iterator bi = ++(neighbors.begin());
             bi != neighbors.end(); ++bi)
        {
          unsigned int old_id = *bi;
          
          while (!clusters[old_id].empty())
          {
            index_t uv = remove_head(clusters[old_id]);
            
            cluster_map->SetPixel(uv, id);
            clusters[id].push_back(uv);
          }
        }
      }
    }
  }
}


#endif // CLUSTER_HXX_
