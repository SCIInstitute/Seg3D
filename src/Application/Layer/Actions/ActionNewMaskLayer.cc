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
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>

#include <Application/Provenance/Provenance.h>
#include <Application/Provenance/ProvenanceStep.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerUndoBufferItem.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/Layer/Actions/ActionNewMaskLayer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, NewMaskLayer )

namespace Seg3D
{

class ActionNewMaskLayerPrivate
{
public:
  // The name of the group where the mask needs to be added
  std::string group_id_;
  // The sandbox in which to run the action
  SandboxID sandbox_;

  // Cached layer handle
  // NOTE: Although the action takes a group ID as input, internally
  // it uses a layer from that group. This is for compatibility with
  // sandbox mode.
  LayerHandle layer_;
};

ActionNewMaskLayer::ActionNewMaskLayer() :
  private_( new ActionNewMaskLayerPrivate )
{
  this->add_group_id( this->private_->group_id_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionNewMaskLayer::validate( Core::ActionContextHandle& context )
{
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) )
  {
    return false;
  }
  
  if ( this->private_->sandbox_ == -1 )
  {
    LayerGroupHandle layer_group = LayerManager::FindGroup( this->private_->group_id_ );
    if ( layer_group )
    {
      this->private_->layer_ = layer_group->bottom_layer();
    }
  }
  else
  {
    this->private_->layer_ = LayerManager::FindLayer( this->private_->group_id_, this->private_->sandbox_ );
  }

  if ( !this->private_->layer_ )
  {
    context->report_error( "Layer group '" + this->private_->group_id_ + "' doesn't exist." );
    return false;
  }

  return true; // validated
}

bool ActionNewMaskLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  // Get the current counters for groups and layers, so we can undo the changes to those counters
  // NOTE: This needs to be done before a new layer is created
  LayerManager::id_count_type id_count = LayerManager::GetLayerIdCount();

  // Create a new mask volume.
  Core::MaskVolumeHandle new_mask_volume; 
  Core::MaskVolume::CreateEmptyMask( this->private_->layer_->get_grid_transform(), new_mask_volume );
  
  // Create a new container to put it in.
  LayerHandle new_mask_layer( new MaskLayer( "MaskLayer", new_mask_volume ) );
  new_mask_layer->set_meta_data( this->private_->layer_->get_meta_data() );

  // Register the new layer with the LayerManager. This will insert it into the right group.
  LayerManager::Instance()->insert_layer( new_mask_layer, this->private_->sandbox_ );
  
  // report the layer ID to action result
  result.reset( new Core::ActionResult( new_mask_layer->get_layer_id() ) );
  
  // The following steps are only needed if not running in a sandbox
  if ( this->private_->sandbox_ == -1 )
  {
    // Now we make it active
    LayerManager::Instance()->set_active_layer( new_mask_layer );

    // Set a new provenance ID for the output
    new_mask_layer->provenance_id_state_->set( this->get_output_provenance_id( 0 ) );

    // Create a provenance record
    ProvenanceStepHandle provenance_step( new ProvenanceStep );
    
    // Get the input provenance ids from the translate step
    provenance_step->set_input_provenance_ids( this->get_input_provenance_ids() );
    
    // Get the output and replace provenance ids from the analysis above
    provenance_step->set_output_provenance_ids( this->get_output_provenance_ids() );
      
    // Get the action and turn it into provenance 
    provenance_step->set_action_name( this->get_type() );
    provenance_step->set_action_params( this->export_params_to_provenance_string() );   
    
    // Add step to provenance record
    ProvenanceStepID step_id = ProjectManager::Instance()->get_current_project()->
      add_provenance_record( provenance_step );   

    // Create an undo item for this action
    LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "New Mask" ) );
    // Tell which action has to be re-executed to obtain the result
    item->set_redo_action( this->shared_from_this() );
    // Tell which provenance record to delete when undone
    item->set_provenance_step_id( step_id );
    // Tell which layer was added so undo can delete it
    item->add_layer_to_delete( new_mask_layer );
    // Tell what the layer/group id counters are so we can undo those as well
    item->add_id_count_to_restore( id_count );
    // Add the complete record to the undo buffer
    UndoBuffer::Instance()->insert_undo_item( context, item );
  }
  
  return true;
}

void ActionNewMaskLayer::clear_cache()
{
  this->private_->layer_.reset();
}

void ActionNewMaskLayer::Dispatch( Core::ActionContextHandle context, const std::string& group_id )
{
  ActionNewMaskLayer* action = new ActionNewMaskLayer;
  action->private_->group_id_ = group_id;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
