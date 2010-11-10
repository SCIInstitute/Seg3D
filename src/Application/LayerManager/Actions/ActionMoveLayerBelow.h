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

#ifndef APPLICATION_LAYERMANAGER_ACTIONS_ACTIONMOVELAYERBELOW_H
#define APPLICATION_LAYERMANAGER_ACTIONS_ACTIONMOVELAYERBELOW_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

namespace Seg3D
{

class ActionMoveLayerBelow : public Core::Action
{

CORE_ACTION( 
  CORE_ACTION_TYPE( "MoveLayerBelow", "Move a group to a new location in the layer manager." )
  CORE_ACTION_ARGUMENT( "layer", "Layer that needs to be moved." )
  CORE_ACTION_ARGUMENT( "groupid", "The groupid of the group slot above which to insert the group. ")
  CORE_ACTION_CHANGES_PROJECT_DATA()
) 
  // -- Constructor/Destructor --
public:
  ActionMoveLayerBelow() :
    layer_(""),
    group_id_("") 
  {
    this->add_argument( layer_ );
    this->add_argument( group_id_ );
  }
  
  virtual ~ActionMoveLayerBelow()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // -- Action parameters --
private:
  // Layer_handle that is requested
  Core::ActionParameter< std::string >  layer_;
  Core::ActionParameter< std::string >  group_id_;
  
  // -- Dispatch this action from the interface --
public:
  // CREATE:
  // Create action that moves the layer above
  static Core::ActionHandle Create( const std::string& layer, 
    const std::string& group_id );
  
  // DISPATCH
  // Create and dispatch action that moves the layer above 
  static void Dispatch( Core::ActionContextHandle context, const std::string& layer, 
    const std::string& group_id );
  
};
  
} // end namespace Seg3D

#endif
