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

#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/Tools/Actions/ActionInvert.h>
#include <Application/Tool/BaseFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Invert )

namespace Seg3D
{

bool ActionInvert::validate( Core::ActionContextHandle& context )
{
  // Check for layer existence and type information
  std::string error;
  if ( ! LayerManager::CheckLayerExistanceAndType( this->layer_id_.value(), 
    Core::VolumeType::DATA_E, error ) )
  {
    context->report_error( error );
    return false;
  }
  
  // Check for layer availability 
  Core::NotifierHandle notifier;
  if ( ! LayerManager::CheckLayerAvailability( this->layer_id_.value(), 
    this->replace_.value(), notifier ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class InvertFilterAlgo : public BaseFilter
{

public:
  DataLayerHandle src_layer_;
  DataLayerHandle dst_layer_;

public:
  template< class T >
  void invert_data( Core::DataBlockHandle dst )
  {
    Core::DataBlockHandle src_data_block = this->src_layer_->
      get_data_volume()->get_data_block();
    const T* src_data = reinterpret_cast< T* >( src_data_block->get_data() );
    T* dst_data = reinterpret_cast< T* >( dst->get_data() );
    size_t z_plane_size = src_data_block->get_nx() * src_data_block->get_ny();
    size_t nz = src_data_block->get_nz();
    size_t tenth_nz = nz / 10;

    Core::DataBlock::shared_lock_type lock( src_data_block->get_mutex() );
    T min_val = static_cast< T >( src_data_block->get_min() );
    T max_val = static_cast< T >( src_data_block->get_max() );
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
      if ( z > 0 && z % tenth_nz == 0 )
      {
        this->dst_layer_->update_progress_signal_( ( z * 1.0 ) / nz );
      }
    }
    this->dst_layer_->update_progress_signal_( 1.0 );
  }

  // RUN:
  // Implementation of run of the Runnable base class, this function is called 
  // when the thread is launched.
  virtual void run()
  {
    Core::DataBlockHandle dst_data_block = Core::StdDataBlock::New(
      this->src_layer_->get_grid_transform(), this->src_layer_->get_data_type() );
    switch ( this->src_layer_->get_data_type() )
    {
    case Core::DataType::CHAR_E:
      this->invert_data< signed char >( dst_data_block );
      break;
    case Core::DataType::UCHAR_E:
      this->invert_data< unsigned char >( dst_data_block );
      break;
    case Core::DataType::SHORT_E:
      this->invert_data< short >( dst_data_block );
      break;
    case Core::DataType::USHORT_E:
      this->invert_data< unsigned short >( dst_data_block );
      break;
    case Core::DataType::INT_E:
      this->invert_data< int >( dst_data_block );
      break;
    case Core::DataType::UINT_E:
      this->invert_data< unsigned int >( dst_data_block );
      break;
    case Core::DataType::FLOAT_E:
      this->invert_data< float >( dst_data_block );
      break;
    case Core::DataType::DOUBLE_E:
      this->invert_data< double >( dst_data_block );
      break;
    }
    
    if ( this->check_abort() )
    {
      return;
    }
    
    this->dispatch_insert_data_volume_into_layer( this->dst_layer_,
      Core::DataVolumeHandle( new Core::DataVolume( 
      this->dst_layer_->get_grid_transform(), dst_data_block ) ), false, true );
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Invert";
  }
};


bool ActionInvert::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< InvertFilterAlgo > algo( new InvertFilterAlgo );

  // Find the handle to the layer
  LayerHandle src_layer;
  algo->find_layer( this->layer_id_.value(), src_layer );
  algo->src_layer_ = boost::dynamic_pointer_cast< DataLayer >( src_layer );

  if ( this->replace_.value() )
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
    LayerHandle dst_layer;
    algo->create_and_lock_data_layer_from_layer( algo->src_layer_, dst_layer );
    algo->dst_layer_ = boost::dynamic_pointer_cast< DataLayer >( dst_layer );
  }

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( 
    algo->dst_layer_->get_layer_id() ) );

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
  action->layer_id_.value() = layer_id;
  action->replace_.value() = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
