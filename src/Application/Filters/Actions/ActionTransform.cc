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
#include <Application/Filters/Actions/ActionTransform.h>
#include <Application/Filters/LayerFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Transform )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class ActionTransformPrivate
//////////////////////////////////////////////////////////////////////////

class ActionTransformPrivate
{
public:
  std::vector< std::string > layer_ids_;
  Core::Point origin_;
  Core::Vector spacing_;
  bool replace_;
  SandboxID sandbox_;

  Core::GridTransform output_grid_trans_;
};

//////////////////////////////////////////////////////////////////////////
// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.
//////////////////////////////////////////////////////////////////////////

class TransformAlgo : public LayerFilter
{

public:
  std::vector< LayerHandle > src_layers_;
  std::vector< LayerHandle > dst_layers_;

  bool replace_;

public:

  void transform_data_layer( DataLayerHandle input, DataLayerHandle output );
  void transform_mask_layer( MaskLayerHandle input, MaskLayerHandle output );

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run_filter();

  // GET_FITLER_NAME:
  // This functions returns the name of the filter that is used in the error report.
  virtual std::string get_filter_name() const
  {
    return "Transform Tool";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Transform";
  }

};

void TransformAlgo::run_filter()
{
  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    switch ( this->src_layers_[ i ]->get_type() )
    {
    case Core::VolumeType::DATA_E:
      this->transform_data_layer(
        boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] ),
        boost::dynamic_pointer_cast< DataLayer >( this->dst_layers_[ i ] ) );
      break;
    case Core::VolumeType::MASK_E:
      this->transform_mask_layer(
        boost::dynamic_pointer_cast< MaskLayer >( this->src_layers_[ i ] ),
        boost::dynamic_pointer_cast< MaskLayer >( this->dst_layers_[ i ] ) );
      break;
    }

    if ( this->check_abort() )
    {
      return;
    }
  }
}

void TransformAlgo::transform_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  Core::DataBlockHandle input_datablock = input->get_data_volume()->get_data_block();
  Core::DataBlockHandle output_datablock = Core::StdDataBlock::New( 
    output->get_grid_transform(), input_datablock->get_data_type() );

  const void* src_data = input_datablock->get_data();
  void* dst_data = output_datablock->get_data();

  size_t data_size = 0;
  Core::DataBlock::shared_lock_type data_lock( input_datablock->get_mutex() );
  switch ( input_datablock->get_data_type() )
  {
  case Core::DataType::CHAR_E:
  case Core::DataType::UCHAR_E:
    data_size = sizeof( char );
    break;
  case Core::DataType::SHORT_E:
  case Core::DataType::USHORT_E:
    data_size = sizeof( short );
    break;
  case Core::DataType::INT_E:
  case Core::DataType::UINT_E:
    data_size = sizeof( int );
    break;
  case Core::DataType::FLOAT_E:
    data_size = sizeof( float );
    break;
  case Core::DataType::DOUBLE_E:
    data_size = sizeof( double );
    break;
  default:
    assert( false );
  }

  memcpy( dst_data, src_data, input_datablock->get_size() * data_size );

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

void TransformAlgo::transform_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
{
  Core::MaskDataBlockHandle input_mask = input->get_mask_volume()->get_mask_data_block();
  Core::DataBlockHandle output_datablock = Core::StdDataBlock::New(
    output->get_grid_transform(), Core::DataType::UCHAR_E );

  const unsigned char* src_data = input_mask->get_mask_data();
  unsigned char src_mask_value = input_mask->get_mask_value();
  unsigned char* dst_data = reinterpret_cast< unsigned char* >( output_datablock->get_data() );

  {
    Core::MaskDataBlock::shared_lock_type data_lock( input_mask->get_mutex() );
    size_t total_voxels = input_mask->get_size();
    for ( size_t i = 0; i < total_voxels; ++i )
    {
      dst_data[ i ] = src_data[ i ] & src_mask_value;
    }
  }

  if ( !this->check_abort() )
  {
    Core::MaskDataBlockHandle dst_mask_data_block;
    Core::MaskDataBlockManager::Convert( output_datablock, output->get_grid_transform(),
      dst_mask_data_block );
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
// Class ActionTransform
//////////////////////////////////////////////////////////////////////////

ActionTransform::ActionTransform() :
  private_( new ActionTransformPrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->origin_ );
  this->add_parameter( this->private_->spacing_ );
  this->add_parameter( this->private_->replace_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionTransform::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->private_->sandbox_, context ) ) return false;

  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;
  if ( layer_ids.size() == 0 )
  {
    context->report_error( "No input layers specified" );
    return false;
  }
  
  Core::GridTransform src_grid_trans;
  for ( size_t i = 0; i < layer_ids.size(); ++i )
  {
    // Check for layer existence
    LayerHandle layer = LayerManager::FindLayer( layer_ids[ i ], this->private_->sandbox_ );
    if ( !layer )
    {
      context->report_error( "Layer '" + layer_ids[ i ] + "' doesn't exist" );
      return false;
    }

    // Make sure that all the layers are in the same group
    if ( i == 0 )
    {
      src_grid_trans = layer->get_grid_transform();
    }
    else if ( src_grid_trans != layer->get_grid_transform() )
    {
      context->report_error( "Input layers do not belong to the same group" );
      return false;
    }
    
    // Check for layer availability 
    if ( !LayerManager::CheckLayerAvailability( layer_ids[ i ], 
      this->private_->replace_, context, this->private_->sandbox_ ) ) return false;
  }
  
  const Core::Vector& spacing = this->private_->spacing_;
  if ( spacing[ 0 ] <= 0 || spacing[ 1 ] <= 0 || spacing[ 2 ] <= 0 )
  {
    context->report_error( "Spacing must be greater than 0" );
    return false;
  }

  // Compute the output grid transform
  this->private_->output_grid_trans_.set_nx( src_grid_trans.get_nx() );
  this->private_->output_grid_trans_.set_ny( src_grid_trans.get_ny() );
  this->private_->output_grid_trans_.set_nz( src_grid_trans.get_nz() );
  this->private_->output_grid_trans_.load_basis( this->private_->origin_, 
    Core::Vector( spacing[ 0 ], 0, 0 ), Core::Vector( 0, spacing[ 1 ], 0 ), 
    Core::Vector( 0, 0, spacing[ 2 ] ) );
  
  // Validation successful
  return true;
}

bool ActionTransform::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< TransformAlgo > algo( new TransformAlgo );

  // Set up parameters
  algo->set_sandbox( this->private_->sandbox_ );
  algo->replace_ = this->private_->replace_;

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

void ActionTransform::Dispatch( Core::ActionContextHandle context, 
                const std::vector< std::string >& layer_ids, 
                const Core::Point& origin, const Core::Vector& spacing,
                bool replace )
{
  ActionTransform* action = new ActionTransform;
  action->private_->layer_ids_ = layer_ids;
  action->private_->origin_ = origin;
  action->private_->spacing_ = spacing;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
