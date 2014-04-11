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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONASSEMBLEFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONASSEMBLEFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{
  
class ActionAssembleFilter : public LayerAction
{

CORE_ACTION(
  CORE_ACTION_TYPE( "AssembleFilter", "ir-assemble" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_mosaic", "Input mosaic file." )
  CORE_ACTION_ARGUMENT( "output_image", "Output image file." )
  CORE_ACTION_ARGUMENT( "directory", "Image file directory." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "num_threads", "0", "Number of threads used (if 0, the number of cores will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "tile_width", "std::numeric_limits<unsigned int>::max()", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "tile_height", "std::numeric_limits<unsigned int>::max()", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "pixel_spacing", "1.0", "Pixel spacing." )          
  CORE_ACTION_OPTIONAL_ARGUMENT( "clahe_slope", "1.0", "Maximum CLAHE slope." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use the default mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "save_int16_image", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "save_uint16_image", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "save_variance", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "remap_values", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "defer_image_loading", "false", "" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "feathering", "none", "Blend edges (none, blend, binary)." ) // none, blend, binary
  CORE_ACTION_OPTIONAL_ARGUMENT( "mask", "", "Apply given mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
  //  CORE_ACTION_CHANGES_PROJECT_DATA()
  //  CORE_ACTION_IS_UNDOABLE()
)
    
public:
  ActionAssembleFilter() :
    CLAHE_DEFAULT_NX(255),
    CLAHE_DEFAULT_NY(255),
    CLAHE_DEFAULT_BINS(256)
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->directory_ );
    this->add_parameter( this->input_mosaic_file_ );
    this->add_parameter( this->output_image_file_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->num_threads_ );
    this->add_parameter( this->tile_width_ );
    this->add_parameter( this->tile_height_ );
    this->add_parameter( this->pixel_spacing_ );
    this->add_parameter( this->clahe_slope_ );
    this->add_parameter( this->use_standard_mask_ );
    this->add_parameter( this->save_int16_image_ );
    this->add_parameter( this->save_uint16_image_ );
    this->add_parameter( this->save_variance_ );
    this->add_parameter( this->defer_image_loading_ );
    this->add_parameter( this->remap_values_ );
    this->add_parameter( this->feathering_ );
    this->add_parameter( this->mask_ );
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
                       unsigned int tile_width,
                       unsigned int tile_height,
                       double pixel_spacing,
                       double clahe_slope,
                       bool use_standard_mask,
                       bool save_int16_image,
                       bool save_uint16_image,
                       bool save_variance,
                       bool remap_values,
                       bool defer_image_loading,
                       std::string input_mosaic_file,
                       std::string output_image_file,
                       std::string directory,
                       std::string mask,
                       std::string feathering);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  
  
  unsigned int shrink_factor_;
  unsigned int num_threads_;
  unsigned int tile_width_;
  unsigned int tile_height_;
  double pixel_spacing_;
  double clahe_slope_;
  bool use_standard_mask_;
  bool save_int16_image_;
  bool save_uint16_image_;
  bool save_variance_;
  bool remap_values_;
  bool defer_image_loading_;
  std::string input_mosaic_file_;
  std::string output_image_file_;
  std::string directory_;
  std::string mask_;
  std::string feathering_;

  const int CLAHE_DEFAULT_NX;
  const int CLAHE_DEFAULT_NY;
  const unsigned int CLAHE_DEFAULT_BINS;
};
  
}

#endif
