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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOVOLUMEFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONSLICETOVOLUMEFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionSliceToVolumeFilter : public LayerAction
{  
CORE_ACTION(
  CORE_ACTION_TYPE( "SliceToVolumeFilter", "ir-stom" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "files", "Stos data file names." )
  CORE_ACTION_ARGUMENT( "output_prefixes", "Output file prefixes." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "clahe_window", "64", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "left_padding", "0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "top_padding", "0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "right_padding", "0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "bottom_padding", "0", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "tile_width", "std::numeric_limits<unsigned int>::max()", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "tile_height", "std::numeric_limits<unsigned int>::max()", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "clahe_slope", "1.0", "Maximum CLAHE slope." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "visualize_warp", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_base_mask", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "add_transforms", "true", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "remap_values", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "save_int16_image", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "save_uint16_image", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
)
  
public:
  ActionSliceToVolumeFilter() :
  DEFAULT_CLAHE_SLOPE(0),
  DEFAULT_CLAHE_WINDOW(0)
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->clahe_window_ );
    this->add_parameter( this->left_padding_ );
    this->add_parameter( this->top_padding_ );
    this->add_parameter( this->right_padding_ );
    this->add_parameter( this->bottom_padding_ );
    this->add_parameter( this->tile_width_ );
    this->add_parameter( this->tile_height_ );
    this->add_parameter( this->clahe_slope_ );
    this->add_parameter( this->visualize_warp_ );
    this->add_parameter( this->use_base_mask_ );
    this->add_parameter( this->add_transforms_ );
    this->add_parameter( this->remap_values_ );
    this->add_parameter( this->save_int16_image_ );
    this->add_parameter( this->save_uint16_image_ );
    this->add_parameter( this->input_files_ );
    this->add_parameter( this->output_files_prefixes_ );
    this->add_parameter( this->sandbox_ );
  }
  
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // DISPATCH:
  // Create and dispatch action that inserts the new layer
  static void Dispatch(Core::ActionContextHandle context,
                       std::string target_layer,
                       unsigned int shrink_factor,
                       unsigned int clahe_window,
                       unsigned int left_padding,
                       unsigned int top_padding,
                       unsigned int right_padding,
                       unsigned int bottom_padding,
                       unsigned int tile_width,
                       unsigned int tile_height,
                       double clahe_slope,
                       bool visualize_warp,
                       bool use_base_mask,
                       bool add_transforms,
                       bool remap_values,
                       bool save_int16_image,
                       bool save_uint16_image,
                       std::vector<std::string> input_files,
                       std::vector<std::string> output_files_prefixes);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  

  unsigned int shrink_factor_;
  unsigned int clahe_window_;
  unsigned int left_padding_;
  unsigned int top_padding_;
  unsigned int right_padding_;
  unsigned int bottom_padding_;
  unsigned int tile_width_;
  unsigned int tile_height_;
  double clahe_slope_;
  bool visualize_warp_;
  bool use_base_mask_;
  bool add_transforms_;
  bool remap_values_;
  bool save_int16_image_;
  bool save_uint16_image_;
  std::vector<std::string> input_files_;
  std::vector<std::string> output_files_prefixes_;

  const double DEFAULT_CLAHE_SLOPE;
  const unsigned int DEFAULT_CLAHE_WINDOW;
};

}

#endif
