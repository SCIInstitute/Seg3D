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

#include <Core/Action/ActionFactory.h>
#include <Core/Volume/MaskVolumeSlice.h>

#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Tools/Actions/ActionCopyPaste.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>
#include <Application/UndoBuffer/UndoBuffer.h>

CORE_REGISTER_ACTION( Seg3D, CopyPaste )

namespace Seg3D
{

class ActionCopyPastePrivate
{
public:
  std::string source_layer_id_;
  int src_slice_type_;
  size_t src_slice_number_;
  std::string target_layer_id_;
  int dst_slice_type_;
  size_t dst_min_slice_;
  size_t dst_max_slice_;

  Core::MaskLayerHandle source_layer_;
  Core::MaskVolumeSliceHandle source_vol_slice_;
  Core::MaskLayerHandle target_layer_;
  Core::MaskVolumeSliceHandle target_vol_slice_;
};

ActionCopyPaste::ActionCopyPaste() :
  private_( new ActionCopyPastePrivate )
{
  this->add_layer_id( this->private_->source_layer_id_ );
  this->add_parameter( this->private_->src_slice_type_ );
  this->add_parameter( this->private_->src_slice_number_ );
  this->add_layer_id( this->private_->target_layer_id_ );
  this->add_parameter( this->private_->dst_slice_type_ );
  this->add_parameter( this->private_->dst_min_slice_ );
  this->add_parameter( this->private_->dst_max_slice_ );
}

bool ActionCopyPaste::validate( Core::ActionContextHandle& context )
{ 
  if ( !LayerManager::CheckLayerExistanceAndType( this->private_->source_layer_id_,
    Core::VolumeType::MASK_E, context ) ) return false;

  if ( !LayerManager::CheckLayerAvailabilityForUse( 
    this->private_->source_layer_id_, context ) ) return false;
  
  if ( !( LayerManager::CheckLayerExistanceAndType( this->private_->target_layer_id_,
    Core::VolumeType::MASK_E, context ) ) ) return false;
  
  if ( !LayerManager::CheckLayerAvailabilityForProcessing(
    this->private_->target_layer_id_, context ) ) return false;
  
  this->private_->source_layer_ = LayerManager::FindMaskLayer( this->private_->source_layer_id_ );
  this->private_->target_layer_ = LayerManager::FindMaskLayer( this->private_->target_layer_id_ );
  
  if ( this->private_->src_slice_type_ != Core::VolumeSliceType::AXIAL_E &&
    this->private_->src_slice_type_ != Core::VolumeSliceType::CORONAL_E &&
    this->private_->src_slice_type_ != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid source slice type" );
    return false;
  }

  if ( this->private_->dst_slice_type_ != Core::VolumeSliceType::AXIAL_E &&
    this->private_->dst_slice_type_ != Core::VolumeSliceType::CORONAL_E &&
    this->private_->dst_slice_type_ != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid target slice type" );
    return false;
  }
  
  if ( this->private_->dst_min_slice_ > this->private_->dst_max_slice_ )
  {
    std::swap( this->private_->dst_min_slice_, this->private_->dst_max_slice_ );
  }
  
  Core::VolumeSliceType src_slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->src_slice_type_ );
  this->private_->source_vol_slice_.reset( new Core::MaskVolumeSlice(
    this->private_->source_layer_->get_mask_volume(), src_slice_type ) );
  if ( this->private_->src_slice_number_ >= this->private_->source_vol_slice_->number_of_slices() )
  {
    context->report_error( "Source slice number is out of range." );
    return false;
  }
  this->private_->source_vol_slice_->set_slice_number( this->private_->src_slice_number_ );

  Core::VolumeSliceType dst_slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->dst_slice_type_ );
  this->private_->target_vol_slice_.reset( new Core::MaskVolumeSlice(
    this->private_->target_layer_->get_mask_volume(), dst_slice_type ) );
  if ( this->private_->dst_min_slice_ >= this->private_->target_vol_slice_->number_of_slices() )
  {
    context->report_error( "Target slice number is out of range." );
    return false;
  }

  this->private_->dst_max_slice_ = Core::Min( this->private_->dst_max_slice_,
    this->private_->target_vol_slice_->number_of_slices() - 1 );

  if ( this->private_->source_vol_slice_->nx() != this->private_->target_vol_slice_->nx() ||
    this->private_->source_vol_slice_->ny() != this->private_->target_vol_slice_->ny() )
  {
    context->report_error( "The source and target slices don't match in dimensions." );
    return false;
  }
    
  return true;
}

bool ActionCopyPaste::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{   
  // Create a provenance record for this action
  ProvenanceStep* prov_step = new ProvenanceStep;
  prov_step->set_input_provenance_ids( this->get_input_provenance_ids() );
  prov_step->set_output_provenance_ids( this->get_output_provenance_ids( 1 ) );
  prov_step->set_replaced_provenance_ids( ProvenanceIDList( 1, 
    this->private_->target_layer_->provenance_id_state_->get() ) );
  prov_step->set_action( this->export_to_provenance_string() );
  ProvenanceStepHandle prov_step_handle( prov_step );
  ProvenanceStepID step_id = ProjectManager::Instance()->get_current_project()->
    add_provenance_record( prov_step_handle );

  // Build the undo/redo for this action
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "CopyPaste" ) );
  // The redo action is the current one
  item->set_redo_action( this->shared_from_this() );
  // Tell which provenance record to delete when undone
  item->set_provenance_step_id( step_id );
  // Create a check point of the slice on which the flood fill will operate
  LayerCheckPointHandle check_point( new LayerCheckPoint( this->private_->target_layer_, 
    this->private_->target_vol_slice_->get_slice_type(), this->private_->dst_min_slice_,
    this->private_->dst_max_slice_ ) );
  // Tell the item which layer to restore with which check point for the undo action
  item->add_layer_to_restore( this->private_->target_layer_, check_point );
  // Now add the undo/redo action to undo buffer
  UndoBuffer::Instance()->insert_undo_item( context, item );

  std::vector< unsigned char > buffer( this->private_->source_vol_slice_->nx() *
    this->private_->source_vol_slice_->ny() );
  this->private_->source_vol_slice_->copy_slice_data( &buffer[ 0 ] );

  for ( size_t i = this->private_->dst_min_slice_; i < this->private_->dst_max_slice_; ++i )
  {
    this->private_->target_vol_slice_->set_slice_number( i );
    this->private_->target_vol_slice_->set_slice_data( &buffer[ 0 ] );
  }
  this->private_->target_vol_slice_->set_slice_number( this->private_->dst_max_slice_ );
  this->private_->target_vol_slice_->set_slice_data( &buffer[ 0 ], true );
  this->private_->target_layer_->provenance_id_state_->set( this->get_output_provenance_id() );
  
  return true;
}

void ActionCopyPaste::clear_cache()
{
  this->private_->source_layer_.reset();
  this->private_->source_vol_slice_.reset();
  this->private_->target_layer_.reset();
  this->private_->target_vol_slice_.reset();
}

} // end namespace Seg3D
