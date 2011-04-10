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
#include <Application/LayerManager/Actions/ActionMoveLayerAbove.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, MoveLayerAbove )

namespace Seg3D
{
  
bool ActionMoveLayerAbove::validate( Core::ActionContextHandle& context )
{
  if( !LayerManager::Instance()->get_layer_by_id( this->layer_to_move_id_ ) )
  {
      context->report_error( std::string( "Layer ID '") + this->layer_to_move_id_ + "' is invalid" );
      return false;
  }
  
  // first to we check to see if we have been given the layer_id.  If not, then we check
  // to see if it is the layer_name, if so, we get that value instead, if not we return false;
  if( !LayerManager::Instance()->get_layer_by_id( this->target_layer_id_ ) )
  {

      context->report_error( std::string( "Layer ID '") + this->target_layer_id_ + "' is invalid" );
    return false;
  }

  return true;
}

bool ActionMoveLayerAbove::run( Core::ActionContextHandle& context, 
                 Core::ActionResultHandle& result )
{
  return LayerManager::Instance()->move_layer_above( 
    LayerManager::Instance()->get_layer_by_id( this->layer_to_move_id_ ),
    LayerManager::Instance()->get_layer_by_id( this->target_layer_id_ ) );
}

void ActionMoveLayerAbove::Dispatch( Core::ActionContextHandle context, 
  const std::string& layer_to_move_id, 
  const std::string& target_layer_id, bool move_above )
{
  // Create new action
  ActionMoveLayerAbove* action = new ActionMoveLayerAbove;
  
  // We need to fill in these to ensure the action can be replayed
  action->layer_to_move_id_ = layer_to_move_id;
  action->target_layer_id_ = target_layer_id;
  action->move_above_ = move_above;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D