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

// Application includes
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>

#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>
#include <Application/LayerManager/Actions/ActionDuplicateLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, DuplicateLayer )

namespace Seg3D
{

bool ActionDuplicateLayer::validate( Core::ActionContextHandle& context )
{
  // Step (1): Check whether the layer actually exists 
  std::string error;
  if ( !( LayerManager::CheckLayerExistance( this->layer_id_.value(), error ) ) )
  {
    context->report_error( error );
    return false;
  }
  
  // Step (2): Check whether the layer is available for making a copy from it
  Core::NotifierHandle notifier;
  if ( !LayerManager::CheckLayerAvailabilityForUse( this->layer_id_.value(), notifier ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  return true; // validated
}

bool ActionDuplicateLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  // Step (1):
  // Get the current counters for groups and layers, so we can undo the changes to those counters
  // NOTE: This needs to be done before a new layer is created
  LayerManager::id_count_type id_count = LayerManager::GetLayerIdCount();

  // Step (2):
  // Find the layer that needs to be duplicated
  LayerHandle layer = LayerManager::FindLayer( this->layer_id_.value() );

  if ( !layer )
  {
    context->report_error( "Could find layer" );
    return false;
  }
  
  // Step (3):
  // Duplicate the layer
  LayerHandle new_layer = layer->duplicate();
  
  if ( !new_layer )
  {
    context->report_error( "Could not duplicate layer, as there is not enough memory available" );
    return false;
  }

  new_layer->set_meta_data( layer->get_meta_data() );

  // Step (4):
  // Register the new layer with the LayerManager. This will insert it into the right group.
  LayerManager::Instance()->insert_layer( new_layer );
  LayerManager::Instance()->set_active_layer( new_layer );

  // Step (5):
  // Create an undo item for this action
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Duplicate layer" ) );
  // Tell which action has to be re-executed to obtain the result
  item->set_redo_action( this->shared_from_this() );
  // Tell which layer was added so undo can delete it
  item->add_layer_to_delete( new_layer );
  // Tell what the layer/group id counters are so we can undo those as well
  item->add_id_count_to_restore( id_count );
  // Add the complete record to the undo buffer
  UndoBuffer::Instance()->insert_undo_item( context, item );
  
  return true;
}

Core::ActionHandle ActionDuplicateLayer::Create( const std::string& layer_id )
{
  ActionDuplicateLayer* action = new ActionDuplicateLayer;
  action->layer_id_.value() = layer_id;
  
  return Core::ActionHandle( action );
}


Core::ActionHandle ActionDuplicateLayer::Create( )
{
  ActionDuplicateLayer* action = new ActionDuplicateLayer;
  LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
  if ( active_layer )
  {
    action->layer_id_.value() = active_layer->get_layer_id();
  }
  return Core::ActionHandle( action );
}

void ActionDuplicateLayer::Dispatch( Core::ActionContextHandle context, 
  const std::string& layer_id )
{
  Core::ActionDispatcher::PostAction( Create( layer_id ), context );
}

void ActionDuplicateLayer::Dispatch( Core::ActionContextHandle context )
{
  Core::ActionDispatcher::PostAction( Create(), context );
}

} // end namespace Seg3D
