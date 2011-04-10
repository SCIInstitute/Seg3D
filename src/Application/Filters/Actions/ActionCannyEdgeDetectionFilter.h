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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONCANNYEDGEDETECTIONFILTER_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONCANNYEDGEDETECTIONFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/LayerManager/LayerAction.h>

namespace Seg3D
{

class ActionCannyEdgeDetectionFilter : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "CannyEdgeDetectionFilter", "ITK filter that detects where edges are in a data layer." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_KEY( "blurring_distance", "2.0", "The distance over which the filter blurs"
    " before computing the gradient." )
  CORE_ACTION_KEY( "threshold", "1.0", "Below this threshold the values are replaced with zero." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionCannyEdgeDetectionFilter()
  {
    // Action arguments
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->blurring_distance_ );
    this->add_parameter( this->threshold_ );
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  std::string target_layer_;
  double blurring_distance_;
  double threshold_;
  
  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, double blurring_distance,
    double threshold );
          
};
  
} // end namespace Seg3D

#endif
