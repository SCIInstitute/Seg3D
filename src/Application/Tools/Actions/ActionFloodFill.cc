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

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <Core/Action/ActionFactory.h>
#include <Core/Volume/MaskVolumeSlice.h>
#include <Core/Volume/DataVolumeSlice.h>
#include <Core/Graphics/Algorithm.h>

#include <Application/Tools/Actions/ActionFloodFill.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerUndoBufferItem.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/ProjectManager/ProjectManager.h>


CORE_REGISTER_ACTION( Seg3D, FloodFill )

namespace Seg3D
{

class ActionFloodFillPrivate
{
public:
  std::string target_layer_id_;
  int slice_type_;
  size_t slice_number_;
  std::vector< Core::Point > seeds_;
  std::string data_cstr_layer_id_;
  double min_val_;
  double max_val_;
  bool negative_data_cstr_;
  std::string mask_cstr1_layer_id_;
  bool negative_mask_cstr1_;
  std::string mask_cstr2_layer_id_;
  bool negative_mask_cstr2_;
  bool erase_;
  SandboxID sandbox_;

  Core::MaskVolumeSliceHandle vol_slice_;
  Core::DataVolumeSliceHandle data_cstr_slice_;
  Core::MaskVolumeSliceHandle mask_cstr1_slice_;
  Core::MaskVolumeSliceHandle mask_cstr2_slice_;

  std::vector< std::pair< int, int > > seeds_2d_;
};

ActionFloodFill::ActionFloodFill() :
  private_( new ActionFloodFillPrivate )
{
  this->add_layer_id( this->private_->target_layer_id_ );
  this->add_parameter( this->private_->slice_type_ );
  this->add_parameter( this->private_->slice_number_ );
  this->add_parameter( this->private_->seeds_ );
  this->add_layer_id( this->private_->data_cstr_layer_id_ );
  this->add_parameter( this->private_->min_val_ );
  this->add_parameter( this->private_->max_val_ );
  this->add_parameter( this->private_->negative_data_cstr_ );
  this->add_layer_id( this->private_->mask_cstr1_layer_id_ );
  this->add_parameter( this->private_->negative_mask_cstr1_ );
  this->add_layer_id( this->private_->mask_cstr2_layer_id_ );
  this->add_parameter( this->private_->negative_mask_cstr2_ );
  this->add_parameter( this->private_->erase_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionFloodFill::validate( Core::ActionContextHandle& context )
{ 
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) )
  {
    return false;
  }

  // Check whether the target layer exists
  MaskLayerHandle target_layer = LayerManager::FindMaskLayer( 
    this->private_->target_layer_id_, this->private_->sandbox_ );
  if ( !target_layer )
  {
    context->report_error( "Layer '" + this->private_->target_layer_id_ +
      "' is not a valid mask layer." );
    return false;
  }

  // Check whether the target layer can be used for processing
  if ( !LayerManager::Instance()->CheckLayerAvailabilityForProcessing(
    this->private_->target_layer_id_, context, this->private_->sandbox_ ) ) return false;
  
  // Check whether slice type has a valid 
  Core::VolumeSliceType slice_type = static_cast< Core::VolumeSliceType::enum_type >(
    this->private_->slice_type_ );

  if ( slice_type != Core::VolumeSliceType::AXIAL_E &&
    slice_type != Core::VolumeSliceType::CORONAL_E &&
    slice_type != Core::VolumeSliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
  
  this->private_->vol_slice_.reset( new Core::MaskVolumeSlice( target_layer->get_mask_volume(), 
    slice_type ) );
  
  if ( this->private_->slice_number_ >= this->private_->vol_slice_->number_of_slices() )
  {
    context->report_error( "Slice number is out of range." );
    return false;
  }

  this->private_->vol_slice_->set_slice_number( this->private_->slice_number_ );

  if ( this->private_->data_cstr_layer_id_ != "" &&
    this->private_->data_cstr_layer_id_ != "<none>" )
  {
    DataLayerHandle data_cstr_layer = LayerManager::FindDataLayer( 
      this->private_->data_cstr_layer_id_, this->private_->sandbox_ );

    // NOTE: Compare layer grid transforms instead of their groups as in a sandbox
    // layers aren't grouped.
    if ( !data_cstr_layer || 
      data_cstr_layer->get_grid_transform() != target_layer->get_grid_transform() )
    {
      context->report_error( "Layer '" + this->private_->data_cstr_layer_id_ +
        "' is not a valid data constraint layer, will proceed as if no data constraint." );
    }
    else if ( !LayerManager::Instance()->CheckLayerAvailabilityForUse( 
      this->private_->data_cstr_layer_id_, context, this->private_->sandbox_ ) )
    {
      return false;
    }
    else
    {
      this->private_->data_cstr_slice_.reset( new Core::DataVolumeSlice( 
        data_cstr_layer->get_data_volume(), slice_type, 
        this->private_->slice_number_ ) );
    }
  }
  
  if ( this->private_->mask_cstr1_layer_id_ != "" &&
    this->private_->mask_cstr1_layer_id_ != "<none>" )
  {
    MaskLayerHandle mask_cstr1_layer = LayerManager::FindMaskLayer( 
      this->private_->mask_cstr1_layer_id_, this->private_->sandbox_ );

    // NOTE: Compare layer grid transforms instead of their groups as in a sandbox
    // layers aren't grouped.
    if ( !mask_cstr1_layer || 
      mask_cstr1_layer->get_grid_transform() != target_layer->get_grid_transform() )
    {
      context->report_error( "Layer '" + this->private_->mask_cstr1_layer_id_ +
        "' is not a valid mask constraint layer, will proceed as if no mask constraint 1." );
    }
    else if ( !LayerManager::Instance()->CheckLayerAvailabilityForUse( 
      this->private_->mask_cstr1_layer_id_, context, this->private_->sandbox_ ) )
    {
      return false;
    }
    else
    {
      this->private_->mask_cstr1_slice_.reset( new Core::MaskVolumeSlice(
        mask_cstr1_layer->get_mask_volume(), slice_type, 
        this->private_->slice_number_ ) );
    }
  }

  if ( this->private_->mask_cstr2_layer_id_ != "" &&
    this->private_->mask_cstr2_layer_id_ != "<none>" )
  {
    MaskLayerHandle mask_cstr2_layer = LayerManager::FindMaskLayer( 
        this->private_->mask_cstr2_layer_id_, this->private_->sandbox_ );

    // NOTE: Compare layer grid transforms instead of their groups as in a sandbox
    // layers aren't grouped.
    if ( !mask_cstr2_layer || 
      mask_cstr2_layer->get_grid_transform() != target_layer->get_grid_transform() )
    {
      context->report_error( "Layer '" + this->private_->mask_cstr2_layer_id_ +
        "' is not a valid mask constraint layer, will proceed as if no mask constraint 2." );
    }
    else if ( !LayerManager::Instance()->CheckLayerAvailabilityForUse( 
      this->private_->mask_cstr2_layer_id_, context, this->private_->sandbox_ ) )
    {
      return false;
    }
    else
    {
      this->private_->mask_cstr2_slice_.reset( new Core::MaskVolumeSlice(
        mask_cstr2_layer->get_mask_volume(), slice_type, 
        this->private_->slice_number_ ) );
    }
  }

  const std::vector< Core::Point >& seeds = this->private_->seeds_;
  int nx = static_cast< int >( this->private_->vol_slice_->nx() );
  int ny = static_cast< int >( this->private_->vol_slice_->ny() );
  this->private_->seeds_2d_.clear();

  if ( seeds.size() > 0 )
  {
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
      this->private_->min_val_, this->private_->max_val_, 
      this->private_->negative_data_cstr_ );
  }
  if ( this->private_->mask_cstr1_slice_ )
  {
    this->private_->mask_cstr1_slice_->copy_slice_data( mask_cstr1,
      this->private_->negative_mask_cstr1_ );
  }
  if ( this->private_->mask_cstr2_slice_ )
  {
    this->private_->mask_cstr2_slice_->copy_slice_data( mask_cstr2,
      this->private_->negative_mask_cstr2_ );
  }

  if ( this->private_->sandbox_ == -1 )
  {
    // Get the layer on which this action operates
    LayerHandle layer = LayerManager::FindLayer( this->private_->target_layer_id_ );

    // Create a provenance record
    ProvenanceStepHandle provenance_step( new ProvenanceStep );
    
    // Get the input provenance ids from the translate step
    provenance_step->set_input_provenance_ids( this->get_input_provenance_ids() );
    
    // Get the output and replace provenance ids from the analysis above
    provenance_step->set_output_provenance_ids(  this->get_output_provenance_ids( 1 )  );
    
    ProvenanceIDList deleted_provenance_ids( 1, layer->provenance_id_state_->get() );
    provenance_step->set_replaced_provenance_ids( deleted_provenance_ids );
  
    provenance_step->set_action_name( this->get_type() );
    provenance_step->set_action_params( this->export_params_to_provenance_string() );   
    
    ProvenanceStepID step_id = ProjectManager::Instance()->get_current_project()->
      add_provenance_record( provenance_step );

    // Build the undo/redo for this action
    LayerUndoBufferItemHandle item( new LayerUndoBufferItem( "FloodFill" ) );

    // Get the axis along which the flood fill works
    Core::SliceType slice_type = static_cast< Core::SliceType::enum_type>(
      this->private_->slice_type_ );
    
    // Get the slice number
    size_t slice_number = this->private_->slice_number_;
    
    // Create a check point of the slice on which the flood fill will operate
    LayerCheckPointHandle check_point( new LayerCheckPoint( layer, slice_type, slice_number ) );

    // The redo action is the current one
    item->set_redo_action( this->shared_from_this() );
    // Tell which provenance record to delete when undone
    item->set_provenance_step_id( step_id );
    // Tell the item which layer to restore with which check point for the undo action
    item->add_layer_to_restore( layer, check_point );

    // Now add the undo/redo action to undo buffer
    UndoBuffer::Instance()->insert_undo_item( context, item );

    // Set the output provenance id
    layer->provenance_id_state_->set( this->get_output_provenance_id( 0 ) );
  }

  {
    Core::MaskVolumeSlice::lock_type lock( volume_slice->get_mutex() );
    unsigned char* slice_cache = volume_slice->get_cached_data();
    unsigned char fill_value = mask_value;
    if ( this->private_->erase_ )
    {
      fill_value = 0;
    }

    // If there is no seed point specified, fill or erase the entire slice
    if ( this->private_->seeds_2d_.size() == 0 )
    {
      memset( slice_cache, fill_value, static_cast< size_t >( nx * ny ) );
    }
    // Else do a normal flood fill
    else
    {
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
  }

  volume_slice->release_cached_data();
  result.reset( new Core::ActionResult( this->private_->target_layer_id_ ) );
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
