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
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/LayerManager/Actions/ActionDeleteLayers.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, DeleteLayers )

namespace Seg3D
{

bool ActionDeleteLayers::validate( Core::ActionContextHandle& context )
{
  if( this->layers_.value() != "" )
  {
    this->layers_vector_ = Core::SplitString( this->layers_.value(), "|" );
  }
  
  if( this->layers_vector_.size() == 0 ) return false;

  return true; // validated
}

bool ActionDeleteLayers::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create an undo item for this action
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Delete layer(s)" ) );
  // Tell which action has to be re-executed to obtain the result
  item->set_redo_action( this->shared_from_this() );
  // Tell which layers are to be deleted so they can be added back 
  for ( size_t i = 0; i < this->layers_vector_.size(); ++i )
  {
    LayerHandle layer = LayerManager::Instance()->get_layer_by_id( this->layers_vector_[ i ] );
    layer->abort_signal_();
    item->add_layer_to_add( layer );
  }
  // Add the complete record to the undo buffer
  UndoBuffer::Instance()->insert_undo_item( context, item );

  LayerManager::Instance()->delete_layers( this->layers_vector_ );
  
  return true;
}

Core::ActionHandle ActionDeleteLayers::Create( std::vector< std::string > layers )
{
  ActionDeleteLayers* action = new ActionDeleteLayers;
  action->layers_vector_ = layers;

  return Core::ActionHandle( action );
}


void ActionDeleteLayers::Dispatch( Core::ActionContextHandle context, std::vector< std::string > layers )
{
  Core::ActionDispatcher::PostAction( Create( layers ), context );
}

} // end namespace Seg3D
