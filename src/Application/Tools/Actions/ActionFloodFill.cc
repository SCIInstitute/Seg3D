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
#include <Core/Graphics/Algorithm.h>

#include <Application/Tools/Actions/ActionFloodFill.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>

CORE_REGISTER_ACTION( Seg3D, FloodFill )

namespace Seg3D
{

class ActionFloodFillPrivate
{
public:
  Core::ActionParameter< std::string > target_layer_id_;
  Core::ActionParameter< int > slice_type_;
  Core::ActionParameter< size_t > slice_number_;
  Core::ActionParameter< bool > erase_;

  Core::ActionCachedHandle< Core::MaskLayerHandle > target_layer_;
  Core::ActionCachedHandle< Core::MaskVolumeSliceHandle > vol_slice_;
};

ActionFloodFill::ActionFloodFill() :
  private_( new ActionFloodFillPrivate )
{
  this->add_argument( this->private_->target_layer_id_ );
  this->add_argument( this->private_->slice_type_ );
  this->add_argument( this->private_->slice_number_ );

  this->add_key( this->private_->erase_ );

  this->add_cachedhandle( this->private_->target_layer_ );
  this->add_cachedhandle( this->private_->vol_slice_ );
}

ActionFloodFill::~ActionFloodFill()
{
}

bool ActionFloodFill::validate( Core::ActionContextHandle& context )
{ 
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
  
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_.value() );
  Core::MaskVolumeSliceHandle volume_slice( new Core::MaskVolumeSlice(
    this->private_->target_layer_.handle()->get_mask_volume(), slice_type ) );
  if ( this->private_->slice_number_.value() >= volume_slice->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }

  volume_slice->set_slice_number( this->private_->slice_number_.value() );
  this->private_->vol_slice_.handle() = volume_slice;
    
  return true;
}

bool ActionFloodFill::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  Core::MaskVolumeSliceHandle volume_slice = this->private_->vol_slice_.handle();
  int nx = static_cast< int >( volume_slice->nx() );
  int ny = static_cast< int >( volume_slice->ny() );
  unsigned char mask_value = volume_slice->get_mask_data_block()->get_mask_value();

  {
    Core::MaskVolumeSlice::lock_type lock( volume_slice->get_mutex() );
    unsigned char* slice_cache = volume_slice->get_cached_data();
    unsigned char fill_value = mask_value;
    if ( this->private_->erase_.value() )
    {
      fill_value = 0;
    }
    Core::FloodFill( slice_cache, nx, ny, 0, 0, fill_value );
    Core::FloodFill( slice_cache, nx, ny, nx - 1, 0, fill_value );
    Core::FloodFill( slice_cache, nx, ny, nx - 1, ny - 1, fill_value );
    Core::FloodFill( slice_cache, nx, ny, 0, ny - 1, fill_value );
  }

  volume_slice->release_cached_data();

  return true;
}

void ActionFloodFill::Dispatch( Core::ActionContextHandle context, 
              const std::string& layer_id, int slice_type, size_t slice_number, bool erase )
{
  ActionFloodFill* action = new ActionFloodFill;
  action->private_->target_layer_id_.value() = layer_id;
  action->private_->slice_type_.value() = slice_type;
  action->private_->slice_number_.value() = slice_number;
  action->private_->erase_.value() = erase;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}


} // end namespace Seg3D