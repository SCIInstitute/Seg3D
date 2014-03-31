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
#include <Core/ITKCommon/visualize.hxx>
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>

// the includes:
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/stos.hxx>
#include <Core/ITKCommon/stos_common.hxx>
#include <Core/ITKCommon/fft_common.hxx>
#include <Core/ITKCommon/grid_common.hxx>
#include <Core/ITKCommon/the_grid_transform.hxx>
#include <Core/ITKCommon/itkGridTransform.h>
#include <Core/ITKCommon/itkMeshTransform.h>
#include <Core/ITKCommon/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/itkNormalizeImageFilterWithMask.h>
#include <Core/ITKCommon/itkRegularStepGradientDescentOptimizer2.h>
#include <Core/ITKCommon/mosaic_refinement_common.hxx>
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/the_boost_mutex.hxx>
#include <Core/ITKCommon/the_boost_thread.hxx>

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

// Local includes:
//#include <Core/ITKCommon/IRPath.h>

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


//----------------------------------------------------------------
// usage
//
static void
usage(const char * message = NULL)
{
  cerr << "USAGE: ir-stos-grid\n"
  << "\t[-threads number_of_threads] \n"
  << "\t[-sh shrink_factor] \n"
  << "\t[-clahe slope] \n"
  << "\t[-no-fft | -fft median_radius minimum_mask_overlap] \n"
  << "\t[-grid_spacing number_of_pixels | -grid rows cols] \n"
  << "\t[-displacement_threshold offset_in_pixels] \n"
  << "\t[-slice_dirs path_to_slice0 path_to_slice1] \n"
  << "\t[-image_dirs path_to_s0_images path_to_s1_images] \n"
  << "\t[-verbose] \n"
  << "\t-neighborhood number_of_pixels \n"
  << "\t-it iterations \n"
  << "\t-load in.stos \n"
  << "\t-save out.stos \n"
  << endl
  << "NOTE: \n"
  << "\t-grid_spacing 256: \n"
  << "\tlayout a mesh such that there is a node every 256 pixels "
  << "(approximately)\n"
  << endl
  << "\t-neighborhood 64: \n"
  << "\tuse 64x64 pixel neighborhoods to refine the grid\n"
  << endl
  << "\t-fft 2 0.25: \n"
  << "\tuse a median filter with radius 2 to denoise the correction "
  << "vectors, \n"
  << "\tdo not use neighborhoods with mask overlap less than 25% "
  << "for refinement.\n"
  << endl
  << "\t-no-fft: \n"
  << "\tuse optimizer instead of FFT to refine the grid.\n"
  << endl
  << "EXAMPLE: ir-stos-grid\n"
  << "\t-sh 16 \n"
  << "\t-fft 2 0.25 \n"
  << "\t-grid_spacing 256 \n"
  << "\t-neighborhood 128 \n"
  << "\t-it 3 \n"
  << "\t-load init/s06-s07.stos \n"
  << "\t-save grid/s06-s01.stos \n"
  << endl
  << endl
  << "Pass in -help for more detailed information about the tool."
  << endl;
  
  if (message != NULL)
  {
    cerr << "ERROR: " << message << endl;
  }
  
//  exit(1);
}

bool
ActionSliceToSliceGridFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionSliceToSliceGridFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // this is so that the printouts look better:
  cout.precision(6);
  cerr.precision(6);
  cout.setf(ios::scientific);
  cerr.setf(ios::scientific);
  
  track_progress(true);
  
  // setup thread storage for the main thread:
  set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
  the_boost_thread_t MAIN_THREAD_DUMMY;
  MAIN_THREAD_DUMMY.set_stopped(false);
  
  // setup thread and mutex interface creators:
  the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
  the_thread_interface_t::set_creator(the_boost_thread_t::create);
  
  // parameters:
//  if (argc < 5) usage(argv);
  
  the_text_t fn_load;
  the_text_t fn_load_directory;
  the_text_t slice_dirs[2];
  the_text_t image_dirs[2];
  
  bool override_image_dir = false;
  bool override_slice_dirs = false;
  
  the_text_t fn_save;
  unsigned int shrink_factor = 1;
  unsigned int iterations = 0;
  unsigned int grid_spacing = 0;
  unsigned int neighborhood = 0;
  unsigned int median_radius = 1;
  double minimum_overlap = 0.5;
  unsigned int grid_cols = 0;
  unsigned int grid_rows = 0;
  bool dont_use_fft = false;
  double clahe_slope = 1.0;
  double displacement_threshold = 1.0;
  bool verbose = false;
  
  // by default run as many threads as there are cores:
  unsigned int num_threads = boost::thread::hardware_concurrency();
  fftwf_init_threads();
  itk_fft::set_num_fftw_threads(1);
  
  // parse the command line arguments:
//  unsigned int consume_default_tokens = 0;
//  for (int i = 1; i < argc; i++)
//  {
//    bool ok = true;
//    
//    if (strcmp(argv[i], "-threads") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -threads option");
//      
//      i++;
//      num_threads = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok) usage("bad -threads option");
//    }
//    else if (strcmp(argv[i], "-sh") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -sh option");
//      
//      i++;
//      shrink_factor = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok) usage("bad -sh option");
//    }
//    else if (strcmp(argv[i], "-clahe") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -clahe option");
//      
//      i++;
//      clahe_slope = the_text_t(argv[i]).toDouble(&ok);
//      if (!ok || clahe_slope < 1.0) usage("bad -clahe option");
//    }
//    else if (strcmp(argv[i], "-it") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -it option");
//      
//      i++;
//      iterations = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok) usage("bad -it option");
//    }
//    else if (strcmp(argv[i], "-grid") == 0)
//    {
//      if ((argc - i) <= 2) usage("could not parse -grid option");
//      
//      i++;
//      grid_rows = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok || grid_rows < 1) usage("bad -grid rows option");
//      
//      i++;
//      grid_cols = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok || grid_cols < 1) usage("bad -grid cols option");
//    }
//    else if (strcmp(argv[i], "-no-fft") == 0)
//    {
//      dont_use_fft = true;
//    }
//    else if (strcmp(argv[i], "-fft") == 0)
//    {
//      if ((argc - i) <= 2) usage("could not parse -fft option");
//      
//      i++;
//      median_radius = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok) usage("bad -fft median_radius option");
//      
//      i++;
//      minimum_overlap = the_text_t(argv[i]).toDouble(&ok);
//      if (!ok) usage("bad -fft minimum_mask_overlap option");
//    }
//    else if (strcmp(argv[i], "-grid_spacing") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -grid_spacing option");
//      
//      i++;
//      grid_spacing = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok || grid_spacing < 1) usage("bad -grid_spacing option");
//    }
//    else if (strcmp(argv[i], "-neighborhood") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -neighborhood option");
//      
//      i++;
//      neighborhood = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok) usage("bad -neighborhood option");
//    }
//    else if (strcmp(argv[i], "-displacement_threshold") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -displacement_threshold option");
//      
//      i++;
//      displacement_threshold = the_text_t(argv[i]).toDouble(&ok);
//      if (!ok) usage("bad -displacement_threshold option");
//    }
//    else if (strcmp(argv[i], "-load") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -load option");
//      
//      i++;
//      fn_load.assign(argv[i]);
//      fn_load_directory.assign( IRPath::DirectoryFromPath( fn_load ) );
//    }
//    else if (strcmp(argv[i], "-slice_dirs") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -slice_dirs option");
//      
//      i++;
//      slice_dirs[0].assign( argv[i] );
//      
//      i++;
//      slice_dirs[1].assign( argv[i] );
//      
//      override_slice_dirs = true;
//    }
//    else if (strcmp(argv[i], "-image_dirs") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -image_dirs option");
//      
//      i++;
//      image_dirs[0].assign( argv[i] );
//      
//      i++;
//      image_dirs[1].assign( argv[i] );
//      
//      override_image_dir = true;
//    }
//    else if (strcmp(argv[i], "-save") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -save option");
//      
//      i++;
//      fn_save = the_text_t(argv[i]);
//    }
//    else if (strcmp(argv[i], "-verbose") == 0)
//    {
//      verbose = true;
//    }
//    else if (strcmp(argv[i], "-help") == 0)
//    {
//      detailed_usage("tools-slice-to-slice.txt", "ir-stos-grid");
//    }
//    else
//    {
//      usage(
//            the_text_t("unknown option \'") +
//            the_text_t(argv[i]) +
//            the_text_t("\'"));
//    }
//    
//    if (consume_default_tokens > 0)
//    {
//      consume_default_tokens--;
//    }
//  }
  
  if (iterations == 0)
  {
    usage("must specify the -it iterations option");
  }
  
  if (fn_load.size() == 0)
  {
    usage("must specify the -load option");
  }
  
  if (fn_save.size() == 0)
  {
    usage("must specify the -save option");
  }
  
  if (neighborhood == 0 &&
      grid_spacing == 0 &&
      (grid_cols == 0 || grid_rows == 0))
  {
    usage(
          "must specify -neighborhood size or -grid_spacing or -grid.\n"
          "NOTE: specifying -neigborhood and either of "
          "the -grid... parameters is perfectly OK");
  }
  
  if (grid_spacing != 0 &&
      (grid_cols != 0 || grid_rows != 0))
  {
    usage(
          "-grid_spacing and -grid options are mutually exclusive, "
          "you can't specify both of them");
  }
  
  // load the coarse level slice to slice registration:
  stos_t<image_t> stos(fn_load, slice_dirs[0], slice_dirs[1]);
  
  // load the two slices:
  std::vector<image_t::Pointer> tile(2);
  std::vector<mask_t::Pointer> mask(2);
  
  if (stos.fn_[0].match_tail(".mosaic"))
  {
    load_mosaic<image_t>(stos.fn_[0],
                         stos.flipped_[0],
                         shrink_factor,
                         clahe_slope,
                         image_dirs[0],
                         tile[0],
                         mask[0]);
  }
  else
  {
    // load first slice:
    tile[0] = std_tile<image_t>(stos.fn_[0],
                                shrink_factor,
                                1.0,        // pixel spacing
                                clahe_slope,
                                64,        // clahe window size
                                true);        // blab
    image_t::SpacingType sp = stos.sp_[0];
    sp[0] *= shrink_factor;
    sp[1] *= shrink_factor;
    tile[0]->SetSpacing(sp);
    
    if (stos.fn_mask_[0].is_empty())
    {
      mask[0] = std_mask<image_t>(tile[0]);
    }
    else
    {
      // load slice mask:
      mask[0] = std_tile<mask_t>(stos.fn_mask_[0],
                                 shrink_factor,
                                 1.0,        // pixel spacing
                                 true);        // blab
      image_t::SpacingType sp = stos.sp_[0];
      sp[0] *= shrink_factor;
      sp[1] *= shrink_factor;
      mask[0]->SetSpacing(sp);
    }
  }
  
  if (stos.fn_[1].match_tail(".mosaic"))
  {
    load_mosaic<image_t>(stos.fn_[1],
                         stos.flipped_[1],
                         shrink_factor,
                         clahe_slope,
                         image_dirs[1],
                         tile[1],
                         mask[1]);
  }
  else
  {
    // load second slice:
    tile[1] = std_tile<image_t>(stos.fn_[1],
                                shrink_factor,
                                1.0,        // pixel spacing
                                clahe_slope,
                                64,        // clahe window size
                                true);        // blab
    image_t::SpacingType sp = stos.sp_[1];
    sp[0] *= shrink_factor;
    sp[1] *= shrink_factor;
    tile[1]->SetSpacing(sp);
    
    if (stos.fn_mask_[1].is_empty())
    {
      mask[1] = std_mask<image_t>(tile[1]);
    }
    else
    {
      // load slice mask:
      mask[1] = std_tile<mask_t>(stos.fn_mask_[1],
                                 shrink_factor,
                                 1.0,        // pixel spacing
                                 true);        // blab
      image_t::SpacingType sp = stos.sp_[1];
      sp[0] *= shrink_factor;
      sp[1] *= shrink_factor;
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
    tile_min[0] + vec2d(double(tile_sz[0][0]) * tile_sp[0][0],
                        double(tile_sz[0][1]) * tile_sp[0][1]),
    tile_min[1] + vec2d(double(tile_sz[1][0]) * tile_sp[1][0],
                        double(tile_sz[1][1]) * tile_sp[1][1])
  };
  
  if (grid_spacing != 0)
  {
    grid_cols = std::max<unsigned int>(2, tile_sz[1][0] / grid_spacing);
    grid_rows = std::max<unsigned int>(2, tile_sz[1][1] / grid_spacing);
  }
  
  if (neighborhood != 0)
  {
    if (grid_cols == 0 || grid_rows == 0)
    {
      // try to layout nodes such that the neighboring tiles overlap:
      grid_cols = (2 * tile_sz[1][0]) / neighborhood;
      grid_rows = (2 * tile_sz[1][1]) / neighborhood;
    }
  }
  else
  {
    if (grid_cols != 0 && grid_rows != 0)
    {
      neighborhood = std::max((2 * tile_sz[1][0]) / grid_cols,
                              (2 * tile_sz[1][1]) / grid_rows);
    }
  }
  
  // dump the status:
  cout << "threads:       " << num_threads << endl
  << "shrink factor: " << shrink_factor << endl
  << "iterations:    " << iterations << endl
  << "grid:          " << grid_cols << " x " << grid_rows << endl
  << "neighborhood:  " << neighborhood << endl;
  if (dont_use_fft)
  {
    cout << "using optimizer on the entire control point grid" << endl;
  }
  else
  {
    cout << "using fft grid refinement: " << endl
    << "median filter radius: " << median_radius << endl
    << "minimum mask overlap: " << minimum_overlap << endl;
  }
  cout << endl;
  
  the_grid_transform_t gt;
  bool gt_ok =
  setup_grid_transform(gt,
                       
                       // grid dimensions:
                       grid_rows,
                       grid_cols,
                       
                       // tile bounding box:
                       tile_min[1],
                       tile_max[1],
                       
                       // tile mask:
                       mask[1],
                       
                       // mapping from mosaic to tile space:
                       stos.t01_.GetPointer());
  if (!gt_ok)
  {
    cerr << "ERROR: could not initialize the mesh transform, aborting..."
    << endl;
    return false;
  }
  
  itk::GridTransform::Pointer itk_gt = itk::GridTransform::New();
  itk_gt->setup(gt, false);
  {
    transform[1] = itk_gt.GetPointer();
    
    make_mosaic_rgb<image_t::Pointer, base_transform_t::ConstPointer>
    (mosaic,
     background_color,
     color,
     omit,
     transform,
     tile,
     idiocy,
     FEATHER_NONE_E);
    //    save_rgb<image_t::Pointer>(mosaic, fn_save + "-a.png");
  }
  
  the_log_t *logger;
  if (verbose)
    logger = cerr_log();
  else
    logger = null_log();
  
  if (dont_use_fft)
  {
    refine_one_pair<itk::GridTransform, image_t>
    (*logger,
     itk_gt,
     tile[0],
     mask[0].GetPointer(),
     tile[1],
     mask[1].GetPointer(),
     iterations,
     1,        // number of pyramid levels
     1e-12,        // min step
     2e+1);        // max step
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
                         1,
                         1,
                         
                         // tile bounding box:
                         tile_min[0],
                         tile_max[0],
                         
                         // tile mask:
                         mask[0],
                         
                         // mapping from mosaic to tile space:
                         identity.GetPointer());
    
    grid_transform[0]->setup(gt, false);
    
    refine_mosaic_mt<image_t::Pointer, mask_t::Pointer>
    (*logger,
     grid_transform,
     tile,
     mask,
     neighborhood,
     true, // prewarp tiles
     minimum_overlap,
     median_radius,
     iterations,
     true, // don't warp the fixed tile
     displacement_threshold,
     num_threads);
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

void
ActionSliceToSliceGridFilter::Dispatch( Core::ActionContextHandle context,
                                std::string target_layer )
{
  // Create a new action
  ActionSliceToSliceGridFilter* action = new ActionSliceToSliceGridFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}


}
