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
#include <Application/Tools/Actions/ActionCopy.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/ViewerManager/ViewerManager.h>

CORE_REGISTER_ACTION( Seg3D, Copy )

namespace Seg3D
{

class ActionCopyPrivate
{
public:
  Core::ActionParameter< std::string > target_layer_id_;
  Core::ActionParameter< int > slice_type_;
  Core::ActionParameter< size_t > slice_number_;
  Core::ActionParameter< size_t > slot_number_;

  Core::MaskLayerHandle target_layer_;
  Core::MaskVolumeSliceHandle vol_slice_;

  bool deduce_params_;
};

ActionCopy::ActionCopy() :
  private_( new ActionCopyPrivate )
{
  this->add_argument( this->private_->target_layer_id_ );
  this->add_argument( this->private_->slice_type_ );
  this->add_argument( this->private_->slice_number_ );

  this->add_key( this->private_->slot_number_ );

  this->private_->deduce_params_ = false;
}

ActionCopy::~ActionCopy()
{
}

bool ActionCopy::validate( Core::ActionContextHandle& context )
{
  if ( this->private_->deduce_params_ )
  {
    size_t active_viewer_id = static_cast< size_t >( ViewerManager::Instance()->
      active_viewer_state_->get() );
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( active_viewer_id );
    if ( viewer->is_volume_view() )
    {
      context->report_error( "Can't copy from a volume view" );
      return false;
    }
    
    LayerHandle active_layer = LayerManager::Instance()->get_active_layer();

    if ( !active_layer || active_layer->get_type() != Core::VolumeType::MASK_E )
    {
      context->report_error( "Nothing to copy from" );
      return false;
    }
    
    Core::VolumeSliceHandle vol_slice = viewer->get_active_volume_slice();
    this->private_->target_layer_ = boost::dynamic_pointer_cast< MaskLayer >( active_layer );
    this->private_->target_layer_id_.value() = active_layer->get_layer_id();
    this->private_->slice_number_.value() = vol_slice->get_slice_number();
    this->private_->slice_type_.value() = vol_slice->get_slice_type();
    this->private_->slot_number_.value() = 0;
  }
  
  // Check whether the layer exists and is of the right type and return an
  // error if not
  std::string error;
  if ( !( LayerManager::CheckLayerExistanceAndType(
    this->private_->target_layer_id_.value(), Core::VolumeType::MASK_E, error ) ) )
  {
    context->report_error( error );
    return false;
  }

  // Check whether the layer is not locked for processing or creating, in which case the
  // data is not data yet, hence we cannot compute an isosurface on it. The function returns
  // a notifier when the action can be completed.
  Core::NotifierHandle notifier;
  if ( !( LayerManager::CheckLayerAvailabilityForProcessing( 
    this->private_->target_layer_id_.value(), notifier ) ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  this->private_->target_layer_ = 
    LayerManager::FindMaskLayer( this->private_->target_layer_id_.value() );
  
  if ( this->private_->slice_type_.value() != Core::VolumeSliceType::AXIAL_E &&
    this->private_->slice_type_.value() != Core::VolumeSliceType::CORONAL_E &&
    this->private_->slice_type_.value() != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_.value() );
    
  Core::MaskVolumeSliceHandle volume_slice( new Core::MaskVolumeSlice(
    this->private_->target_layer_->get_mask_volume(), slice_type ) );
  if ( this->private_->slice_number_.value() >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }

  volume_slice->set_slice_number( this->private_->slice_number_.value() );
  this->private_->vol_slice_ = volume_slice;
    
  return true;
}

bool ActionCopy::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  size_t nx = this->private_->vol_slice_->nx();
  size_t ny = this->private_->vol_slice_->ny();

  ClipboardItemHandle clipboard_item = Clipboard::Instance()->get_item( nx, ny,
    Core::DataType::UCHAR_E, this->private_->slot_number_.value() );
   this->private_->vol_slice_->copy_slice_data( reinterpret_cast< unsigned char* >( 
    clipboard_item->get_buffer() ) );

  return true;
}

void ActionCopy::clear_cache()
{
  this->private_->target_layer_.reset();
  this->private_->vol_slice_.reset();
}

void ActionCopy::Dispatch( Core::ActionContextHandle context )
{
  ActionCopy* action = new ActionCopy;
  action->private_->deduce_params_ = true;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

void ActionCopy::Dispatch( Core::ActionContextHandle context, 
              const std::string& layer_id, int slice_type, size_t slice_number )
{
  ActionCopy* action = new ActionCopy;
  action->private_->deduce_params_ = false;
  action->private_->target_layer_id_.value() = layer_id;
  action->private_->slice_type_.value() = slice_type;
  action->private_->slice_number_.value() = slice_number;
  action->private_->slot_number_.value() = 0;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
