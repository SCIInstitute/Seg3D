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

#ifndef APPLICATION_LAYERMANAGER_ACTIONS_ACTIONDUPLICATELAYER_H
#define APPLICATION_LAYERMANAGER_ACTIONS_ACTIONDUPLICATELAYER_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/Layer/LayerFWD.h>
#include <Application/LayerManager/LayerAction.h>

namespace Seg3D
{

class ActionDuplicateLayer : public LayerAction
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "DuplicateLayer", "Duplicate a layer" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid of the layer that needs to be duplicated.")
  CORE_ACTION_CHANGES_PROJECT_DATA()
  CORE_ACTION_IS_UNDOABLE()
)
  
  // -- Constructor/Destructor --
public:
  ActionDuplicateLayer()
  {
    this->add_argument( layer_id_ );
  }
  
  virtual ~ActionDuplicateLayer()
  {
  }
  
// -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, 
    Core::ActionResultHandle& result );
  
private:
  // The name of the group where the mask needs to be added
  Core::ActionParameter< std::string > layer_id_;
  
  // -- Dispatch this action from the interface --
public:
  // CREATE:
  // Create an action that duplicates a certain layer
  static Core::ActionHandle Create( const std::string& layer_id );

  // CREATE:
  // Create an action that duplicates the active layer
  static Core::ActionHandle Create();
  
  // DISPATCH:
  // Dispatch action that duplicates a certain layer
  static void Dispatch( Core::ActionContextHandle context, const std::string& layer_id );

  // DISPATCH:
  // Dispatch action that duplicates the active layer
  static void Dispatch( Core::ActionContextHandle context);
};
  
} // end namespace Seg3D

#endif
