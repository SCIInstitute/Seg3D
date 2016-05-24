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

// File         : cluster.cxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/04/05 12:36
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for identifying pixel clusters in
//                thresholded 1D/2D discrete function/image.

// local includes:
#include <Core/ITKCommon/cluster.hxx>


//----------------------------------------------------------------
// identify_clusters_cm
// 
// Identify clusters in the 1D data (anything that's not 0),
// calculate a center of mass for each cluster.
// 
bool
identify_clusters_cm(const double * data,
                     const unsigned int & size,
                     std::list<centerofmass_t<1> > & centerofmass,
                     const bool periodic)
{
  // classify the clusters:
  typedef std::list<int> cluster_1d_t;
  the_dynamic_array_t<cluster_1d_t> clusters;
  std::vector<unsigned int> cluster_map;
  cluster_map.assign(size, ~0);
  
  for (unsigned int i = 0; i < size; i++)
  {
    const double & v = data[i];
    if (v == 0.0) continue;
    
    unsigned int prev = (i + size - 1) % size;
    unsigned int id = cluster_map[prev];
    if (id == ~0u)
    {
      // make a new cluster:
      clusters.append(cluster_1d_t());
      id = clusters.end_index(true);
      
      cluster_map[i] = id;
      clusters[id].push_back(i);
    }
    else
    {
      // add to the existing cluster:
      cluster_map[i] = id;
      clusters[id].push_back(i);
    }
  }
  
  // merge the clusters that are broken up across the periodic boundary:
  if (periodic &&
      cluster_map[0] != ~0u &&
      cluster_map[size - 1] != ~0u)
  {
    unsigned int id = cluster_map[0];
    cluster_1d_t & cluster_a = clusters[id];
    cluster_1d_t & cluster_b = clusters[cluster_map[size - 1]];
    
    if (&cluster_a != &cluster_b)
    {
      while (!cluster_b.empty() )
      {
        int i = remove_head(cluster_b);
        
        cluster_map[i] = id;
        cluster_a.push_back(i - size);
      }
    }
  }
  
  // calculate the center of mass for each cluster:
  for (unsigned int i = 0; i < clusters.size(); i++)
  {
    const cluster_1d_t & cluster = clusters[i];
    if (cluster.empty()) continue;
    
    double mx = 0.0;
    double mt = 0.0;
    
    for (std::list<int>::const_iterator j = cluster.begin();
         j != cluster.end(); ++j)
    {
      double x = double(*j);
      
      // adjust index for periodicity:
      unsigned int index = (*j + size) % size;
      
      const double & m = data[index];
      mx += m * x;
      mt += m;
    }
    
    centerofmass_t<1> cm;
    cm[0] = fmod(double(size) + mx / mt, double(size));
    cm.mass_ = mt / double(cluster.size());
    centerofmass.push_back(cm);
  }
  
  // sort the max points so that the best candidate is firts:
  centerofmass.sort(std::greater<centerofmass_t<1> >());
  
  return !centerofmass.empty();
}


//----------------------------------------------------------------
// identify_clusters_cm
// 
// Calculate the centers of mass for a given set of
// 2D pixel clusters.
// 
bool
identify_clusters_cm(const image_t * image,
                     const the_dynamic_array_t<cluster_t> & clusters,
                     std::list<centerofmass_t<2> > & centerofmass)
{
  typedef image_t::IndexType index_t;
  
  const unsigned int num_clusters = clusters.size();
  
  for (unsigned int j = 0; j < num_clusters; j++)
  {
    const cluster_t & cluster = clusters[j];
    if (cluster.empty()) continue;
    
    double cluster_min = std::numeric_limits<double>::max();
    double cluster_max = -cluster_min;
    
    for (std::list<index_t>::const_iterator k = cluster.begin();
         k != cluster.end(); ++k)
    {
      const index_t & index = *k;
      double pixel = static_cast<double>(image->GetPixel(index));
      cluster_min = std::min(pixel, cluster_min);
      cluster_max = std::max(pixel, cluster_max);
    }
    
    double cluster_x = 0.0;
    double cluster_y = 0.0;
    double cluster_m = 0.0;
    
    for (std::list<index_t>::const_iterator k = cluster.begin();
         k != cluster.end(); ++k)
    {
      const index_t & index = *k;
      double pixel = static_cast<double>(image->GetPixel(index));
      double mass = pixel;
      
      cluster_x += static_cast<double>(index[0]) * mass;
      cluster_y += static_cast<double>(index[1]) * mass;
      cluster_m += mass;
    }
    
    assert(cluster_m != 0.0);
    
    centerofmass_t<2> cm;
    cm[0] = cluster_x / static_cast<double>(cluster_m);
    cm[1] = cluster_y / static_cast<double>(cluster_m);
    cm.mass_ = cluster_m / static_cast<double>(cluster.size());
    centerofmass.push_back(cm);
  }
  
  return !centerofmass.empty();
}
