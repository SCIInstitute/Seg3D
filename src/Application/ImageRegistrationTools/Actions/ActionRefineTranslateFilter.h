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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONREFINETRANSLATEFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONREFINETRANSLATEFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionRefineTranslateFilter : public LayerAction
{

CORE_ACTION(
  CORE_ACTION_TYPE( "RefineTranslateFilter", "ir-refine-translate" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_mosaic", "Input mosaic file." )
  CORE_ACTION_ARGUMENT( "output_mosaic", "Output mosaic file." )
  CORE_ACTION_ARGUMENT( "directory", "Image file directory." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "num_threads", "0", "Number of threads used (if 0, number of cores will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "prune_tile_size", "32", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "pixel_spacing", "1.0", "Pixel spacing." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "intensity_tolerance", "0", "Intensity tolerance." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "max_offset_x", "max", "" )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "max_offset_y", "max", "" )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "black_mask_x", "max", "" )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "black_mask_y", "max", "" )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use the default mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_clahe", "true", "Use CLAHE." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
//  CORE_ACTION_CHANGES_PROJECT_DATA()
//  CORE_ACTION_IS_UNDOABLE()
)
  
public:
  ActionRefineTranslateFilter()
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->input_mosaic_ );
    this->add_parameter( this->output_mosaic_ );
    this->add_parameter( this->directory_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->num_threads_ );
    this->add_parameter( this->prune_tile_size_ );
    this->add_parameter( this->pixel_spacing_ );
    this->add_parameter( this->intensity_tolerance_ );
    this->add_parameter( this->max_offset_x_ );
    this->add_parameter( this->max_offset_y_ );
    this->add_parameter( this->black_mask_x_ );
    this->add_parameter( this->black_mask_y_ );
    this->add_parameter( this->use_standard_mask_ );
    this->add_parameter( this->use_clahe_ );
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
                       unsigned int prune_tile_size,
                       double pixel_spacing,
                       double intensity_tolerance,
                       double max_offset_x,
                       double max_offset_y,
                       double black_mask_x,
                       double black_mask_y,
                       bool use_standard_mask,
                       bool use_clahe,
                       std::string input_mosaic,
                       std::string output_mosaic,
                       std::string directory);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  

  unsigned int shrink_factor_;
  unsigned int num_threads_;
  unsigned int prune_tile_size_;
  double pixel_spacing_;
  double intensity_tolerance_;
  double max_offset_x_;
  double max_offset_y_;
  double black_mask_x_;
  double black_mask_y_;
  bool use_standard_mask_;
  bool use_clahe_;
  std::string input_mosaic_;
  std::string output_mosaic_;
  std::string directory_;
};

}

#endif
