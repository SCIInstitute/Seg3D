/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_LAYER_ACTIONS_ACTIONGETLAYERGROUP_H
#define APPLICATION_LAYER_ACTIONS_ACTIONGETLAYERGROUP_H

#include <Core/Action/Actions.h>

// Application includes
#include <Application/Layer/LayerFWD.h>

namespace Seg3D
{

class ActionGetLayerGroup : public Core::Action
{

CORE_ACTION(
  CORE_ACTION_TYPE( "GetLayerGroup", "Get the group ID for a given layer." )
  CORE_ACTION_ARGUMENT( "layerid", "Find the group ID for this layer." )
)

  // -- Constructor/Destructor --
public:
  ActionGetLayerGroup()
  {
    this->add_parameter( this->target_layer_ );
  }

//  virtual ~ActionGetLayerGroup() {}

  // -- Functions that describe action --
public:
  /// VALIDATE:
  /// Each action needs to be validated just before it is posted. This way we
  /// enforce that every action that hits the main post_action signal will be
  /// a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context );

  /// RUN:
  /// Each action needs to have this piece implemented. It spells out how the
  /// action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  std::string target_layer_;

  // -- Dispatch this action --
public:
  /// DISPATCH
  /// Create and dispatch action that moves the layer
  /// NOTE: We don't pass in layer handles directly so validate function can correctly
  /// check if the layers still exist.
  static void Dispatch( Core::ActionContextHandle context, const std::string& target_layer );
};

} // end namespace Seg3D

#endif