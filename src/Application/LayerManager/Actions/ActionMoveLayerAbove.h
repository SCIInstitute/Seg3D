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

#ifndef APPLICATION_LAYERMANAGER_ACTIONS_ACTIONMOVELAYERABOVE_H
#define APPLICATION_LAYERMANAGER_ACTIONS_ACTIONMOVELAYERABOVE_H

#include <Application/Action/Actions.h>
#include <Application/Interface/Interface.h>

namespace Seg3D
{

class ActionMoveLayerAbove : public Action
{
  CORE_ACTION( "MoveLayerAbove","<name_above> <name_below>" );
  
  // -- Constructor/Destructor --
public:
  ActionMoveLayerAbove() :
    layer_to_move_id_(""),
    layer_below_id_("")
    
  {
    add_parameter("layertomove", layer_to_move_id_ );
    add_parameter("layerbelow", layer_below_id_ );
  }
  
  virtual ~ActionMoveLayerAbove()
  {
  }
  
  // -- Functions that describe action --
public:
  virtual bool validate( ActionContextHandle& context );
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result );
  
  // -- Action parameters --
private:
  // Layer_handle that is requested
  ActionParameter< std::string > layer_to_move_id_;
  ActionParameter< std::string > layer_below_id_;
  
  // -- Dispatch this action from the interface --
public:
  // CREATE:
  // Create action that moves the layer above
  static ActionHandle Create( const std::string& layer_to_move_id, const std::string& layer_below_id );
  
  // DISPATCH
  // Create and dispatch action that moves the layer above 
  static void Dispatch( const std::string& layer_to_move_id, const std::string& layer_below_id );
  
};
  
} // end namespace Seg3D

#endif