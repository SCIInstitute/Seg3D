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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONREFINEGRIDFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONREFINEGRIDFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionRefineGridFilter : public LayerAction
{
  
CORE_ACTION(
  CORE_ACTION_TYPE( "RefineGridFilter", "ir-refine-grid" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_mosaic", "Input mosaic file." )
  CORE_ACTION_ARGUMENT( "output_mosaic", "Output mosaic file." )
  CORE_ACTION_ARGUMENT( "directory", "Image file directory." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "num_threads", "0", "Number of threads used (if 0, number of cores will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "iterations", "10", "Run algorithm for given number of iterations." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "cell_size", "0", "Cell size (4 or larger). Set either cell_size, or mesh_rows and mesh_cols." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mesh_rows", "0", "Transform mesh rows (2 or larger). Set either cell_size, or mesh_rows and mesh_cols." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mesh_cols", "0", "Transform mesh columns (2 or larger). Set either cell_size, or mesh_rows and mesh_cols." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "pixel_spacing", "1.0", "Pixel spacing." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "displacement_threshold", "1.0", "Pixel spacing." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use the default mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
//  CORE_ACTION_CHANGES_PROJECT_DATA()
//  CORE_ACTION_IS_UNDOABLE()
)
  
public:
  ActionRefineGridFilter() :
    MAX_ITERATIONS(100),
    MIN_STEP_SCALE(1e-12),
    MIN_ERROR_SQ(1e-16),
    PICKUP_PACE_STEPS(5),
    PREWARP_TILES(true),
    MIN_OVERLAP(0.25),
    DEFAULT_MEDIAN_FILTER_RADIUS(1),
    MOVE_ALL_TILES(false)
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->input_mosaic_ );
    this->add_parameter( this->output_mosaic_ );
    this->add_parameter( this->directory_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->num_threads_ );
    this->add_parameter( this->iterations_ );
    this->add_parameter( this->cell_size_ );
    this->add_parameter( this->mesh_rows_ );
    this->add_parameter( this->mesh_cols_ );
    this->add_parameter( this->pixel_spacing_ );
    this->add_parameter( this->displacement_threshold_ );
    this->add_parameter( this->use_standard_mask_ );
    this->add_parameter( this->sandbox_ );
  }
  
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // DISPATCH:
  // Create and dispatch action that inserts the new layer
  static void Dispatch(Core::ActionContextHandle context,
                       std::string target_layer,
                       unsigned int shrink_factor,
                       unsigned int num_threads,
                       unsigned int iterations,
                       unsigned int cell_size,
                       unsigned int mesh_rows,
                       unsigned int mesh_cols,
                       double pixel_spacing,
                       double displacement_threshold,
                       bool use_standard_mask,
                       std::string input_mosaic,
                       std::string output_mosaic,
                       std::string directory);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  

  unsigned int shrink_factor_;
  unsigned int num_threads_;
  unsigned int iterations_;
  unsigned int cell_size_;
  unsigned int mesh_rows_;
  unsigned int mesh_cols_;
  double pixel_spacing_;
  double displacement_threshold_;
  bool use_standard_mask_;  
  std::string input_mosaic_;
  std::string output_mosaic_;
  std::string directory_;
  
  const unsigned int MAX_ITERATIONS;
  const double MIN_STEP_SCALE;
  const double MIN_ERROR_SQ;
  const unsigned int PICKUP_PACE_STEPS;
  const bool PREWARP_TILES;
  const double MIN_OVERLAP;
  const unsigned int DEFAULT_MEDIAN_FILTER_RADIUS;
  const bool MOVE_ALL_TILES;
};

}

#endif
