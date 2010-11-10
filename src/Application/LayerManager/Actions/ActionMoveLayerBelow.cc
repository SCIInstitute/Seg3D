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


#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionMoveLayerBelow.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, MoveLayerBelow )

namespace Seg3D
{
  
bool ActionMoveLayerBelow::validate( Core::ActionContextHandle& context )
{
  if( !LayerManager::Instance()->get_layer_by_id( this->layer_.value() ) )
  {
    if( LayerManager::Instance()->get_layer_by_name( this->layer_.value() ) )
    {
      this->layer_.value() = LayerManager::Instance()->get_layer_by_name( 
        this->layer_.value() )->get_layer_id();
    }
    else
    {
      return false;
    }
  }

  if( !LayerManager::Instance()->get_layer_group( this->group_id_.value() ) )
  {
    return false;
  }

  return true;
}

bool ActionMoveLayerBelow::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  return LayerManager::Instance()->move_layer_below( this->layer_.value(),
    this->group_id_.value() );
}

Core::ActionHandle ActionMoveLayerBelow::Create( const std::string& layer, 
  const std::string& group_id )
{
  // Create new action
  ActionMoveLayerBelow* action = new ActionMoveLayerBelow;
  
  // We need to fill in these to ensure the action can be replayed
  action->layer_.value() = layer;
  action->group_id_.value() = group_id;
  
  // Post the new action
  return Core::ActionHandle( action );
}


void ActionMoveLayerBelow::Dispatch( Core::ActionContextHandle context, 
  const std::string& layer, const std::string& group_id )
{
  Core::ActionDispatcher::PostAction( Create( layer, group_id ), context );
}
  
} // end namespace Seg3D
