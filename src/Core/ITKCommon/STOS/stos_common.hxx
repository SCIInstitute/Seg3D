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

// File         : stos_common.hxx
// Author       : Pavel A. Koshevoy
// Created      : 2006/05/31 14:29
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for slice to slice registration.

#ifndef STOS_COMMON_HXX_
#define STOS_COMMON_HXX_

// local includes:
#include <Core/ITKCommon/STOS/stos.hxx>
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/the_text.hxx>

// Boost includes:
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
namespace bfs=boost::filesystem;

// system includes:
#include <fstream>
#include <list>


//----------------------------------------------------------------
// sort
// 
// Sort the list so that the slices are sequential.
// 
// Return true if the list was sorted successfully.
// 
// Return false otherwise (may happen if there are
// disconnects or duplicates present).
//
template <typename TImage>
bool
sort(std::list<stos_t<TImage> > & stos)
{
  if (stos.empty()) return true;
  
  // FIXME: this function should be retired, use stos_tree_t instead
  // 
  // this function only works if there is at most 1 outbound mapping
  // from any node to any other node
  // 
  // so  0:1, 1:2, 2:3 will work, only 1 outbound mapping from each source node
  // but 0:1, 0:2, 2:3 will not work, 0 node has 2 outbound mappings to 1 and 2
  // 
  std::list<stos_t<TImage> > sorted;
  sorted.push_back(remove_head(stos));
  
  while (!stos.empty())
  {
    // find a match for the head:
    bool found_head_match = false;
    for (typename std::list<stos_t<TImage> >::iterator i = stos.begin();
         i != stos.end();)
    {
      const bfs::path & head = sorted.front().fn_[0];
      const bfs::path & tail = (*i).fn_[1];
      if (head == tail)
      {
        found_head_match = true;
        sorted.push_front(*i);
        i = stos.erase(i);
      }
      else
      {
        ++i;
      }
    }
    
    // find a match for the tail:
    bool found_tail_match = false;
    for (typename std::list<stos_t<TImage> >::iterator i = stos.begin();
         i != stos.end();)
    {
      const bfs::path & tail = sorted.back().fn_[1];
      const bfs::path & head = (*i).fn_[0];
      if (head == tail)
      {
        found_tail_match = true;
        sorted.push_back(*i);
        i = stos.erase(i);
      }
      else
      {
        ++i;
      }
    }
    
    if (!found_head_match && !found_tail_match)
    {
      // could not find a match for either head or tail:
      break;
    }
  }
  
  if (!stos.empty()) return false;
  
  stos.splice(stos.end(), sorted);
  return true;
}


//----------------------------------------------------------------
// load_mosaic
// 
template <typename T>
bool
load_mosaic(const bfs::path & fn_mosaic,
            const bool & flip_mosaic,
            const unsigned int & shrink_factor,
            const bool & assemble_mosaic,
            const bfs::path & image_path,
            typename T::Pointer & mosaic,
            const bool & assemble_mosaic_mask,
            mask_t::Pointer & mosaic_mask,
            const double & clahe_slope = 1.0,
            const unsigned int & clahe_window = 64,
            typename T::PixelType clahe_new_min = 0,
            typename T::PixelType clahe_new_max = 255,
            const bool & dont_allocate = false)
{
//  std::cout << "    reading " << fn_mosaic;
  
  std::fstream fin;
  fin.open(fn_mosaic.c_str(), std::ios::in);
  if (! fin.is_open())
  {
//    std::cout << ", failed...." << std::endl;
    return false;
  }
//  std::cout << std::endl;
  
  std::list<bfs::path> fn_tiles;
  std::vector<base_transform_t::Pointer> transform;
  std::vector<typename T::ConstPointer> image;
  std::vector<mask_t::ConstPointer> mask;
  
  double pixel_spacing = 1.0;
  bool use_std_mask = false;
  load_mosaic<base_transform_t>(fin,
                                pixel_spacing,
                                use_std_mask,
                                fn_tiles,
                                transform,
                                image_path);
  fin.close();
  
  unsigned int num_images = transform.size();
  assert(pixel_spacing != 0);
  
  image.resize(num_images);
  mask.resize(num_images);

//  bfs::path mosaic_dir = IRPath::DirectoryFromPath( fn_mosaic );
  bfs::path mosaic_dir = fn_mosaic.parent_path();
  
  unsigned int i = 0;
  for (std::list<bfs::path>::const_iterator iter = fn_tiles.begin();
       iter != fn_tiles.end(); ++iter, i++)
  {
    const bfs::path & fn_image = *iter;
    
    // read the image:
//    std::cout << std::setw(3) << i << " ";
    image[i] = std_tile<T>(fn_image, shrink_factor, pixel_spacing);
    mask[i] = std_mask<T>(image[i], use_std_mask);
  }
  
  // assempble the mosaic:
  if (assemble_mosaic)
  {
    std::cout << "    assembling a mosaic " << std::endl;
    mosaic = make_mosaic<typename T::ConstPointer, base_transform_t::Pointer>
      (FEATHER_BLEND_E, transform, image, mask, dont_allocate);
    
    // reset the mosaic image origin:
    typename T::PointType origin = mosaic->GetOrigin();
    origin[0] = 0;
    origin[1] = 0;
    mosaic->SetOrigin(origin);
    
    if (flip_mosaic && !dont_allocate)
    {
      mosaic = flip<T>(mosaic);
    }
  }
  
  if (assemble_mosaic_mask)
  {
    std::cout << "    assembling a mosaic mask " << std::endl;
    mosaic_mask = make_mask<base_transform_t::Pointer>(transform, mask);
    
    // reset the mosaic image origin:
    mask_t::PointType origin = mosaic_mask->GetOrigin();
    origin[0] = 0;
    origin[1] = 0;
    mosaic_mask->SetOrigin(origin);
    
    if (flip_mosaic)
    {
      mosaic_mask = flip<mask_t>(mosaic_mask);
    }
  }
  
  // preprocess the image with Contrast Limited Adaptive Histogram
  // Equalization algorithm, remap the intensities into the [0, 1] range:
  if (assemble_mosaic && !dont_allocate && clahe_slope > 1.0)
  {
    std::cout << "    enhancing contrast with CLAHE " << std::endl;
    mosaic = CLAHE<T>(mosaic,
                      clahe_window,
                      clahe_window,
                      clahe_slope,
                      256,        // number of bins
                      clahe_new_min,
                      clahe_new_max,
                      mosaic_mask);
  }
  
  return true;
}

//----------------------------------------------------------------
// load_mosaic
// 
// Backwards compatibility API
// 
template <typename T>
bool
load_mosaic(const bfs::path & fn_mosaic,
            const bool & flip_mosaic,
            const unsigned int & shrink_factor,
            const double & clahe_slope,
            const bfs::path & image_path,
            typename T::Pointer & mosaic,
            mask_t::Pointer & mosaic_mask,
            bool mosaic_mask_enabled = true,
            bool dont_allocate = false,
            typename T::PixelType clahe_new_min = 0,
            typename T::PixelType clahe_new_max = 255)
{
  return load_mosaic<T>(fn_mosaic,
                        flip_mosaic,
                        shrink_factor,
                        true,                        // assemble mosaic
                        image_path,
                        mosaic,
                        mosaic_mask_enabled,        // assemble mosaic mask
                        mosaic_mask,
                        clahe_slope,
                        64,                        // clahe window
                        clahe_new_min,
                        clahe_new_max,
                        dont_allocate);
}

//----------------------------------------------------------------
// load_slice
// 
inline bool
load_slice(const bfs::path & fn_mosaic,
           const bool & flip,
           const unsigned int & shrink_factor,
           const double & clahe_slope,
           const bfs::path & image_path,
           image_t::Pointer & mosaic,
           mask_t::Pointer & mosaic_mask,
           bool mosaic_mask_enabled = true,
           bool dont_allocate = false)
{
  return load_mosaic<image_t>(fn_mosaic,
                              flip,
                              shrink_factor,
                              clahe_slope,
                              image_path,
                              mosaic,
                              mosaic_mask,
                              mosaic_mask_enabled,
                              dont_allocate,
                              0.0,
                              1.0);
}

//----------------------------------------------------------------
// load_slice
// 
inline static bool
load_slice(const bfs::path & fn_mosaic,
           const bool & flip,
           const unsigned int & shrink_factor,
           const double & clahe_slope,
           const bfs::path & image_path,
           image_t::Pointer & mosaic,
           bool dont_allocate = false)
{
  mask_t::Pointer dummy;
  return load_slice(fn_mosaic,
                    flip,
                    shrink_factor,
                    clahe_slope,
                    image_path,
                    mosaic,
                    dummy,
                    false,
                    dont_allocate);
}


//----------------------------------------------------------------
// stos_tree_t
// 
template <typename TImage>
class stos_tree_t
{
public:
  
  struct node_t;

  //----------------------------------------------------------------
  // link_t
  // 
  struct link_t
  {
    link_t()
    {}
    
    link_t(const boost::shared_ptr<node_t> & a,
           const boost::shared_ptr<node_t> & b,
           const stos_t<TImage> & stos):
      a_(a),
      b_(b),
      stos_(stos)
    {}
    
    inline bool operator < (const link_t & l) const
    {
      return (stos_.fn_[1] < l.stos_.fn_[1]);
    }
    
    boost::shared_ptr<node_t> a_;
    boost::shared_ptr<node_t> b_;
    stos_t<TImage> stos_;
  };

  //----------------------------------------------------------------
  // node_t
  // 
  struct node_t
  {
    node_t(const bfs::path & fn_data,
           const bfs::path & image_dir,
           const bfs::path & fn_mask,
           bool flipped,
           const vec2d_t & spacing):
      fn_data_(fn_data),
      image_dir_(image_dir),
      fn_mask_(fn_mask),
      flipped_(flipped),
      spacing_(spacing)
    {}
    
    // load the image data and image mask for this section:
    void
    load_data(typename TImage::Pointer & data,
              mask_t::Pointer & mask,
              unsigned int shrink_factor = 1,
              double clahe_slope = 1,
              unsigned int clahe_window = 64) const
    {
      if (fn_data_.extension() == ".mosaic")
      {
        load_mosaic<TImage>(fn_data_,
                            flipped_,
                            shrink_factor,
                            false,// assemble mosaic
                            image_dir_,
                            data,
                            true, // assemble mosaic mask
                            mask,
                            clahe_slope,
                            clahe_window);
      }
      else
      {
        data = std_tile<TImage>(fn_data_,
                                shrink_factor,
                                std::max(spacing_[0], spacing_[1]));
        
        if (fn_mask_.empty())
        {
          mask = cast<TImage, mask_t>(data);
          mask->FillBuffer(1);
        }
        else
        {
          mask = std_tile<mask_t>(fn_mask_,
                                  shrink_factor,
                                  std::max(spacing_[0], spacing_[1]));
        }
      }
    }
    
    // calc complete overlap region between this node
    // and all its children, recursively:
    mask_t::Pointer
    calc_overlap_mask(unsigned int shrink_factor = 1,
                      double clahe_slope = 1,
                      unsigned int clahe_window = 64) const
    {
      // load this nodes mask:
      typename TImage::Pointer data;
      mask_t::Pointer mask;
      load_data(data, mask, shrink_factor, clahe_slope, clahe_window);
      
      // we don't care about the image data here, get rid of it:
      data = NULL;
      
      // gather childrens masks and clip this nodes mask against it children:
      for (typename std::list<link_t>::const_iterator
             i = children_.begin(); i != children_.end(); ++i)
      {
        const link_t & link = *i;
        
        // recurse on the child:
        mask_t::Pointer imask = link.b_->calc_overlap_mask(shrink_factor,
                                                           clahe_slope,
                                                           clahe_window);

        // NOTE: this was added per James request --  don't clip
        // againt the child node if it is a leaf node.
        if (!link.b_->children_.empty())
        {
          // clip this nodes mask against the childs mask:
          base_transform_t::ConstPointer t = link.stos_.t01_.GetPointer();

          if ( mask != mask_t::Pointer(NULL) && imask != mask_t::Pointer(NULL) )
            clip_mask_a_by_b(mask, imask, t);
        }
      }
      
      // done:
      return mask;
    }
    
    // helper used by calc_overlap_mask:
    static void clip_mask_a_by_b(mask_t::Pointer & mask_a,
                                 mask_t::Pointer & mask_b,
                                 const base_transform_t::ConstPointer & t_ab)
    {
      // FIXME: this could be parallelized:
      
      // iterate over the previous mask, generate the next mask:
      typedef itk::LinearInterpolateImageFunction<mask_t, double> imask_t;
      imask_t::Pointer imask_b = imask_t::New();
      imask_b->SetInputImage(mask_b);
      
      // temporaries:
      pnt2d_t pt_a;
      pnt2d_t pt_b;
      
      const base_transform_t * t = t_ab.GetPointer();
      
      typedef typename TImage::RegionType rn_t;
      typedef typename TImage::RegionType::SizeType sz_t;
      typedef typename TImage::RegionType::IndexType ix_t;
      
      rn_t region = mask_a->GetLargestPossibleRegion();
      ix_t origin = region.GetIndex();
      sz_t extent = region.GetSize();
      
      typename ix_t::IndexValueType x_end = origin[0] + extent[0];
      typename ix_t::IndexValueType y_end = origin[1] + extent[1];
      
      ix_t ix = origin;
      for (ix[1] = origin[1]; ix[1] < y_end; ++ix[1])
      {
        for (ix[0] = origin[0]; ix[0] < x_end; ++ix[0])
        {
          if (mask_a->GetPixel(ix) == 0)
          {
            continue;
          }
          
          mask_a->TransformIndexToPhysicalPoint(ix, pt_a);
          pt_b = t->TransformPoint(pt_a);
          
          if (!imask_b->IsInsideBuffer(pt_b))
          {
            mask_a->SetPixel(ix, 0);
          }
          else
          {
            double val_mask_b = imask_b->Evaluate(pt_b);
            if (val_mask_b < 0.5)
            {
              mask_a->SetPixel(ix, 0);
            }
          }
        }
      }
    }
    
    void collect_stos(std::list<stos_t<TImage> > & stos) const
    {
      // gather mappings to children:
      for (typename std::list<link_t>::const_iterator
             i = children_.begin(); i != children_.end(); ++i)
      {
        const link_t & link = *i;
        stos.push_back(link.stos_);
      }
      
      // recurse on children:
      for (typename std::list<link_t>::const_iterator
             i = children_.begin(); i != children_.end(); ++i)
      {
        const link_t & link = *i;
        link.b_->collect_stos(stos);
      }
    }
    
    void
    dump(std::ostream & os)
    {
      for (typename std::list<link_t>::const_iterator
             i = children_.begin(); i != children_.end(); ++i)
      {
        const link_t & link = *i;
        os << fn_data_ << ':' << link.b_->fn_data_
           << ", " << link.stos_.fn_load_
           << '\n';
      }
      
      // recurse on children:
      for (typename std::list<link_t>::const_iterator
             i = children_.begin(); i != children_.end(); ++i)
      {
        const link_t & link = *i;
        link.b_->dump(os);
      }
    }
    
    // section info:
    bfs::path fn_data_;
    bfs::path image_dir_;
    bfs::path fn_mask_;
    bool flipped_;
    vec2d_t spacing_;
    
    // tree info:
    link_t parent_;
    std::list<link_t> children_;
  };
  
  //----------------------------------------------------------------
  // lookup
  // 
  // lookup a node by its name:
  boost::shared_ptr<node_t>
  lookup(const bfs::path & fn_data) const
  {
    for (typename std::list<boost::shared_ptr<node_t> >::const_iterator
           i = nodes_.begin(); i != nodes_.end(); ++i)
    {
      const boost::shared_ptr<node_t> & node = *i;

      if ( boost::iequals(node->fn_data_.string(), fn_data.string()) )
      {
        return node;
      }
    }
    
    return boost::shared_ptr<node_t>();
  }
  
  //----------------------------------------------------------------
  // get_root
  // 
  // lookup a root node:
  boost::shared_ptr<node_t>
  get_root(unsigned int root_index = 0)
  {
    unsigned int num_roots = 0;
    for (typename std::list<boost::shared_ptr<node_t> >::iterator
           i = nodes_.begin(); i != nodes_.end(); ++i)
    {
      const typename boost::shared_ptr<node_t> & node = *i;
      if (!node->parent_.a_)
      {
        if (num_roots == root_index)
        {
          return node;
        }
        
        ++num_roots;
      }
    }
    
    return boost::shared_ptr<node_t>();
  }
  
  //----------------------------------------------------------------
  // build
  // 
  // build a tree of slices from a given list of slice-to-slice mappings,
  // return the number of root nodes in the tree:
  // 
  std::size_t
  build(const std::list<stos_t<TImage> > & stos_list)
  {
    nodes_.clear();
    
    // add nodes to the tree:
    for (typename std::list<stos_t<TImage> >::const_iterator
           i = stos_list.begin(); i != stos_list.end(); ++i)
    {
      const stos_t<TImage> & stos = *i;

      boost::shared_ptr<node_t> a = lookup(stos.fn_[0]);
      boost::shared_ptr<node_t> b = lookup(stos.fn_[1]);
      
      if (!a)
      {
        a = boost::shared_ptr<node_t>(new node_t(stos.fn_[0],
                                                 stos.image_dirs_[0],
                                                 stos.fn_mask_[0],
                                                 stos.flipped_[0],
                                                 stos.sp_[0]));
        nodes_.push_back(a);
      }
      
      if (!b)
      {
        b = boost::shared_ptr<node_t>(new node_t(stos.fn_[1],
                                                 stos.image_dirs_[1],
                                                 stos.fn_mask_[1],
                                                 stos.flipped_[1],
                                                 stos.sp_[1]));
        nodes_.push_back(b);
      }
      
      boost::shared_ptr<node_t> prev_parent = b->parent_.a_;
      b->parent_ = link_t(a, b, stos);
      a->children_.push_back(link_t(a, b, stos));
      a->children_.sort();
      
      if (prev_parent)
      {
        // this shouldn't happen, a node can't have 2 parents in a tree:
        return 0;
      }
    }
    
    // count root nodes:
    std::size_t num_roots = 0;
    for (typename std::list<boost::shared_ptr<node_t> >::iterator
           i = nodes_.begin(); i != nodes_.end(); ++i)
    {
      const boost::shared_ptr<node_t> & node = *i;
      if (!node->parent_.a_)
      {
        num_roots++;
      }
    }
    
    return num_roots;
  }
  
  //----------------------------------------------------------------
  // get_cascade
  // 
  bool
  get_cascade(const bfs::path & src,
              const bfs::path & dst,
              std::vector<base_transform_t::Pointer> & cascade)
  {
    cascade.clear();
    
    if (src == dst)
    {
      return true;
    }
    
    boost::shared_ptr<node_t> node = lookup(dst);
    if (!node)
    {
      return false;
    }
    
    std::list<base_transform_t::Pointer> transforms;
    while (node->parent_.a_)
    {
      base_transform_t::Pointer t =
        const_cast<base_transform_t *>(node->parent_.stos_.t01_.GetPointer());
      transforms.push_front(t);
      
      if ( boost::iequals(node->parent_.a_->fn_data_.string(), src.string()) )
      {
        cascade.assign(transforms.begin(), transforms.end());
        return true;
      }
      
      node = node->parent_.a_;
    }
    
    // the dst node does not have src as a parent:
    return false;
  }
  
  // traverse the tree and collect the stos nodes:
  void
  collect_stos(std::list<stos_t<TImage> > & stos)
  {
    for (typename std::list<boost::shared_ptr<node_t> >::iterator
           i = nodes_.begin(); i != nodes_.end(); ++i)
    {
      const typename boost::shared_ptr<node_t> & node = *i;
      if (!node->parent_.a_)
      {
        node->collect_stos(stos);
      }
    }
  }
  
  void
  dump(std::ostream & os)
  {
    std::size_t root_count = 0;
    for (typename std::list<boost::shared_ptr<node_t> >::iterator
           i = nodes_.begin(); i != nodes_.end(); ++i)
    {
      const typename boost::shared_ptr<node_t> & node = *i;
      if (!node->parent_.a_)
      {
        root_count++;
        os << root_count << ". --------------------------------------------\n";
        node->dump(os);
        os << std::endl;
      }
    }
  }
  
  // the tree nodes (there may be more than one tree among them):
  std::list<boost::shared_ptr<node_t> > nodes_;
};


#endif // STOS_COMMON_HXX_
