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

#include <Application/ImageRegistrationTools/Actions/ActionRefineTranslateFilter.h>

#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/mosaic_refinement_common.hxx>
#include <Core/ITKCommon/Optimizers/itkImageMosaicVarianceMetric.h>
#include <Core/ITKCommon/Optimizers/itkRegularStepGradientDescentOptimizer2.h>
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/Transform/IRRefineTranslateCanvas.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>
#include <Core/ITKCommon/the_dynamic_array.hxx>
#include <Core/ITKCommon/IRImageLoader.hxx>
#include <Core/ITKCommon/IRPruner.h>

// system includes:
#include <math.h>

// ITK includes:
#include <itkDiscreteGaussianImageFilter.h>
#include <itkShrinkImageFilter.h>


// Application includes
#include <Application/Layer/LayerManager.h>

#include <Core/Utils/Log.h>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;


CORE_REGISTER_ACTION( Seg3D, RefineTranslateFilter )

namespace Seg3D
{

//----------------------------------------------------------------
// affine_transform_t
// 
typedef itk::LegendrePolynomialTransform<double, 1>
affine_transform_t;

//----------------------------------------------------------------
// cubic_transform_t
// 
typedef itk::LegendrePolynomialTransform<double, 3>
cubic_transform_t;

//----------------------------------------------------------------
// image_interpolator_t
// 
typedef itk::LinearInterpolateImageFunction<image_t, double>
image_interpolator_t;
//
//static void
//usage(const char * message = NULL)
//{
//  cerr << "USAGE: ir-refine-translate\n"
//  << "\t[-sh shrink_factor] \n"
//  << "\t[-sp pixel_spacing] \n"
//  << "\t[-threads number_of_threads] \n"
//  << "\t[-tolerance intensity_tolerance] \n"
//  << "\t[-prune_tile_size length] \n"
//  << "\t[-image_dir path_to_images] \n"
//  << "\t[-max_offset x y | x_percent% y_percent%] \n"
//  << "\t[-black_mask x_boarder_percent y_boarder_percent] \n"
//  << "\t[-noclahe] \n"
//  << "\t[-overlap_percent] \n"
//  << "\t-load any_transform_in \n"
//  << "\t-save grid_transform_out \n"
//  << endl
//  << "EXAMPLE: ir-refine-translate\n"
//  << "\t-sh 8 \n"
//  << "\t-load fft/s01.mosaic \n"
//  << "\t-save grid/s01.mosaic \n"
//  << endl
//  << endl
//  << "Pass in -help for more detailed information about the tool."
//  << endl;
//  
//  if (message != NULL)
//  {
//    cerr << "ERROR: " << message << endl;
//  }
//}

bool
ActionRefineTranslateFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionRefineTranslateFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // this is so that the printouts look better:
  std::cout.precision(3);
  std::cout.setf(std::ios::scientific);
  
  track_progress(true);
  
  // setup thread storage for the main thread:
  set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
  the_boost_thread_t MAIN_THREAD_DUMMY;
  MAIN_THREAD_DUMMY.set_stopped(false);
  
  // setup thread and mutex interface creators:
  the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
  the_thread_interface_t::set_creator(the_boost_thread_t::create);
  
  // parse the command line arguments:
//  unsigned int shrink_factor = 8;
  bool pixel_spacing_user_override = false;
  if (this->pixel_spacing_ != 1.0) // TODO: improve this check
  {
    pixel_spacing_user_override = true;
  }

  //  unsigned int num_threads = boost::thread::hardware_concurrency();
  if (this->num_threads_ == 0)
  {
    this->num_threads_ = boost::thread::hardware_concurrency();
  }

  bfs::path fn_results(this->output_mosaic_file_);
  std::vector<bfs::path> imagePathVector;
  std::vector<bfs::path> maskPathVector;
  double max_offset[2];
//  max_offset[0] = std::numeric_limits<double>::max();
//  max_offset[1] = std::numeric_limits<double>::max();
  max_offset[0] = this->max_offset_x_;
  max_offset[1] = this->max_offset_y_;

  double black_mask_percent[2];
//  black_mask_percent[0] = std::numeric_limits<double>::max();
//  black_mask_percent[1] = std::numeric_limits<double>::max();
  black_mask_percent[0] = this->black_mask_x_;
  black_mask_percent[1] = this->black_mask_y_;

  // TODO: expose?
  bool verbose = true;
  
  // TODO: ignoring this for now. Add option to action parameters?
  bool offsetIsPercent = false;
  // code from original tool:
//  offsetIsPercent = argx.match_tail("%") && argy.match_tail("%");
//  if ( offsetIsPercent )
//  {
//    argx = argx.cut('%', 0);
//    argy = argy.cut('%', 0);
//    cout << argx << endl;
//  }
//  if ( offsetIsPercent )
//  {
//    max_offset[0] /= 100.0f;
//    max_offset[1] /= 100.0f;
//  }
  
  std::list<bfs::path> in;
  std::vector<base_transform_t::Pointer> tbase;
  
  bfs::path fn_in(this->input_mosaic_file_);
  bfs::path image_dir(this->directory_);
  
  IRRefineTranslateCanvas canvas;

  std::fstream fin;
  fin.open(fn_in.c_str(), ios::in);
  if (!fin.is_open())
  {
    std::ostringstream oss;
    oss << "Could not open " << fn_in << " for reading.";
    CORE_LOG_ERROR(oss.str());
    return false;
  }
  
  mask_t::Pointer mosaic_mask;
  double mosaic_pixel_spacing = 0;
  load_mosaic<base_transform_t>(fin,
                                mosaic_pixel_spacing,
                                this->use_standard_mask_,
                                in,
                                tbase,
                                image_dir);
  if (! pixel_spacing_user_override)
  {
    this->pixel_spacing_ = mosaic_pixel_spacing;
  }
  
  if ( this->intensity_tolerance_ > 0.0 )
  {
    IRPruner::PruneImages(in,
                          tbase,
                          this->shrink_factor_,
                          this->pixel_spacing_,
                          this->use_standard_mask_,
                          this->prune_tile_size_,
                          this->intensity_tolerance_);
  }
  
  unsigned int num_images = tbase.size();
  //if (num_images < 2) { usage("less than 2 images in the mosaic"); return false; }
  if (!this->pixel_spacing_)
  {
    CORE_LOG_ERROR("pixel spacing is zero");
    return false;
  }
  
  for (std::list<bfs::path>::const_iterator iter = in.begin(); iter != in.end(); ++iter)
  {
    imagePathVector.push_back(*iter);
    // Note: This mask is a mask on the data included in the cross correlation.
    // It was implemented, but didn't help the Korenberg data so was left 
    // without any flags to supply the mask.
    maskPathVector.push_back("");
  }
  
  fin.close();
  
  set_major_progress(0.1);
  
  // a minimum of two images are required for the registration:
  num_images = in.size();
  if (num_images == 0)
  {
    std::cout << "No images passed the test, nothing to save out." << std::endl;
  }
  
  std::cout << "shrink factor: " << this->shrink_factor_ << std::endl
  << "pixel spacing: " << this->pixel_spacing_ << std::endl
  << "threads:       " << this->num_threads_ << std::endl
  << "max offset: " << max_offset[0] << " " << max_offset[1]
  << std::endl;
  
  IRImageLoader::sharedImageLoader()->setShrinkFactor(this->shrink_factor_);
  IRImageLoader::sharedImageLoader()->setPixelSpacing(this->pixel_spacing_);
  
  canvas.doClahe( this->use_clahe_ );
  canvas.setMaxOffset(max_offset, offsetIsPercent);
  canvas.setBlackMaskPercent(black_mask_percent);
  canvas.setBaseTransforms(tbase, imagePathVector, maskPathVector);
  canvas.buildConnections( verbose );
  canvas.findIdealTransformationOffsets(this->num_threads_);
  canvas.fillGroupIDs();
  
  set_major_progress(0.99);
  
  std::cout << "system energy is " << canvas.systemEnergy()
  << " max tension is " << canvas.maximumTension()
  << " max pull is " << canvas.maximumPullOnTransformation() << std::endl;
  
  const size_t MAX_ITERATIONS = 1000000;
  const double MAX_PULL_GOAL = 1e-2;
  size_t i = 0;
  while (i < MAX_ITERATIONS &&
         canvas.maximumPullOnTransformation() > MAX_PULL_GOAL)
  {
    canvas.releaseTensionOnSystem();
    i++;
  }
  
  if (i == MAX_ITERATIONS)
  {
    std::cout << "WARNING: did not reach maximum pull goal of "
    << MAX_PULL_GOAL
    << " after having reached the maximum iterations of "
    << MAX_ITERATIONS << std::endl;
  }
  
  std::cout << "iterated " << i << " times" << std::endl;
  std::cout << "system energy is " << canvas.systemEnergy()
  << " max tesion is " << canvas.maximumTension()
  << " max pull is " << canvas.maximumPullOnTransformation() << std::endl;
  
  std::vector<base_transform_t::Pointer> transforms;
  std::list<bfs::path> imageIDs;
  std::list<bfs::path> maskIDs;
  canvas.fillTransformAndImageIDVectors(transforms, imageIDs, maskIDs);
  
  // save the results:
  std::fstream fout;
  fout.open(fn_results.c_str(), ios::out);
  
  if (!fout.is_open())
  {
    std::ostringstream oss;
    oss << "Could not open " << fn_results << " for writing.";
    CORE_LOG_ERROR(oss.str());
    return false;
  }
  
  set_major_progress(0.95);
  
  if (num_images == 1)
  {
    std::cout << "Only one image passed the test, saving out without any registration." << std::endl;
    
    save_mosaic<base_transform_t>(fout,
                                  this->pixel_spacing_,
                                  this->use_standard_mask_,
                                  in,
                                  tbase);
    return true;
  }
  
  if ( imageIDs.size() == 0 )
  {
    std::cout << "No overlapping images found, nothing to save out." << std::endl;
  }
  else
  {
    save_mosaic<base_transform_t>(fout,
                                  this->pixel_spacing_,
                                  this->use_standard_mask_,
                                  imageIDs,
                                  transforms);
  }
  fout.close();
  
  set_major_progress(1.0);

  CORE_LOG_SUCCESS("ir-refine-translate done");
  
  // done:
  return true;
}

void
ActionRefineTranslateFilter::Dispatch(Core::ActionContextHandle context,
                                      std::string target_layer,
                                      unsigned int shrink_factor,
                                      unsigned int num_threads,
                                      unsigned int prune_tile_size,
                                      double pixel_spacing,
                                      double intensity_tolerance,
                                      double max_offset_x,
                                      double max_offset_y,
                                      double black_mask_x,
                                      double black_mask_y,
                                      bool use_standard_mask,
                                      bool use_clahe,
                                      std::string input_mosaic_file,
                                      std::string output_mosaic_file,
                                      std::string directory)
  {
    // Create a new action
    ActionRefineTranslateFilter* action = new ActionRefineTranslateFilter;
    
    // Setup the parameters
    action->target_layer_ = target_layer;
    action->shrink_factor_ = shrink_factor;
    action->num_threads_ = num_threads;
    action->prune_tile_size_ = prune_tile_size;
    action->pixel_spacing_ = pixel_spacing;
    action->intensity_tolerance_ = intensity_tolerance;
    action->max_offset_x_ = max_offset_x;
    action->max_offset_y_ = max_offset_y;
    action->black_mask_x_ = black_mask_x;
    action->black_mask_y_ = black_mask_y;
    action->use_standard_mask_ = use_standard_mask;
    action->use_clahe_ = use_clahe;
    action->input_mosaic_file_ = input_mosaic_file;
    action->output_mosaic_file_ = output_mosaic_file;
    action->directory_ = directory;
    
    // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
