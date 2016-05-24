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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOSLICEBRUTEFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOSLICEBRUTEFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionSliceToSliceBruteFilter : public LayerAction
{

CORE_ACTION(
  CORE_ACTION_TYPE( "SliceToSliceBruteFilter", "ir-stos-brute" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_fixed", "Fixed image file (images, .mosaic, .pyramid files)." )
  CORE_ACTION_ARGUMENT( "input_moving", "Moving image file (images, .mosaic, .pyramid files)." )
  CORE_ACTION_ARGUMENT( "output_stos", "Output slice to slice (.stos) data file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "pixel_spacing", "1.0", "Pixel spacing." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "clahe_slope", "1.0", "Maximum CLAHE slope." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "best_shift_x", "0.0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "best_shift_y", "0.0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "best_angle", "0.0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "best_overlap", "0.0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use the default mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_refinement", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_cubic", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "regularize", "false", "Regularize using low order intermediate." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "flip_fixed", "false", "Flip fixed image file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "flip_moving", "false", "Flip moving image file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mask_fixed", "<none>", "Fixed image mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mask_moving", "<none>", "Moving image mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "image_dir_fixed", "<none>", "Fixed image directory." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "image_dir_moving", "<none>", "Moving image directory." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
)
  
public:
  ActionSliceToSliceBruteFilter() :
  DEFAULT_ITERATIONS(500),
  DEFAULT_MIN_STEP(1e-12),
  DEFAULT_MAX_STEP(1e-3), // original code has note: FIXME: 1e-6
  STOS_BRUTE_PYRAMID_KEY(4),
  SCALES_PER_OCTAVE(1),
  DEFAULT_KEY_GENERATE_SETTING(false)
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->input_fixed_ );
    this->add_parameter( this->input_moving_ );
    this->add_parameter( this->output_stos_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->pixel_spacing_ );
    this->add_parameter( this->clahe_slope_ );
    this->add_parameter( this->best_shift_x_ );
    this->add_parameter( this->best_shift_y_ );
    this->add_parameter( this->best_angle_ );
    this->add_parameter( this->best_overlap_ );
    this->add_parameter( this->use_standard_mask_ );
    this->add_parameter( this->use_refinement_ );
    this->add_parameter( this->use_cubic_ );
    this->add_parameter( this->regularize_ );
    this->add_parameter( this->flip_fixed_ );
    this->add_parameter( this->flip_moving_ );
    this->add_parameter( this->mask_fixed_ );
    this->add_parameter( this->mask_moving_ );
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
                       double pixel_spacing,
                       double clahe_slope,
                       double best_shift_x,
                       double best_shift_y,
                       double best_angle,
                       double best_overlap,
                       bool use_standard_mask,
                       bool use_refinement,
                       bool use_cubic,
                       bool regularize,
                       bool flip_fixed,
                       bool flip_moving,
                       std::string input_fixed,
                       std::string input_moving,
                       std::string output_stos,
                       std::string mask_fixed,
                       std::string mask_moving,
                       std::string image_dir_fixed,
                       std::string image_dir_moving);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  

  unsigned int shrink_factor_;
  double pixel_spacing_;
  double clahe_slope_;
  double best_shift_x_;
  double best_shift_y_;
  double best_angle_;
  double best_overlap_;
  bool use_standard_mask_;
  bool use_refinement_;
  bool use_cubic_;
  bool regularize_;
  bool flip_fixed_;
  bool flip_moving_;
  std::string input_fixed_;
  std::string input_moving_;
  std::string output_stos_;
  std::string mask_fixed_;
  std::string mask_moving_;
  std::string image_dir_fixed_;
  std::string image_dir_moving_;

  const unsigned int DEFAULT_ITERATIONS;
  const double DEFAULT_MIN_STEP;
  const double DEFAULT_MAX_STEP;
  const unsigned int STOS_BRUTE_PYRAMID_KEY;
  const unsigned int SCALES_PER_OCTAVE;
  const bool DEFAULT_KEY_GENERATE_SETTING;
};

}

#endif
