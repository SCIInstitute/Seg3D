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

#ifndef APPLICATION_LAYER_ACTIONS_ACTIONDELETELAYERS_H
#define APPLICATION_LAYER_ACTIONS_ACTIONDELETELAYERS_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>

namespace Seg3D
{

class ActionDeleteLayers : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "DeleteLayers", "Delete selected layers from a group and the group if it will become empty.")
  CORE_ACTION_ARGUMENT( "layers", "A Pipe delimited list of layers to delete." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)
  
  // -- Constructor/Destructor --
public:
  ActionDeleteLayers()
  {
    this->add_layer_id_list( this->layers_ );
  }
  
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
  // This parameter contains the id of the layer group
  std::vector< std::string > layers_;

  // -- Dispatch this action from the interface --
public:

  /// DISPATCH
  /// Create and dispatch action that deletes the selected layers
  static void Dispatch( Core::ActionContextHandle context, std::vector< std::string > layers ); 
};
  
} // end namespace Seg3D

#endif
