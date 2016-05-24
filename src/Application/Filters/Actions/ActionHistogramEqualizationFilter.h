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

#ifndef APPLICATION_FILTERS_ACTIONS_ACTIONHISTOGRAMEQUALIZATIONFILTER_H
#define APPLICATION_FILTERS_ACTIONS_ACTIONHISTOGRAMEQUALIZATIONFILTER_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionHistogramEqualizationFilter : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "HistogramEqualizationFilter", "Teem filter equalizes the histogram of the data." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "replace", "true", "Replace the old layer (true), or add an new layer (false)" )
  CORE_ACTION_OPTIONAL_ARGUMENT( "amount", "1.0", "Amount of equalization (between 0.0 and 1.0)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "bins", "3000", "Number of bins to use for histogram equalization." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "ignore", "1", "Number of bins to ignore for histogram equalization." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionHistogramEqualizationFilter()
  {
    // Action arguments
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->replace_ );
    this->add_parameter( this->amount_ );
    this->add_parameter( this->bins_ );
    this->add_parameter( this->ignore_bins_ );
    this->add_parameter( this->sandbox_ );
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:

  std::string target_layer_;

  bool replace_;  
  double amount_;
  int bins_;
  int ignore_bins_;
  SandboxID sandbox_;
  
  // -- Dispatch this action from the interface --
public:

  // DISPATCH:
  // Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, bool replace,
    double alpha, int bins, int ignore_bins );
          
};
  
} // end namespace Seg3D

#endif
