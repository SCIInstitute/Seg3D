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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONBLOBFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONBLOBFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionBlobFeatureEnhancementFilter : public LayerAction
{
  
CORE_ACTION(
  CORE_ACTION_TYPE( "BlobFeatureEnhancementFilter", "ir-blob" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_image", "Input image file." )
  CORE_ACTION_ARGUMENT( "output_image", "Output image file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "num_threads", "0", "Number of threads used (if 0, number of cores will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "radius", "2", "Radius in pixels." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "threshold", "3", "Maxiumum threshold." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "median_radius", "0", "Median radius in pixels." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use the default mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
)
  
public:
  ActionBlobFeatureEnhancementFilter()
  {
    this->add_layer_id( this->target_layer_ ); // tmp
    this->add_parameter( this->input_image_ );
    this->add_parameter( this->output_image_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->num_threads_ );
    this->add_parameter( this->radius_ );
    this->add_parameter( this->threshold_ );
    this->add_parameter( this->median_radius_ );
    this->add_parameter( this->use_standard_mask_ );
    this->add_parameter( this->sandbox_ );
  }
  
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // DISPATCH:
  // Create and dispatch action that inserts the new layer
  static void Dispatch(Core::ActionContextHandle context,
                       std::string target_layer,
                       std::string input_image,
                       std::string output_image,
                       unsigned int shrink_factor,
                       unsigned int num_threads,
                       int radius,
                       double threshold,
                       unsigned int median_radius,
                       bool use_standard_mask);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  
  
  std::string input_image_;
  std::string output_image_;
  unsigned int shrink_factor_;
  unsigned int num_threads_;
  int radius_;
  double threshold_;
  unsigned int median_radius_;
  bool use_standard_mask_;
};
  
}

#endif
