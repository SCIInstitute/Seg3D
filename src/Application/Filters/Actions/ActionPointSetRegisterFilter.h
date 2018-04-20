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

#ifndef APPLICATION_FILTERS_ACTIONS_ActionPointSetRegisterFilter_H
#define APPLICATION_FILTERS_ACTIONS_ActionPointSetRegisterFilter_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionPointSetRegisterFilter : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "PointSetRegisterFilter", "Filter that does a Iterative Closest Points Registration on two mask." )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "mask", "The layerid of the mask that needs to be applied." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "iterations", "5", "Number of iterations to perform." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "transform_state", "", "The statid of the state variable into which the transformation values will be written." )
  CORE_ACTION_OPTIONAL_ARGUMENT("complete_transform_state", "", "The statid of the state variable into which the transformation matrix values will be written.")
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )  
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionPointSetRegisterFilter()
  {
    this->add_layer_id( this->target_layer_ );
    this->add_layer_id( this->mask_layer_ );
    this->add_parameter( this->transform_state_id_ );
	this->add_parameter(this->complete_transform_state_id_);
    this->add_parameter( this->iterations_ ); 
    this->add_parameter( this->sandbox_ );
  }


  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context ) override;
  virtual bool run( Core::ActionContextHandle& context, 
    Core::ActionResultHandle& result ) override;
  
  // -- Action parameters --
private:

  std::string target_layer_;
  std::string mask_layer_;
  std::string transform_state_id_;
  std::string complete_transform_state_id_;
  int iterations_;
  SandboxID sandbox_;
  
  // -- Dispatch this action from the interface --
public:
  /// DISPATCH:
  /// Create and dispatch action that inserts the new layer 
  static void Dispatch( Core::ActionContextHandle context, 
    const std::string& target_layer, const std::string& mask_layer, int iterations, 
	const std::string& transform_state_id, const std::string& complete_transform_state_id);
};
  
} // end namespace Seg3D

#endif
