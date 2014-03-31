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
  

//----------------------------------------------------------------
// usage
//
//static void
//usage(const char * message = NULL)
//{
//  std::cerr << "USAGE: ir-clahe\n"
//  << "\t[-sh shrink_factor] \n"
//  << "\t[-bins num_bins] \n"
//  << "\t[-window nx ny] \n"
//  << "\t[-slope max] \n"
//  << "\t[-remap min max] \n"
//  << "\t[-smooth sigma] \n"
//  << "\t[-median radius] \n"
//  << "\t[-std_mask] \n"
//  << "\t[-mask mask_in] \n"
//  << "\t-load image_in \n"
//  << "\t-save image_out \n"
//  << std::endl
//  << "EXAMPLE: ir-clahe\n"
//  << "\t-bins 256 \n"
//  << "\t-window 255 255 \n"
//  << "\t-median 3 \n"
//  << "\t-remap 0 255 \n"
//  << "\t-slope 3.0 \n"
//  << "\t-load in/48755.tif -save he/48755.tif \n"
//  << std::endl
//  << std::endl
//  << "Pass in -help for more detailed information about the tool."
//  << std::endl;
//  
//  if (message != NULL)
//  {
//    std::cerr << "ERROR: " << message << std::endl;
//  }
//}

bool
ActionCLAHEFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionCLAHEFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
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

  const unsigned int DEFAULT_PIXEL_SPACING = 1;

  bfs::path fn_mask;
  bfs::path fn_load(this->input_image_);
  bfs::path fn_save(this->output_image_);
  
  unsigned int nx = this->window_x_; //~0;
  unsigned int ny = this->window_y_; //~0;
  // TODO: make parameters
//  pixel_t new_min = std::numeric_limits<pixel_t>::max();
//  pixel_t new_max = -new_min;
  pixel_t new_min = static_cast<pixel_t>(this->remap_min_);
  pixel_t new_max = static_cast<pixel_t>(this->remap_max_);
  double sigma = std::numeric_limits<double>::max();
  mask_t::Pointer mask;
  
  if ( fn_load.empty() )
  {
//    usage("must specify a file to open with the -load option");
    CORE_LOG_ERROR("Missing input image file name.");
    return false;
  }
  
  if ( fn_save.empty() )
  {
//    usage("must specify a file to open with the -save option");
    CORE_LOG_ERROR("Missing output image file name.");
    return false;
  }
  
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
//std::cerr << "~0u=" << ~0u << "std::numeric_limits<pixel_t>::max()=" << std::numeric_limits<pixel_t>::max() << "-std::numeric_limits<pixel_t>::max()=" << -std::numeric_limits<pixel_t>::max() <<std::endl;
  if (nx == ~0u) nx = sz[0];
  if (ny == ~0u) ny = sz[1];
//std::cerr << "nx=" << nx << ", ny=" << ny << std::endl;
  
  // filter the image with a median filter:
  if (this->median_radius_ != 0)
  {
    image = median<image_t>(image, this->median_radius_);
  }
  
  // smooth the image prior to histogram equalization:
  if (sigma != std::numeric_limits<double>::max())
  {
    image = smooth<image_t>(image, sigma);
  }
  
//  image_t::Pointer out =
//  CLAHE<image_t>(image, nx, ny, max_slope, bins, new_min, new_max, mask);

  // hack! ignore supplied values...
  image_t::Pointer out =
  CLAHE<image_t>(image, nx, ny, this->max_slope_, this->bins_);
  
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
