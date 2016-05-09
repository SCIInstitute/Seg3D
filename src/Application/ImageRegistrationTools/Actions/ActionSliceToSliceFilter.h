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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOSLICEFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOSLICEFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionSliceToSliceFilter : public LayerAction
{

CORE_ACTION(
  CORE_ACTION_TYPE( "SliceToSliceFilter", "ir-stos" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_fixed", "Fixed image file (images, .mosaic, .pyramid files)." )
  CORE_ACTION_ARGUMENT( "input_moving", "Moving image file (images, .mosaic, .pyramid files)." )
  CORE_ACTION_ARGUMENT( "output_stos", "Output slice to slice (.stos) data file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "descriptor_version", "4", "" )
// TODO: parameter commented out in original code.
// Investigate code to see if useful or deprecated.
//  CORE_ACTION_OPTIONAL_ARGUMENT( "transform_order", "2", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "pixel_spacing", "1.0", "Pixel spacing." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "clahe_slope", "1.0", "Maximum CLAHE slope." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use the default mask." )
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
  ActionSliceToSliceFilter()
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->input_fixed_ );
    this->add_parameter( this->input_moving_ );
    this->add_parameter( this->output_stos_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->descriptor_version_ );
    this->add_parameter( this->pixel_spacing_ );
    this->add_parameter( this->clahe_slope_ );
    this->add_parameter( this->use_standard_mask_ );
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
                       unsigned int descriptor_version,
                       double pixel_spacing,
                       double clahe_slope,
                       bool use_standard_mask,
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
  unsigned int descriptor_version_;
  double pixel_spacing_;
  double clahe_slope_;
  bool use_standard_mask_;
  bool flip_fixed_;
  bool flip_moving_;
  std::string input_fixed_;
  std::string input_moving_;
  std::string output_stos_;
  std::string mask_fixed_;
  std::string mask_moving_;
  std::string image_dir_fixed_;
  std::string image_dir_moving_;
};

}

#endif
