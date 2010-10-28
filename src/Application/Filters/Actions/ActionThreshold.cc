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

#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/DataBlock/StdDataBlock.h>

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Actions/ActionThreshold.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, Threshold )

namespace Seg3D
{

ActionThreshold::ActionThreshold()
{
  this->add_argument( this->target_layer_ );
  this->add_argument( this->upper_threshold_ );
  this->add_argument( this->lower_threshold_ );
}


bool ActionThreshold::validate( Core::ActionContextHandle& context )
{
  // Check for layer existence and type information
  std::string error;
  if ( ! LayerManager::CheckLayerExistanceAndType( this->target_layer_.value(), 
    Core::VolumeType::DATA_E, error ) )
  {
    context->report_error( error );
    return false;
  }
  
  // Check for layer availability 
  Core::NotifierHandle notifier;
  if ( ! LayerManager::CheckLayerAvailabilityForProcessing( this->target_layer_.value(), 
    notifier ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  if ( this->lower_threshold_.value() > this->upper_threshold_.value() )
  {
    std::swap( this->lower_threshold_.value(), this->upper_threshold_.value() );
  }
  
  DataLayerHandle data_layer = LayerManager::Instance()->
    get_data_layer_by_id( this->target_layer_.value() );
  double min_val = data_layer->get_data_volume()->get_data_block()->get_min();
  double max_val = data_layer->get_data_volume()->get_data_block()->get_max();

  if( this->lower_threshold_.value() > max_val ||
    this->upper_threshold_.value() < min_val )
  {
    context->report_error( "The threshold is out of data range." );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class ThresholdFilterAlgo : public LayerFilter
{

public:
  DataLayerHandle src_layer_;
  MaskLayerHandle dst_layer_;

  double lower_threshold_;
  double upper_threshold_;
  
public:
  template< class T >
  void threshold_data( Core::DataBlockHandle dst, double min_val, double max_val )
  {
    Core::DataBlockHandle src_data_block = this->src_layer_->
      get_data_volume()->get_data_block();
    T* src_data = reinterpret_cast< T* >( src_data_block->get_data() );
    unsigned char* dst_data = reinterpret_cast< unsigned char* >( dst->get_data() );
    size_t z_plane_size = src_data_block->get_nx() * src_data_block->get_ny();
    size_t nz = src_data_block->get_nz();
    size_t tenth_nz = nz / 10;

    // Lock the source data block
    Core::DataBlock::shared_lock_type lock( src_data_block->get_mutex() );
    size_t index = 0;
    for ( size_t z = 0; z < nz; ++z )
    {
      for ( size_t i = 0; i < z_plane_size; ++i, ++index )
      {
        dst_data[ index ] = ( src_data[ index ] >= min_val && 
          src_data[ index ] <= max_val ) ? 1 : 0;
      }
      if ( this->check_abort() )
      {
        return;
      }
      if ( z > 0 && z % tenth_nz == 0 )
      {
        this->dst_layer_->update_progress_signal_( ( z * 0.5 ) / nz );
      }
    }
    this->dst_layer_->update_progress_signal_( 0.5 );
  }

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.
  virtual void run_filter()
  {
    Core::DataBlockHandle threshold_result = Core::StdDataBlock::New( 
      this->src_layer_->get_grid_transform(), 
      Core::DataType::UCHAR_E );
    switch ( this->src_layer_->get_data_type() )
    {
    case Core::DataType::CHAR_E:
      this->threshold_data< signed char >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    case Core::DataType::UCHAR_E:
      this->threshold_data< unsigned char >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    case Core::DataType::SHORT_E:
      this->threshold_data< short >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    case Core::DataType::USHORT_E:
      this->threshold_data< unsigned short >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    case Core::DataType::INT_E:
      this->threshold_data< int >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    case Core::DataType::UINT_E:
      this->threshold_data< unsigned int >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    case Core::DataType::FLOAT_E:
      this->threshold_data< float >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    case Core::DataType::DOUBLE_E:
      this->threshold_data< double >( threshold_result, 
        this->lower_threshold_, this->upper_threshold_ );
      break;
    }
    
    if ( this->check_abort() )
    {
      return;
    }
    
    Core::MaskDataBlockHandle threshold_mask;
    Core::MaskDataBlockManager::Convert( threshold_result, 
      this->dst_layer_->get_grid_transform(), threshold_mask );

    if ( this->check_abort() )
    {
      return;
    }

    this->dst_layer_->update_progress_signal_( 1.0 );

    this->dispatch_insert_mask_volume_into_layer( this->dst_layer_,
      Core::MaskVolumeHandle( new Core::MaskVolume( 
      this->dst_layer_->get_grid_transform(), threshold_mask ) ) );
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Threshold Tool";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Threshold"; 
  }
};


bool ActionThreshold::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr< ThresholdFilterAlgo > algo( new ThresholdFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->lower_threshold_ = this->lower_threshold_.value();
  algo->upper_threshold_ = this->upper_threshold_.value();

  // Find the handle to the layer
  LayerHandle src_layer;
  algo->find_layer( this->target_layer_.value(), src_layer );
  algo->src_layer_ = boost::dynamic_pointer_cast< DataLayer >( src_layer );

  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( src_layer );
  
  // Create the destination layer, which will show progress
  LayerHandle dst_layer;
  algo->create_and_lock_mask_layer_from_layer( algo->src_layer_, dst_layer );
  algo->dst_layer_ = boost::dynamic_pointer_cast< MaskLayer >( dst_layer );
  algo->connect_abort( algo->dst_layer_ );

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );

  // Start the filter on a separate thread.
  Core::Runnable::Start( algo );

  return true;
}


void ActionThreshold::Dispatch( Core::ActionContextHandle context, 
                 std::string target_layer, double lower_threshold,
                 double upper_threshold )
{ 
  // Create a new action
  ActionThreshold* action = new ActionThreshold;

  // Setup the parameters
  action->target_layer_.value() = target_layer;
  action->lower_threshold_.value() = lower_threshold;
  action->upper_threshold_.value() = upper_threshold;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}


} // end namespace Seg3D
