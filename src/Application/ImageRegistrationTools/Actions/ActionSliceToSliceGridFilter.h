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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOSLICEGRIDFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOSLICEGRIDFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionSliceToSliceGridFilter : public LayerAction
{

CORE_ACTION(
  CORE_ACTION_TYPE( "SliceToSliceGridFilter", "ir-stos-grid" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_stos", "Input slice to slice (.stos) data file." )
  CORE_ACTION_ARGUMENT( "output_stos", "Output slice to slice (.stos) data file." )
  CORE_ACTION_ARGUMENT( "neighborhood", "Pixel neighborhood to refine the grid." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "iterations", "1", "Pyramid iterations." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "grid_spacing", "0", "Grid spacing (set either this, or set both grid_rows and grid_cols)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "grid_rows", "0", "Grid rows (set either this with grid_cols, or set grid_spacing)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "grid_cols", "0", "Grid columns (set either this with grid_rows, or set grid_spacing)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "median_radius", "1", "Median filter radius." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "num_threads", "0", "Number of threads used (if 0, number of cores will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "clahe_slope", "1.0", "Maximum CLAHE slope." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "minimum_overlap", "0.5", "Minimum mask overlap." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "displacement_threshold", "1.0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "disable_fft", "false", "Do not use FFT." )
  // TODO: better path handling?
  CORE_ACTION_OPTIONAL_ARGUMENT( "slice_dir_fixed", "<none>", "Override path to fixed slice." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "slice_dir_moving", "<none>", "Override path to fixed slice." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "image_dir_fixed", "<none>", "Override path to fixed slice images." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "image_dir_moving", "<none>", "Override path to moving slice images." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
)
  
public:
  ActionSliceToSliceGridFilter() :
  DEFAULT_MIN_STEP(1e-12),
  DEFAULT_MAX_STEP(2e+1),
  DEFAULT_PIXEL_SPACING(1.0),
  DEFAULT_PYRAMID_LEVELS(1),
  DEFAULT_GRID_ROWS(1),
  DEFAULT_GRID_COLS(1),
  DEFAULT_CLAHE_WINDOW_SIZE(64)
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->input_stos_ );
    this->add_parameter( this->output_stos_ );
    this->add_parameter( this->neighborhood_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->iterations_ );
    this->add_parameter( this->grid_spacing_ );
    this->add_parameter( this->grid_rows_ );
    this->add_parameter( this->grid_cols_ );
    this->add_parameter( this->median_radius_ );
    this->add_parameter( this->num_threads_ );
    this->add_parameter( this->clahe_slope_ );
    this->add_parameter( this->minimum_overlap_ );
    this->add_parameter( this->displacement_threshold_ );
    this->add_parameter( this->disable_fft_ );
    this->add_parameter( this->slice_dir_fixed_ );
    this->add_parameter( this->slice_dir_moving_ );
    this->add_parameter( this->image_dir_fixed_ );
    this->add_parameter( this->image_dir_moving_ );
    this->add_parameter( this->sandbox_ );
  }
  
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // DISPATCH:
  // Create and dispatch action that inserts the new layer
  static void Dispatch(Core::ActionContextHandle context,
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
                       bool disable_fft,
                       std::string input_stos,
                       std::string output_stos,
                       std::string slice_dir_fixed,
                       std::string slice_dir_moving,
                       std::string image_dir_fixed,
                       std::string image_dir_moving);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  

  unsigned int shrink_factor_;
  unsigned int iterations_;
  unsigned int grid_spacing_;
  unsigned int grid_rows_;
  unsigned int grid_cols_;
  unsigned int neighborhood_;
  unsigned int median_radius_;
  unsigned int num_threads_;
  double clahe_slope_;
  double minimum_overlap_;
  double displacement_threshold_;
  bool disable_fft_;
  std::string input_stos_;
  std::string output_stos_;
  std::string slice_dir_fixed_;
  std::string slice_dir_moving_;
  std::string image_dir_fixed_;
  std::string image_dir_moving_;
  
  const double DEFAULT_MIN_STEP;
  const double DEFAULT_MAX_STEP;
  const double DEFAULT_PIXEL_SPACING;
  const unsigned int DEFAULT_PYRAMID_LEVELS;
  const unsigned int DEFAULT_GRID_ROWS;
  const unsigned int DEFAULT_GRID_COLS;
  const unsigned int DEFAULT_CLAHE_WINDOW_SIZE;
};

}

#endif
