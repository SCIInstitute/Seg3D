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
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/stos.hxx>
#include <Core/ITKCommon/stos_common.hxx>
#include <Core/ITKCommon/itkGridTransform.h>
#include <Core/ITKCommon/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/itkCascadedTransform.h>
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/the_boost_mutex.hxx>
#include <Core/ITKCommon/the_boost_thread.hxx>

// ITK includes:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>
#include <itkMeanSquaresImageToImageMetric.h>

// system includes:
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>

#include <Core/Utils/Log.h>

// boost:
#include <boost/filesystem.hpp>

// Local includes:
//#include <Core/ITKCommon/IRPath.h>

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


//----------------------------------------------------------------
// usage
//
static void
usage(const char * message = NULL)
{
  std::cerr << "USAGE: ir-stom\n"
  << "\t[-sh shrink_factor] \n"
  << "\t[-clahe window slope] \n"
  << "\t[-draw-grid] \n"
  << "\t[-base-mask] \n"
  << "\t[-tilesize tile_width tile_height] \n"
  << "\t[-preadded] \n"
  << "\t[-remap | -saveas-int16 | -saveas-uint16 ] \n"
  << "\t[-slice_dirs stos0_slice0_path stos0_slice1_path ...]\n"
  << "\t[-image_dirs stos0_image0_path stos0_image1_path ...]\n"
  << "\t[-pad left top right bottom]\n"
  << "\t[-slice slice_num]     (required for .idx) \n"
  << "\t[-channel channel_num] (required for .idx) \n"
  << "\t[-extension [.png | .tif | .idx]] \n"
  << "\t-save slice1 slice2 ... sliceN \n"
  << "\t-load stos_0 [stos_i]*\n"
  << std::endl
  << "EXAMPLE: ir-stom\n"
  << "\t-sh 1 \n"
  << "\t-clahe 64 3.0 \n"
  << "\t-tilesize 512 512 \n"
  << "\t-save slice/ \n"
  << "\t-load 000.stos 001.stos\n"
  << std::endl
  << std::endl
  << "Pass in -help for more detailed information about the tool."
  << std::endl;
  
  if (message != NULL)
  {
    std::cerr << "ERROR: " << message << std::endl;
  }
}

  
bool
ActionSliceToVolumeFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool
ActionSliceToVolumeFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  // this is so that the printouts look better:
  std::cout.precision(6);
  std::cout.setf(ios::scientific);
  
  track_progress(true);
  
  // setup thread storage for the main thread:
  set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
  the_boost_thread_t MAIN_THREAD_DUMMY;
  MAIN_THREAD_DUMMY.set_stopped(false);
  
  // setup thread and mutex interface creators:
  the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
  the_thread_interface_t::set_creator(the_boost_thread_t::create);
  
  // parameters:
//  std::list<the_text_t > stos_fn;
  std::list<bfs::path> stos_fn;

  std::list<stos_t<image_t> > stos_list;
  std::vector<std::vector<the_text_t > > image_dirs;
  std::vector<std::vector<the_text_t > > slice_dirs;
  bool old_image_list_type = true, old_slice_dirs_type = true;
  unsigned int image_list_idx = 0, slice_dirs_idx = 0;

//  std::list<the_text_t>  fn_prefixes;
  std::list<bfs::path>  fn_prefixes;

  bool visualize_warp = false;
  bool use_base_mask = false;
  bool add_transforms = true;
//  unsigned int shrink_factor = 1;
  bool remap_values = false;
  unsigned int clahe_window = 64;
  double clahe_slope = 1.0;
//  int z_slice = -1;
//  int channel = -1;
  unsigned int tile_width = std::numeric_limits<unsigned int>::max();
  unsigned int tile_height = std::numeric_limits<unsigned int>::max();
//  the_text_t fn_extension(".tif");
  bfs::path fn_extension(".tif");
  
  unsigned int left_padding = 0;
  unsigned int top_padding = 0;
  unsigned int right_padding = 0;
  unsigned int bottom_padding = 0;
  bool using_extra_padding = false;
  bool saveas_int16 = false;
  bool saveas_uint16 = false;
  
  // parse the command line arguments:
//  unsigned int consume_default_tokens = 0;
//  unsigned int consume_image_tokens = 0;
//  unsigned int consume_slice_tokens = 0;
//  unsigned int consume_save_tokens = 0;
//  for (int i = 1; i < argc; i++)
//  {
//    bool ok = true;
//    
//    if (strcmp(argv[i], "-sh") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -sh option");
//      
//      i++;
//      shrink_factor = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok) usage("bad -sh option");
//    }
//    else if (strcmp(argv[i], "-remap") == 0)
//    {
//      remap_values = true;
//    }
//    else if (strcmp(argv[i], "-slice") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -slice option");
//      
//      i++;
//      z_slice = the_text_t(argv[i]).toInt(&ok);
//      if (!ok) usage("bad -slice option");
//    }
//    else if (strcmp(argv[i], "-channel") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -channel option");
//      
//      i++;
//      channel = the_text_t(argv[i]).toInt(&ok);
//      if (!ok) usage("bad -channel option");
//    }
//    else if (strcmp(argv[i], "-clahe") == 0)
//    {
//      if ((argc - i) <= 2) usage("could not parse -clahe option");
//      
//      i++;
//      clahe_window = the_text_t(argv[i]).toUInt(&ok);
//      if (!ok || clahe_window < 2) usage("bad -clahe window option");
//      
//      i++;
//      clahe_slope = the_text_t(argv[i]).toDouble(&ok);
//      if (!ok || clahe_slope < 1.0) usage("bad -clahe slope option");
//    }
//    else if (strcmp(argv[i], "-draw-grid") == 0)
//    {
//      visualize_warp = true;
//    }
//    else if (strcmp(argv[i], "-base-mask") == 0)
//    {
//      use_base_mask = true;
//    }
//    else if (strcmp(argv[i], "-extension") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -extension option");
//      
//      i++;
//      fn_extension = the_text_t(argv[i]);
//    }
//    else if (strcmp(argv[i], "-save") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -save option");
//      consume_save_tokens = 2;
//    }
//    else if (strcmp(argv[i], "-load") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -load option");
//      consume_default_tokens = 2;
//    }
//    else if (strcmp(argv[i], "-slice_dirs") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -slice_dirs option");
//      consume_slice_tokens = 2;
//    }
//    else if (strcmp(argv[i], "-image_dirs") == 0)
//    {
//      if ((argc - i) <= 1) usage("could not parse -image_dirs option");
//      consume_image_tokens = 2;
//    }
//    else if (strcmp(argv[i], "-help") == 0)
//    {
//      detailed_usage("tools-slice-to-slice.txt", "ir-stom");
//    }
//    else if (strcmp(argv[i], "-tilesize") == 0)
//    {
//      if ((argc - i) <= 2) usage("could not parse -tilesize option");
//      
//      i++;
//      tile_width = the_text_t(argv[i]).toUInt(&ok);
//      
//      i++;
//      tile_height = the_text_t(argv[i]).toUInt(&ok);
//    }
//    else if (strcmp(argv[i], "-preadded") == 0)
//    {
//      add_transforms = false;
//    }
//    else if (strcmp(argv[i], "-pad") == 0)
//    {
//      if ((argc - i) <= 4) usage("could not parse -pad option");
//      
//      i++;
//      left_padding = the_text_t(argv[i]).toUInt(&ok);
//      
//      i++;
//      top_padding = the_text_t(argv[i]).toUInt(&ok);
//      
//      i++;
//      right_padding = the_text_t(argv[i]).toUInt(&ok);
//      
//      i++;
//      bottom_padding = the_text_t(argv[i]).toUInt(&ok);
//      
//      using_extra_padding = true;
//    }
//    else if (strcmp(argv[i], "-saveas-int16") == 0)
//    {
//      saveas_int16 = true;
//    }
//    else if (strcmp(argv[i], "-saveas-uint16") == 0)
//    {
//      saveas_uint16 = true;
//    }
//    else if (consume_save_tokens > 0)
//    {
//      fn_prefixes.push_back( the_text_t( argv[i] ) );
//      consume_save_tokens++;
//    }
//    else if (consume_default_tokens > 0)
//    {
//      stos_fn.push_back( the_text_t( argv[i] ) );
//      consume_default_tokens++;
//    }
//    else if (consume_image_tokens > 0)
//    {
//      the_text_t token( argv[i] );
//      std::vector<the_text_t > tokens;
//      
//      // Check for number;
//      bool ok = false;
//      int idx = token.toInt( &ok );
//      
//      if ( ok && !(token.match_tail("/") || token.match_tail("\\")) )
//      {
//        image_list_idx = idx;
//        old_image_list_type = false;
//      }
//      
//      if ( !old_image_list_type )
//        the_text_t( argv[++i] ).split(tokens, ',');
//      else
//        tokens.push_back( token );
//      
//      int needs = stos_fn.size() + 1;
//      if ( image_list_idx + 1 > needs )
//        needs = image_list_idx + 1;
//      if ( image_dirs.capacity() < needs )
//        image_dirs.resize(needs);
//      
//      
//      std::vector<the_text_t >::iterator tokIt = tokens.begin();
//      for ( ; tokIt != tokens.end(); ++tokIt )
//      {
//        image_dirs[image_list_idx].push_back( IRPath::CleanPath( *tokIt ) );
//      }
//      
//      if ( old_image_list_type )
//        image_list_idx++;
//      
//      consume_image_tokens++;
//    }
//    else if (consume_slice_tokens > 0)
//    {
//      the_text_t token( argv[i] );
//      std::vector<the_text_t > tokens;
//      
//      // Check for number;
//      bool ok = false;
//      int idx = token.toInt( &ok );
//      if ( ok && !(token.match_tail("/") || token.match_tail("\\")) )
//      {
//        slice_dirs_idx = idx;
//        old_slice_dirs_type = false;
//      }
//      
//      if ( !old_slice_dirs_type )
//        the_text_t( argv[++i] ).split(tokens, ',');
//      else
//        tokens.push_back( token );
//      
//      int needs = stos_fn.size() + 1;
//      if ( slice_dirs_idx + 1 > needs )
//        needs = slice_dirs_idx + 1;
//      if ( slice_dirs.capacity() < needs )
//        slice_dirs.resize(needs);
//      
//      std::vector<the_text_t >::iterator tokIt = tokens.begin();
//      for ( ; tokIt != tokens.end(); ++tokIt )
//      {
//        slice_dirs[slice_dirs_idx].push_back( IRPath::CleanPath( *tokIt ) );
//      }
//      
//      if ( old_slice_dirs_type )
//        slice_dirs_idx++;
//      
//      consume_slice_tokens++;
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
//    if (consume_image_tokens > 0)
//    {
//      consume_image_tokens--;
//    }
//    if (consume_slice_tokens > 0)
//    {
//      consume_slice_tokens--;
//    }
//    if (consume_save_tokens > 0)
//    {
//      consume_save_tokens--;
//    }
//  }

  for (size_t i = 0; i < this->input_files.size(); ++i)
  {
    stos_fn.push_back(this->input_files[i].c_str());
  }

//  for (size_t i = 0; i < this->output_files_prefixes.size(); ++i)
//  {
//    fn_prefixes.push_back(this->output_files_prefixes[i].c_str());
//  }
//  
//  fn_prefixes.push_back("/Users/aylakhan/devel/seg3d2_irtools/TestData/Polsonshale_nq060_set2b/test/stom/");
  
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
      slice_dirs[i].push_back("");
    if ( slice_dirs[i].size() > 1 )
      std::cout << "WARNING: Overriding slice dirs with new .mosaic files has not been tested "
      << "thoroughly.  Problems may arise as a result of building the mosaic mask.  "
      << "Use at your own risk." << std::endl;
    if ( image_dirs[i].empty() )
      image_dirs[i].push_back("");
  }
  
  if ( fn_prefixes.size() == 0 || fn_prefixes.size() > 1 && fn_prefixes.size() != stos_fn.size() + 1 )
  {
    usage("There must be exactly one prefix for each slice, or one prefix for all slices.");
    return false;
  }
  
  // Load up all of the stos files.
  bool override_slice_dirs = ( slice_dirs.size() > 0 );
  bool override_image_dirs = ( image_dirs.size() > 0 );
  
  int i = 0;
//  for (std::list<the_text_t>::const_iterator fn_iter = stos_fn.begin();
//       fn_iter != stos_fn.end(); ++fn_iter, ++i)
  for (std::list<bfs::path>::const_iterator fn_iter = stos_fn.begin();
       fn_iter != stos_fn.end(); ++fn_iter, ++i)
  {
    bfs::path slice0_dir, slice1_dir, image0_dir, image1_dir;
    
    if ( slice_dirs[i].size() > 0 )
      slice0_dir = slice_dirs[i][0];
    if ( slice_dirs[i+1].size() > 0 )
      slice1_dir = slice_dirs[i+1][0];
    
    if ( image_dirs[i].size() > 0 )
      image0_dir = image_dirs[i][0];
    if ( image_dirs[i+1].size() > 0 )
      image1_dir = image_dirs[i+1][0];
    
    stos_t<image_t> stos( *fn_iter, slice0_dir, slice1_dir );
    stos.image_dirs_[0] = image0_dir;
    stos.image_dirs_[1] = image1_dir;
    
    stos_list.push_back( stos );
  }
  
  // a minimum of two images are required for the registration:
  const unsigned int num_pairs = stos_list.size();
  if (num_pairs == 0)
  {
    usage("must specify the -load option");
    return false;
  }
  
  if (visualize_warp)
  {
    // no point in wasting time on CLAHE -- the results
    // will be destroyed anyway:
    clahe_slope = 1.0;
  }
  
  // dump the status:
  std::cout << "shrink factor: " << shrink_factor << std::endl
  << "remap min max: " << remap_values << std::endl;
  
  if (clahe_slope > 1.0)
  {
    std::cout << "clahe window: " << clahe_window << std::endl
    << "clahe slope: " << clahe_slope << std::endl;
  }
  
  // shortcut:
  const unsigned int num_slices = num_pairs + 1;
  
  stos_tree_t<image_t> stos_tree;
  std::size_t num_roots = stos_tree.build(stos_list);
  
  if (num_roots != 1)
  {
    if (num_roots == 0)
    {
      std::cerr << "ERROR: There is more than one possible mapping for "
      << "one or more slices, aborting... "
      << "Remove one of the ambiguous stos transforms and try again."
      << std::endl;
    }
    else
    {
      std::cerr << "ERROR: There are " << num_roots
      << " sets of interconnected stos transforms present, " << std::endl
      << " this tool only works with one set, aborting..." << std::endl;
    }
    stos_tree.dump(std::cerr);
    
    ::exit(1);
  }
  
  // this will sort the transforms in order of dependence and alphabetically:
  stos_list.clear();
  stos_tree.collect_stos(stos_list);
  
  // setup random access vectors:
  std::vector<stos_t<image_t> > stos(stos_list.begin(), stos_list.end());
  std::vector<the_text_t> fn_mosaic(num_slices);
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
      if ( add_transforms )
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
  boost::shared_ptr<stos_tree_t<image_t>::node_t> tree_root =
  stos_tree.get_root();
  
  mask_t::Pointer overlap_mask;
  
  if (!use_base_mask && !using_extra_padding)
  {
    overlap_mask = tree_root->calc_overlap_mask(shrink_factor,
                                                0, // clahe slope
                                                0);// clahe window
  }
  else
  {
    image_t::Pointer data;
    tree_root->load_data(data,
                         overlap_mask,
                         shrink_factor,
                         0, // clahe slope
                         0);// clahe window
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
      if (!use_base_mask)
      {
        std::cerr << "WARNING: There are no fully-overlapping pixels "
        << "in this dataset, using the base slice mask instead..." << std::endl;
        
        image_t::Pointer data;
        tree_root->load_data(data,
                             cropped_mask,
                             shrink_factor,
                             0, // clahe slope
                             0);// clahe window
      }
      else
      {
        std::cerr << "ERROR: the base slice mask is empty, aborting..." << std::endl;
        ::exit(1);
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
    padded_origin[0] -= left_padding;
    padded_origin[1] -= top_padding;
    
    mask_t::SizeType padded_size = cropped_mask->GetLargestPossibleRegion().GetSize();
    padded_size[0] += left_padding + right_padding;
    padded_size[1] += top_padding + bottom_padding;
    
    mask_t::Pointer large_mask = make_image<mask_t>(padded_origin,
                                                    cropped_mask->GetSpacing(),
                                                    padded_size);
    large_mask->FillBuffer(1);
    cropped_mask = large_mask;
  }
  
  // calculate downsample.
  double downsample = shrink_factor;
  
  // FIXME: this is for debugging:
//#if 0
//  save<mask_t>(remap_min_max<mask_t>(cropped_mask),
//               fn_prefix + "mask.nrrd");
//#endif
  
  bool save_tiles = (tile_width != std::numeric_limits<unsigned int>::max() ||
                     tile_height != std::numeric_limits<unsigned int>::max());
  
  std::list<the_text_t>::const_iterator prefix_iter = fn_prefixes.begin();
  
  set_major_progress(0.05);
  
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
      bool valid_slice = (slice_dirs[i].size() > g) && !slice_dirs[i][g].is_empty();
      the_text_t filename = IRPath::FilenameFromPath(fn_mosaic[i]);
      the_text_t new_slice_name = valid_slice ?  slice_dirs[i][g] + filename : fn_mosaic[i];
      
      bool valid_image = (image_dirs[i].size() > g);
      the_text_t new_image_name = (valid_image) ? image_dirs[i][g] : "";
      
      if (fn_mosaic[i].match_tail(".mosaic"))
      {
        load_mosaic<image_t>(new_slice_name,
                             flip_mosaic[i],
                             shrink_factor,
                             true,        // assemble mosaic
                             new_image_name,
                             slice[g],
                             // assemble mosaic mask only if CLAHE
                             // is requested, otherwise it's useless:
                             clahe_slope > 1.0,
                             slice_mask,
                             clahe_slope,
                             clahe_window);
      }
      else
      {
        unsigned int j = i == 0 ? 0 : i - 1;
        unsigned int k = i == 0 ? 0 : 1;
        
        slice[g] = std_tile<image_t>(new_slice_name,
                                     shrink_factor,
                                     std::max(stos[j].sp_[k][0],
                                              stos[j].sp_[k][1]),
                                     clahe_slope,
                                     clahe_window,
                                     true);
      }
    }
    
    if (visualize_warp)
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
      
      if ( add_transforms )
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
    bfs::path fn_filename = *prefix_iter;
    
    if ( fn_prefixes.size() != 1 )
      ++prefix_iter;
    else
      fn_filename += the_text_t::number(i, 3, '0');
    
    for ( int mosIdx = 0; mosIdx < grand_total; ++mosIdx )
    {
      // save the slice:
      the_text_t fn_save = fn_filename;
      if ( grand_total > 1 && strcmp(fn_extension,".idx")!=0)
        fn_save += the_text_t("_") + the_text_t::number(mosIdx, 3, '0');
      fn_save += fn_extension;
      
      if (remap_values)
      {
        remap_min_max_inplace<image_t>(mosaic[mosIdx]);
      }
      
      if (save_tiles)
      {
        if ( saveas_int16 )
        {
          typedef itk::Image<short int, 2> int16_image_t;
          save_as_tiles<int16_image_t>(cast<image_t, int16_image_t>(mosaic[mosIdx]),
                                       fn_filename,
                                       fn_extension,
                                       tile_width,
                                       tile_height,
                                       downsample);
        }
        else if ( saveas_uint16 )
        {
          typedef itk::Image<unsigned short int, 2> uint16_image_t;
          save_as_tiles<uint16_image_t>(cast<image_t, uint16_image_t>(mosaic[mosIdx]),
                                        fn_filename,
                                        fn_extension,
                                        tile_width,
                                        tile_height,
                                        downsample);
        }
        else
        {
          save_as_tiles<native_image_t>(cast<image_t, native_image_t>(mosaic[mosIdx]),
                                        fn_filename,
                                        fn_extension,
                                        tile_width,
                                        tile_height,
                                        downsample);
        }
      }
      else
      {
        if ( saveas_int16 )
        {
          typedef itk::Image<short int, 2> int16_image_t;
          save<int16_image_t>(cast<image_t, int16_image_t>(mosaic[mosIdx]), fn_save);
        }
        else if ( saveas_uint16 )
        {
          typedef itk::Image<unsigned short int, 2> uint16_image_t;
          save<uint16_image_t>(cast<image_t, uint16_image_t>(mosaic[mosIdx]), fn_save);
        }
//#if IDX_ENABLED
//        else if (strcmp(fn_extension,".idx")==0)
//        {
//          std::cerr << "Writing IDX (slice "<<mosIdx<<",channel "<<channel<<",idx "<<fn_save<<")"  << std::endl;
//          // save idx
//          save_to_idx(fn_save, cast<image_t, native_image_t>(mosaic[mosIdx]), mosIdx, channel);
//        }
//#endif
        else
        {
          save<native_image_t>(cast<image_t, native_image_t>(mosaic[mosIdx]), fn_save);
        }
        
      }
    }
    set_major_progress(0.05 + 0.95 * ((i+1) / (fn_mosaic.size() + 1)));
  }
  
  CORE_LOG_SUCCESS("ir-stom done");

  return true;
}

void
  ActionSliceToVolumeFilter::Dispatch( Core::ActionContextHandle context,
                                       std::string target_layer,
                                       unsigned int shrink_factor,
                                       double pixel_spacing,
                                       std::vector<std::string> input_files,
                                       std::vector<std::string> output_files_prefixes )
{
  // Create a new action
  ActionSliceToVolumeFilter* action = new ActionSliceToVolumeFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->shrink_factor = shrink_factor;
  action->input_files = input_files;
  action->output_files_prefixes = output_files_prefixes;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}
