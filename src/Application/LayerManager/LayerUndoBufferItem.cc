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

#include <Application/LayerManager/LayerUndoBufferItem.h>
#include <Application/LayerManager/LayerManager.h>

namespace Seg3D
{

class LayerUndoBufferItemPrivate
{
public:
  // -- tag for the menu --
  std::string tag_;
  
  // -- Which filters need to be stopped --
  std::vector< LayerAbstractFilterWeakHandle > layer_filters_to_abort_;
  
  // -- Which layers need to be deleted --
  std::vector<LayerHandle> layers_to_delete_;

  // -- Which layers need to be added in again --
  std::vector<LayerHandle> layers_to_add_;

  // -- Which layers need to be restored --
  std::vector< std::pair<LayerHandle, LayerCheckPointHandle > > layers_to_restore_;

  // An action can always be redone by executing the action again 
  Core::ActionHandle redo_action_;
  
  // The count of the id of a layer, this one has to be roled back in case of undo
  LayerManager::id_count_type id_count_;
  
  // Size of the item
  size_t size_;
};

LayerUndoBufferItem::LayerUndoBufferItem( const std::string& tag ) :
  private_( new LayerUndoBufferItemPrivate )
{
  this->private_->tag_ = tag;
  this->private_->size_ = 0;
  this->private_->id_count_ = LayerManager::GetLayerInvalidIdCount();
}

LayerUndoBufferItem::~LayerUndoBufferItem()
{
}

void LayerUndoBufferItem::set_redo_action( Core::ActionHandle action )
{
  this->private_->redo_action_ = action;
}

void LayerUndoBufferItem::add_filter_to_abort( LayerAbstractFilterHandle filter )
{
  this->private_->layer_filters_to_abort_.push_back( LayerAbstractFilterWeakHandle ( filter ) );
}

void LayerUndoBufferItem::add_layer_to_delete( LayerHandle layer )
{
  this->private_->layers_to_delete_.push_back( layer );
}

void LayerUndoBufferItem::add_layer_to_add( LayerHandle layer )
{
  this->private_->layers_to_add_.push_back( layer );
}

void LayerUndoBufferItem::add_layer_to_restore( LayerHandle layer, 
  LayerCheckPointHandle checkpoint )
{
  this->private_->layers_to_restore_.push_back( std::make_pair( layer, checkpoint ) );
}

bool LayerUndoBufferItem::apply_redo( Core::ActionContextHandle& context )
{
  // Clear the redo cache, which records the current identifier counters
  this->private_->redo_action_->clear_redo_cache( context );

  // Validate the action. It should validate, but if it doesn't it should fail
  // gracefully. Hence we check anyway.
  if ( !( this->private_->redo_action_->validate( context ) ) )
  {
    return false;
  }
  
  // Run the action. Make the changes to the state engine.
  // NOTE: We do not use the result yet
  // TODO: Need to implent result handling 
  // --JGS
  
  Core::ActionResultHandle result;
  if ( !( this->private_->redo_action_->run( context, result ) ) )
  {
    return false;
  }
  
  this->private_->redo_action_->clear_cache();
  
  return true;
}

bool LayerUndoBufferItem::apply_and_clear_undo()
{
  // Step 1: 
  // Any filter action that is still running needs to be aborted to do an undo
  // Hence every asynchronous filter will register the filter thread with the
  // Undo action. This is then used to abort the action and we need to unfortunately
  // wait until the abort is successful to reapply the check point, as otherwise there
  // are all kinds of locking issues that are resolvable but at a high memory overhead.
  // Hence to keep the program running smoothly, we will wait until the action has been
  // fully terminated. 

  for ( size_t j = 0; j < this->private_->layer_filters_to_abort_.size(); j++ )
  {
    // Get a handle to the filter if it still exist
    LayerAbstractFilterHandle filter = this->private_->layer_filters_to_abort_[ j ].lock();
    
    // If it is still there...
    if ( filter )
    {
      // Notify the user that undoing an asynchronous filter may take a bit of time
      std::string message = std::string("Undoing filter '") + this->get_tag() 
        + std::string("'");
    
      // Create application wide progress bar, that blocks the full program
      Core::ActionProgressHandle progress = 
        Core::ActionProgressHandle( new Core::ActionProgress( message ) );      

      // Start the spinning wheel
      progress->begin_progress_reporting();

      // Kill the filter. 
      // NOTE: For a few teem filters we currently do not have a decent abort yet
      filter->abort_and_wait();

      // Stop spinning wheel
      progress->end_progress_reporting();
    }
    // The filter should no longer be running and all the locks on the support layers
    // should have been released.
  }
  
  // Remove this list as we are done with it.
  this->private_->layer_filters_to_abort_.clear();
  
  // Step 2:
  // Now the filter is no longer running, we need to restore the data in layer
  
  for ( size_t j = 0; j < this->private_->layers_to_restore_.size(); j++ )
  { 
    // Get the layer handle of the layer that needs to be restored.
    LayerHandle layer = this->private_->layers_to_restore_[ j ].first;

    // Apply the check point, this should restore the data
    this->private_->layers_to_restore_[ j ].second->apply( layer );
  }
  
  // Remove the layers from the undo mechanism so the program can actually delete them afterwards
  this->private_->layers_to_restore_.clear();

  // Step 3:
  // Delete the layers that were created by the action.

  // For layers that were created by the action, delete them again.
  for ( size_t j = 0; j < this->private_->layers_to_delete_.size(); j++ )
  {
    LayerHandle layer = this->private_->layers_to_delete_[ j ];

    // Delete the layer from the layer manager
    LayerManager::Instance()->delete_layer( layer );
  } 
  
  // Remove the layers from the undo mechanism so the program can actually delete them
  this->private_->layers_to_delete_.clear();
  
  // Step 4:
  // A delete action can have deleted layers. Hence we need to restore layers that may have been
  // deleted.
  
  for ( size_t j = 0; j < this->private_->layers_to_add_.size(); j++ )
  {
    LayerHandle layer = this->private_->layers_to_add_[ j ];
    if ( layer->data_state_->get() == Layer::CREATING_C ||  
      layer->data_state_->get() == Layer::PROCESSING_C )
    {
      LayerAbstractFilterHandle filter = layer->get_filter_handle();
      if ( filter ) 
      {
        // Notify the user that undoing an asynchronous filter may take a bit of time
        std::string message = std::string( "Undoing delete layer" );
    
        // Create application wide progress bar, that blocks the full program
        Core::ActionProgressHandle progress = 
        Core::ActionProgressHandle( new Core::ActionProgress( message ) );      

        // Start the spinning wheel
        progress->begin_progress_reporting();
            
        filter->abort_and_wait();

        // Start the spinning wheel
        progress->end_progress_reporting();
      }
    }
  
    if ( !( LayerManager::Instance()->insert_layer( this->private_->layers_to_add_[ j ] ) ) )
    {
      return false;
    }
  } 
  
  // Remove the layers from the undo mechanism so the program can actually delete them afterwards
  this->private_->layers_to_add_.clear(); 
  
  // The counters need to be roled back so when redo is done, it actually redos the same action
  // resulting in the same counters. This only needs to be done if layers got created and deleted
  // in the undo.
  
  LayerManager::SetLayerIdCount( this->private_->id_count_ );
  
  return true;
}

size_t LayerUndoBufferItem::get_byte_size() const
{
  return this->private_->size_;
}

void LayerUndoBufferItem::compute_size()
{
  size_t size = 0;

  for ( size_t j = 0; j < this->private_->layers_to_restore_.size(); j++ )
  { 
    // Get the layer handle of the layer that needs to be restored and check the size
    // of the data stored to make the undo point
    size += this->private_->layers_to_restore_[ j ].second->get_byte_size();
  }
    
  for ( size_t j = 0; j < this->private_->layers_to_add_.size(); j++ )
  {   
    size += this->private_->layers_to_add_[ j ]->get_byte_size();
  }

  this->private_->size_ = size;
}


std::string LayerUndoBufferItem::get_tag() const
{
  return this->private_->tag_;
}

void LayerUndoBufferItem::add_id_count_to_restore( LayerManager::id_count_type id_count )
{
    this->private_->id_count_ = id_count;
}


} // end namespace Seg3D
