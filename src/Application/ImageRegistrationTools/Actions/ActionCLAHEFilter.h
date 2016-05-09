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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONCLAHEFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONCLAHEFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionCLAHEFilter : public LayerAction
{

// remap_min is wrong!!!
CORE_ACTION(
  CORE_ACTION_TYPE( "CLAHEFilter", "ir-clahe" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_image", "Input image file." )
  CORE_ACTION_ARGUMENT( "output_image", "Output image file." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "max_slope", "0", "Maximum CLAHE slope" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "shrink_factor", "1", "Downsample factor." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "window_x", "1", "Window size, X direction (if 1, default setting will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "window_y", "1", "Window size, Y direction (if 1, default setting will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "remap_min", "max", "Remap minumum (if max, default setting will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "remap_max", "-max", "Remap maximum (if -max, default setting will be used)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "median_radius", "0", "Median radius in pixels." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "bins", "256", "Number of bins." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sigma", "max", "Smoothing scale." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "use_standard_mask", "false", "Use the default mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "mask", "<none>", "Apply given mask." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
)
  
public:
  ActionCLAHEFilter() :
    DEFAULT_PIXEL_SPACING(1)
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->input_image_ );
    this->add_parameter( this->output_image_ );
    this->add_parameter( this->max_slope_ );
    this->add_parameter( this->shrink_factor_ );
    this->add_parameter( this->window_x_ );
    this->add_parameter( this->window_y_ );
    this->add_parameter( this->remap_min_ );
    this->add_parameter( this->remap_max_ );
    this->add_parameter( this->median_radius_ );
    this->add_parameter( this->bins_ );
    this->add_parameter( this->sigma_ );
    this->add_parameter( this->use_standard_mask_ );
    this->add_parameter( this->mask_ );
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
                       unsigned int window_x,
                       unsigned int window_y,
                       unsigned int median_radius,
                       unsigned int bins,
                       long long remap_min,
                       long long remap_max,
                       double max_slope,
                       double sigma,
                       bool use_standard_mask,
                       std::string mask);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;  

  std::string input_image_;
  std::string output_image_;
  unsigned int shrink_factor_;
  unsigned int window_x_;
  unsigned int window_y_;
  unsigned int median_radius_;
  unsigned int bins_;
  long long remap_min_;
  long long remap_max_;
  double max_slope_;
  double sigma_;
  bool use_standard_mask_;
  std::string mask_;

  const unsigned int DEFAULT_PIXEL_SPACING;
};
  
}

#endif
