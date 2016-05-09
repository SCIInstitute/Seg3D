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

#include <Core/Action/ActionFactory.h>
#include <Core/Volume/MaskVolumeSlice.h>

#include <Application/Clipboard/Clipboard.h>
#include <Application/Tools/Actions/ActionPaste.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerUndoBufferItem.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/ViewerManager/ViewerManager.h>

CORE_REGISTER_ACTION( Seg3D, Paste )

namespace Seg3D
{

class ActionPastePrivate
{
public:
  std::string target_layer_id_;
  int slice_type_;
  size_t min_slice_;
  size_t max_slice_;
  long long sandbox_;

  Core::MaskLayerHandle target_layer_;
  Core::MaskVolumeSliceHandle vol_slice_;

  bool deduce_params_;
  bool punch_through_;
};

ActionPaste::ActionPaste() :
  LayerAction(),
  private_( new ActionPastePrivate )
{
  this->add_layer_id( this->private_->target_layer_id_ );
  this->add_parameter( this->private_->slice_type_ );
  this->add_parameter( this->private_->min_slice_ );
  this->add_parameter( this->private_->max_slice_ );
  this->add_parameter( this->private_->sandbox_ );

  this->private_->deduce_params_ = false;
  this->private_->punch_through_ = false;
}

bool ActionPaste::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) )
  {
    return false;
  }

  if ( this->private_->deduce_params_ )
  {
    size_t active_viewer_id = static_cast< size_t >( ViewerManager::Instance()->
      active_viewer_state_->get() );
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( active_viewer_id );
    if ( viewer->is_volume_view() )
    {
      context->report_error( "Can't paste into a volume view" );
      return false;
    }
    
    LayerHandle active_layer = LayerManager::Instance()->get_active_layer();
    if ( !active_layer || active_layer->get_type() != Core::VolumeType::MASK_E )
    {
      context->report_error( "Invalid paste target" );
      return false;
    }
    
    if ( !( active_layer->master_visible_state_->get() ) || 
      !( active_layer->visible_state_[ active_viewer_id ]->get() ) )
    {
      if ( context->source() == Core::ActionSource::INTERFACE_WIDGET_E ||
        context->source() == Core::ActionSource::INTERFACE_MENU_E  )
      {
        context->report_error( "Cannot paste into invisible layer" );
        return false;
      }
    }
    
    Core::VolumeSliceHandle vol_slice = viewer->get_active_volume_slice();
    this->private_->target_layer_ = boost::dynamic_pointer_cast< MaskLayer >( active_layer );
    this->private_->target_layer_id_ = active_layer->get_layer_id();
    this->private_->slice_type_ = vol_slice->get_slice_type();
    if ( this->private_->punch_through_ )
    {
      this->private_->min_slice_ = 0;
      this->private_->max_slice_ = vol_slice->number_of_slices() - 1;
    }
    else
    {
      this->private_->min_slice_ = vol_slice->get_slice_number();
      this->private_->max_slice_ = vol_slice->get_slice_number();
    }

    // Need to translate the action again because the parameters weren't ready
    if ( !this->translate( context ) ) return false;

    // Set the deduce_params_ flag to false so if the action is re-run (by the undo buffer),
    // it won't need to go through the process again. 
    this->private_->deduce_params_ = false;
  }
  
  if ( !( LayerManager::CheckLayerExistenceAndType( this->private_->target_layer_id_,
    Core::VolumeType::MASK_E, context, this->private_->sandbox_ ) ) ) return false;
  
  if ( !LayerManager::CheckLayerAvailabilityForProcessing(
    this->private_->target_layer_id_, context, this->private_->sandbox_ ) ) return false;
  
  this->private_->target_layer_ = LayerManager::FindMaskLayer(
    this->private_->target_layer_id_, this->private_->sandbox_ );
  
  if ( this->private_->slice_type_ != Core::VolumeSliceType::AXIAL_E &&
    this->private_->slice_type_ != Core::VolumeSliceType::CORONAL_E &&
    this->private_->slice_type_ != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  
  if ( this->private_->min_slice_ > this->private_->max_slice_ )
  {
    std::swap( this->private_->min_slice_, this->private_->max_slice_ );
  }
  
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_ );
  Core::MaskVolumeSliceHandle volume_slice( new Core::MaskVolumeSlice(
    this->private_->target_layer_->get_mask_volume(), slice_type ) );
  if ( this->private_->min_slice_ >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }

  this->private_->max_slice_ = Core::Min( this->private_->max_slice_,
    volume_slice->number_of_slices() - 1 );
  this->private_->vol_slice_ = volume_slice;

  ClipboardItemConstHandle clipboard_item = Clipboard::Instance()->get_item(
    this->private_->sandbox_ );

  if ( !clipboard_item )
  {
    context->report_error( "Nothing to paste" );
    return false;
  }
  if ( clipboard_item->get_width() != volume_slice->nx() ||
    clipboard_item->get_height() != volume_slice->ny() ||
    clipboard_item->get_data_type() != Core::DataType::UCHAR_E )
  {
    context->report_error( "The clipboard content doesn't match the target slice" );
    return false;
  }
    
  return true;
}

bool ActionPaste::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  ClipboardItemConstHandle clipboard_item = Clipboard::Instance()->get_item( this->private_->sandbox_ );

  // Only create provenance and undo record if the action is not running in a sandbox
  if ( this->private_->sandbox_ == -1 )
  {
    ProvenanceID clipboard_pid = clipboard_item->get_provenance_id();
    ProvenanceIDList input_pids = this->get_input_provenance_ids();
    input_pids.push_back( clipboard_pid );
    ProvenanceIDList deleted_pids;
    deleted_pids.push_back( input_pids[ 0 ] );
    
    ProvenanceStepHandle prov_step( new ProvenanceStep );
    prov_step->set_input_provenance_ids( input_pids );
    prov_step->set_output_provenance_ids( this->get_output_provenance_ids( 1 ) );
    prov_step->set_replaced_provenance_ids( deleted_pids );
    prov_step->set_action_name( this->get_type() );
    prov_step->set_action_params( this->export_params_to_provenance_string() );
    
    ProvenanceStepID step_id = ProjectManager::Instance()->get_current_project()->
      add_provenance_record( prov_step );

    // Build the undo/redo for this action
    LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Paste" ) );

    // The redo action is the current one
    item->set_redo_action( this->shared_from_this() );
    
    // Tell which provenance record to delete when undone
    item->set_provenance_step_id( step_id );
          
    // Create a check point of the slice on which the flood fill will operate
    LayerCheckPointHandle check_point( new LayerCheckPoint( 
      this->private_->target_layer_, this->private_->vol_slice_->get_slice_type(), 
      this->private_->min_slice_, this->private_->max_slice_ ) );

    // Tell the item which layer to restore with which check point for the undo action
    item->add_layer_to_restore( this->private_->target_layer_, check_point );

    // Now add the undo/redo action to undo buffer
    UndoBuffer::Instance()->insert_undo_item( context, item );

    this->private_->target_layer_->provenance_id_state_->set(
      this->get_output_provenance_id() );
  }

  Core::MaskVolumeSliceHandle volume_slice = this->private_->vol_slice_;
  for ( size_t i = this->private_->min_slice_; i < this->private_->max_slice_; ++i )
  {
    volume_slice->set_slice_number( i );
    volume_slice->set_slice_data( reinterpret_cast< const unsigned char* >( 
      clipboard_item->get_buffer() ) );
  }
  volume_slice->set_slice_number( this->private_->max_slice_ );
  volume_slice->set_slice_data( reinterpret_cast< const unsigned char* >( 
    clipboard_item->get_buffer() ), true );
    
  result.reset( new Core::ActionResult( this->private_->target_layer_id_ ) );
  return true;
}

void ActionPaste::clear_cache()
{
  this->private_->target_layer_.reset();
  this->private_->vol_slice_.reset();
}

void ActionPaste::Dispatch( Core::ActionContextHandle context, bool punch_through )
{
  ActionPaste* action = new ActionPaste;
  action->private_->deduce_params_ = true;
  action->private_->punch_through_ = punch_through;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionPaste::Dispatch( Core::ActionContextHandle context, 
               const std::string& layer_id, int slice_type, 
               size_t min_slice, size_t max_slice )
{
  ActionPaste* action = new ActionPaste;
  action->private_->target_layer_id_ = layer_id;
  action->private_->slice_type_ = slice_type;
  action->private_->min_slice_ = min_slice;
  action->private_->max_slice_ = max_slice;
  action->private_->deduce_params_ = false;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
