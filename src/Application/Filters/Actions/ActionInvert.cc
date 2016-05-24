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
#include <Application/Layer/LayerManager.h>
#include <Application/Filters/Actions/ActionInvert.h>
#include <Application/Filters/LayerFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Invert )

namespace Seg3D
{

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class InvertFilterAlgo : public LayerFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;

public:
  template< class T >
  void invert_data( Core::DataBlockHandle src, DataLayerHandle dst )
  {
    Core::DataBlockHandle dst_data_block = Core::StdDataBlock::New(
      dst->get_grid_transform(), src->get_data_type() );
    if ( !dst_data_block )  
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }

    const T* src_data = reinterpret_cast< T* >( src->get_data() );
    T* dst_data = reinterpret_cast< T* >( dst_data_block->get_data() );

    size_t z_plane_size = src->get_nx() * src->get_ny();
    size_t nz = src->get_nz();
    size_t tenth_nz = nz / 10;

    Core::DataBlock::shared_lock_type lock( src->get_mutex() );
    T min_val = static_cast< T >( src->get_min() );
    T max_val = static_cast< T >( src->get_max() );
    T bias = max_val + min_val;
    
    size_t index = 0;
    for ( size_t z = 0; z < nz; ++z )
    {
      for ( size_t i = 0; i < z_plane_size; ++i, ++index )
      {
        dst_data[ index ] = bias - src_data[ index ];
      }
      if ( this->check_abort() )
      {
        return;
      }
      if ( tenth_nz > 0 && z > 0 && z % tenth_nz == 0 )
      {
        dst->update_progress_signal_( ( z * 1.0 ) / nz );
      }
    }
    dst->update_progress_signal_( 1.0 );

    if ( this->check_abort() )
    {
      return;
    }

    this->dispatch_insert_data_volume_into_layer( dst,
      Core::DataVolumeHandle( new Core::DataVolume( 
      dst->get_grid_transform(), dst_data_block ) ), true );
  }

  void invert_mask( MaskLayerHandle input, MaskLayerHandle output )
  {
    Core::MaskDataBlockHandle mask_datablock = input->get_mask_volume()->
      get_mask_data_block();
    const unsigned char* src_data = mask_datablock->get_mask_data();
    unsigned char mask_value = mask_datablock->get_mask_value();
    Core::DataBlockHandle output_datablock = Core::StdDataBlock::New(
      output->get_grid_transform(), Core::DataType::UCHAR_E );
    if ( !output_datablock )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
    
    Core::MaskDataBlock::shared_lock_type lock( mask_datablock->get_mutex() );
    unsigned char* dst_data = reinterpret_cast< unsigned char* >( output_datablock->get_data() );
    size_t z_plane_size = mask_datablock->get_nx() * mask_datablock->get_ny();
    size_t nz = mask_datablock->get_nz();
    size_t tenth_nz = nz / 10;
    size_t index = 0;
    for ( size_t z = 0; z < nz; ++z )
    {
      for ( size_t i = 0; i < z_plane_size; ++i, ++index )
      {
        dst_data[ index ] = !( src_data[ index ] & mask_value );
      }
      if ( this->check_abort() )
      {
        return;
      }
      if ( z > 0 && z % tenth_nz == 0 )
      {
        output->update_progress_signal_( ( z * 1.0 ) / nz );
      }
    }
    output->update_progress_signal_( 1.0 );

    lock.unlock();

    Core::MaskDataBlockHandle output_mask;
    Core::MaskDataBlockManager::Convert( output_datablock, 
      output->get_grid_transform(), output_mask );
    if ( !output_mask )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
    
    this->dispatch_insert_mask_volume_into_layer( output, Core::MaskVolumeHandle( 
      new Core::MaskVolume( output->get_grid_transform(), output_mask ) ) );
  }

  void invert_data( DataLayerHandle input, DataLayerHandle output )
  {
    Core::DataBlockHandle src_data_block = input->get_data_volume()->get_data_block();

    switch ( input->get_data_type() )
    {
    case Core::DataType::CHAR_E:
      this->invert_data< signed char >( src_data_block, output );
      break;
    case Core::DataType::UCHAR_E:
      this->invert_data< unsigned char >( src_data_block, output );
      break;
    case Core::DataType::SHORT_E:
      this->invert_data< short >( src_data_block, output );
      break;
    case Core::DataType::USHORT_E:
      this->invert_data< unsigned short >( src_data_block, output );
      break;
    case Core::DataType::INT_E:
      this->invert_data< int >( src_data_block, output );
      break;
    case Core::DataType::UINT_E:
      this->invert_data< unsigned int >( src_data_block, output );
      break;
    case Core::DataType::FLOAT_E:
      this->invert_data< float >( src_data_block, output );
      break;
    case Core::DataType::DOUBLE_E:
      this->invert_data< double >( src_data_block, output );
      break;
    }
  }

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run_filter()
  {
    switch ( this->src_layer_->get_type() )
    {
    case Core::VolumeType::MASK_E:
      this->invert_mask( boost::dynamic_pointer_cast< MaskLayer >( this->src_layer_ ),
        boost::dynamic_pointer_cast< MaskLayer >( this->dst_layer_ ) );
      break;
    case Core::VolumeType::DATA_E:
      this->invert_data( boost::dynamic_pointer_cast< DataLayer >( this->src_layer_ ),
        boost::dynamic_pointer_cast< DataLayer >( this->dst_layer_ ) );
      break;
    }
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Invert Tool";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Invert";  
  }
};

bool ActionInvert::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistence( this->layer_id_, context, this->sandbox_ ) ) return false;

  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->layer_id_, 
    this->replace_, context, this->sandbox_ ) ) return false;

  // Validation successful
  return true;
}

bool ActionInvert::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< InvertFilterAlgo > algo( new InvertFilterAlgo );
  algo->set_sandbox( this->sandbox_ );

  // Find the handle to the layer 
  if ( !( algo->find_layer( this->layer_id_, algo->src_layer_ ) ) )
  {
    return false;
  }

  if ( this->replace_ )
  {
    // Copy the handles as destination and source will be the same
    algo->dst_layer_ = algo->src_layer_;
    // Mark the layer for processing.
    algo->lock_for_processing( algo->dst_layer_ );  
  }
  else
  {
    // Lock the src layer, so it cannot be used else where
    algo->lock_for_use( algo->src_layer_ );
    
    // Create the destination layer, which will show progress
    switch ( algo->src_layer_->get_type() )
    {
    case Core::VolumeType::MASK_E:
      algo->create_and_lock_mask_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
      break;
    case Core::VolumeType::DATA_E:
      algo->create_and_lock_data_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
      break;
    default:
      return false;
    }
  }

  algo->connect_abort( algo->dst_layer_ );

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( 
    algo->dst_layer_->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );
  
  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}


void ActionInvert::Dispatch( Core::ActionContextHandle context, 
  std::string layer_id, bool replace )
{ 
  // Create a new action
  ActionInvert* action = new ActionInvert;

  // Setup the parameters
  action->layer_id_ = layer_id;
  action->replace_ = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
