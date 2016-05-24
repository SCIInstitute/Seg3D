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

#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/LayerGroup.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/Actions/ActionCrop.h>
#include <Application/Filters/LayerFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Crop )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class ActionCropPrivate
//////////////////////////////////////////////////////////////////////////

class ActionCropPrivate
{
public:
  std::vector< std::string > layer_ids_;
  Core::Point origin_;
  Core::Vector size_;
  bool replace_;
  SandboxID sandbox_;

  int start_x_;
  int start_y_;
  int start_z_;
  int end_x_;
  int end_y_;
  int end_z_;
  Core::GridTransform output_grid_trans_;
};

//////////////////////////////////////////////////////////////////////////
// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.
//////////////////////////////////////////////////////////////////////////

class CropAlgo : public LayerFilter
{

public:
  std::vector< LayerHandle > src_layers_;
  std::vector< LayerHandle > dst_layers_;
  bool replace_;
  size_t start_x_;
  size_t start_y_;
  size_t start_z_;
  size_t end_x_;
  size_t end_y_;
  size_t end_z_;

public:

  template< class T >
  void crop_typed_data( Core::DataBlockHandle src, Core::DataBlockHandle dst, 
    LayerHandle dst_layer );

  void crop_data_layer( DataLayerHandle input, DataLayerHandle output );
  void crop_mask_layer( MaskLayerHandle input, MaskLayerHandle output );

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run_filter();

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Crop Tool";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Crop";  
  }
};

template< class T >
void CropAlgo::crop_typed_data( Core::DataBlockHandle src, Core::DataBlockHandle dst,
                 LayerHandle dst_layer )
{
  T* src_data = reinterpret_cast< T* >( src->get_data() );
  T* dst_data = reinterpret_cast< T* >( dst->get_data() );
  size_t current_index = src->to_index( 0, 0, 0 );
  size_t stride_x = src->to_index( 1, 0, 0 ) - current_index;
  size_t stride_y = src->to_index( 0, 1, 0 ) - current_index;
  size_t stride_z = src->to_index( 0, 0, 1 ) - current_index;
  size_t current_z = src->to_index( this->start_x_, this->start_y_, this->start_z_ );
  size_t current_y;
  size_t dst_index = 0;
  size_t total_z_slice = this->end_z_ - this->start_z_ + 1;
  for ( size_t z = this->start_z_; z <= this->end_z_; ++z, current_z += stride_z )
  {
    current_y = current_z;
    for ( size_t y = this->start_y_; y <= this->end_y_; ++y, current_y += stride_y )
    {
      current_index = current_y;
      for ( size_t x = this->start_x_; x <= this->end_x_; ++x, current_index += stride_x )
      {
        dst_data[ dst_index++ ] = src_data[ current_index ];
      }
    }
    dst_layer->update_progress_signal_( ( z - this->start_z_ + 1.0 ) / total_z_slice * 0.8 );
  }
}

void CropAlgo::run_filter()
{
  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    switch ( this->src_layers_[ i ]->get_type() )
    {
    case Core::VolumeType::DATA_E:
      this->crop_data_layer(
        boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] ),
        boost::dynamic_pointer_cast< DataLayer >( this->dst_layers_[ i ] ) );
      break;
    case Core::VolumeType::MASK_E:
      this->crop_mask_layer(
        boost::dynamic_pointer_cast< MaskLayer >( this->src_layers_[ i ] ),
        boost::dynamic_pointer_cast< MaskLayer >( this->dst_layers_[ i ] ) );
      break;
    case Core::VolumeType::LARGE_DATA_E:
      if (this->replace_)
      {
        this->dispatch_delete_layer( this->src_layers_[ i ] );
      }
      else
      {
        this->dispatch_unlock_layer( this->src_layers_[ i ] );
      }
      break;
    }

    if ( this->check_abort() )
    {
      return;
    }
  }
}

void CropAlgo::crop_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  Core::DataBlockHandle input_datablock = input->get_data_volume()->get_data_block();
  Core::DataBlockHandle output_datablock = Core::StdDataBlock::New( 
    output->get_grid_transform(), input_datablock->get_data_type() );
  if ( !output_datablock ) 
  {
    this->report_error( "Could not allocate enough memory" );
    return;
  }
  
  Core::DataBlock::shared_lock_type data_lock( input_datablock->get_mutex() );
  switch ( input_datablock->get_data_type() )
  {
  case Core::DataType::CHAR_E:
    this->crop_typed_data< signed char >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::UCHAR_E:
    this->crop_typed_data< unsigned char >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::SHORT_E:
    this->crop_typed_data< short >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::USHORT_E:
    this->crop_typed_data< unsigned short >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::INT_E:
    this->crop_typed_data< int >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::UINT_E:
    this->crop_typed_data< unsigned int >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::FLOAT_E:
    this->crop_typed_data< float >( input_datablock, output_datablock, output );
    break;
  case Core::DataType::DOUBLE_E:
    this->crop_typed_data< double >( input_datablock, output_datablock, output );
    break;
  default:
    assert( false );
  }

  data_lock.unlock();

  if ( !this->check_abort() )
  {
    // Centering should be preserved for each layer
    Core::GridTransform output_grid_transform = output->get_grid_transform();
    output_grid_transform.set_originally_node_centered( 
      input->get_grid_transform().get_originally_node_centered() );

    this->dispatch_insert_data_volume_into_layer( output, Core::DataVolumeHandle(
      new Core::DataVolume( output_grid_transform, output_datablock ) ), 
      true );
    output->update_progress_signal_( 1.0 );
    this->dispatch_unlock_layer( output );
    if ( this->replace_ )
    {
      this->dispatch_delete_layer( input );
    }
    else
    {
      this->dispatch_unlock_layer( input );
    }
  }
}

void CropAlgo::crop_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
{
  Core::MaskDataBlockHandle input_mask = input->get_mask_volume()->get_mask_data_block();
  Core::DataBlockHandle output_mask = Core::StdDataBlock::New(
    output->get_grid_transform(), Core::DataType::UCHAR_E );
  if ( !output_mask ) 
  {
    this->report_error( "Could not allocate enough memory" );
    return;
  }
  
  Core::MaskDataBlock::shared_lock_type data_lock( input_mask->get_mutex() );
  const unsigned char* src_data = input_mask->get_mask_data();
  unsigned char* dst_data = reinterpret_cast< unsigned char* >( output_mask->get_data() );
  unsigned char mask_value = input_mask->get_mask_value();

  size_t current_index = input_mask->to_index( 0, 0, 0 );
  size_t stride_x = input_mask->to_index( 1, 0, 0 ) - current_index;
  size_t stride_y = input_mask->to_index( 0, 1, 0 ) - current_index;
  size_t stride_z = input_mask->to_index( 0, 0, 1 ) - current_index;
  size_t current_z = input_mask->to_index( this->start_x_, this->start_y_, this->start_z_ );
  size_t current_y;
  size_t dst_index = 0;
  size_t total_z_slice = this->end_z_ - this->start_z_ + 1;
  for ( size_t z = this->start_z_; z <= this->end_z_; ++z, current_z += stride_z )
  {
    current_y = current_z;
    for ( size_t y = this->start_y_; y <= this->end_y_; ++y, current_y += stride_y )
    {
      current_index = current_y;
      for ( size_t x = this->start_x_; x <= this->end_x_; ++x, current_index += stride_x )
      {
        dst_data[ dst_index++ ] = ( src_data[ current_index ] & mask_value );
      }
    }
    output->update_progress_signal_( ( z - this->start_z_ + 1.0 ) / total_z_slice * 0.8 );
  }

  data_lock.unlock();
  if ( !this->check_abort() )
  {
    Core::MaskDataBlockHandle dst_mask_data_block;
    if ( !Core::MaskDataBlockManager::Convert( output_mask, output->get_grid_transform(),
      dst_mask_data_block ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume(
      output->get_grid_transform(), dst_mask_data_block ) );

    this->dispatch_insert_mask_volume_into_layer( output, mask_volume );
    output->update_progress_signal_( 1.0 );
    this->dispatch_unlock_layer( output );
    if ( this->replace_ )
    {
      this->dispatch_delete_layer( input );
    }
    else
    {
      this->dispatch_unlock_layer( input );
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Class ActionCrop
//////////////////////////////////////////////////////////////////////////

ActionCrop::ActionCrop() :
  private_( new ActionCropPrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->origin_ );
  this->add_parameter( this->private_->size_ );
  this->add_parameter( this->private_->replace_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionCrop::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) ) return false;

  if ( this->private_->layer_ids_.size() == 0 )
  {
    context->report_error( "No input layers specified" );
    return false;
  }
  
  Core::GridTransform grid_trans;
  for ( size_t i = 0; i < this->private_->layer_ids_.size(); ++i )
  {
    // Check for layer existence
    LayerHandle layer = LayerManager::FindLayer( this->private_->layer_ids_[ i ],
      this->private_->sandbox_ );
    if ( !layer )
    {
      context->report_error( "Layer '" + this->private_->layer_ids_[ i ] + "' doesn't exist" );
      return false;
    }

    // Make sure that all the layers are in the same group
    if ( i == 0 )
    {
      grid_trans = layer->get_grid_transform();
    }
    else if ( grid_trans != layer->get_grid_transform() )
    {
      context->report_error( "Input layers do not belong to the same group" );
      return false;
    }
    
    // Check for layer availability 
    if ( !LayerManager::CheckLayerAvailability( this->private_->layer_ids_[ i ], 
      this->private_->replace_, context, this->private_->sandbox_ ) ) return false;
  }
  
  const Core::Point& origin = this->private_->origin_;
  const Core::Vector& size = this->private_->size_;

  if ( size[ 0 ] < 0 || size[ 1 ] < 0 || size[ 2 ] < 0 )
  {
    context->report_error( "Crop size can not be negative" );
    return false;
  }

  // Convert the crop box to index space and clamp to layer boundary
  Core::Point end = origin + size;
  int nx = static_cast< int >( grid_trans.get_nx() );
  int ny = static_cast< int >( grid_trans.get_ny() );
  int nz = static_cast< int >( grid_trans.get_nz() );
  Core::Matrix trans = grid_trans.transform().get_matrix();
  Core::Matrix inverse_trans;
  Core::Matrix::Invert( trans, inverse_trans );
  Core::Point origin_index = inverse_trans * origin;
  Core::Point end_index = inverse_trans * end;
  this->private_->start_x_ = Core::Max( Core::Round( origin_index[ 0 ] ), 0 );
  this->private_->start_y_ = Core::Max( Core::Round( origin_index[ 1 ] ), 0 );
  this->private_->start_z_ = Core::Max( Core::Round( origin_index[ 2 ] ), 0 );
  this->private_->end_x_ = Core::Min( Core::Round( end_index[ 0 ] ), nx - 1 );
  this->private_->end_y_ = Core::Min( Core::Round( end_index[ 1 ] ), ny - 1 );
  this->private_->end_z_ = Core::Min( Core::Round( end_index[ 2 ] ), nz - 1 );

  if ( this->private_->start_x_ >= nx ||
    this->private_->start_y_ >= ny ||
    this->private_->start_z_ >= nz ||
    this->private_->end_x_ < 0 ||
    this->private_->end_y_ < 0 ||
    this->private_->end_z_ < 0 )
  {
    context->report_error( "Crop box doesn't overlap the layers" );
    return false;
  }

  // Compute the cropped grid transform 
  Core::Point clamped_origin( this->private_->start_x_, 
    this->private_->start_y_, this->private_->start_z_ );
  clamped_origin = trans * clamped_origin;
  trans( 0, 3 ) = clamped_origin[ 0 ];
  trans( 1, 3 ) = clamped_origin[ 1 ];
  trans( 2, 3 ) = clamped_origin[ 2 ];
  this->private_->output_grid_trans_.load_matrix( trans );
  this->private_->output_grid_trans_.set_nx( static_cast< size_t >( 
    this->private_->end_x_ - this->private_->start_x_ + 1 ) );
  this->private_->output_grid_trans_.set_ny( static_cast< size_t >( 
    this->private_->end_y_ - this->private_->start_y_ + 1 ) );
  this->private_->output_grid_trans_.set_nz( static_cast< size_t >( 
    this->private_->end_z_ - this->private_->start_z_ + 1 ) );
  this->private_->output_grid_trans_.set_originally_node_centered(
    grid_trans.get_originally_node_centered() );

  // Validation successful
  return true;
}

bool ActionCrop::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< CropAlgo > algo( new CropAlgo );

  // Set up parameters
  algo->set_sandbox( this->private_->sandbox_ );
  algo->replace_ = this->private_->replace_;
  algo->start_x_ = this->private_->start_x_;
  algo->start_y_ = this->private_->start_y_;
  algo->start_z_ = this->private_->start_z_;
  algo->end_x_ = this->private_->end_x_;
  algo->end_y_ = this->private_->end_y_;
  algo->end_z_ = this->private_->end_z_;

  // Set up input and output layers
  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;
  size_t num_of_layers = layer_ids.size();
  algo->src_layers_.resize( num_of_layers );
  algo->dst_layers_.resize( num_of_layers );
  std::vector< std::string > dst_layer_ids( num_of_layers );
  for ( size_t i = 0; i < num_of_layers; ++i )
  {
    algo->find_layer( layer_ids[ i ], algo->src_layers_[ i ] );
    if ( algo->replace_ )
    {
      algo->lock_for_deletion( algo->src_layers_[ i ] );
    }
    else
    {
      algo->lock_for_use( algo->src_layers_[ i ] );
    }

    switch ( algo->src_layers_[ i ]->get_type() )
    {
    case Core::VolumeType::DATA_E:
      algo->create_and_lock_data_layer( this->private_->output_grid_trans_, 
        algo->src_layers_[ i ], algo->dst_layers_[ i ] );
      break;
    case Core::VolumeType::MASK_E:
      algo->create_and_lock_mask_layer( this->private_->output_grid_trans_,
        algo->src_layers_[ i ], algo->dst_layers_[ i ] );
      static_cast< MaskLayer* >( algo->dst_layers_[ i ].get() )->color_state_->set(
        static_cast< MaskLayer* >( algo->src_layers_[ i ].get() )->color_state_->get() );
      break;
    case Core::VolumeType::LARGE_DATA_E:
      algo->create_cropped_large_volume_layer( this->private_->output_grid_trans_,
        algo->src_layers_[ i ], algo->dst_layers_[ i ] );
      break;
    default:
      assert( false );
    }
    dst_layer_ids[ i ] = algo->dst_layers_[ i ]->get_layer_id();
  }
  
  // Return the ids of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( dst_layer_ids ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this(), true );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionCrop::Dispatch( Core::ActionContextHandle context, 
                const std::vector< std::string >& layer_ids, 
                const Core::Point& origin, 
                const Core::Vector& size, bool replace )
{
  ActionCrop* action = new ActionCrop;
  action->private_->layer_ids_ = layer_ids;
  action->private_->origin_ = origin;
  action->private_->size_ = size;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
