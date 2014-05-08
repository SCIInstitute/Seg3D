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

#include <Application/ImageRegistrationTools/Actions/ActionSliceToSliceGridFilter.h>

// local includes:
//#include <Core/ITKCommon/visualize.hxx>
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/grid_common.hxx>
#include <Core/ITKCommon/mosaic_refinement_common.hxx>
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/STOS/stos.hxx>
#include <Core/ITKCommon/STOS/stos_common.hxx>
#include <Core/ITKCommon/FFT/fft_common.hxx>
#include <Core/ITKCommon/Transform/the_grid_transform.hxx>
#include <Core/ITKCommon/Transform/itkGridTransform.h>
#include <Core/ITKCommon/Transform/itkMeshTransform.h>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Filtering/itkNormalizeImageFilterWithMask.h>
#include <Core/ITKCommon/Optimizers/itkRegularStepGradientDescentOptimizer2.h>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>

// ITK includes:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImageRegistrationMethod.h>
#include <itkMultiResolutionImageRegistrationMethod.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkFixedCenterOfRotationAffineTransform.h>
#include <itkCenteredRigid2DTransform.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>

// system includes:
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <time.h>

#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

CORE_REGISTER_ACTION( Seg3D, SliceToSliceGridFilter )

namespace Seg3D
{

//----------------------------------------------------------------
// DEBUG_EVERYTHING
//
// #define DEBUG_EVERYTHING


bool
ActionSliceToSliceGridFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionSliceToSliceGridFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(6);
    std::cerr.precision(6);
    std::cout.setf(std::ios::scientific);
    std::cerr.setf(std::ios::scientific);
    
    track_progress(true);
    
    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);
    
    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);
    
    bfs::path fn_load(this->input_stos_);
    //  bfs::path fn_load_directory;

    // override slice and image directories
    // TODO: unclear how useful this is
    bfs::path slice_dirs[2];
    if ( (this->slice_dir0_ != "<none>") && (this->slice_dir1_ != "<none>") )
    {
      slice_dirs[0] = this->slice_dir0_;
      slice_dirs[1] = this->slice_dir1_;
    }
    bfs::path image_dirs[2];
    if ( (this->image_dir0_ != "<none>") && (this->image_dir1_ != "<none>") )
    {
      image_dirs[0] = this->image_dir0_;
      image_dirs[1] = this->image_dir1_;
    }
    
  //  bool override_image_dir = false;
  //  bool override_slice_dirs = false;
    
    bfs::path fn_save(this->output_stos_);
    
    // TODO: expose?
    bool verbose = false;
    
    // by default run as many threads as there are cores:
    //  unsigned int num_threads = boost::thread::hardware_concurrency();
    if (this->num_threads_ == 0)
    {
      this->num_threads_ = boost::thread::hardware_concurrency();
    }
    
    fftwf_init_threads();
    itk_fft::set_num_fftw_threads(1);
    
    if (this->iterations_ == 0)
    {
      CORE_LOG_ERROR("Missing iterations");
      return false;
    }
    
    if (fn_load.empty())
    {
      CORE_LOG_ERROR("Missing input file.");
      return false;
    }
    
    if (fn_save.empty())
    {
      CORE_LOG_ERROR("Missing output file.");
      return false;
    }
    
    if (this->neighborhood_ == 0 &&
        this->grid_spacing_ == 0 &&
        (this->grid_cols_ == 0 || this->grid_rows_ == 0))
    {
      CORE_LOG_ERROR("Either provide neighborhood, grid_spacing or (grid_rows and grid_cols).");
      return false;
  //    usage(
  //          "must specify -neighborhood size or -grid_spacing or -grid.\n"
  //          "NOTE: specifying -neigborhood and either of "
  //          "the -grid... parameters is perfectly OK");
    }
    
    if (this->grid_spacing_ != 0 &&
        (this->grid_cols_ != 0 || this->grid_rows_ != 0))
    {
      CORE_LOG_ERROR("Either provide grid_spacing or (grid_rows and grid_cols).");
      return false;
  //    usage(
  //          "-grid_spacing and -grid options are mutually exclusive, "
  //          "you can't specify both of them");
    }
    
    // load the coarse level slice to slice registration:
    stos_t<image_t> stos(fn_load, slice_dirs[0], slice_dirs[1]);
    
    // load the two slices:
    std::vector<image_t::Pointer> tile(2);
    std::vector<mask_t::Pointer> mask(2);
    
    if (stos.fn_[0].extension() == ".mosaic")
    {
      load_mosaic<image_t>(stos.fn_[0],
                           stos.flipped_[0],
                           this->shrink_factor_,
                           this->clahe_slope_,
                           image_dirs[0],
                           tile[0],
                           mask[0]);
    }
    else
    {
      // load first slice:
      tile[0] = std_tile<image_t>(stos.fn_[0],
                                  this->shrink_factor_,
                                  DEFAULT_PIXEL_SPACING,
                                  this->clahe_slope_,
                                  DEFAULT_CLAHE_WINDOW_SIZE,
                                  true);        // blab
      image_t::SpacingType sp = stos.sp_[0];
      sp[0] *= this->shrink_factor_;
      sp[1] *= this->shrink_factor_;
      tile[0]->SetSpacing(sp);
      
      if (stos.fn_mask_[0].empty())
      {
        mask[0] = std_mask<image_t>(tile[0]);
      }
      else
      {
        // load slice mask:
        mask[0] = std_tile<mask_t>(stos.fn_mask_[0],
                                   this->shrink_factor_,
                                   DEFAULT_PIXEL_SPACING,
                                   true);        // blab
        image_t::SpacingType sp = stos.sp_[0];
        sp[0] *= this->shrink_factor_;
        sp[1] *= this->shrink_factor_;
        mask[0]->SetSpacing(sp);
      }
    }
    
    if (stos.fn_[1].extension() == ".mosaic")
    {
      load_mosaic<image_t>(stos.fn_[1],
                           stos.flipped_[1],
                           this->shrink_factor_,
                           this->clahe_slope_,
                           image_dirs[1],
                           tile[1],
                           mask[1]);
    }
    else
    {
      // load second slice:
      tile[1] = std_tile<image_t>(stos.fn_[1],
                                  this->shrink_factor_,
                                  DEFAULT_PIXEL_SPACING,
                                  this->clahe_slope_,
                                  DEFAULT_CLAHE_WINDOW_SIZE,
                                  true);        // blab
      image_t::SpacingType sp = stos.sp_[1];
      sp[0] *= this->shrink_factor_;
      sp[1] *= this->shrink_factor_;
      tile[1]->SetSpacing(sp);
      
      if (stos.fn_mask_[1].empty())
      {
        mask[1] = std_mask<image_t>(tile[1]);
      }
      else
      {
        // load slice mask:
        mask[1] = std_tile<mask_t>(stos.fn_mask_[1],
                                   this->shrink_factor_,
                                   DEFAULT_PIXEL_SPACING,
                                   true);        // blab
        image_t::SpacingType sp = stos.sp_[1];
        sp[0] *= this->shrink_factor_;
        sp[1] *= this->shrink_factor_;
        mask[1]->SetSpacing(sp);
      }
    }
    
    // temporaries used to generate the mosaic:
    image_t::Pointer mosaic[3];
    std::vector<xyz_t> color(2);
    color[0] = xyz(0, 255, 255);        // fixed
    color[1] = xyz(255, 0, 0);        // moving
    xyz_t background_color = xyz(0, 0, 0);
    std::vector<bool> omit(2, false);
    std::vector<mask_t::ConstPointer> idiocy(2);
    idiocy[0] = mask[0];
    idiocy[1] = mask[1];
    std::vector<base_transform_t::ConstPointer> transform(2);
    transform[0] = identity_transform_t::New();
    transform[1] = stos.t01_;
    
    // this is for debugging:
    {
      make_mosaic_rgb<image_t::Pointer, base_transform_t::ConstPointer>
      (mosaic,
       background_color,
       color,
       omit,
       transform,
       tile,
       idiocy,
       FEATHER_NONE_E);
  //    save_rgb<image_t::Pointer>(mosaic, fn_save + "-e.png");
    }
    
    image_t::SizeType tile_sz[] = {
      tile[0]->GetLargestPossibleRegion().GetSize(),
      tile[1]->GetLargestPossibleRegion().GetSize()
    };
    
    image_t::SpacingType tile_sp[] = {
      tile[0]->GetSpacing(),
      tile[1]->GetSpacing()
    };
    
    pnt2d_t tile_min[] = {
      tile[0]->GetOrigin(),
      tile[1]->GetOrigin()
    };
    
    pnt2d_t tile_max[] = {
      tile_min[0] + vec2d(static_cast<double>(tile_sz[0][0]) * tile_sp[0][0],
                          static_cast<double>(tile_sz[0][1]) * tile_sp[0][1]),
      tile_min[1] + vec2d(static_cast<double>(tile_sz[1][0]) * tile_sp[1][0],
                          static_cast<double>(tile_sz[1][1]) * tile_sp[1][1])
    };
    
    if (this->grid_spacing_ != 0)
    {
      this->grid_cols_ = std::max<unsigned int>(2, tile_sz[1][0] / this->grid_spacing_);
      this->grid_rows_ = std::max<unsigned int>(2, tile_sz[1][1] / this->grid_spacing_);
    }
    
    if (this->neighborhood_ != 0)
    {
      if (this->grid_cols_ == 0 || this->grid_rows_ == 0)
      {
        // try to layout nodes such that the neighboring tiles overlap:
        this->grid_cols_ = (2 * tile_sz[1][0]) / this->neighborhood_;
        this->grid_rows_ = (2 * tile_sz[1][1]) / this->neighborhood_;
      }
    }
    else
    {
      if (this->grid_cols_ != 0 && this->grid_rows_ != 0)
      {
        this->neighborhood_ = std::max((2 * tile_sz[1][0]) / this->grid_cols_,
                                (2 * tile_sz[1][1]) / this->grid_rows_);
      }
    }
    
    // dump the status:
    std::cout << "threads:       " << this->num_threads_ << std::endl
    << "shrink factor: " << this->shrink_factor_ << std::endl
    << "iterations:    " << this->iterations_ << std::endl
    << "grid:          " << this->grid_cols_ << " x " << this->grid_rows_ << std::endl
    << "neighborhood:  " << this->neighborhood_ << std::endl;
    if (this->disable_fft_)
    {
      std::cout << "using optimizer on the entire control point grid" << std::endl;
    }
    else
    {
      std::cout << "using fft grid refinement: " << std::endl
      << "median filter radius: " << this->median_radius_ << std::endl
      << "minimum mask overlap: " << this->minimum_overlap_ << std::endl;
    }
    std::cout << std::endl;
    
    the_grid_transform_t gt;
    bool gt_ok =
    setup_grid_transform(gt,
                         
                         // grid dimensions:
                         this->grid_rows_,
                         this->grid_cols_,
                         
                         // tile bounding box:
                         tile_min[1],
                         tile_max[1],
                         
                         // tile mask:
                         mask[1],
                         
                         // mapping from mosaic to tile space:
                         stos.t01_.GetPointer());
    if (!gt_ok)
    {
      std::cerr << "ERROR: could not initialize the mesh transform, aborting..."
      << std::endl;
      return false;
    }
    
    itk::GridTransform::Pointer itk_gt = itk::GridTransform::New();
    itk_gt->setup(gt, false);
    {
      transform[1] = itk_gt.GetPointer();
      
      make_mosaic_rgb<image_t::Pointer, base_transform_t::ConstPointer>(mosaic,
                                                                        background_color,
                                                                        color,
                                                                        omit,
                                                                        transform,
                                                                        tile,
                                                                        idiocy,
                                                                        FEATHER_NONE_E);
  //    save_rgb<image_t::Pointer>(mosaic, fn_save + "-a.png");
    }
    
    if (this->disable_fft_)
    {
      refine_one_pair<itk::GridTransform, image_t>(itk_gt,
                                                   tile[0],
                                                   mask[0].GetPointer(),
                                                   tile[1],
                                                   mask[1].GetPointer(),
                                                   this->iterations_,
                                                   DEFAULT_PYRAMID_LEVELS,
                                                   DEFAULT_MIN_STEP,
                                                   DEFAULT_MAX_STEP);
    }
    else
    {
      // FIXME: expand the fixed image bounding box:
      std::vector<itk::GridTransform::Pointer> grid_transform(2);
      grid_transform[1] = itk_gt;
      grid_transform[0] = itk::GridTransform::New();
      
      identity_transform_t::Pointer identity = identity_transform_t::New();
      the_grid_transform_t gt;
      setup_grid_transform(gt,
                           
                           // grid dimensions:
                           DEFAULT_GRID_ROWS,
                           DEFAULT_GRID_COLS,
                           
                           // tile bounding box:
                           tile_min[0],
                           tile_max[0],
                           
                           // tile mask:
                           mask[0],
                           
                           // mapping from mosaic to tile space:
                           identity.GetPointer());
      
      grid_transform[0]->setup(gt, false);
      
      refine_mosaic_mt<image_t::Pointer, mask_t::Pointer>(grid_transform,
                                                          tile,
                                                          mask,
                                                          this->neighborhood_,
                                                          true, // prewarp tiles
                                                          this->minimum_overlap_,
                                                          this->median_radius_,
                                                          this->iterations_,
                                                          true, // don't warp the fixed tile
                                                          this->displacement_threshold_,
                                                          this->num_threads_);
    }
    
    // save a debugging image:
    //make_mosaic_rgb<image_t::Pointer, base_transform_t::ConstPointer>
    //  (mosaic,
    //   background_color,
    //   color,
    //   omit,
    //   transform,
    //   tile,
    //   idiocy,
    //   FEATHER_NONE_E);
    //  save_rgb<image_t::Pointer>(mosaic, fn_save + "-final.png");
    
    // save the registration:
    stos.t01_ = itk_gt.GetPointer();
    stos.save(fn_save);
    
    CORE_LOG_SUCCESS("ir-stos-grid done");
    
    // done:
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
ActionSliceToSliceGridFilter::Dispatch(Core::ActionContextHandle context,
                                       std::string target_layer,
                                       unsigned int shrink_factor,
                                       unsigned int iterations,
                                       unsigned int grid_spacing,
                                       unsigned int grid_rows,
                                       unsigned int grid_cols,
                                       unsigned int neighborhood,
                                       unsigned int median_radius,
                                       unsigned int num_threads,
                                       double clahe_slope,
                                       double minimum_overlap,
                                       double displacement_threshold,
                                       double disable_fft,
                                       std::string input_stos,
                                       std::string output_stos,
                                       std::string slice_dir0,
                                       std::string slice_dir1,
                                       std::string image_dir0,
                                       std::string image_dir1)
{
  // Create a new action
  ActionSliceToSliceGridFilter* action = new ActionSliceToSliceGridFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->shrink_factor_ = shrink_factor;
  action->iterations_ = iterations;
  action->grid_spacing_ = grid_spacing;
  action->grid_rows_ = grid_rows;
  action->grid_cols_ = grid_cols;
  action->neighborhood_ = neighborhood;
  action->median_radius_ = median_radius;
  action->num_threads_ = num_threads;
  action->clahe_slope_ = clahe_slope;
  action->minimum_overlap_ = minimum_overlap;
  action->displacement_threshold_ = displacement_threshold;
  action->disable_fft_ = disable_fft;
  action->input_stos_ = input_stos;
  action->output_stos_ = output_stos;
  action->slice_dir0_ = slice_dir0;
  action->slice_dir1_ = slice_dir1;
  action->image_dir0_ = image_dir0;
  action->image_dir1_ = image_dir1;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}


}
