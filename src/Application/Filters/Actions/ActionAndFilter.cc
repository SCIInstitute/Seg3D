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

// Core includes
#include <Core/Math/MathFunctions.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Actions/ActionAndFilter.h>


// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, AndFilter )

namespace Seg3D
{

bool ActionAndFilter::validate( Core::ActionContextHandle& context )
{
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, Core::VolumeType::MASK_E, 
    context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, this->replace_, 
    context, this->sandbox_ ) ) return false;
  
  // Check for layer existence and type information mask layer
  if ( ! LayerManager::CheckLayerExistenceAndType( this->mask_layer_, Core::VolumeType::MASK_E,
    context, this->sandbox_ ) ) return false;

  // Check whether mask and data have the same size
  if ( ! LayerManager::CheckLayerSize( this->mask_layer_, this->target_layer_,
    context, this->sandbox_ ) ) return false;
    
  // Check for layer availability mask layer
  if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_, 
    this->replace_, context, this->sandbox_ ) ) return false;
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class AndFilterAlgo : public LayerFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;

public:
  // RUN:
  // Implementation of run of the Runnable base class, this function is called when the thread
  // is launched.

  SCI_BEGIN_RUN( ) 
  {
    MaskLayerHandle input_mask1_layer = 
      boost::dynamic_pointer_cast<MaskLayer>( this->src_layer_ );
    MaskLayerHandle input_mask2_layer = 
      boost::dynamic_pointer_cast<MaskLayer>( this->mask_layer_ );
  
    Core::MaskVolumeHandle input_mask1_volume = input_mask1_layer->get_mask_volume();
    Core::MaskVolumeHandle input_mask2_volume = input_mask2_layer->get_mask_volume();

    Core::MaskDataBlockHandle mask1_data_block = input_mask1_volume->get_mask_data_block();
    Core::MaskDataBlockHandle mask2_data_block = input_mask2_volume->get_mask_data_block();

    Core::MaskDataBlockHandle output_mask_data_block;
    if ( !( Core::MaskDataBlockManager::Instance()->create( 
      input_mask1_volume->get_grid_transform(), output_mask_data_block ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return;   
    }
    
    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume(
      input_mask1_volume->get_grid_transform(), output_mask_data_block ) );

    if ( !mask_volume )
    {
      this->report_error( "Could not allocate enough memory." );
      return;   
    } 

    unsigned char* mask1_data = mask1_data_block->get_mask_data();
    unsigned char* mask2_data = mask2_data_block->get_mask_data();
    unsigned char* mask_data = output_mask_data_block->get_mask_data();

    unsigned char mask1_value = mask1_data_block->get_mask_value();
    unsigned char mask2_value = mask2_data_block->get_mask_value();
    unsigned char mask_value = output_mask_data_block->get_mask_value();

    { 
      // NOTE: As this filters works directly on the mask data blocks, one needs to lock
      // them carefully as they share locks between layers in the same memory space 
      // eight layers can share one datablock hence we need to be careful.
      Core::MaskDataBlock::lock_type wlock( output_mask_data_block->get_mutex() );
      
      Core::MaskDataBlock::shared_lock_type slock1;
      Core::MaskDataBlock::shared_lock_type slock2;

      // NOTE: As a shared lock is not recursive the write lock may have already locked the
      // layers we need for reading hence only lock the underlying data if the write
      // lock does not cover it
      if ( mask1_data_block->get_data_block() != output_mask_data_block->get_data_block() )
      {
        Core::MaskDataBlock::shared_lock_type mutex( mask1_data_block->get_mutex() );
        slock1.swap( mutex );
      }
    
      if ( mask2_data_block->get_data_block() != output_mask_data_block->get_data_block() )
      {
        Core::MaskDataBlock::shared_lock_type mutex( mask2_data_block->get_mutex() );
        slock2.swap( mutex );
      }
    
      size_t size = mask1_data_block->get_size();
      size_t size8 = Core::RemoveRemainder8( size );
      size_t progress_count = 0;
      for ( size_t j = 0; j < size8; j+=8 )
      {
        if ( ( mask1_data[ j ] & mask1_value ) && ( mask2_data[ j ] & mask2_value ) )
          mask_data[ j ] |= mask_value;
        if ( ( mask1_data[ j + 1 ] & mask1_value ) && ( mask2_data[ j + 1 ] & mask2_value ) )
          mask_data[ j + 1 ] |= mask_value;
        if ( ( mask1_data[ j + 2 ] & mask1_value ) && ( mask2_data[ j + 2 ] & mask2_value ) )
          mask_data[ j + 2 ] |= mask_value;
        if ( ( mask1_data[ j + 3 ] & mask1_value ) && ( mask2_data[ j + 3 ] & mask2_value ) )
          mask_data[ j + 3 ] |= mask_value;
        if ( ( mask1_data[ j + 4 ] & mask1_value ) && ( mask2_data[ j + 4 ] & mask2_value ) )
          mask_data[ j + 4 ] |= mask_value;
        if ( ( mask1_data[ j + 5 ] & mask1_value ) && ( mask2_data[ j + 5 ] & mask2_value ) )
          mask_data[ j + 5 ] |= mask_value;
        if ( ( mask1_data[ j + 6 ] & mask1_value ) && ( mask2_data[ j + 6 ] & mask2_value ) )
          mask_data[ j + 6 ] |= mask_value;
        if ( ( mask1_data[ j + 7 ] & mask1_value ) && ( mask2_data[ j + 7 ] & mask2_value ) )
          mask_data[ j + 7 ] |= mask_value;
        progress_count++;
        if ( progress_count == 1000 )
        {
          progress_count = 0;
          this->dst_layer_->update_progress( 
            static_cast<float>( j )/static_cast<float>( size ) );
          if ( this->check_abort() ) return;
        }     
      }
      
      for ( size_t j = size8; j < size; j++ )
      {
        if ( ( mask1_data[ j ] & mask1_value ) && ( mask2_data[ j ] & mask2_value ) )
          mask_data[ j ] |= mask_value;     
      }     

      if ( this->check_abort() ) return;
    }
      
    this->dispatch_insert_mask_volume_into_layer( this->dst_layer_, mask_volume );
  }
  SCI_END_RUN()
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "And Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "AND"; 
  }
};


bool ActionAndFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<AndFilterAlgo> algo( new AndFilterAlgo );

  // Find the handle to the layer
  algo->set_sandbox( this->sandbox_ );
  algo->src_layer_ = LayerManager::FindLayer( this->target_layer_, this->sandbox_ );
  algo->mask_layer_ = LayerManager::FindLayer( this->mask_layer_, this->sandbox_ );
  
  // Check whether the source layer was found
  if ( !algo->src_layer_ || !algo->mask_layer_ ) return false;
  
  // Lock the mask layer, so no other layer can access it
  algo->lock_for_use( algo->mask_layer_ );

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
    algo->create_and_lock_mask_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
  }

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );
  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == Core::ActionSource::SCRIPT_E ||
    context->source() == Core::ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record for this action
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionAndFilter::Dispatch( Core::ActionContextHandle context, std::string target_layer,
  std::string mask_layer, bool replace )
{ 
  // Create a new action
  ActionAndFilter* action = new ActionAndFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->mask_layer_ = mask_layer; 
  action->replace_ = replace;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
