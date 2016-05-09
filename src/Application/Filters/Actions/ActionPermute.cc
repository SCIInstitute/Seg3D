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
#include <Application/Filters/Actions/ActionPermute.h>
#include <Application/Filters/LayerFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Permute )

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class ActionPermutePrivate
//////////////////////////////////////////////////////////////////////////

class ActionPermutePrivate
{
public:
  std::vector< std::string > layer_ids_;
  std::vector< int > permutation_;
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

class PermuteAlgo : public LayerFilter
{

public:
  std::vector< LayerHandle > src_layers_;
  std::vector< LayerHandle > dst_layers_;
  std::vector< int > permutation_;
  bool replace_;

public:

  void permute_data_layer( DataLayerHandle input, DataLayerHandle output );
  void permute_mask_layer( MaskLayerHandle input, MaskLayerHandle output );

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run_filter();

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Flip/Rotate Filter";
  }
  
  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "FlipRotate";  
  }
};

void PermuteAlgo::run_filter()
{
  for ( size_t i = 0; i < this->src_layers_.size(); ++i )
  {
    switch ( this->src_layers_[ i ]->get_type() )
    {
    case Core::VolumeType::DATA_E:
      this->permute_data_layer(
        boost::dynamic_pointer_cast< DataLayer >( this->src_layers_[ i ] ),
        boost::dynamic_pointer_cast< DataLayer >( this->dst_layers_[ i ] ) );
      break;
    case Core::VolumeType::MASK_E:
      this->permute_mask_layer(
        boost::dynamic_pointer_cast< MaskLayer >( this->src_layers_[ i ] ),
        boost::dynamic_pointer_cast< MaskLayer >( this->dst_layers_[ i ] ) );
      break;
    }

    if ( ! this->dst_layers_[ i ] )
    {
      this->report_error( "Could not alocate enough memory." );
      return;
    }
  
    if ( this->check_abort() )
    {
      return;
    }
  }
}

void PermuteAlgo::permute_data_layer( DataLayerHandle input, DataLayerHandle output )
{
  Core::DataBlockHandle input_datablock = input->get_data_volume()->get_data_block();

  Core::DataBlockHandle output_datablock;
  bool succeeded = Core::DataBlock::PermuteData( input_datablock, 
    output_datablock, this->permutation_ );

  if ( succeeded && !this->check_abort() )
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

void PermuteAlgo::permute_mask_layer( MaskLayerHandle input, MaskLayerHandle output )
{
  Core::MaskDataBlockHandle input_mask = input->get_mask_volume()->get_mask_data_block();
  Core::DataBlockHandle input_data_block;
  Core::MaskDataBlockManager::Convert( input_mask, input_data_block, 
    Core::DataType::UCHAR_E );

  Core::DataBlockHandle output_datablock;
  bool succeeded = Core::DataBlock::PermuteData( input_data_block, 
    output_datablock, this->permutation_ );

  if ( succeeded && !this->check_abort() )
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
// Class ActionPermute
//////////////////////////////////////////////////////////////////////////

ActionPermute::ActionPermute() :
  private_( new ActionPermutePrivate )
{
  // Action arguments
  this->add_layer_id_list( this->private_->layer_ids_ );
  this->add_parameter( this->private_->permutation_ );
  this->add_parameter( this->private_->replace_ );
  this->add_parameter( this->private_->sandbox_ );
}

bool ActionPermute::validate( Core::ActionContextHandle& context )
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
  
  const std::vector< int >& permutation = this->private_->permutation_;
  bool found_x = false, found_y = false, found_z = false;
  if ( permutation.size() == 3 )
  {
    for ( int i = 0; i < 3; ++i )
    {
      if ( permutation[ i ] == 1 || permutation[ i ] == -1 )
      {
        found_x = true;
      }
      else if ( permutation[ i ] == 2 || permutation[ i ] == -2 )
      {
        found_y = true;
      }
      else if ( permutation[ i ] == 3 || permutation[ i ] == -3 )
      {
        found_z = true;
      }
    }
  }
  
  if ( !found_x || !found_y || !found_z )
  {
    context->report_error( "Invalid permutation array" );
    return false;
  }

  // Compute the output grid transform
  std::vector< size_t > src_size( 3 );
  src_size[ 0 ] = src_grid_trans.get_nx();
  src_size[ 1 ] = src_grid_trans.get_ny();
  src_size[ 2 ] = src_grid_trans.get_nz();
  Core::Point src_origin = src_grid_trans * Core::Point( 0, 0, 0 );
  Core::Point src_max = src_grid_trans * Core::Point( static_cast< double >( src_size[ 0 ] - 1 ), 
    static_cast< double >( src_size[ 1 ] - 1 ), static_cast< double >( src_size[ 2 ] - 1 ) );
  Core::Point src_center( ( src_origin + src_max ) * 0.5 );
  Core::Vector src_extend = src_max - src_origin;
  Core::Vector src_spacing = src_grid_trans * Core::Vector( 1, 1, 1 );
  Core::Point dst_orgin;
  Core::Vector dst_spacing;
  std::vector< size_t > dst_size( 3 );
  for ( int i = 0; i < 3; ++i )
  {
    int original_index = Core::Abs( permutation[ i ] ) - 1;
    dst_orgin[ i ] = src_center[ i ] - src_extend[ original_index ] * 0.5;
    dst_spacing[ i ] = src_spacing[ original_index ];
    dst_size[ i ] = src_size[ original_index ];
  }

  this->private_->output_grid_trans_.set_nx( dst_size[ 0 ] );
  this->private_->output_grid_trans_.set_ny( dst_size[ 1 ] );
  this->private_->output_grid_trans_.set_nz( dst_size[ 2 ] );
  this->private_->output_grid_trans_.load_basis( dst_orgin, Core::Vector( dst_spacing[ 0 ], 0, 0 ),
    Core::Vector( 0, dst_spacing[ 1 ], 0 ), Core::Vector( 0, 0, dst_spacing[ 2 ] ) );

  // Validation successful
  return true;
}

bool ActionPermute::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< PermuteAlgo > algo( new PermuteAlgo );

  // Set up parameters
  algo->set_sandbox( this->private_->sandbox_ );
  algo->permutation_ = this->private_->permutation_;
  algo->replace_ = this->private_->replace_;

  // Set up input and output layers
  const std::vector< std::string >& layer_ids = this->private_->layer_ids_;
  size_t num_of_layers = layer_ids.size();
  algo->src_layers_.resize( num_of_layers );
  algo->dst_layers_.resize( num_of_layers );
  std::vector< std::string > dst_layer_ids( num_of_layers );
  
  for ( size_t i = 0; i < num_of_layers; ++i )
  {
    if ( !( algo->find_layer( layer_ids[ i ], algo->src_layers_[ i ] ) ) )
    {
      return false;
    }
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
      if ( !( algo->create_and_lock_data_layer( this->private_->output_grid_trans_, 
        algo->src_layers_[ i ], algo->dst_layers_[ i ] ) ) )
      {
        return false;
      }
      break;
    case Core::VolumeType::MASK_E:
      if ( !( algo->create_and_lock_mask_layer( this->private_->output_grid_trans_,
        algo->src_layers_[ i ], algo->dst_layers_[ i ] ) ) )
      {
        return false;
      }
      static_cast< MaskLayer* >( algo->dst_layers_[ i ].get() )->color_state_->set(
        static_cast< MaskLayer* >( algo->src_layers_[ i ].get() )->color_state_->get() );
      break;
    default:
      return false;
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

void ActionPermute::Dispatch( Core::ActionContextHandle context, 
                const std::vector< std::string >& layer_ids, 
                const std::vector< int >& permutation,
                bool replace )
{
  ActionPermute* action = new ActionPermute;
  action->private_->layer_ids_ = layer_ids;
  action->private_->permutation_ = permutation;
  action->private_->replace_ = replace;

  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
