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

#include <Application/ImageRegistrationTools/Actions/ActionBlobFeatureEnhancementFilter.h>

// system includes:
#include <stdlib.h>

// the includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/ThreadUtils/the_thread_pool.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>

//#include <Core/Utils/Log.h>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

CORE_REGISTER_ACTION( Seg3D, BlobFeatureEnhancementFilter )

namespace Seg3D
{


bool
ActionBlobFeatureEnhancementFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

//----------------------------------------------------------------
// calc_variance
//
// NOTE:
// The caller is responsible for pixel_cache memory management.
// The pixel_cache should be allocated to hold (2 * radius + 1)^2 pixels.
//
void
calc_variance(pixel_t * pixel_cache,
              const image_t * image,
              const mask_t * mask,
              const image_t::SizeType & image_size,
              const image_t::IndexType & requested_center,
              const int radius,
              pixel_t & variance,
              pixel_t & mean,
              size_t & mass)
{
  variance = 0;
  mean = 0;
  mass = 0;
  
  typedef image_t::RegionType rn_t;
  typedef image_t::SizeType sz_t;
  typedef image_t::IndexType ix_t;
  
  ix_t ix;
  
  int x0 = requested_center[0] - radius;
  if (x0 < 0) x0 = 0;
  
  int x1 = x0 + radius * 2 + 1;
  if (size_t(x1) > image_size[0])
  {
    int d = x1 - image_size[0];
    x0 -= d;
    x1 -= d;
    if (x0 < 0) x0 = 0;
  }
  
  int y0 = requested_center[1] - radius;
  if (y0 < 0) y0 = 0;
  
  int y1 = y0 + radius * 2 + 1;
  if (size_t(y1) > image_size[1])
  {
    int d = y1 - image_size[1];
    y0 -= d;
    y1 -= d;
    if (y0 < 0) y0 = 0;
  }
  
  
  ix[0] = x0;
  ix[1] = y0;
  
  sz_t sz;
  sz[0] = x1 - x0;
  sz[1] = y1 - y0;
  
  rn_t rn;
  rn.SetIndex(ix);
  rn.SetSize(sz);
  
  typedef itk::ImageRegionConstIterator<image_t> img_iter_t;
  img_iter_t img_iter(image, rn);
  
  typedef itk::ImageRegionConstIterator<mask_t> msk_iter_t;
  msk_iter_t msk_iter(mask, rn);
  
  // calculate the mean:
  for (img_iter.GoToBegin(), msk_iter.GoToBegin();
       !img_iter.IsAtEnd(); ++img_iter, ++msk_iter)
  {
    if (!msk_iter.Get()) continue;
    
    pixel_t p = img_iter.Get();
    mean += p;
    
    pixel_cache[mass] = p;
    mass++;
  }
  
  if (!mass) return;
  mean /= static_cast<float>(mass);
  
  // calculate variance:
  for (size_t i = 0; i < mass; i++)
  {
    pixel_t d = pixel_cache[i] - mean;
    variance += d * d;
  }
  
  variance /= static_cast<float>(mass);
}


//----------------------------------------------------------------
// calc_variance_transaction_t
//
class calc_variance_transaction_t : public the_transaction_t
{
public:
  calc_variance_transaction_t(image_t::ConstPointer image,
                              mask_t::ConstPointer mask,
                              const int radius,
                              const image_t::SizeType & image_size,
                              const image_t::RegionType & region,
                              image_t::Pointer output):
  m_image(image),
  m_mask(mask),
  m_radius(radius),
  m_image_size(image_size),
  m_region(region),
  m_output(output)
  {
    const int cell_size = radius * 2 + 1;
    const size_t cache_size = cell_size * cell_size;
    m_pixel_cache.resize(cache_size);
  }
  
  // virtual:
  void execute(the_thread_interface_t * thread)
  {
    // shortcuts:
    pixel_t * pixel_cache = &m_pixel_cache[0];
    const image_t * image = m_image.GetPointer();
    const mask_t * mask = m_mask.GetPointer();
    
    typedef itk::ImageRegionConstIteratorWithIndex<image_t> citex_t;
    citex_t citex(image, m_region);
    for (citex.GoToBegin(); !citex.IsAtEnd(); ++citex)
    {
      const image_t::IndexType & center = citex.GetIndex();
      
      pixel_t variance = 0;
      pixel_t mean = 0;
      size_t mass = 0;
      calc_variance(pixel_cache,
                    image,
                    mask,
                    m_image_size,
                    center,
                    m_radius,
                    variance,
                    mean,
                    mass);
      if (mass)
      {
        m_output->SetPixel(center, variance);
      }
    }
  }
  
  // image:
  image_t::ConstPointer m_image;
  
  // image mask:
  mask_t::ConstPointer m_mask;
  
  // neighborhood window radius:
  const int m_radius;
  
  // image size:
  image_t::SizeType m_image_size;
  
  // region of interest:
  image_t::RegionType m_region;
  
  // output variance image:
  image_t::Pointer m_output;
  
  // neighborhood pixel cache:
  std::vector<pixel_t> m_pixel_cache;
};


//----------------------------------------------------------------
// qsort_pixel_comparator
//
static int
qsort_pixel_comparator(const void * a, const void * b)
{
  const pixel_t * p0 = (const pixel_t *)a;
  const pixel_t * p1 = (const pixel_t *)b;
  pixel_t d = (*p0 - *p1);
  return (d < 0) ? -1 : (d > 0 ? 1 : 0);
}

//----------------------------------------------------------------
// enhance_blobs
//
// for each pixel calculate its neignborhood variance, then
// calculate the output pixels as the ratio of local
// neignborhood variance to global median variance
//
image_t::Pointer
enhance_blobs(const image_t * image,
              const mask_t * mask,
              const int radius,
              const double & threshold,
              unsigned int num_threads)
{
  const float float_max = std::numeric_limits<float>::max();
  
  image_t::RegionType rn = image->GetLargestPossibleRegion();
  image_t::SizeType sz = rn.GetSize();
  typedef image_t::IndexType ix_t;
  ix_t pos = rn.GetIndex();
  
  image_t::Pointer variance = cast<image_t, image_t>(image);
  variance->FillBuffer(float_max);
  
  the_thread_pool_t thread_pool(num_threads);
  thread_pool.set_idle_sleep_duration(50); // 50 usec
  std::list<the_transaction_t *> schedule;
  
  const size_t pixels_per_transaction = std::min<size_t>(64, sz[0]);
  
  // Create a region for the smaller tile size.
  image_t::RegionType tile_region;
  image_t::SizeType move_sz;
  move_sz[0] = pixels_per_transaction;
  move_sz[1] = 1;
  tile_region.SetSize(move_sz);
  
  // Take care of the image majority.
  for (pixel_t startY = pos[0]; startY < sz[1]; startY += move_sz[1])
  {
    for (pixel_t startX = pos[1]; startX < sz[0]; startX += move_sz[0])
    {
      // Clear up extra stip later
      if ( sz[0] - startX < pixels_per_transaction )
        continue;
      
      // Set the tile position
      ix_t move_pos;
      move_pos[0] = startX;
      move_pos[1] = startY;
      tile_region.SetIndex(move_pos);
      
      // Schedule a blob job
      calc_variance_transaction_t * t =
      new calc_variance_transaction_t(image,
                                      mask,
                                      radius,
                                      sz,
                                      tile_region,
                                      variance);
      schedule.push_back(t);
    }
  }
  
  // Extra strip to the right that doesn't fit into pixels_per_transaction.
  size_t extra_sz = sz[0] % pixels_per_transaction;
  
  if ( extra_sz > 0 )
  {
    int round_up = (pixels_per_transaction % extra_sz > 0) ? 1 : 0;
    pixel_t rows_per_step = pixels_per_transaction / extra_sz + round_up;
    
    move_sz[0] = extra_sz;
    move_sz[1] = rows_per_step;
    tile_region.SetSize(move_sz);
    
    ix_t move_pos;
    move_pos[0] = sz[0] - extra_sz;
    
    // Take care of the extra strip...
    for (pixel_t startY = pos[1]; startY < sz[1]; startY += rows_per_step)
    {
      // Make sure there's enough rows.
      if ( sz[1] - startY < rows_per_step )
      {
        rows_per_step = sz[1] - startY;
        move_sz[1] = rows_per_step;
        tile_region.SetSize(move_sz);
      }
      
      // Set the tile position
      move_pos[1] = startY;
      tile_region.SetIndex(move_pos);
      
      // Schedule a blob job
      calc_variance_transaction_t * t =
      new calc_variance_transaction_t(image,
                                      mask,
                                      radius,
                                      sz,
                                      tile_region,
                                      variance);
      schedule.push_back(t);
    }
  }
  
  thread_pool.push_back(schedule);
  thread_pool.pre_distribute_work();
  thread_pool.start();
  thread_pool.wait();
  
  // find the median:
  std::vector<pixel_t> variance_vector(sz[0] * sz[1]);
  pixel_t * variance_pixels = &variance_vector[0];
  unsigned int variance_pixel_count = 0;
  
  // first, get rid of masked out variance pixels:
  typedef itk::ImageRegionConstIterator<image_t> citer_t;
  citer_t citer(variance, rn);
  for (citer.GoToBegin(); !citer.IsAtEnd(); ++citer)
  {
    pixel_t v = citer.Get();
    if (v != float_max)
    {
      variance_pixels[variance_pixel_count] = v;
      variance_pixel_count++;
    }
  }
  
  qsort(variance_pixels,
        variance_pixel_count,
        sizeof(pixel_t),
        &qsort_pixel_comparator);
  
  const pixel_t global_variance_median =
  variance_pixels[variance_pixel_count / 2];
  
  std::cout << "global variance median: " << global_variance_median << std::endl
  << std::endl;
  
  // enhance the blobs:
  pixel_t blob_image_mean = 0;
  size_t blob_image_mass = 0;
  
  typedef itk::ImageRegionIterator<image_t> iter_t;
  iter_t iter(variance, rn);
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    pixel_t v = iter.Get();
    if (v != float_max)
    {
      double metric =
      std::min<double>(threshold,
                       static_cast<double>(global_variance_median + 1) /
                       static_cast<double>(v + 1));
      iter.Set(metric);
      
      blob_image_mean += metric;
      blob_image_mass++;
    }
  }
  
  if (blob_image_mass)
  {
    blob_image_mean /= pixel_t(blob_image_mass);
  }
  
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    pixel_t v = iter.Get();
    if (v == float_max)
    {
      iter.Set(blob_image_mean);
    }
  }
  
  return variance;
}


bool
ActionBlobFeatureEnhancementFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(6);
    std::cout.setf(std::ios::scientific);
    
    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);
    
    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);
    
    bfs::path fn_mask;
    bfs::path fn_load(this->input_image_);
    if (fn_load.empty())
    {
      context->report_error("Filter requires an input image file");
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
    if (fn_save.empty())
    {
      context->report_error("Filter requires an output image file");
      return false;
    }

    if (! bfs::is_directory( fn_save.parent_path() ) )
    {
      CORE_LOG_DEBUG(std::string("Creating parent path to ") + this->output_image_);
      if (! boost::filesystem::create_directories(fn_save.parent_path()))
      {
        std::ostringstream oss;
        oss << "Could not create missing directory " << fn_save.parent_path() << " required to create output image.";
        context->report_error(oss.str());
        return false;
      }
    }
    
    mask_t::Pointer image_mask;
    
    // by default run as many threads as there are cores:
    if (this->num_threads_ == 0)
    {
      this->num_threads_ = boost::thread::hardware_concurrency();
    }
    
    const unsigned int DEFAULT_PIXEL_SPACING = 1;
    const unsigned int DEFAULT_COLS = 1;
    const unsigned int DEFAULT_ROWS = 1;
    
    if (! bfs::exists(fn_load) )
    {
      context->report_error("Filter requires an input image file");
      return false;
    }
    
    std::cout << "neighborhood:  " << this->radius_ * 2 + 1 << std::endl
    << "threads:       " << this->num_threads_ << std::endl
    << std::endl;
    
    // load an image:
    image_t::Pointer image = std_tile<image_t>(fn_load, this->shrink_factor_, DEFAULT_PIXEL_SPACING);
    
    // setup the mask:
    if (!fn_mask.empty())
    {
      image_mask = std_tile<mask_t>(fn_mask, this->shrink_factor_, DEFAULT_PIXEL_SPACING);
    }
    else
    {
      image_mask = std_mask<image_t>(image, this->use_standard_mask_);
    }
    
    bool remap_values =
      fn_save.extension() != ".mha" &&
      fn_save.extension() != ".mhd" &&
      fn_save.extension() != ".nrrd";
    
    // filter the image with a median filter:
    if (this->median_radius_ != 0)
    {
      image = median<image_t>(image, this->median_radius_);
    }
    
    image_t::Pointer blobs = enhance_blobs(image,
                                           image_mask,
                                           this->radius_,
                                           this->threshold_,
                                           this->num_threads_);

    // pixel type == unsigned?
    blobs = normalize<image_t>(blobs, DEFAULT_COLS, DEFAULT_ROWS, 0, 255, image_mask);
    
    if (remap_values)
    {
      save<native_image_t>(cast<image_t, native_image_t>(blobs), fn_save);
    }
    else
    {
      save<image_t>(blobs, fn_save);
    }

    CORE_LOG_SUCCESS("ir-blob done");
    
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


void
ActionBlobFeatureEnhancementFilter::Dispatch(Core::ActionContextHandle context,
                                             std::string target_layer,
                                             std::string input_image,
                                             std::string output_image,
                                             unsigned int shrink_factor,
                                             unsigned int num_threads,
                                             int radius,
                                             double threshold,
                                             unsigned int median_radius,
                                             bool use_standard_mask)
{
  // Create a new action
  ActionBlobFeatureEnhancementFilter* action = new ActionBlobFeatureEnhancementFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->input_image_ = input_image;
  action->output_image_ = output_image;
  action->shrink_factor_ = shrink_factor;
  action->num_threads_ = num_threads;
  action->radius_ = radius;
  action->threshold_ = threshold;
  action->median_radius_ = median_radius;
  action->use_standard_mask_ = use_standard_mask;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
