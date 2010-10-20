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

// Application includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/BaseFilter.h>
#include <Application/Filters/Actions/ActionMaskDataFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, MaskDataFilter )

namespace Seg3D
{

bool ActionMaskDataFilter::validate( Core::ActionContextHandle& context )
{
  // Check for layer existance and type information
  std::string error;
  if ( ! LayerManager::CheckLayerExistanceAndType( this->target_layer_.value(), 
    Core::VolumeType::DATA_E, error ) )
  {
    context->report_error( error );
    return false;
  }
  
  // Check for layer availability 
  Core::NotifierHandle notifier;
  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_.value(), 
    this->replace_.value(), notifier ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  // Check for layer existance and type information mask layer
  if ( ! LayerManager::CheckLayerExistanceAndType( this->mask_layer_.value(), 
    Core::VolumeType::MASK_E, error ) )
  {
    context->report_error( error );
    return false;
  }

  // Check whether mask and data have the same size
  if ( ! LayerManager::CheckLayerSize( this->mask_layer_.value(), this->target_layer_.value(),
    error ) )
  {
    context->report_error( error );
    return false; 
  }
    
  // Check for layer availability mask layer
  if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_.value(), 
    this->replace_.value(), notifier ) )
  {
    context->report_need_resource( notifier );
    return false;
  }
  
  
  
  // If the number of iterations is lower than one, we cannot run the filter
  if( ! ( this->replace_with_.value() == "zero" || this->replace_with_.value() == "max_value" ||
    this->replace_with_.value() == "min_value" || this->replace_with_.value() == "new_max_value" ||
    this->replace_with_.value() == "new_min_value" ) )
  {
    context->report_error( "Replace_with needs be zero, max_value, min_value, new_max_value,"
      " or new_min_value." );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class MaskDataFilterAlgo : public BaseFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;

  bool invert_mask_;
  std::string replace_with_;

public:
  // RUN:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  // NOTE: The macro needs a data type to select which version to run. This needs to be
  // a member variable of the algorithm class.
  SCI_BEGIN_TYPED_RUN( this->src_layer_->get_data_type() )
  {
    DataLayerHandle input_data_layer = 
      boost::dynamic_pointer_cast<DataLayer>( this->src_layer_ );
    MaskLayerHandle input_mask_layer = 
      boost::dynamic_pointer_cast<MaskLayer>( this->mask_layer_ );
  
    Core::DataVolumeHandle input_data_volume = input_data_layer->get_data_volume();
    Core::MaskVolumeHandle input_mask_volume = input_mask_layer->get_mask_volume();

    Core::DataBlockHandle output_data_block;
    if ( ! ( Core::DataBlock::Clone( input_data_volume->get_data_block(), output_data_block ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
    
    Core::MaskDataBlockHandle mask_data_block = input_mask_volume->get_mask_data_block();

    VALUE_TYPE replace_value( 0 );
    if ( replace_with_ == "new_max_value" )
    {
      replace_value = std::numeric_limits<VALUE_TYPE>::min();
      VALUE_TYPE* data = reinterpret_cast< VALUE_TYPE* >( output_data_block->get_data() );
      
      unsigned char mask_value = mask_data_block->get_mask_value();
      unsigned char* mask = mask_data_block->get_mask_data();
      size_t size = mask_data_block->get_size();
      
      Core::DataBlock::shared_lock_type lock( mask_data_block->get_mutex() );
      if ( ! invert_mask_ )
      {
        for ( size_t j = 0; j < size ; j++ )
        {
          if ( mask[ j ] & mask_value )
          {
            if ( data[ j ] > replace_value ) replace_value = data[ j ];
          }
        }     
      }
      else
      {
        for ( size_t j = 0; j < size ; j++ )
        {
          if ( !( mask[ j ] & mask_value ) )
          {
            if ( data[ j ] > replace_value ) replace_value = data[ j ];
          }
        }
      }
    }
    else if ( replace_with_ == "new_min_value" )
    {
      replace_value = std::numeric_limits<VALUE_TYPE>::max();
      VALUE_TYPE* data = reinterpret_cast< VALUE_TYPE* >( output_data_block->get_data() );
      
      unsigned char mask_value = mask_data_block->get_mask_value();
      unsigned char* mask = mask_data_block->get_mask_data();
      size_t size = mask_data_block->get_size();
      
      Core::DataBlock::shared_lock_type lock( mask_data_block->get_mutex() );
      if ( ! invert_mask_ )
      {
        for ( size_t j = 0; j < size ; j++ )
        {
          if ( mask[ j ] & mask_value )
          {
            if ( data[ j ] < replace_value ) replace_value = data[ j ];
          }
        }     
      }
      else
      {
        for ( size_t j = 0; j < size ; j++ )
        {
          if ( !( mask[ j ] & mask_value ) )
          {
            if ( data[ j ] < replace_value ) replace_value = data[ j ];
          }
        }
      }
    }
    else if ( replace_with_ == "max_value" )
    {
      replace_value = static_cast<VALUE_TYPE>( 
        input_data_volume->get_data_block()->get_max() );
    }
    else if ( replace_with_ == "min_value" )
    {
      replace_value = static_cast<VALUE_TYPE>( 
        input_data_volume->get_data_block()->get_min() );
    }
  
    VALUE_TYPE* data = reinterpret_cast< VALUE_TYPE* >( output_data_block->get_data() );
    
    unsigned char mask_value = mask_data_block->get_mask_value();
    unsigned char* mask = mask_data_block->get_mask_data();
    size_t size = mask_data_block->get_size();
    
    Core::DataBlock::shared_lock_type lock( mask_data_block->get_mutex() );
    if ( invert_mask_ )
    {
      for ( size_t j = 0; j < size ; j++ )
      {
        if ( ( mask[ j ] & mask_value ) )
        {
          data[ j ] = replace_value;
        }
      }     
    }
    else
    {
      for ( size_t j = 0; j < size ; j++ )
      {
        if ( ! ( mask[ j ] & mask_value ) )
        {
          data[ j ] = replace_value;
        }
      }
    }

    Core::DataVolumeHandle output_data_volume( new Core::DataVolume( 
      this->src_layer_->get_grid_transform(), output_data_block ) );
      
    if ( !output_data_volume )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }
      
    this->dispatch_insert_data_volume_into_layer( this->dst_layer_, output_data_volume, 
      true );
  }
  SCI_END_TYPED_RUN()
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "MaskData Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "MaskData";  
  }
};


bool ActionMaskDataFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<MaskDataFilterAlgo> algo( new MaskDataFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->invert_mask_ = this->invert_mask_.value();
  algo->replace_with_ = this->replace_with_.value();

  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_.value(), algo->src_layer_ ) ) )
  {
    return false;
  }
  
  algo->find_layer( this->mask_layer_.value(), algo->mask_layer_ );
  algo->lock_for_use( algo->mask_layer_ );

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
    algo->create_and_lock_data_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
  }

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionMaskDataFilter::Dispatch( Core::ActionContextHandle context, std::string target_layer,
  std::string mask_layer, bool replace, bool invert_mask, std::string replace_with )
{ 
  // Create a new action
  ActionMaskDataFilter* action = new ActionMaskDataFilter;

  // Setup the parameters
  action->target_layer_.value() = target_layer;
  action->mask_layer_.value() = mask_layer; 
  action->replace_.value() = replace;
  action->invert_mask_.value() = invert_mask;
  action->replace_with_.value() = replace_with;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
