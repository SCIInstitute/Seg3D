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

#include <Application/ImageRegistrationTools/Actions/ActionCLAHEFilter.h>

// the includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>

// system includes:
#include <math.h>
#include <algorithm>

// ITK includes:
#include <itkMedianImageFilter.h>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;


CORE_REGISTER_ACTION( Seg3D, CLAHEFilter )

namespace Seg3D
{

bool
ActionCLAHEFilter::validate( Core::ActionContextHandle& context )
{
  if ( this->max_slope_ < 1.0 && this->max_slope_ != 0 )
  {
    context->report_error("The max_slope parameter needs to be greater than 1.0 or equal to 0.");
    return false;
  }

  if (this->window_x_ < 2)
  {
    context->report_error("The window_x parameter needs to be greater than or equal to 2.");
    return false;
  }

  if (this->window_y_ < 2)
  {
    context->report_error("The window_y parameter needs to be greater than or equal to 2.");
    return false;
  }

  return true;
}

bool
ActionCLAHEFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(2);
    std::cout.setf(std::ios::scientific);
    
    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);
    
    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);

    bfs::path fn_mask;
    if (this->mask_ != "<none>")
    {
      fn_mask = this->mask_;
    }
    if ( fn_mask.empty() )
    {
      context->report_warning("Empty mask path, ignoring mask parameter.");
    }
    if (! bfs::exists(fn_mask) )
    {
      std::ostringstream oss;
      oss << "Could not find mask file " << fn_mask << ", ignoring mask parameter.";
      context->report_warning(oss.str());
      fn_mask.clear();
    }
    
    bfs::path fn_load(this->input_image_);    
    if ( fn_load.empty() )
    {
      context->report_error("Missing input image file name.");
      return false;
    }
    if (! bfs::exists(fn_load) )
    {
      std::ostringstream oss;
      oss << "Could not find image file " << fn_load;
      context->report_error(oss.str());
      return false;
    }

    bfs::path fn_save(this->output_image_);    
    if ( fn_save.empty() )
    {
      context->report_error("Missing output image file name.");
      return false;
    }
    if (! bfs::is_directory( fn_save.parent_path() ) )
    {
      CORE_LOG_DEBUG(std::string("Creating parent path to ") + this->output_image_);
      if (! boost::filesystem::create_directories(fn_save.parent_path()))
      {
        std::ostringstream oss;
        oss << "Could not create missing directory " << fn_save.parent_path() << " required to create output mosaic.";
        context->report_error(oss.str());
        return false;
      }
    }
    
    unsigned int nx = this->window_x_;
    unsigned int ny = this->window_y_;
    mask_t::Pointer mask;
    
    // read the input image:
    image_t::Pointer image = std_tile<image_t>(fn_load, this->shrink_factor_, DEFAULT_PIXEL_SPACING);
    if (this->use_standard_mask_)
    {
      mask = std_mask<image_t>(image);
    }
    else if (! fn_mask.empty() )
    {
      mask = std_tile<mask_t>(fn_mask, this->shrink_factor_, DEFAULT_PIXEL_SPACING);
    }
    
    image_t::SizeType sz = image->GetLargestPossibleRegion().GetSize();
    if (nx == 1) nx = sz[0];
    if (ny == 1) ny = sz[1];
    
    // filter the image with a median filter:
    if (this->median_radius_ != 0)
    {
      image = median<image_t>(image, this->median_radius_);
    }
    
    // smooth the image prior to histogram equalization:
    if (this->sigma_ != std::numeric_limits<double>::max())
    {
      image = smooth<image_t>(image, this->sigma_);
    }

    pixel_t new_min, new_max;
    if ( this->remap_min_ == std::numeric_limits<long long>::max() )
    {
      new_min = std::numeric_limits<pixel_t>::max();
    }
    else
    {
      new_min = static_cast<pixel_t>(this->remap_min_);
    }

    if ( this->remap_max_ == -std::numeric_limits<long long>::max() )
    {
      new_max = -std::numeric_limits<pixel_t>::max();
    }
    else
    {
      new_max = static_cast<pixel_t>(this->remap_max_);
    }

    image_t::Pointer out =
    CLAHE<image_t>(image, nx, ny, this->max_slope_, this->bins_, new_min, new_max, mask);
    
    bool remap_values = (fn_save.extension() == ".png" ||
                         fn_save.extension() == ".tif" ||
                         fn_save.extension() == ".bmp");
    if (remap_values)
    {
      save<native_image_t>(cast<image_t, native_image_t>(out), fn_save);
    }
    else
    {
      save<image_t>(out, fn_save);
    }

    CORE_LOG_SUCCESS("ir-clahe done");
    
    // done:
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


void ActionCLAHEFilter::Dispatch(Core::ActionContextHandle context,
                                 std::string target_layer,
                                 std::string input_image,
                                 std::string output_image,
                                 unsigned int shrink_factor,
                                 unsigned int window_x,
                                 unsigned int window_y,
                                 unsigned int median_radius,
                                 unsigned int bins,
                                 long long remap_min,
                                 long long remap_max,
                                 double max_slope,
                                 double sigma,
                                 bool use_standard_mask,
                                 std::string mask)
{
  // Create a new action
  ActionCLAHEFilter* action = new ActionCLAHEFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->input_image_ = input_image;
  action->output_image_ = output_image;
  action->shrink_factor_ = shrink_factor;
  action->window_x_ = window_x;
  action->window_y_ = window_y;
  action->median_radius_ = median_radius;
  action->bins_ = bins;
  action->remap_min_ = remap_min;
  action->remap_max_ = remap_max;
  action->max_slope_ = max_slope;
  action->sigma_ = sigma;
  action->use_standard_mask_ = use_standard_mask;
  action->mask_ = mask;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
  
}
