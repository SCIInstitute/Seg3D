/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONFFTFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONFFTFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

#include <string>
#include <vector>

namespace Seg3D
{

class ActionFFTFilter : public LayerAction
{

CORE_ACTION(
  CORE_ACTION_TYPE( "FFTFilter", "ir-fft" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "directory",
    "Image file directory. If 'images' parameter is not used, then filter will search directory for image files." )
  CORE_ACTION_ARGUMENT( "output_mosaic", "Output mosaic file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "tile_strategy", "default", "The strategy for comparing tiles (if known). Options: default, top_left_book, top_left_snake. For the latter 2, the first row always goes to the right." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "min_peak", "0.1", "The minimum peak required to consider a match." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "peak_threshold", "0.3", "The percent of the best peak for matches to be the minimum allowed peak value." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "images", "[]",
    "Image file names (optional, images can be detected in image file directory). Do not use full path." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "num_threads", "0", "Number of threads used (if 0, the number of cores will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "pyramid_levels", "1", "Number of multiresolution pyramid levels." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "iterations_per_level", "5", "Iterations per pyramid level." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "pixel_spacing", "1.0", "Pixel spacing." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "clahe_slope", "1.0", "Maximum CLAHE slope." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "overlap_min", "0.05", "Minimum overlap ratio." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "overlap_max", "1.0", "Maximum overlap ratio." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use standard mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "try_refining", "false", "Try refining image." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "run_on_one", "false", "Run on one image only." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
//  CORE_ACTION_CHANGES_PROJECT_DATA()
//  CORE_ACTION_IS_UNDOABLE()
)
  
public:
  ActionFFTFilter()
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->directory_ );
    this->add_parameter( this->output_mosaic_ );
    this->add_parameter( this->strategy_ );
    this->add_parameter( this->min_peak_ );
    this->add_parameter( this->peak_threshold_ );
    this->add_parameter( this->images_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->num_threads_ );
    this->add_parameter( this->pyramid_levels_ );
    this->add_parameter( this->iterations_per_level_ );
    this->add_parameter( this->pixel_spacing_ );
    this->add_parameter( this->clahe_slope_ );
    this->add_parameter( this->overlap_min_ );
    this->add_parameter( this->overlap_max_ );
    this->add_parameter( this->use_standard_mask_ );
    this->add_parameter( this->try_refining_ );
    this->add_parameter( this->run_on_one_ );
    this->add_parameter( this->sandbox_ );
  }
  
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // DISPATCH:
  // Create and dispatch action that inserts the new layer
  static void Dispatch(Core::ActionContextHandle context,
                       std::string target_layer,
                       std::string directory,
                       std::string output_mosaic,
                       std::string strategy,
                       double min_peak,
                       double peakThreshold,
                       std::vector<std::string> images,
                       unsigned int shrink_factor,
                       unsigned int num_threads,
                       unsigned int pyramid_levels,
                       unsigned int iterations_per_level,
                       double pixel_spacing,
                       double clahe_slope,
                       double overlap_min,
                       double overlap_max,
                       bool use_standard_mask,
                       bool try_refining,
                       bool run_on_one);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  
  
  std::string directory_;
  std::string output_mosaic_;
  std::string strategy_;
  double min_peak_;
  double peak_threshold_;
  std::vector<std::string> images_;
  unsigned int shrink_factor_;
  unsigned int num_threads_;
  unsigned int pyramid_levels_;
  unsigned int iterations_per_level_;
  double pixel_spacing_;
  double clahe_slope_;
  double overlap_min_;
  double overlap_max_;
  bool use_standard_mask_;
  bool try_refining_;
  bool run_on_one_;
};

}

#endif
