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

#include <Application/ImageRegistrationTools/Actions/ActionSliceToVolumeFilter.h>


// local includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/STOS/stos.hxx>
#include <Core/ITKCommon/STOS/stos_common.hxx>
#include <Core/ITKCommon/Transform/itkGridTransform.h>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Transform/itkCascadedTransform.h>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>

// ITK includes:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>
#include <itkMeanSquaresImageToImageMetric.h>

// system includes:
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <sstream>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

// boost:
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace bfs=boost::filesystem;


CORE_REGISTER_ACTION( Seg3D, SliceToVolumeFilter )

namespace Seg3D
{
  

//----------------------------------------------------------------
// cascaded_transform_t
//
typedef itk::CascadedTransform<double, 2> cascaded_transform_t;

//----------------------------------------------------------------
// grid_transform_t
//
typedef itk::GridTransform grid_transform_t;

typedef std::list<bfs::path> pathList;

  
bool
ActionSliceToVolumeFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionSliceToVolumeFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(6);
    std::cout.setf(std::ios::scientific);
    
    track_progress(true);
    
    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);
    
    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);
    
    // parameters:
    pathList stos_fn;    
    for (size_t i = 0; i < this->input_files_.size(); ++i)
    {
      bfs::path stosInput(this->input_files_[i]);
      // needed, since mosaic loading functions do not check for file existence
      if (! bfs::exists(stosInput) )
      {
        std::ostringstream oss;
        oss << "STOS input file " << stosInput << " not found. Skipping...";
        CORE_LOG_WARNING(oss.str());
      }
      else
      {
        stos_fn.push_back(stosInput);
      }
    }
    
    std::list<stos_t<image_t> > stos_list;

    std::vector<std::vector<bfs::path > > image_dirs;

    std::vector<std::vector<bfs::path > > slice_dirs;
//    std::vector<std::vector<bfs::path > > slice_dirs(stos_fn.size() + 1);
//    for (size_t i = 0; i < this->slice_dirs_.size(); ++i)
//    {
//      slice_dirs[i].push_back(bfs::path(this->slice_dirs_[i]));
//    }

    // test
    std::vector<bfs::path> slice_dirs_test;
    for (size_t i = 0; i < this->slice_dirs_.size(); ++i)
    {
      slice_dirs_test.push_back(this->slice_dirs_[i]);
    }

    std::vector<bfs::path> image_dirs_test;
    for (size_t i = 0; i < this->image_dirs_.size(); ++i)
    {
      image_dirs_test.push_back(this->image_dirs_[i]);
    }
    // test
    
    bool old_image_list_type = true, old_slice_dirs_type = true;
    unsigned int image_list_idx = 0, slice_dirs_idx = 0;

  //  std::list<the_text_t>  fn_prefixes;
    pathList fn_prefixes;

//    bfs::path fn_extension(".tif");
    bfs::path fn_extension(".png");

    bool using_extra_padding = false;
    if (this->left_padding_ > 0 || this->right_padding_ > 0 ||
        this->top_padding_ > 0 || this->bottom_padding_ > 0)
    {
      using_extra_padding = true;
    }

    for (size_t i = 0; i < this->output_prefixes_.size(); ++i)
    {
      bfs::path prefix(this->output_prefixes_[i]);
      fn_prefixes.push_back(prefix);
    }

    int stos_names = stos_fn.size() + 1;
    slice_dirs.resize(stos_names);
    image_dirs.resize(stos_names);
    
    // Note: (Slice/Image Dirs) - BCG - I'm not exactly sure of the ramifications of
    //       overriding the .mosaic slices below.  It seems to work, but it hasn't
    //       been propogated through to the masks, to masks could still cause it to fail.
    //       At this point I'm opting to just notify the user with a warning.
    
    // If no slice_dirs were specified, use defaults.
    for (int i = 0; i < stos_names; ++i)
    {
      if ( slice_dirs[i].empty() )
        slice_dirs[i].push_back( bfs::path() );
      if ( slice_dirs[i].size() > 1 )
        std::cout << "WARNING: Overriding slice dirs with new .mosaic files has not been tested "
        << "thoroughly.  Problems may arise as a result of building the mosaic mask.  "
        << "Use at your own risk." << std::endl;
      if ( image_dirs[i].empty() )
        image_dirs[i].push_back( bfs::path() );
    }
    
    if ( fn_prefixes.size() == 0 || (fn_prefixes.size() > 1 && (fn_prefixes.size() != stos_fn.size() + 1) ) )
    {
      context->report_error("There must be exactly one prefix for each slice, or one prefix for all slices.");
      return false;
    }
    
    int i = 0;
  //  for (std::list<the_text_t>::const_iterator fn_iter = stos_fn.begin();
  //       fn_iter != stos_fn.end(); ++fn_iter, ++i)
    for (pathList::const_iterator fn_iter = stos_fn.begin(); fn_iter != stos_fn.end(); ++fn_iter, ++i)
    {
      bfs::path slice0_dir, slice1_dir, image0_dir, image1_dir;
      
// temporarily disabling...
//      if ( slice_dirs[i].size() > 0 )
//        slice0_dir = slice_dirs[i][0];
//      if ( slice_dirs[i+1].size() > 0 )
//        slice1_dir = slice_dirs[i+1][0];
//      
//      if ( image_dirs[i].size() > 0 )
//        image0_dir = image_dirs[i][0];
//      if ( image_dirs[i+1].size() > 0 )
//        image1_dir = image_dirs[i+1][0];

      // test
      slice0_dir = image_dirs_test.front();
      if (! bfs::is_directory(slice0_dir) )
      {
        std::ostringstream oss;
        oss << "Creating path to " << slice0_dir;
        CORE_LOG_DEBUG(oss.str());
        if (! boost::filesystem::create_directories(slice0_dir))
        {
          std::ostringstream oss;
          oss << "Could not create missing directory " << slice0_dir << " required to create output image.";
          context->report_error(oss.str());
          return false;
        }
      }

      slice1_dir = image_dirs_test.front();
      if (! bfs::is_directory(slice1_dir) )
      {
        std::ostringstream oss;
        oss << "Creating path to " << slice1_dir;
        CORE_LOG_DEBUG(oss.str());
        if (! boost::filesystem::create_directories(slice1_dir))
        {
          std::ostringstream oss;
          oss << "Could not create missing directory " << slice1_dir << " required to create output image.";
          context->report_error(oss.str());
          return false;
        }
      }

      // TODO: terrible - get a better handle on parameter
      if (image_dirs_test.size() > 1)
      {
        image0_dir = image_dirs_test[i];
        image1_dir = image_dirs_test[i+1];
      }
      else
      {
        image0_dir = image_dirs_test[0];
        image1_dir = image_dirs_test[0];
      }
      // test
      
      stos_t<image_t> stos( *fn_iter, slice0_dir, slice1_dir );
      stos.image_dirs_[0] = image0_dir;
      stos.image_dirs_[1] = image1_dir;
      
      stos_list.push_back( stos );
    }
    
    // a minimum of two images are required for the registration:
    const unsigned int num_pairs = stos_list.size();
    if (num_pairs == 0)
    {
      context->report_error("A minimum of two images are required for registration.");
      return false;
    }
    
    if (this->visualize_warp_)
    {
      // no point in wasting time on CLAHE -- the results
      // will be destroyed anyway:
      this->clahe_slope_ = 1.0;
    }
    
    // dump the status:
    std::cout << "shrink factor: " << this->shrink_factor_ << std::endl
    << "remap min max: " << this->remap_values_ << std::endl;
    
    if (this->clahe_slope_ > 1.0)
    {
      std::cout << "clahe window: " << this->clahe_window_ << std::endl
      << "clahe slope: " << this->clahe_slope_ << std::endl;
    }
    
    // shortcut:
    const unsigned int num_slices = num_pairs + 1;
    
    stos_tree_t<image_t> stos_tree;
    std::size_t num_roots = stos_tree.build(stos_list);
    
    if (num_roots != 1)
    {
      if (num_roots == 0)
      {
        context->report_error("There is more than one possible mapping for one or more slices, aborting...");
        context->report_error("Remove one of the ambiguous stos transforms and try again.");
      }
      else
      {
        std::ostringstream oss;
        oss << "There are " << num_roots << " sets of interconnected stos transforms present."
        << std::endl << "This tool only works with one set, aborting...";
        context->report_error(oss.str());
      }
      stos_tree.dump(std::cerr);
      
  //    ::exit(1);
      return false;
    }
    
    // this will sort the transforms in order of dependence and alphabetically:
    stos_list.clear();
    stos_tree.collect_stos(stos_list);
    
    // setup random access vectors:
    std::vector<stos_t<image_t> > stos(stos_list.begin(), stos_list.end());
    std::vector<bfs::path> fn_mosaic(num_slices);
    std::vector<bool> flip_mosaic(num_slices);
    std::vector<base_transform_t::Pointer> transform_stos(num_slices);
    std::vector<base_transform_t::Pointer> cascade(num_slices);
    {
      fn_mosaic[0] = stos[0].fn_[0];
      flip_mosaic[0] = stos[0].flipped_[0];
      
      // first slice is fixed:
      transform_stos[0] = identity_transform_t::New();
      cascade[0] = identity_transform_t::New();
      
      for (unsigned int i = 0; i < num_pairs; i++)
      {
        fn_mosaic[i + 1] = stos[i].fn_[1];
        flip_mosaic[i + 1] = stos[i].flipped_[1];
        transform_stos[i + 1] =
        const_cast<base_transform_t *>(stos[i].t01_.GetPointer());
        
        // collect transforms involved in mapping from slice 0 to slice i + 1:
        if ( this->add_transforms_ )
        {
          std::vector<base_transform_t::Pointer> cascade_0i1;
          stos_tree.get_cascade(fn_mosaic[0],
                                fn_mosaic[i + 1],
                                cascade_0i1);
          
          cascaded_transform_t::Pointer t = cascaded_transform_t::New();
          t->setup(cascade_0i1, cascade_0i1.size());
          cascade[i + 1] = t.GetPointer();
        }
      }
    }
    
    // find the mosaic section mask such that for each pixel there exists
    // a mapping to all of the sections:
    boost::shared_ptr<stos_tree_t<image_t>::node_t> tree_root = stos_tree.get_root();
    
    mask_t::Pointer overlap_mask;
    
    if ( (! this->use_base_mask_) && (! using_extra_padding) )
    {
      overlap_mask = tree_root->calc_overlap_mask(this->shrink_factor_,
                                                  DEFAULT_CLAHE_SLOPE,
                                                  DEFAULT_CLAHE_WINDOW);
    }
    else
    {
      image_t::Pointer data;
      tree_root->load_data(data,
                           overlap_mask,
                           this->shrink_factor_,
                           DEFAULT_CLAHE_SLOPE,
                           DEFAULT_CLAHE_WINDOW);
    }
    
    // find the bounding box of the final mask:
    mask_t::Pointer cropped_mask;
    {
      mask_t::IndexType min;
      mask_t::IndexType max;
      
      min[0] = std::numeric_limits<int>::max();
      min[1] = std::numeric_limits<int>::max();
      
      max[0] = std::numeric_limits<int>::min();
      max[1] = std::numeric_limits<int>::min();
      
      mask_t::IndexType index;
      
      typedef itk::ImageRegionConstIteratorWithIndex<mask_t> iter_t;
      iter_t iter(overlap_mask, overlap_mask->GetLargestPossibleRegion());
      
      for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
      {
        if (iter.Get() == 0) continue;
        
        index = iter.GetIndex();
        
        min[0] = std::min(min[0], index[0]);
        min[1] = std::min(min[1], index[1]);
        
        max[0] = std::max(max[0], index[0]);
        max[1] = std::max(max[1], index[1]);
      }
      
      std::cout << "bounding box (minx,miny maxx,maxy): "
      << min[0] << " " << min[1] << "  "
      << max[0] << " " << max[1] << std::endl;
      
      if (min[0] > max[0] || min[1] > max[1])
      {
        if (!this->use_base_mask_)
        {
          CORE_LOG_WARNING("There are no fully-overlapping pixels in this dataset. Using the base slice mask instead...");
          
          image_t::Pointer data;
          tree_root->load_data(data,
                               cropped_mask,
                               this->shrink_factor_,
                               DEFAULT_CLAHE_SLOPE,
                               DEFAULT_CLAHE_WINDOW);
        }
        else
        {
          context->report_error("The base slice mask is empty, aborting...");
          return false;
        }
      }
      else
      {
        cropped_mask = crop<mask_t>(overlap_mask, min, max);
      }
    }
    
    if ( using_extra_padding )
    {
      // Create a larger image of ones to use as a padding mask.
      mask_t::PointType padded_origin = cropped_mask->GetOrigin();
      padded_origin[0] -= this->left_padding_;
      padded_origin[1] -= this->top_padding_;
      
      mask_t::SizeType padded_size = cropped_mask->GetLargestPossibleRegion().GetSize();
      padded_size[0] += this->left_padding_ + this->right_padding_;
      padded_size[1] += this->top_padding_ + this->bottom_padding_;
      
      mask_t::Pointer large_mask = make_image<mask_t>(padded_origin,
                                                      cropped_mask->GetSpacing(),
                                                      padded_size);
      large_mask->FillBuffer(1);
      cropped_mask = large_mask;
    }
    
    // calculate downsample.
    double downsample = this->shrink_factor_;
    
    // FIXME: this is for debugging:
//#if 0
//  save<mask_t>(remap_min_max<mask_t>(cropped_mask),
//               fn_prefix + "mask.nrrd");
//#endif
    
    bool save_tiles = (this->tile_width_ != std::numeric_limits<unsigned int>::max() ||
                       this->tile_height_ != std::numeric_limits<unsigned int>::max());
    
    pathList::const_iterator prefix_iter = fn_prefixes.begin();
    
    // TODO: this needs to be a state instead
//    set_major_progress(0.05);
    
    // generate the slices:
    for (unsigned int i = 0; i < fn_mosaic.size(); i++)
    {
      int tot_slices = slice_dirs[i].size();
      int tot_images = image_dirs[i].size();
      int grand_total = (tot_slices > tot_images) ? tot_slices : tot_images;
      
      // load the image:
      std::vector<image_t::Pointer> slice(grand_total);
      mask_t::Pointer slice_mask;
      
      for ( int g = 0; g < grand_total; ++g )
      {
        bool valid_slice = (slice_dirs[i].size() > g) && (! slice_dirs[i][g].empty());
        bfs::path filename = fn_mosaic[i].filename();
        bfs::path new_slice_name = valid_slice ?  slice_dirs[i][g] / filename : fn_mosaic[i];
        
        bool valid_image = (image_dirs[i].size() > g);
        bfs::path new_image_name = (valid_image) ? image_dirs[i][g] : bfs::path();
        
        if (fn_mosaic[i].extension() == ".mosaic")
        {
          // assemble mosaic mask only if CLAHE
          // is requested, otherwise it's useless:
          const bool assemble_mosaic = true, assemble_mosaic_mask = this->clahe_slope_ > 1.0;
          load_mosaic<image_t>(new_slice_name,
                               flip_mosaic[i],
                               this->shrink_factor_,
                               assemble_mosaic,
                               new_image_name,
                               slice[g],
                               assemble_mosaic_mask,
                               slice_mask,
                               this->clahe_slope_,
                               this->clahe_window_);
        }
        else
        {
          unsigned int j = i == 0 ? 0 : i - 1;
          unsigned int k = i == 0 ? 0 : 1;
          const bool verbose = true;

          slice[g] = std_tile<image_t>(new_slice_name,
                                       this->shrink_factor_,
                                       std::max(stos[j].sp_[k][0], stos[j].sp_[k][1]),
                                       this->clahe_slope_,
                                       this->clahe_window_,
                                       verbose);
        }
      }
      
      if (this->visualize_warp_)
      {
        // draw a grid every N pixels:
        const unsigned int N = 16;
        for ( int sliceIdx = 0; sliceIdx < grand_total; ++sliceIdx )
        {
          slice[sliceIdx]->FillBuffer(0);
          
          typedef itk::ImageRegionIteratorWithIndex<image_t> iter_t;
          iter_t iter(slice[sliceIdx], slice[sliceIdx]->GetLargestPossibleRegion());
          
          for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
          {
            image_t::IndexType index = iter.GetIndex();
            if (index[0] % N == 0 || index[1] % N == 0)
            {
              iter.Set(255);
            }
          }
        }
      }
      
      std::vector<image_t::Pointer> mosaic(grand_total);
      for (int mosIdx = 0; mosIdx < grand_total; ++mosIdx)
      {
        mosaic[mosIdx] =
        make_image<image_t>(cropped_mask->GetOrigin(),
                            cropped_mask->GetSpacing(),
                            cropped_mask->GetLargestPossibleRegion().GetSize());
      }
      
      // temporaries:
      image_t::IndexType index;
      pnt2d_t pt[2];
      typedef itk::LinearInterpolateImageFunction<image_t, double>
      interpolator_t;
      std::vector<interpolator_t::Pointer> interpolator(grand_total);
      for (int ipIdx = 0; ipIdx < grand_total; ++ipIdx)
      {
        interpolator[ipIdx] = interpolator_t::New();
        interpolator[ipIdx]->SetInputImage(slice[ipIdx]);
      }
      
      // iterate over the mosaic image, check against the mask,
      // and fill in the slice pixels:
      typedef itk::ImageRegionIteratorWithIndex<image_t> iter_t;
      std::vector<iter_t> iter;
      iter.reserve(grand_total);
      for ( int itIdx = 0; itIdx < grand_total; ++itIdx )
      {
        iter.push_back(iter_t(mosaic[itIdx],
                              mosaic[itIdx]->GetLargestPossibleRegion()));
      }
      
      for ( int itIdx = 0; itIdx < grand_total; ++itIdx )
        iter[itIdx].GoToBegin();
      for ( ; !iter[0].IsAtEnd(); )
      {
        index = iter[0].GetIndex();
        
        // 1. check the mask:
        if (cropped_mask->GetPixel(index) == 0)
        {
          for ( int itIdx = 0; itIdx < grand_total; ++itIdx )
            ++iter[itIdx];
          continue;
        }
        
        // 2. find where this point maps to in the current slice:
        mosaic[0]->TransformIndexToPhysicalPoint(index, pt[0]);
        
        if ( this->add_transforms_ )
        {
          // Run through entire cascade.
          const base_transform_t * t = cascade[i].GetPointer();
          pt[1] = t->TransformPoint(pt[0]);
          pt[0] = pt[1];
        }
        else
        {
          // Use single transform.
          pt[1] = transform_stos[i]->TransformPoint(pt[0]);
          pt[0] = pt[1];
        }
        
        for ( int ipIdx = 0; ipIdx < grand_total; ++ipIdx )
        {
          if (!interpolator[ipIdx]->IsInsideBuffer(pt[0]))
            break;
          
          pixel_t p = interpolator[ipIdx]->Evaluate(pt[0]);
          iter[ipIdx].Set(p);
        }
        
        for ( int itIdx = 0; itIdx < grand_total; ++itIdx )
          ++iter[itIdx];
      }
      
  //    the_text_t fn_filename = *prefix_iter;
      std::ostringstream fn_filename;
      fn_filename << prefix_iter->string();
      
      if ( fn_prefixes.size() != 1 )
        ++prefix_iter;
      else
        fn_filename << the_text_t::number(i, 3, '0');
      
      for ( int mosIdx = 0; mosIdx < grand_total; ++mosIdx )
      {
        // save the slice:
        std::ostringstream fn_save;
        fn_save << fn_filename.str();
        if ( grand_total > 1 && boost::iequals(fn_extension.string(), ".idx") )
        {
          fn_save << "_" << the_text_t::number(mosIdx, 3, '0');
        }
        fn_save << fn_extension;
        
        if (this->remap_values_)
        {
          remap_min_max_inplace<image_t>(mosaic[mosIdx]);
        }
        
        if (save_tiles)
        {
          if ( this->save_int16_image_ )
          {
            typedef itk::Image<short int, 2> int16_image_t;
            save_as_tiles<int16_image_t>(cast<image_t, int16_image_t>(mosaic[mosIdx]),
                                         bfs::path(fn_filename.str()),
                                         fn_extension,
                                         this->tile_width_,
                                         this->tile_height_,
                                         downsample);
          }
          else if ( this->save_uint16_image_ )
          {
            typedef itk::Image<unsigned short int, 2> uint16_image_t;
            save_as_tiles<uint16_image_t>(cast<image_t, uint16_image_t>(mosaic[mosIdx]),
                                          bfs::path(fn_filename.str()),
                                          fn_extension,
                                          this->tile_width_,
                                          this->tile_height_,
                                          downsample);
          }
          else
          {
            save_as_tiles<native_image_t>(cast<image_t, native_image_t>(mosaic[mosIdx]),
                                          bfs::path(fn_filename.str()),
                                          fn_extension,
                                          this->tile_width_,
                                          this->tile_height_,
                                          downsample);
          }
        }
        else
        {
          if ( this->save_int16_image_ )
          {
            typedef itk::Image<short int, 2> int16_image_t;
            save<int16_image_t>(cast<image_t, int16_image_t>(mosaic[mosIdx]), bfs::path(fn_save.str()));
          }
          else if ( this->save_uint16_image_ )
          {
            typedef itk::Image<unsigned short int, 2> uint16_image_t;
            save<uint16_image_t>(cast<image_t, uint16_image_t>(mosaic[mosIdx]), bfs::path(fn_save.str()));
          }
          else
          {
//            save<native_image_t>(cast<image_t, native_image_t>(mosaic[mosIdx]), bfs::path(fn_save.str()));
            bfs::path save_path = fn_prefixes.front() / fn_mosaic[i].stem();
            save_path = save_path.replace_extension(fn_extension);
            save<native_image_t>(cast<image_t, native_image_t>(mosaic[mosIdx]), save_path);
          }
          
        }
      }
      // TODO: this needs to be a state instead
//      set_major_progress(0.05 + 0.95 * ((i+1) / (fn_mosaic.size() + 1)));
    }
    
    CORE_LOG_SUCCESS("ir-stom done");

    return true;
  }
  catch (bfs::filesystem_error &err)
  {
    context->report_error(err.what());
  }
  catch (itk::ExceptionObject &err)
  {
    context->report_error(err.GetDescription());
  }
  catch (Core::Exception &err)
  {
    context->report_error(err.what());
    context->report_error(err.message());
  }
  catch (std::exception &err)
  {
    context->report_error(err.what());
  }
  catch (...)
  {
    context->report_error("Unknown exception type caught.");
  }
  return false;
}

void
  ActionSliceToVolumeFilter::Dispatch(Core::ActionContextHandle context,
                                      std::string target_layer,
                                      unsigned int shrink_factor,
                                      unsigned int clahe_window,
                                      unsigned int left_padding,
                                      unsigned int top_padding,
                                      unsigned int right_padding,
                                      unsigned int bottom_padding,
                                      unsigned int tile_width,
                                      unsigned int tile_height,
                                      double clahe_slope,
                                      bool visualize_warp,
                                      bool use_base_mask,
                                      bool add_transforms,
                                      bool remap_values,
                                      bool save_int16_image,
                                      bool save_uint16_image,
                                      std::vector<std::string> input_files,
                                      std::vector<std::string> output_prefixes,
                                      std::vector<std::string> slice_dirs,
                                      std::vector<std::string> image_dirs)
{
  // Create a new action
  ActionSliceToVolumeFilter* action = new ActionSliceToVolumeFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->shrink_factor_ = shrink_factor;
  action->clahe_window_ = clahe_window;
  action->left_padding_ = left_padding;
  action->top_padding_ = top_padding;
  action->right_padding_ = right_padding;
  action->bottom_padding_ = bottom_padding;
  action->tile_width_ = tile_width;
  action->tile_height_ = tile_height;
  action->clahe_slope_ = clahe_slope;
  action->visualize_warp_ = visualize_warp;
  action->use_base_mask_ = use_base_mask;
  action->add_transforms_ = add_transforms;
  action->remap_values_ = remap_values;
  action->save_int16_image_ = save_int16_image;
  action->save_uint16_image_ = save_uint16_image;
  action->input_files_ = input_files;
  action->output_prefixes_ = output_prefixes;
  action->slice_dirs_ = slice_dirs;
  action->image_dirs_ = image_dirs;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
