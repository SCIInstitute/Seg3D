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

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <Core/Action/ActionFactory.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Graphics/Algorithm.h>

#include <Application/Tools/Actions/ActionFloodFill.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/LayerManager/LayerUndoBufferItem.h>

CORE_REGISTER_ACTION( Seg3D, FloodFill )

namespace Seg3D
{

class ActionFloodFillPrivate
{
public:
  Core::ActionParameter< std::string > target_layer_id_;
  Core::ActionParameter< int > slice_type_;
  Core::ActionParameter< size_t > slice_number_;
  Core::ActionParameter< std::vector< Core::Point > > seeds_;
  Core::ActionParameter< std::string > data_cstr_layer_id_;
  Core::ActionParameter< double > min_val_;
  Core::ActionParameter< double > max_val_;
  Core::ActionParameter< bool > negative_data_cstr_;
  Core::ActionParameter< std::string > mask_cstr1_layer_id_;
  Core::ActionParameter< bool > negative_mask_cstr1_;
  Core::ActionParameter< std::string > mask_cstr2_layer_id_;
  Core::ActionParameter< bool > negative_mask_cstr2_;
  Core::ActionParameter< bool > erase_;

  Core::MaskVolumeSliceHandle vol_slice_;
  Core::DataVolumeSliceHandle data_cstr_slice_;
  Core::MaskVolumeSliceHandle mask_cstr1_slice_;
  Core::MaskVolumeSliceHandle mask_cstr2_slice_;

  std::vector< std::pair< int, int > > seeds_2d_;
};

ActionFloodFill::ActionFloodFill() :
  private_( new ActionFloodFillPrivate )
{
  this->add_argument( this->private_->target_layer_id_ );
  this->add_argument( this->private_->slice_type_ );
  this->add_argument( this->private_->slice_number_ );
  this->add_argument( this->private_->seeds_ );

  this->add_key( this->private_->data_cstr_layer_id_ );
  this->add_key( this->private_->min_val_ );
  this->add_key( this->private_->max_val_ );
  this->add_key( this->private_->negative_data_cstr_ );
  this->add_key( this->private_->mask_cstr1_layer_id_ );
  this->add_key( this->private_->negative_mask_cstr1_ );
  this->add_key( this->private_->mask_cstr2_layer_id_ );
  this->add_key( this->private_->negative_mask_cstr2_ );
  this->add_key( this->private_->erase_ );
}

ActionFloodFill::~ActionFloodFill()
{
}

bool ActionFloodFill::validate( Core::ActionContextHandle& context )
{ 
  // Check whether the target layer exists
  MaskLayerHandle target_layer = boost::dynamic_pointer_cast< MaskLayer >( 
    LayerManager::Instance()->get_layer_by_id( this->private_->target_layer_id_.value() ) );
  if ( !target_layer )
  {
    context->report_error( "Layer '" + this->private_->target_layer_id_.value() +
      "' is not a valid mask layer." );
    return false;
  }

  if ( target_layer->locked_state_->get() )
  {
    context->report_error( "Layer '" + this->private_->target_layer_id_.value() +
      "' is locked." );
    return false;
  }
  
  // Check whether the target layer can be used for processing
  Core::NotifierHandle notifier;
  if ( !LayerManager::Instance()->CheckLayerAvailabilityForProcessing(
    this->private_->target_layer_id_.value(), notifier ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  // Check whether slice type has a valid 
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_.value() );

  if ( slice_type != Core::VolumeSliceType::AXIAL_E &&
    slice_type != Core::VolumeSliceType::CORONAL_E &&
    slice_type != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  

  this->private_->vol_slice_.reset( new Core::MaskVolumeSlice( target_layer->get_mask_volume(), 
    slice_type ) );
  
  if ( this->private_->slice_number_.value() >= this->private_->vol_slice_->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }

  this->private_->vol_slice_->set_slice_number( this->private_->slice_number_.value() );

  if ( this->private_->data_cstr_layer_id_.value() != "" &&
    this->private_->data_cstr_layer_id_.value() != "<none>" )
  {
    DataLayerHandle data_cstr_layer = boost::dynamic_pointer_cast< DataLayer >(
      LayerManager::Instance()->get_layer_by_id( 
      this->private_->data_cstr_layer_id_.value() ) );

    if ( !data_cstr_layer || 
      data_cstr_layer->get_layer_group() != target_layer->get_layer_group() )
    {
      context->report_error( "Layer '" + this->private_->data_cstr_layer_id_.value() +
        "' is not a valid data constraint layer, will proceed as if no data constraint." );
    }
    else if ( !LayerManager::Instance()->CheckLayerAvailabilityForUse( 
      this->private_->data_cstr_layer_id_.value(), notifier ) )
    {
      context->report_error( "Data layer '" + this->private_->data_cstr_layer_id_.value() +
        "' not available for reading, will proceed as if no data constraint." );
    }
    else
    {
      this->private_->data_cstr_slice_.reset( new Core::DataVolumeSlice( 
        data_cstr_layer->get_data_volume(), slice_type, 
        this->private_->slice_number_.value() ) );
    }
  }
  
  if ( this->private_->mask_cstr1_layer_id_.value() != "" &&
    this->private_->mask_cstr1_layer_id_.value() != "<none>" )
  {
    MaskLayerHandle mask_cstr1_layer = boost::dynamic_pointer_cast< MaskLayer >(
      LayerManager::Instance()->get_layer_by_id( 
      this->private_->mask_cstr1_layer_id_.value() ) );
    if ( !mask_cstr1_layer || 
      mask_cstr1_layer->get_layer_group() != target_layer->get_layer_group() )
    {
      context->report_error( "Layer '" + this->private_->mask_cstr1_layer_id_.value() +
        "' is not a valid mask constraint layer, will proceed as if no mask constraint 1." );
    }
    else if ( !LayerManager::Instance()->CheckLayerAvailabilityForUse( 
      this->private_->mask_cstr1_layer_id_.value(), notifier ) )
    {
      context->report_error( "Mask layer '" + this->private_->mask_cstr1_layer_id_.value() +
        "' not available for reading, will proceed as if no mask constraint 1." );
    }
    else
    {
      this->private_->mask_cstr1_slice_.reset( new Core::MaskVolumeSlice(
        mask_cstr1_layer->get_mask_volume(), slice_type, 
        this->private_->slice_number_.value() ) );
    }
  }

  if ( this->private_->mask_cstr2_layer_id_.value() != "" &&
    this->private_->mask_cstr2_layer_id_.value() != "<none>" )
  {
    MaskLayerHandle mask_cstr2_layer = boost::dynamic_pointer_cast< MaskLayer >(
      LayerManager::Instance()->get_layer_by_id( 
        this->private_->mask_cstr2_layer_id_.value() ) );
    if ( !mask_cstr2_layer || 
      mask_cstr2_layer->get_layer_group() != target_layer->get_layer_group() )
    {
      context->report_error( "Layer '" + this->private_->mask_cstr2_layer_id_.value() +
        "' is not a valid mask constraint layer, will proceed as if no mask constraint 2." );
    }
    else if ( !LayerManager::Instance()->CheckLayerAvailabilityForUse( 
      this->private_->mask_cstr2_layer_id_.value(), notifier ) )
    {
      context->report_error( "Mask layer '" + this->private_->mask_cstr2_layer_id_.value() +
        "' not available for reading, will proceed as if no mask constraint 2." );
    }
    else
    {
      this->private_->mask_cstr2_slice_.reset( new Core::MaskVolumeSlice(
        mask_cstr2_layer->get_mask_volume(), slice_type, 
        this->private_->slice_number_.value() ) );
    }
  }

  const std::vector< Core::Point >& seeds = this->private_->seeds_.value();
  if ( seeds.size() == 0 )
  {
    context->report_error( "No seed points provided." );
    return false;
  }
  
  int nx = static_cast< int >( this->private_->vol_slice_->nx() );
  int ny = static_cast< int >( this->private_->vol_slice_->ny() );
  this->private_->seeds_2d_.clear();
  for ( size_t i = 0; i < seeds.size(); ++i )
  {
    double world_x, world_y;
    this->private_->vol_slice_->project_onto_slice( seeds[ i ], world_x, world_y );
    int x, y;
    this->private_->vol_slice_->world_to_index( world_x, world_y, x, y );
    if ( x >= 0 && x < nx && y >= 0 && y < ny )
    {
      this->private_->seeds_2d_.push_back( std::make_pair( x, y ) );
    }
  }
  
  if ( this->private_->seeds_2d_.size() == 0 )
  {
    context->report_error( "All seed points are outside the boundary of the slice." );
    return false;
  }

  return true;
}

bool ActionFloodFill::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  Core::MaskVolumeSliceHandle volume_slice = this->private_->vol_slice_;
  int nx = static_cast< int >( volume_slice->nx() );
  int ny = static_cast< int >( volume_slice->ny() );
  unsigned char mask_value = volume_slice->get_mask_data_block()->get_mask_value();
  
  typedef std::vector< unsigned char > cstr_buffer_type;
  std::vector< unsigned char > data_cstr( nx * ny, 1 );
  std::vector< unsigned char > mask_cstr1( nx * ny, 1 );
  std::vector< unsigned char > mask_cstr2( nx * ny, 1 );
  if ( this->private_->data_cstr_slice_ )
  {
    this->private_->data_cstr_slice_->create_threshold_mask( data_cstr,
      this->private_->min_val_.value(), this->private_->max_val_.value(), 
      this->private_->negative_data_cstr_.value() );
  }
  if ( this->private_->mask_cstr1_slice_ )
  {
    this->private_->mask_cstr1_slice_->copy_slice_data( mask_cstr1,
      this->private_->negative_mask_cstr1_.value() );
  }
  if ( this->private_->mask_cstr2_slice_ )
  {
    this->private_->mask_cstr2_slice_->copy_slice_data( mask_cstr2,
      this->private_->negative_mask_cstr2_.value() );
  }

  {
    // Build the undo/redo for this action
    LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "FloodFill" ) );

    // Get the axis along which the flood fill works
    Core::SliceType slice_type = static_cast< Core::SliceType::enum_type>(
      this->private_->slice_type_.value() );
    
    // Get the slice number
    size_t slice_number = this->private_->slice_number_.value();
    
    // Get the layer on which this action operates
    LayerHandle layer = LayerManager::Instance()->get_layer_by_id( 
      this->private_->target_layer_id_.value() );
    // Create a check point of the slice on which the flood fill will operate
    LayerCheckPointHandle check_point( new LayerCheckPoint( layer, slice_type, slice_number ) );

    // The redo action is the current one
    item->set_redo_action( this->shared_from_this() );
    // Tell the item which layer to restore with which check point for the undo action
    item->add_layer_to_restore( layer, check_point );

    // Now add the undo/redo action to undo buffer
    UndoBuffer::Instance()->insert_undo_item( context, item );
  }

  {
    Core::MaskVolumeSlice::lock_type lock( volume_slice->get_mutex() );
    unsigned char* slice_cache = volume_slice->get_cached_data();
    unsigned char fill_value = mask_value;
    if ( this->private_->erase_.value() )
    {
      fill_value = 0;
    }

    for ( size_t i = 0; i < this->private_->seeds_2d_.size(); ++i )
    {
      if ( !this->private_->data_cstr_slice_ &&
        !this->private_->mask_cstr1_slice_ &&
        !this->private_->mask_cstr2_slice_ )
      {
        Core::FloodFill( slice_cache, nx, ny, this->private_->seeds_2d_[ i ].first,
          this->private_->seeds_2d_[ i ].second, fill_value );
      }
      else
      {
        Core::FloodFill( slice_cache, nx, ny, this->private_->seeds_2d_[ i ].first, 
          this->private_->seeds_2d_[ i ].second, fill_value,
          ( *( boost::lambda::constant( &data_cstr[ 0 ] ) + 
          boost::lambda::_2 * nx + boost::lambda::_1 ) != 0 &&
          *( boost::lambda::constant( &mask_cstr1[ 0 ] ) + 
          boost::lambda::_2 * nx + boost::lambda::_1 ) != 0 &&
          *( boost::lambda::constant( &mask_cstr2[ 0 ] ) + 
          boost::lambda::_2 * nx + boost::lambda::_1 ) != 0 ) );
      }
    }
  }

  volume_slice->release_cached_data();

  return true;
}

void ActionFloodFill::clear_cache()
{
  this->private_->data_cstr_slice_.reset();
  this->private_->mask_cstr1_slice_.reset();
  this->private_->mask_cstr2_slice_.reset();
  this->private_->vol_slice_.reset();
  this->private_->seeds_2d_.clear();
}

void ActionFloodFill::Dispatch( Core::ActionContextHandle context, 
              const FloodFillInfo& params )
{
  ActionFloodFill* action = new ActionFloodFill;
  action->private_->target_layer_id_ = params.target_layer_id_;
  action->private_->slice_type_ = params.slice_type_;
  action->private_->slice_number_ = params.slice_number_;
  action->private_->seeds_ = params.seeds_;
  action->private_->data_cstr_layer_id_ = params.data_constraint_layer_id_;
  action->private_->min_val_ = params.min_val_;
  action->private_->max_val_ = params.max_val_;
  action->private_->negative_data_cstr_ = params.negative_data_constraint_;
  action->private_->mask_cstr1_layer_id_ = params.mask_constraint1_layer_id_;
  action->private_->negative_mask_cstr1_ = params.negative_mask_constraint1_;
  action->private_->mask_cstr2_layer_id_ = params.mask_constraint2_layer_id_;
  action->private_->negative_mask_cstr2_ = params.negative_mask_constraint2_;
  action->private_->erase_ = params.erase_;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}


} // end namespace Seg3D