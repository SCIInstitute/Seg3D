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
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerUndoBufferItem.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>

namespace Seg3D
{

class LayerDeletionUndoRecord
{
public:
  // The layer group from which layers have been deleted
  LayerGroupHandle layer_group_;

  // A map of deleted layers keyed by their position
  std::map< size_t, LayerHandle > layer_pos_map_;
};

typedef boost::shared_ptr< LayerDeletionUndoRecord > LayerDeletionUndoRecordHandle;

class LayerUndoBufferItemPrivate
{
public: 
  // -- Which filters need to be stopped --
  std::vector< LayerAbstractFilterWeakHandle > layer_filters_to_abort_;
  
  // -- Which layers need to be deleted --
  std::vector<LayerHandle> layers_to_delete_;

  // -- Which layers need to be added in again --
  std::map< size_t, LayerDeletionUndoRecordHandle > layers_to_add_;

  // -- Which layers need to be restored --
  std::vector< std::pair<LayerHandle, LayerCheckPointHandle > > layers_to_restore_;
  
  // The count of the id of a layer, this one has to be rolled back in case of undo
  LayerManager::id_count_type id_count_;

  // Provenance step ID of the associated action.
  std::vector< ProvenanceStepID > prov_step_ids_;
  
  // Size of the item
  size_t size_;
};

LayerUndoBufferItem::LayerUndoBufferItem( const std::string& tag ) :
  UndoBufferItem( tag ),
  private_( new LayerUndoBufferItemPrivate )
{
  this->private_->size_ = 0;
  this->private_->id_count_ = LayerManager::GetLayerInvalidIdCount();
}

LayerUndoBufferItem::~LayerUndoBufferItem()
{
  ASSERT_IS_APPLICATION_THREAD();

  // Invalidates all the deleted layers cached for undelete
  std::map< size_t, LayerDeletionUndoRecordHandle >::iterator group_it = 
    this->private_->layers_to_add_.begin();
  while ( group_it != this->private_->layers_to_add_.end() )
  {
    LayerDeletionUndoRecordHandle record = ( *group_it ).second;
    if ( record->layer_group_->is_empty() )
    {
      record->layer_group_->invalidate();
    }
    std::map< size_t, LayerHandle >::iterator layer_it = record->layer_pos_map_.begin();
    while ( layer_it != record->layer_pos_map_.end() )
    {
      ( *layer_it ).second->invalidate();
      ++layer_it;
    }
    ++group_it;
  }
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
  if ( !layer || !layer->has_valid_data() )
  {
    return;
  }
  
  LayerGroupHandle layer_group = layer->get_layer_group();
  size_t group_pos = LayerManager::Instance()->get_group_position( layer_group );
  size_t layer_pos = layer_group->get_layer_position( layer );

  std::map< size_t, LayerDeletionUndoRecordHandle >::iterator group_it =
    this->private_->layers_to_add_.find( group_pos );
  LayerDeletionUndoRecordHandle undo_record;


  if ( group_it != this->private_->layers_to_add_.end() )
  {
    undo_record = ( *group_it ).second;
    assert( undo_record->layer_group_ == layer_group );
  }
  else
  {
    undo_record.reset( new LayerDeletionUndoRecord );
    undo_record->layer_group_ = layer_group;
    this->private_->layers_to_add_[ group_pos ] = undo_record;
  }

  assert( undo_record->layer_pos_map_.count( layer_pos ) == 0 );
  undo_record->layer_pos_map_[ layer_pos ] = layer;

  // Modify the name, so another layer can take the same name
  std::string new_name = std::string( "UNDOBUFFER_" ) + layer->name_state_->get();
  layer->name_state_->set( new_name );
}

void LayerUndoBufferItem::set_provenance_step_id( ProvenanceStepID step_id )
{
  this->private_->prov_step_ids_.resize( 1 );
  this->private_->prov_step_ids_[ 0 ] = step_id;
}

void LayerUndoBufferItem::set_provenance_step_ids( const std::vector< ProvenanceStepID >& step_ids )
{
  this->private_->prov_step_ids_ = step_ids;
}

void LayerUndoBufferItem::add_layer_to_restore( LayerHandle layer, 
  LayerCheckPointHandle checkpoint )
{
  this->private_->layers_to_restore_.push_back( std::make_pair( layer, checkpoint ) );
}

void LayerUndoBufferItem::rollback_layer_changes()
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
  // A delete action can have deleted layers. Hence we need to restore layers that may have been
  // deleted.

  std::vector< LayerHandle > layers;
  std::vector< size_t > group_positions;
  std::vector< size_t > layer_positions;

  std::map< size_t, LayerDeletionUndoRecordHandle >::const_iterator group_it = 
    this->private_->layers_to_add_.begin();
  while ( group_it != this->private_->layers_to_add_.end() )
  {
    LayerDeletionUndoRecordHandle undo_record = ( *group_it ).second;
    size_t group_pos = ( *group_it ).first;
    std::map< size_t, LayerHandle >::const_iterator layer_it = undo_record->layer_pos_map_.begin();
    while ( layer_it != undo_record->layer_pos_map_.end() )
    {
      LayerHandle layer = ( *layer_it ).second;
      size_t layer_pos = ( *layer_it ).first;

      // Restore the old name
      std::string old_name = layer->name_state_->get().substr( 11 );
      layer->name_state_->set( old_name );


      LayerAbstractFilterHandle filter = layer->get_filter_handle();
      if ( filter ) 
      {
        // Notify the user that undoing an asynchronous filter may take a bit of time
        std::string message = std::string( "Undoing delete layer" );

        // Create application wide progress bar, that blocks the full program
        Core::ActionProgressHandle progress( new Core::ActionProgress( message ) );     

        // Start the spinning wheel
        progress->begin_progress_reporting();

        filter->abort_and_wait();

        // Start the spinning wheel
        progress->end_progress_reporting();
      }

      layers.push_back( layer );
      group_positions.push_back( group_pos );
      layer_positions.push_back( layer_pos );

      ++layer_it;
    }
    ++group_it;
  }
  // Undelete layers
  LayerManager::Instance()->undelete_layers( layers, group_positions, layer_positions );

  // Remove the layers from the undo mechanism so the program can actually delete them afterwards
  this->private_->layers_to_add_.clear(); 

  // Step 4:
  // Delete the layers that were created by the action.

  // For layers that were created by the action, delete them again.
  for ( size_t j = 0; j < this->private_->layers_to_delete_.size(); j++ )
  {
    LayerHandle layer = this->private_->layers_to_delete_[ j ];
    // Invalidate the layer so the layer ID can be reused
    layer->invalidate();

    // Delete the layer from the layer manager
    LayerManager::Instance()->delete_layer( layer );
  } 

  // Remove the layers from the undo mechanism so the program can actually delete them
  this->private_->layers_to_delete_.clear();

  // Step 5:
  // Delete the provenance record.
  for ( size_t i = 0 ; i < this->private_->prov_step_ids_.size(); ++i )
  {
    ProjectManager::Instance()->get_current_project()->
      delete_provenance_record( this->private_->prov_step_ids_[ i ] );
  } 
  this->private_->prov_step_ids_.clear();
}

bool LayerUndoBufferItem::apply_and_clear_undo()
{
  this->rollback_layer_changes();

  // The counters need to be rolled back so when redo is done, it actually redos the same action
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
    
  std::map< size_t, LayerDeletionUndoRecordHandle >::iterator group_it = 
    this->private_->layers_to_add_.begin();
  while ( group_it != this->private_->layers_to_add_.end() )
  {
    LayerDeletionUndoRecordHandle record = ( *group_it ).second;
    std::map< size_t, LayerHandle >::iterator layer_it = record->layer_pos_map_.begin();
    while ( layer_it != record->layer_pos_map_.end() )
    {
      if ( ( *layer_it ).second->get_type() == Core::VolumeType::MASK_E )
      {
        // NOTE: Mask volumes only use 1 bit out of a byte
        size += ( *layer_it ).second->get_byte_size() / 8;
      }
      else
      {
        size += ( *layer_it ).second->get_byte_size();
      }
      ++layer_it;
    }
    ++group_it;
  }

  this->private_->size_ = size;
}

void LayerUndoBufferItem::add_id_count_to_restore( LayerManager::id_count_type id_count )
{
    this->private_->id_count_ = id_count;
}

} // end namespace Seg3D
