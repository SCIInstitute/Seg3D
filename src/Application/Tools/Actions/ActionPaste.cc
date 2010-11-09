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

#include <Application/Clipboard/Clipboard.h>
#include <Application/Tools/Actions/ActionPaste.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/LayerUndoBuffer.h>
#include <Application/ViewerManager/ViewerManager.h>


CORE_REGISTER_ACTION( Seg3D, Paste )

namespace Seg3D
{

class ActionPastePrivate
{
public:
  Core::ActionParameter< std::string > target_layer_id_;
  Core::ActionParameter< int > slice_type_;
  Core::ActionParameter< size_t > min_slice_;
  Core::ActionParameter< size_t > max_slice_;
  Core::ActionParameter< size_t > slot_number_;

  Core::ActionCachedHandle< Core::MaskLayerHandle > target_layer_;
  Core::ActionCachedHandle< Core::MaskVolumeSliceHandle > vol_slice_;

  bool deduce_params_;
};

ActionPaste::ActionPaste() :
  private_( new ActionPastePrivate )
{
  this->add_argument( this->private_->target_layer_id_ );
  this->add_argument( this->private_->slice_type_ );
  this->add_argument( this->private_->min_slice_ );
  this->add_argument( this->private_->max_slice_ );

  this->add_key( this->private_->slot_number_ );

  this->add_cachedhandle( this->private_->target_layer_ );
  this->add_cachedhandle( this->private_->vol_slice_ );

  this->private_->deduce_params_ = false;
}

ActionPaste::~ActionPaste()
{
}

bool ActionPaste::validate( Core::ActionContextHandle& context )
{
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
    
    Core::VolumeSliceHandle vol_slice = viewer->get_active_volume_slice();
    this->private_->target_layer_.handle() = boost::dynamic_pointer_cast< MaskLayer >( active_layer );
    this->private_->target_layer_id_.value() = active_layer->get_layer_id();
    this->private_->min_slice_.value() = vol_slice->get_slice_number();
    this->private_->max_slice_.value() = vol_slice->get_slice_number();
    this->private_->slice_type_.value() = vol_slice->get_slice_type();
    this->private_->slot_number_.value() = 0;
  }
  
  if ( !this->cache_mask_layer_handle( context, this->private_->target_layer_id_,
    this->private_->target_layer_ ) )
  {
    return false;
  }

  Core::NotifierHandle notifier;
  if ( !LayerManager::Instance()->CheckLayerAvailabilityForProcessing(
    this->private_->target_layer_id_.value(), notifier ) )
  {
    context->report_error( "Mask layer '" + this->private_->target_layer_id_.value() + 
      "' is not available for editing." );
  }
  
  if ( this->private_->slice_type_.value() != Core::VolumeSliceType::AXIAL_E &&
    this->private_->slice_type_.value() != Core::VolumeSliceType::CORONAL_E &&
    this->private_->slice_type_.value() != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  
  if ( this->private_->min_slice_.value() > this->private_->max_slice_.value() )
  {
    std::swap( this->private_->min_slice_.value(), this->private_->max_slice_.value() );
  }
  
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_.value() );
  Core::MaskVolumeSliceHandle volume_slice( new Core::MaskVolumeSlice(
    this->private_->target_layer_.handle()->get_mask_volume(), slice_type ) );
  if ( this->private_->min_slice_.value() >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }

  this->private_->max_slice_.value() = Core::Min( this->private_->max_slice_.value(),
    volume_slice->number_of_slices() - 1 );
  this->private_->vol_slice_.handle() = volume_slice;

  ClipboardItemConstHandle clipboard_item = Clipboard::Instance()->get_item(
    this->private_->slot_number_.value() );
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
  // Build the undo/redo for this action
  LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "Paste" ) );
  
  // Get the axis along which the flood fill works
  int axis = this->private_->slice_type_.value();
  
  // Get the slice number
  size_t min_slice = this->private_->min_slice_.value();
  size_t max_slice = this->private_->max_slice_.value();
  
  // Get the layer on which this action operates
  LayerHandle layer = LayerManager::Instance()->get_layer_by_id( 
    this->private_->target_layer_id_.value() );
  // Create a check point of the slice on which the flood fill will operate
  LayerCheckPointHandle check_point( new LayerCheckPoint( layer, min_slice, max_slice, axis ) );

  // The redo action is the current one
  item->set_redo_action( this->shared_from_this() );
  // Tell the item which layer to restore with which check point for the undo action
  item->add_layer_to_restore( layer, check_point );

  // Now add the undo/redo action to undo buffer
  LayerUndoBuffer::Instance()->insert_undo_item( context, item );

  Core::MaskVolumeSliceHandle volume_slice = this->private_->vol_slice_.handle();
  ClipboardItemConstHandle clipboard_item = Clipboard::Instance()->get_item( 
    this->private_->slot_number_.value() );
  for ( size_t i = this->private_->min_slice_.value(); i < this->private_->max_slice_.value(); ++i )
  {
    volume_slice->set_slice_number( i );
    volume_slice->set_slice_data( reinterpret_cast< const unsigned char* >( 
      clipboard_item->get_buffer() ) );
  }
  volume_slice->set_slice_number( this->private_->max_slice_.value() );
  volume_slice->set_slice_data( reinterpret_cast< const unsigned char* >( 
    clipboard_item->get_buffer() ), true );
  
  return true;
}

void ActionPaste::Dispatch( Core::ActionContextHandle context )
{
  ActionPaste* action = new ActionPaste;
  action->private_->deduce_params_ = true;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionPaste::Dispatch( Core::ActionContextHandle context, 
               const std::string& layer_id, int slice_type, 
               size_t min_slice, size_t max_slice )
{
  ActionPaste* action = new ActionPaste;
  action->private_->target_layer_id_.value() = layer_id;
  action->private_->slice_type_.value() = slice_type;
  action->private_->min_slice_.value() = min_slice;
  action->private_->max_slice_.value() = max_slice;
  action->private_->slot_number_.value() = 0;
  action->private_->deduce_params_ = false;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}


} // end namespace Seg3D