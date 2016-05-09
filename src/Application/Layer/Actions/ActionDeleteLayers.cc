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

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/Layer/Actions/ActionDeleteLayers.h>
#include <Application/Layer/LayerUndoBufferItem.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, DeleteLayers )

namespace Seg3D
{

bool ActionDeleteLayers::validate( Core::ActionContextHandle& context )
{
  if ( this->layers_.size() == 0 )
  {
    context->report_error( "No layers to delete." );
    return false;
  }

  for ( size_t j = 0; j < this->layers_.size(); j++ )
  {
    if ( !( LayerManager::CheckLayerExistence( this->layers_[ j ], context ) ) ) return false;  
  }
  
  return true;
}

bool ActionDeleteLayers::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{   
  // Create an undo item for this action
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Delete layer(s)" ) );
  
  // Tell which action has to be re-executed to obtain the result
  item->set_redo_action( this->shared_from_this() );
  
  
  std::vector< LayerHandle > layers;
  
  for ( size_t j = 0; j < this->layers_.size(); j++ )
  {
    // Find the layer handle
    LayerHandle layer = LayerManager::Instance()->find_layer_by_id( this->layers_[ j ] );

    // If a filter is still running on the layer, try to abort it. 
    // NOTE: The program will continue while the layer and its filter are being deleted
    layer->abort_signal_();
    
    // Tell which layers are to be deleted so they can be added back 
    item->add_layer_to_add( layer );

    // Add layer to list of the layers that need to be deleted from the layer manager
    layers.push_back( layer );
  }

  // Add the complete record to the undo buffer
  UndoBuffer::Instance()->insert_undo_item( context, item );

  // Remove the layers from the layer manager
  LayerManager::Instance()->delete_layers( layers );
  
  return true;
}

void ActionDeleteLayers::Dispatch( Core::ActionContextHandle context, std::vector< std::string > layers )
{
  ActionDeleteLayers* action = new ActionDeleteLayers;
  action->layers_ = layers;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
