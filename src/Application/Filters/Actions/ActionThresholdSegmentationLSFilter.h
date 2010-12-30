/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>
#include <Application/Layer/Layer.h>

namespace Seg3D
{
  
class ActionThresholdSegmentationLSFilter : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "ThresholdSegmentationLSFilter", "Run the ITK ThresholdSegmentLevelSet Filter." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "mask", "The layerid on which this filter needs to be run." )
  CORE_ACTION_KEY( "iterations", "50", "Number of iterations to perform." )
  CORE_ACTION_KEY( "threshold_range", "2.5", "Range in variance of initially segmented data of"
    " what is considered part of the segmentation target." )
  CORE_ACTION_KEY( "curvature", "1.0", "Curvature weight." )
  CORE_ACTION_KEY( "propagation", "1.0", "Propagation weight." )
  CORE_ACTION_KEY( "edge", "0.0", "Edge weight." )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionThresholdSegmentationLSFilter()
  {
    // Action arguments
    this->add_argument( this->layer_id_ );
    this->add_argument( this->mask_ );
    
    // Action options
    this->add_key( this->iterations_ );
    this->add_key( this->threshold_range_ );
    this->add_key( this->curvature_ );
    this->add_key( this->propagation_ );
    this->add_key( this->edge_ );
  }
  
  virtual ~ActionThresholdSegmentationLSFilter()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  void set_iterations( int iterations ) { this->iterations_.value() = iterations; }
  // -- Action parameters --
private:

  Core::ActionParameter< std::string > layer_id_;
  Core::ActionParameter< std::string > mask_;
  
  Core::ActionParameter< int > iterations_;
  Core::ActionParameter< double > threshold_range_;
  Core::ActionParameter< double > curvature_;
  Core::ActionParameter< double > propagation_;
  Core::ActionParameter< double > edge_;
  
  // -- Dispatch this action from the interface --
public:
        
  // DISPATCH:
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, std::string layer_id, 
    std::string mask, int iterations, double threshold_range, 
    double curvature, double propagation, double edge );  
};
  
} // end namespace Seg3D

#endif
