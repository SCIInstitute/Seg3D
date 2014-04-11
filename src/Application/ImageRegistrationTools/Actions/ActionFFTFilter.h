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
  CORE_ACTION_ARGUMENT( "directory", "Image file directory. If files is not used, then filter will search directory for image files." )
  CORE_ACTION_ARGUMENT( "output_mosaic_file", "Output mosaic file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "files", "[]", "Image file names." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
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
  : MAX_PEAKS(16)
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->directory_ );
    this->add_parameter( this->output_mosaic_file_ );
    this->add_parameter( this->files_ );
    this->add_parameter( this->shrink_factor_ );
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
                       std::string output_mosaic_file,
                       std::vector<std::string> files,
                       unsigned int shrink_factor,
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
  std::string output_mosaic_file_;
  std::vector<std::string> files_;
  unsigned int shrink_factor_;
  unsigned int pyramid_levels_;
  unsigned int iterations_per_level_;
  double pixel_spacing_;
  double clahe_slope_;
  double overlap_min_;
  double overlap_max_;
  bool use_standard_mask_;
  bool try_refining_;
  bool run_on_one_;
  
  const int MAX_PEAKS;
};

}

#endif
