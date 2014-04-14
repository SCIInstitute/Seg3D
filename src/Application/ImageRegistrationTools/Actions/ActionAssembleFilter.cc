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

#include <Application/ImageRegistrationTools/Actions/ActionAssembleFilter.h>

// the includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/Optimizers/itkImageMosaicVarianceMetric.h>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>
#include <Core/ITKCommon/the_dynamic_array.hxx>
#include <Core/ITKCommon/AsyncMosiacSave.h>

// system includes:
#include <cmath>
#include <sstream>

// Application includes
//#include <Application/Layer/LayerManager.h>

#include <Core/Utils/Log.h>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

CORE_REGISTER_ACTION( Seg3D, AssembleFilter )

namespace Seg3D
{
  
//class ActionAssembleFilterPrivate
//{
//};

bool
ActionAssembleFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionAssembleFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(2);
    std::cout.setf(std::ios::scientific);
    
    track_progress(true);
    
    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);
    
    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);
    
    bool pixel_spacing_user_override = false;
    if (this->pixel_spacing_ != 1.0) // TODO: improve this check
    {
      pixel_spacing_user_override = true;
    }
    std::list<bfs::path> in;
    bfs::path fn_save(this->output_image_);
    if (! bfs::is_directory( fn_save.parent_path() ) )
    {
      CORE_LOG_DEBUG(std::string("Creating parent path to ") + this->output_image_);
      if (! boost::filesystem::create_directories(fn_save.parent_path()))
      {
        std::ostringstream oss;
        oss << "Could not create missing directory " << fn_save.parent_path() << " required to create output image.";
        CORE_LOG_ERROR(oss.str());
        return false;
      }
    }

    bfs::path extension;
    bfs::path filename;
    bfs::path fn_mask;
    if (this->mask_ != "<none>")
    {
     fn_mask = this->mask_;
    }
    feathering_t feathering_val = FEATHER_NONE_E;
    if (this->feathering_ == "blend")
    {
      feathering_val = FEATHER_BLEND_E;
    }
    else if (this->feathering_ == "binary")
    {
      feathering_val = FEATHER_BINARY_E;
    }

    // debug...
    // TODO: expose?
    bool verbose = true;
    
    bfs::path fn_in(this->input_mosaic_);
    if (! bfs::exists(fn_in) && ! bfs::is_regular_file(this->input_mosaic_) )
    {
      std::ostringstream oss;
      oss << fn_in << " cannot be found." << std::endl;
      CORE_LOG_ERROR(oss.str());
      // return false;
    }

    if (fn_in.extension() != ".mosaic")
    {
      CORE_LOG_WARNING(fn_in.string() + " does not have the .mosaic extension.");
    }

    bfs::path image_dir(this->directory_);
    if (! bfs::exists(image_dir) )
    {
      std::ostringstream oss;
      oss << image_dir << " cannot be found." << std::endl;
      CORE_LOG_ERROR(oss.str());
      // return false;
    }
    
    // use as many threads as supported by hardware:
  //  unsigned int num_threads = boost::thread::hardware_concurrency();
    if (this->num_threads_ == 0)
    {
      this->num_threads_ = boost::thread::hardware_concurrency();
    }

    std::vector<base_transform_t::Pointer> transform;
    std::vector<image_t::ConstPointer> image;
    std::vector<mask_t::ConstPointer> mask;
    
    // extract output filename extension (excluding the '.'):
    //  extension = fn_save.reverse().cut('.', 0, 0).reverse();
    extension = fn_save.extension(); // includes '.'
    
    // extract output filename (excluding the '.' and extension):
    //  filename = fn_save.extract(0, fn_save.size() - extension.size() - 1);
    filename = fn_save.stem();
    
    // Note: shouldn't be needed for boost::filesystem::path
    // prepend '.' to the output filename extension:
    //  if (strcmp(extension.c_str(), "xml") != 0)
    //  {
    //    extension = the_text_t(".") + extension;
    //  }
    
    // Load in cached .mosaic:
    std::fstream fin;
    fin.open(fn_in.c_str(), std::ios::in);
    if (!fin.is_open())
    {
      std::ostringstream oss;
      oss << "could not open " << fn_in << " for reading";
      CORE_LOG_ERROR(oss.str());
      return false;
    }
    
    double mosaic_pixel_spacing = 0;
    load_mosaic<base_transform_t>(fin,
                                  mosaic_pixel_spacing,
                                  this->use_standard_mask_,
                                  in,
                                  transform,
                                  image_dir);
    
    if (! pixel_spacing_user_override)
    {
      this->pixel_spacing_ = mosaic_pixel_spacing;
    }
    
    unsigned int num_images = transform.size();
  //  assert(this->pixel_spacing_ != 0);
    if (this->pixel_spacing_ == 0)
    {
      CORE_LOG_ERROR("Pixel spacing is zero");
      return false;
    }
    
    image.resize(num_images);
    mask.resize(num_images);
    
    if ( ! this->defer_image_loading_ )
    {
      unsigned int i = 0;
      //    for (std::list<the_text_t>::const_iterator iter = in.begin();
      //         iter != in.end(); ++iter, i++)
      for (std::list<bfs::path>::const_iterator iter = in.begin();
           iter != in.end(); ++iter, i++)
      {
        // Preload all images, instead of loading them as needed.
        // TODO: debug log...
        std::cout << std::setw(3) << i << " ";
        image[i] = std_tile<image_t>((*iter).c_str(), 
                                     this->shrink_factor_, this->pixel_spacing_);
        if (this->use_standard_mask_)
        {
          mask[i] = std_mask<image_t>(image[i]);
        }
        
        if (this->clahe_slope_ > 1.0)
        {
          image[i] = CLAHE<image_t>(image[i],
                                    CLAHE_DEFAULT_NX,
                                    CLAHE_DEFAULT_NY,
                                    this->clahe_slope_,
                                    CLAHE_DEFAULT_BINS,
                                    0.0,
                                    255.0,
                                    mask[i]);
        }
      }
    }
    
    fin.close();
    
    set_major_progress(0.05);
    
    // calculate downsample.
    double downsample = this->shrink_factor_ * this->pixel_spacing_;
    
    bool save_tiles = (this->tile_width_ != std::numeric_limits<unsigned int>::max() ||
                       this->tile_height_ != std::numeric_limits<unsigned int>::max());
    
    // sanity checks:
    if (fn_save.empty())
    {
      CORE_LOG_ERROR("Missing output filename.");
      return false;
    }
    
    if (in.empty())
    {
      CORE_LOG_ERROR("Missing input file(s).");
      return false;
    }
    
    if (this->defer_image_loading_ && !save_tiles)
    {
      CORE_LOG_ERROR("Defer image loading requires tile_width and tile_height arguments.");
      return false;
    }
    
    num_images = in.size();
    std::cout << "threads:       " << this->num_threads_ << std::endl
    << "mosaic tiles:  " << num_images << std::endl
    << "shrink factor: " << this->shrink_factor_ << std::endl
    << "pixel spacing: " << this->pixel_spacing_ << std::endl
    << "save variance: " << this->save_variance_ << std::endl
    << "use feathering: " << feathering_val << std::endl
    << "remap min max: " << this->remap_values_ << std::endl
    << std::endl;
    
    image_t::Pointer mosaic;
    image_t::Pointer full_mosaic;
    mask_t::Pointer mosaic_mask;
    image_t::SizeType mosaic_sz;
    the_thread_pool_t thread_pool(1);
    
    if ( !save_tiles && this->defer_image_loading_ )
    {
      // Setup the full mosaic to copy to.
    }
    
    if (this->save_variance_)
    {
      std::cout << "assembling mosaic variance image..." << std::endl;
      
      typedef itk::LinearInterpolateImageFunction<image_t, double>
      image_interpolator_t;
      
      typedef itk::ImageMosaicVarianceMetric<image_t, image_interpolator_t>
      mosaic_metric_t;
      
      mosaic_metric_t::Pointer mosaic_metric = mosaic_metric_t::New();
      mosaic_metric->image_.resize(num_images);
      mosaic_metric->mask_.resize(num_images);
      mosaic_metric->transform_.resize(num_images);
      for (unsigned int i = 0; i < num_images; i++)
      {
        mosaic_metric->image_[i] = image[i];
        mosaic_metric->mask_[i] = mask[i];
        mosaic_metric->transform_[i] = transform[i];
      }
      
      std::vector<bool> param_shared;
      std::vector<bool> param_active;
      param_shared.assign(transform[0]->GetNumberOfParameters(), false);
      param_active.assign(transform[0]->GetNumberOfParameters(), false);
      mosaic_metric->setup_param_map(param_shared, param_active);
      mosaic_metric->Initialize();
      
      mosaic_metric_t::measure_t max_var;
      mosaic_metric_t::measure_t avg_var;
      mosaic = mosaic_metric->variance(max_var, avg_var);
      std::cout << "max variance: " << max_var << std::endl
      << "avg variance: " << avg_var << std::endl;
    }
    else
    {
      const std::vector<double> tint(num_images, 1.0);
      const std::vector<bool> omit(num_images, false);
      
      // mosaic bounding box:
      image_t::PointType mosaic_min;
      image_t::PointType mosaic_max;
      std::vector<image_t::PointType> image_min;
      std::vector<image_t::PointType> image_max;
      if ( this->defer_image_loading_ )
      {
        // Creates the bounding boxes for all images, but loads and unloads
        // each image as needed.
        calc_mosaic_bbox_load_images<image_t::ConstPointer, 
        base_transform_t::Pointer>
        (transform,
         in,
         mosaic_min,
         mosaic_max,
         image_min,
         image_max,
         this->shrink_factor_,
         this->pixel_spacing_);
      }
      else
      {
        calc_mosaic_bbox<image_t::ConstPointer, base_transform_t::Pointer>
        (transform,
         image,
         mosaic_min,
         mosaic_max);
      }
      
      if ( this->defer_image_loading_ )
      {
        image[0] = std_tile<image_t>(in.front(), 
                                     this->shrink_factor_, 
                                     this->pixel_spacing_);
      }
      
      image_t::SpacingType mosaic_sp = image.front()->GetSpacing();
      if ( this->defer_image_loading_ )
      {
        image[0] = image_t::Pointer(NULL);
      }
      
      mosaic_sz[0] = static_cast<unsigned int>((mosaic_max[0] - mosaic_min[0]) / mosaic_sp[0]);
      mosaic_sz[1] = static_cast<unsigned int>((mosaic_max[1] - mosaic_min[1]) / mosaic_sp[1]);
      
      bool defer_for_w = (this->defer_image_loading_ 
                          && this->tile_width_ != std::numeric_limits<unsigned int>::max());
      bool defer_for_h = (this->defer_image_loading_ 
                          && this->tile_height_ != std::numeric_limits<unsigned int>::max());
      
      image_t::PointType step_sz;
      step_sz[0] = defer_for_w ? this->tile_width_  : (mosaic_max[0] - mosaic_min[0]);
      step_sz[1] = defer_for_h ? this->tile_height_ : (mosaic_max[1] - mosaic_min[1]);
      
      image_t::PointType deferred_tile_sz;
      deferred_tile_sz[0] = defer_for_w ? step_sz[0] : mosaic_sz[0];
      deferred_tile_sz[1] = defer_for_h ? step_sz[1] : mosaic_sz[1];
      
      std::cout << std::endl << std::endl << "assembling mosaic..." << std::endl;
      
      image_t::PointType tile_min = mosaic_min;
      image_t::PointType tile_max = mosaic_max;
      unsigned int column_number = 0;
      unsigned int total_columns = (mosaic_sz[0] + (this->tile_width_ - 1)) / this->tile_width_;
      for (unsigned int xid = 0; tile_min[0] + 0.001 < mosaic_max[0]; 
           tile_min[0] += step_sz[0], xid++)
      {
        tile_min[1] = mosaic_min[1];
        bool load_column = true;
        column_number++;
        for (unsigned int yid = 0; tile_min[1] + 0.001 < mosaic_max[1]; 
             tile_min[1] += step_sz[1], yid++)
        {
          tile_max[0] = tile_min[0] + deferred_tile_sz[0];
          tile_max[1] = tile_min[1] + deferred_tile_sz[1];
          
          image_t::SizeType tile_sz;
          tile_sz[0] = static_cast<unsigned int>(tile_max[0] - tile_min[0]);
          tile_sz[1] = static_cast<unsigned int>(tile_max[1] - tile_min[1]);
          
          bool assemble_mosaic_mask = ! fn_mask.empty();
          
          bool overlaps = !this->defer_image_loading_;
          if ( this->defer_image_loading_ )
          {
            // Check which images are needed.  Load them and apply mask/clahe.
            // Note: using clahe and mask with defer_image_loading is a bad idea.
            
            unsigned int i = 0;
            //          for (std::list<the_text_t>::const_iterator iter = in.begin();
            //               iter != in.end(); ++iter, i++)
            for (std::list<bfs::path>::const_iterator iter = in.begin();
                 iter != in.end(); ++iter, i++)
            {
              // Check if this specific tile overlaps
              if ( bbox_overlap(tile_min, tile_max, image_min[i], image_max[i]) )
                overlaps = true;
              
              if ( !load_column )
                continue;
              
              image_t::PointType column_max = tile_max;
              column_max[1] = static_cast<unsigned int>(mosaic_max[1]);
              if ( bbox_overlap(tile_min, column_max, image_min[i], image_max[i]) )
              {
                if ( image[i].IsNull() )
                {
                  image[i] = std_tile<image_t>((*iter).c_str(), 
                                               this->shrink_factor_, 
                                               this->pixel_spacing_,
                                               verbose);
                  
                  if (this->use_standard_mask_)
                  {
                    mask[i] = std_mask<image_t>(image[i]);
                  }
                  
                  if (this->clahe_slope_ > 1.0)
                  {
                    image[i] = CLAHE<image_t>(image[i],
                                              255,
                                              255,
                                              this->clahe_slope_,
                                              256,
                                              0.0,
                                              255.0,
                                              mask[i]);
                  }
                }
              }
              else if ( image[i].IsNotNull() )
              {
                if ( verbose )
                  std::cout << "unloading " << (*iter) << std::endl;
                image[i] = image_t::Pointer(NULL);
              }
            }
          }
          
          // Only load the column from the first tile.
          load_column = false;
          
          if ( !overlaps )
          {
            if ( verbose )
            {
              std::cout << "Skipping tile, no overlap was found..." << std::endl;
            }
            continue;
          }
          
          if ( verbose )
          {
            if (!assemble_mosaic_mask)
            {
              std::cout << "assembling mosaic..." << std::endl;
            }
            else
            {
              std::cout << "assembling mosaic and mosaic mask..." << std::endl;
            }
          }
          
          mosaic =
          make_mosaic_mt<image_t::ConstPointer, base_transform_t::Pointer>(this->num_threads_,
                                                                           assemble_mosaic_mask,
                                                                           mosaic_mask,
                                                                           mosaic_sp,
                                                                           tile_min,
                                                                           tile_sz,
                                                                           image.size(),
                                                                           omit,
                                                                           tint,
                                                                           transform,
                                                                           image,
                                                                           mask,
                                                                           feathering_val,
                                                                           0.0,           // background
                                                                           false);        // don't allocate flag
          
          // TODO: duplicated code in common.hxx
          if ( this->defer_image_loading_ && save_tiles )
          {
            //          the_text_t fn_partialSave = filename;
            bfs::path fn_partialSave = filename;
            fn_partialSave += "_X";
            fn_partialSave += the_text_t::number(xid, 3, '0');
            fn_partialSave += "_Y";
            fn_partialSave += the_text_t::number(yid, 3, '0');
            fn_partialSave += extension;
            
            if (fn_save.extension() == ".mha" ||
                fn_save.extension() == ".mhd" ||
                fn_save.extension() == ".nrrd")
            {
              save_mosaic_async<image_t>(mosaic, fn_partialSave, thread_pool);
            }
            else if (this->save_uint16_image_)
            {
              typedef itk::Image<unsigned short int, 2> uint16_image_t;
              
              save_mosaic_async<uint16_image_t>(cast<image_t, uint16_image_t>(mosaic), 
                                                fn_partialSave, thread_pool, verbose);
            }
            else if (this->save_int16_image_)
            {
              typedef itk::Image<short int, 2> int16_image_t;
              
              save_mosaic_async<int16_image_t>(cast<image_t, int16_image_t>(mosaic), 
                                               fn_partialSave, thread_pool, verbose);
            }
            else
            {
              save_mosaic_async<native_image_t>(cast<image_t, native_image_t>(mosaic), 
                                                fn_partialSave, thread_pool, verbose);
            }
          }
          else
          {
            // Copy mosaic to larger mosaic and continue...
          }
        }
        std::cout << "Column " << column_number << " of " << total_columns << " was completed." << std::endl;
      }
    }
    
    if ( this->defer_image_loading_ && save_tiles )
    {
      thread_pool.wait();
    }
    
    // We want to unload them as quick as we can.
    if ( this->defer_image_loading_ )
    {
      unsigned int i = 0;
      //    for (std::list<the_text_t>::const_iterator iter = in.begin();
      //         iter != in.end(); ++iter, i++)
      for (std::list<bfs::path>::const_iterator iter = in.begin();
           iter != in.end(); ++iter, i++)
      {
        if ( image[i].IsNull() )
        {
          continue;
        }
        
        if ( verbose )
          std::cout << "unloading " << (*iter).string() << std::endl;
        image[i] = image_t::Pointer(NULL);
      }
    }
    
    // Write out the xml, signaling the finish.
    if ( this->defer_image_loading_ && save_tiles )
    {
      save_tile_xml<native_image_t>(filename,
                                    extension,
                                    this->tile_width_,
                                    this->tile_height_,
                                    mosaic_sz[0],
                                    mosaic_sz[1],
                                    downsample,
                                    false);
    }
    
    if (this->remap_values_)
    {
      remap_min_max_inplace<image_t>(mosaic);
    }
    
    if ( !this->defer_image_loading_ )
    {
      if (fn_save.extension() == ".mha" ||
          fn_save.extension() == ".mhd" ||
          fn_save.extension() == ".nrrd")
      {
        if (save_tiles)
        {
          save_as_tiles<image_t>(mosaic,
                                 filename,
                                 extension,
                                 this->tile_width_,
                                 this->tile_height_,
                                 downsample);
        }
        else
        {
          save<image_t>(mosaic, fn_save);
        }
      }
      else if (this->save_uint16_image_)
      {
        typedef itk::Image<unsigned short int, 2> uint16_image_t;
        if (save_tiles)
        {
          save_as_tiles<uint16_image_t>(cast<image_t, uint16_image_t>(mosaic),
                                        filename,
                                        extension,
                                        this->tile_width_,
                                        this->tile_height_, 
                                        downsample);
        }
        else
        {
          save<uint16_image_t>(cast<image_t, uint16_image_t>(mosaic), fn_save);
        }
      }
      else if (this->save_int16_image_)
      {
        typedef itk::Image<short int, 2> int16_image_t;
        if (save_tiles)
        {
          save_as_tiles<int16_image_t>(cast<image_t, int16_image_t>(mosaic),
                                       filename,
                                       extension,
                                       this->tile_width_,
                                       this->tile_height_,
                                       downsample);
        }
        else
        {
          save<int16_image_t>(cast<image_t, int16_image_t>(mosaic), fn_save);
        }
      }
      else
      {
        if (save_tiles)
        {
          save_as_tiles<native_image_t>(cast<image_t, native_image_t>(mosaic),
                                        filename,
                                        extension,
                                        this->tile_width_,
                                        this->tile_height_,
                                        downsample);
        }
        else
        {
          save<native_image_t>(cast<image_t, native_image_t>(mosaic), fn_save);
        }
      }
    }
    
    if (fn_mask.empty())
    {
      // done, no mask:
      CORE_LOG_SUCCESS("ir-assemble done");
      return true;
    }
    
    if (mosaic_mask.GetPointer() == NULL)
    {
      // assemble the mosaic mask:
      std::cout << "assembling mosaic mask..." << std::endl;
      if (!this->use_standard_mask_)
      {
        for (unsigned int i = 0; i < num_images; i++)
        {
          // since the standard mask was not used
          // assume the whole image is masked:
          mask[i] = std_mask<image_t>(image[i], false);
          
          // free up the image memory, we no longer need it:
          image[i] = image_t::Pointer(NULL);
        }
      }
      
      mosaic_mask = make_mask<base_transform_t::Pointer>(transform, mask);
    }
    
    // save the mosaic mask:
    save<mask_t>(mosaic_mask, fn_mask);
    
    CORE_LOG_SUCCESS("ir-assemble done");
    
    return true;
  }
  catch (bfs::filesystem_error &err)
  {
    CORE_LOG_ERROR(err.what());
  }
  catch (itk::ExceptionObject &err)
  {
    CORE_LOG_ERROR(err.GetDescription());
  }
  catch (Core::Exception &err)
  {
    CORE_LOG_ERROR(err.what());
    CORE_LOG_ERROR(err.message());
  }
  catch (std::exception &err)
  {
    CORE_LOG_ERROR(err.what());
  }
  catch (...)
  {
    CORE_LOG_ERROR("Unknown exception type caught.");
  }
  return false;
}

void
ActionAssembleFilter::Dispatch(Core::ActionContextHandle context,
                               std::string target_layer,
                               unsigned int shrink_factor,
                               unsigned int num_threads,
                               unsigned int tile_width,
                               unsigned int tile_height,
                               double pixel_spacing,
                               double clahe_slope,
                               bool use_standard_mask,
                               bool save_int16_image,
                               bool save_uint16_image,
                               bool save_variance,
                               bool remap_values,
                               bool defer_image_loading,
                               std::string input_mosaic,
                               std::string output_image,
                               std::string directory,
                               std::string mask,
                               std::string feathering)
{
  // Create a new action
  ActionAssembleFilter* action = new ActionAssembleFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->shrink_factor_ = shrink_factor;
  action->num_threads_ = num_threads;
  action->tile_width_ = tile_width;
  action->tile_height_ = tile_height;
  action->pixel_spacing_ = pixel_spacing;
  action->clahe_slope_ = clahe_slope;
  action->use_standard_mask_ = use_standard_mask;
  action->save_int16_image_ = save_int16_image;
  action->save_uint16_image_ = save_uint16_image;
  action->save_variance_ = save_variance;
  action->remap_values_ = remap_values;
  action->defer_image_loading_ = defer_image_loading;
  action->input_mosaic_ = input_mosaic;
  action->output_image_ = output_image;
  action->directory_ = directory;
  action->mask_ = mask;
  action->feathering_ = feathering;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
