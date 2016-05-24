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

// ITK includes
#include <itkNeighborhoodConnectedImageFilter.h>

#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Actions/ActionNeighborhoodConnectedFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, NeighborhoodConnectedFilter )

namespace Seg3D
{

bool ActionNeighborhoodConnectedFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::DATA_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailabilityForProcessing( this->target_layer_, 
    context, this->sandbox_ ) ) return false;

  if ( this->seeds_.size() == 0 )
  {
    context->report_error( "There needs to be at least one seed point." );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class NeighborhoodConnectedFilterAlgo : public LayerFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;

  std::vector< std::vector< int > > seeds_;
  
public:
  template< class T >
  void typed_run( Core::DataBlock* src )
  {
    const T* src_data = reinterpret_cast< T* >( src->get_data() );

    // Allocate the data block for storing the results
    Core::DataBlockHandle dst_mask = Core::StdDataBlock::New( 
      this->src_layer_->get_grid_transform(), Core::DataType::UCHAR_E );

    if ( !dst_mask )
    {
      this->report_error( "Could not allocate enough memory." );
      return;   
    }

    // Get the dimensions
    int dimensions[ 3 ];
    dimensions[ 0 ] = static_cast< int >( src->get_nx() );
    dimensions[ 1 ] = static_cast< int >( src->get_ny() );
    dimensions[ 2 ] = static_cast< int >( src->get_nz() );

    // Initialize the result data with 0
    unsigned char* dst_data = reinterpret_cast< unsigned char* >( dst_mask->get_data() );
    size_t total_voxels = dst_mask->get_size();
    memset( dst_data, 0, total_voxels );

    // Initialize the seeds and compute the threshold
    T min_val = std::numeric_limits< T >::max();
    T max_val = std::numeric_limits< T >::min();

    std::stack< std::vector< int > > seeds;
    for ( size_t i = 0; i < this->seeds_.size(); ++i )
    {
      const std::vector< int >& seed = this->seeds_[ i ];
      size_t index = src->to_index( static_cast< size_t >( seed[ 0 ] ),
        static_cast< size_t >( seed[ 1 ] ), static_cast< size_t >( seed[ 2 ] ) );
      T value = src_data[ index ];
      dst_data[ index ] = 1;
      min_val = Core::Min( min_val, value );
      max_val = Core::Max( max_val, value );
      seeds.push( seed );
    }
    
    std::vector< int > neighbor_index( 3 );
    size_t visited_voxels = seeds.size();
    size_t last_report_voxels = visited_voxels;
    size_t progress_threshold = total_voxels / 40;

    while ( !seeds.empty() )
    {
      std::vector< int > seed_index = seeds.top();
      seeds.pop();

      // Expand along three axes
      for ( int i = 0; i < 3; ++i )
      {
        neighbor_index = seed_index;
        if ( neighbor_index[ i ] > 0 )
        {
          neighbor_index[ i ] -= 1;
          size_t offset = src->to_index( static_cast< size_t >( neighbor_index[ 0 ] ),
            static_cast< size_t >( neighbor_index[ 1 ] ), 
            static_cast< size_t >( neighbor_index[ 2 ] ) );
          if ( dst_data[ offset ] != 1 &&
            src_data[ offset ] >= min_val &&
            src_data[ offset ] <= max_val )
          {
            dst_data[ offset ] = 1;
            seeds.push( neighbor_index );
          }
          ++visited_voxels;
        }
        
        neighbor_index = seed_index;
        if ( neighbor_index[ i ] < dimensions[ i ] - 1 )
        {
          neighbor_index[ i ] += 1;
          size_t offset = src->to_index( static_cast< size_t >( neighbor_index[ 0 ] ),
            static_cast< size_t >( neighbor_index[ 1 ] ), 
            static_cast< size_t >( neighbor_index[ 2 ] ) );
          if ( dst_data[ offset ] != 1 &&
            src_data[ offset ] >= min_val &&
            src_data[ offset ] <= max_val )
          {
            dst_data[ offset ] = 1;
            seeds.push( neighbor_index );
          }
          ++visited_voxels;
        }
      }

      if ( visited_voxels - last_report_voxels > progress_threshold )
      {
        if ( this->check_abort() )  return;
        
        this->dst_layer_->update_progress_signal_( visited_voxels * 0.8 / total_voxels );
        last_report_voxels = visited_voxels;
      }
    }

    if ( this->check_abort() )
    {
      return;
    }
    
    this->dst_layer_->update_progress_signal_( 0.8 );
    Core::MaskDataBlockHandle mask_datablock;
    if ( !( Core::MaskDataBlockManager::Convert( dst_mask, 
      this->dst_layer_->get_grid_transform(), mask_datablock ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return;   
    }
      
    this->dst_layer_->update_progress_signal_( 1.0 );
    
    this->dispatch_insert_mask_volume_into_layer( this->dst_layer_,
      Core::MaskVolumeHandle( new Core::MaskVolume(
      this->dst_layer_->get_grid_transform(), mask_datablock ) ) );
      
  }

  // RUN_FILTER:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.
  virtual void run_filter()
  {

    // Add the seeds to the filter, and compute the threshold values
    Core::DataBlockHandle datablock = dynamic_cast< DataLayer* >( 
      this->src_layer_.get() )->get_data_volume()->get_data_block();

    switch ( datablock->get_data_type() )
    {
    case Core::DataType::CHAR_E:
      this->typed_run< signed char >( datablock.get() );
      break;
    case Core::DataType::UCHAR_E:
      this->typed_run< unsigned char >( datablock.get() );
      break;
    case Core::DataType::SHORT_E:
      this->typed_run< short >( datablock.get() );
      break;
    case Core::DataType::USHORT_E:
      this->typed_run< unsigned short >( datablock.get() );
      break;
    case Core::DataType::INT_E:
      this->typed_run< int >( datablock.get() );
      break;
    case Core::DataType::UINT_E:
      this->typed_run< unsigned int >( datablock.get() );
      break;
    case Core::DataType::FLOAT_E:
      this->typed_run< float >( datablock.get() );
      break;
    case Core::DataType::DOUBLE_E:
      this->typed_run< double >( datablock.get() );
      break;
    }   
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "NeighborhoodConnected Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "NeighborhoodConnected"; 
  }
};


bool ActionNeighborhoodConnectedFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<NeighborhoodConnectedFilterAlgo> algo( new NeighborhoodConnectedFilterAlgo );
  algo->set_sandbox( sandbox_ );

  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
  {
    return false;
  }

  // Check the seed points against the source layer dimensions
  const Core::GridTransform& grid_trans = algo->src_layer_->get_grid_transform();
  Core::Transform inverse_trans = grid_trans.get_inverse();
  int nx = static_cast< int >( grid_trans.get_nx() );
  int ny = static_cast< int >( grid_trans.get_ny() );
  int nz = static_cast< int >( grid_trans.get_nz() );

  try
  {
    std::vector< int > index( 3 );
    const std::vector< Core::Point > seeds = this->seeds_;
    for ( size_t i = 0; i < seeds.size(); ++i )
    {
      Core::Point seed = inverse_trans * seeds[ i ];
      index[ 0 ] = Core::Round( seed[ 0 ] );
      index[ 1 ] = Core::Round( seed[ 1 ] );
      index[ 2 ] = Core::Round( seed[ 2 ] );
      if ( index[ 0 ] >= 0 && index[ 0 ] < nx &&
        index[ 1 ] >= 0 && index[ 1 ] < ny &&
        index[ 2 ] >= 0 && index[ 2 ] < nz )
      {
        algo->seeds_.push_back( index );
      }
    }
  }
  catch ( ... )
  {
    algo->report_error( "Could not allocate enough memory." );
    return false; 
  }
  
  if ( algo->seeds_.size() == 0 )
  {
    context->report_error( "All seed points are out of the volume boundary" );
    return false;
  }
  
  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( algo->src_layer_ );
  
  // Create the destination layer, which will show progress
  algo->create_and_lock_mask_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
  algo->connect_abort( algo->dst_layer_ );

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );
  
  // Start the filter on a separate thread.
  Core::Runnable::Start( algo );

  return true;
}

void ActionNeighborhoodConnectedFilter::Dispatch( Core::ActionContextHandle context, 
    std::string target_layer, const std::vector< Core::Point >& seeds )
{ 
  // Create a new action
  ActionNeighborhoodConnectedFilter* action = new ActionNeighborhoodConnectedFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->seeds_ = seeds;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
