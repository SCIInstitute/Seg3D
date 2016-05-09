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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONTHRESHOLDSEGMENTLSFILTER_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONTHRESHOLDSEGMENTLSFILTER_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{
  
class ActionThresholdSegmentationLSFilter : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ThresholdSegmentationLSFilter", "Run the ITK ThresholdSegmentLevelSet Filter." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "mask", "The layerid on which this filter needs to be run." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "iterations", "50", "Number of iterations to perform." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "threshold_range", "2.5", "Range in variance of initially segmented data of"
    " what is considered part of the segmentation target." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "curvature", "1.0", "Curvature weight." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "propagation", "1.0", "Propagation weight." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "edge", "0.0", "Edge weight." )  
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionThresholdSegmentationLSFilter()
  {
    // Action arguments
    this->add_layer_id( this->layer_id_ );
    this->add_layer_id( this->mask_ );
    
    // Action options
    this->add_parameter( this->iterations_ );
    this->add_parameter( this->threshold_range_ );
    this->add_parameter( this->curvature_ );
    this->add_parameter( this->propagation_ );
    this->add_parameter( this->edge_ );
    this->add_parameter( this->sandbox_ );
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  void set_iterations( int iterations ) { this->iterations_ = iterations; }
  // -- Action parameters --
private:

  std::string layer_id_;
  std::string mask_;
  
  int iterations_;
  double threshold_range_;
  double curvature_;
  double propagation_;
  double edge_;
  SandboxID sandbox_;
  
  // -- Dispatch this action from the interface --
public:     
  /// DISPATCH:
  /// Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, std::string layer_id, 
    std::string mask, int iterations, double threshold_range, 
    double curvature, double propagation, double edge );  
};
  
} // end namespace Seg3D

#endif
