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
#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Actions/ActionErodeFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, ErodeFilter )

namespace Seg3D
{

bool ActionErodeFilter::validate( Core::ActionContextHandle& context )
{
  // Check for layer existance and type information
  std::string error;
  if ( ! LayerManager::CheckLayerExistanceAndType( this->target_layer_.value(), 
    Core::VolumeType::MASK_E, error ) )
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
    
  // Check for layer existance and type information
  if ( ( this->mask_layer_.value() != "" ) && ( this->mask_layer_.value() != "<none>" ) )
  {
    std::string error;
    if ( ! LayerManager::CheckLayerExistanceAndType( this->mask_layer_.value(), 
      Core::VolumeType::MASK_E, error ) )
    {
      context->report_error( error );
      return false;
    }
    
    if ( ! LayerManager::CheckLayerSize( this->mask_layer_.value(), this->target_layer_.value(),
      error ) )
    {
      context->report_error( error );
      return false;   
    }
    
    // Check for layer availability 
    if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_.value(), false, notifier ) )
    {
      context->report_need_resource( notifier );
      return false;
    }
  }

  // If the number of iterations is lower than one, we cannot run the filter
  if( this->radius_.value() < 1 )
  {
    context->report_error( "The radius needs to be larger than or equal to one." );
    return false;
  }

  if( this->radius_.value() > 254 )
  {
    context->report_error( "The radius is too large." );
    return false;
  }
  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class ErodeFilterAlgo : public LayerFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;

  int radius_;
  bool invert_mask_;
  
public:
  // RUN_FILTER:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  virtual void run_filter()
  {
    MaskLayerHandle input_mask = boost::dynamic_pointer_cast<MaskLayer>( this->src_layer_ );
    Core::MaskVolumeHandle input_volume = input_mask->get_mask_volume();
    Core::DataBlockHandle input_data_block;
    
    if ( ! ( Core::MaskDataBlockManager::Convert( input_volume->get_mask_data_block(), 
      input_data_block, Core::DataType::UCHAR_E ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }   
    
    Core::DataBlock::index_type nx = input_data_block->get_nx();
    Core::DataBlock::index_type ny = input_data_block->get_ny();
    Core::DataBlock::index_type nz = input_data_block->get_nz();
    Core::DataBlock::index_type size = input_data_block->get_size();
    
    unsigned char* data = reinterpret_cast<unsigned char*>( input_data_block->get_data() );
  
    std::vector< std::vector<Core::DataBlock::index_type> > neighbors;
    try
    {
      neighbors.resize( 0x40 );

      for ( size_t k = 0; k < neighbors.size(); k++ )
      {
        if ( ! ( k & 0x1 ) ) neighbors[ k ].push_back( -1 );
        if ( ! ( k & 0x2 ) ) neighbors[ k ].push_back( 1 );

        if ( ! ( k & 0x4 ) ) neighbors[ k ].push_back( -nx );
        if ( ! ( k & 0x8 ) ) neighbors[ k ].push_back( nx );

        if ( ! ( k & 0x10 ) ) neighbors[ k ].push_back( -nx * ny );
        if ( ! ( k & 0x20 ) ) neighbors[ k ].push_back( nx * ny );
      }
    }
    catch ( ... )
    {
      this->report_error( "Could not allocate enough memory." );
      return;   
    }


    // Inscribe mask
    if ( this->mask_layer_ )
    {
      MaskLayerHandle mask_layer = boost::dynamic_pointer_cast<MaskLayer>( this->mask_layer_ );
      if ( mask_layer )
      {
        Core::MaskDataBlockHandle mask_data_block = mask_layer->get_mask_volume()->
          get_mask_data_block();      
        Core::MaskDataBlock::shared_lock_type lock( mask_data_block->get_mutex() );
        
        unsigned char* mask_data = mask_data_block->get_mask_data();
        unsigned char mask_value = mask_data_block->get_mask_value();
        
        if ( this->invert_mask_ )
        {
          for ( Core::DataBlock::index_type j = 0; j < size; j++ )
          {
            if ( data[ j ] == 1 && ( mask_data[ j ] & mask_value ) ) data[ j ] = 255;
          }       
        }
        else
        {
          for ( Core::DataBlock::index_type j = 0; j < size; j++ )
          {
            if ( data[ j ] == 1 && !( mask_data[ j ] & mask_value ) ) data[ j ] = 255;
          }
        }
      }
    }
      
    unsigned char current_label = 2;
    unsigned char previous_label = 1;
    float current_progress = 0.0f;
    float progress_multiplier = 1.0f / static_cast<float>( this->radius_ * nz );
    
    for ( int i = 0; i < this->radius_; i++)
    {
      int border = 0;
      Core::DataBlock::index_type k = 0;
      for ( Core::DataBlock::index_type z = 0; z < nz; z++ )
      {
        if ( z == 0 ) border |= 0x10; else border &= ~( 0x10 );
        if ( z == nz - 1 ) border |= 0x20; else border &= ~( 0x20 );
        
        for ( Core::DataBlock::index_type y = 0; y < ny; y++ )
        {
          if ( y == 0 ) border |= 0x04; else border &= ~( 0x04 );
          if ( y == ny - 1 ) border |= 0x08; else border &= ~( 0x08 );

          for ( Core::DataBlock::index_type x = 0; x < nx; x++, k++ )
          {
            if ( data[ k ] != 1 ) continue;
            
            int border_x = border;
            if ( x == 0 ) border_x |= 0x1;
            if ( x == nx - 1 ) border_x |= 0x2;

            const std::vector<Core::DataBlock::index_type>& neigh = neighbors[ border_x ];
            for ( size_t m = 0; m < neigh.size(); m++ )
            {
              Core::DataBlock::index_type index = k + neigh[ m ];
              if ( data[ index ] == 255 || data[ index ] == 1  || data[ index ] == current_label ) continue;
              data[ k ] = current_label;
            }
          }
        }
        
        float progress = static_cast<float>( i *  nz + z ) * progress_multiplier;
      
        if ( current_progress + 0.02f < progress )
        {
          current_progress = progress;
          this->dst_layer_->update_progress( current_progress );
          if ( this->check_abort() ) return;
        }
      }
      current_label++;
      previous_label++;
    }

    if ( this->mask_layer_ )
    {
      for ( Core::DataBlock::index_type j = 0; j < size; j++ )
      {
        if ( data[ j ] == 255 ) data[ j ] = 1;
      }
    }
  
    Core::MaskDataBlockHandle output_mask;

    if (!( Core::MaskDataBlockManager::ConvertLabel( input_data_block, 
      this->src_layer_->get_grid_transform(), output_mask, 1.0 ) ) )
    {
      this->report_error( "Could not allocate enough memory." );
      return; 
    }

    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
      this->src_layer_->get_grid_transform(), output_mask ) );

    if ( !mask_volume )
    {
      this->report_error( "Could not allocate enough memory." );
      return;     
    } 
      
    this->dispatch_insert_mask_volume_into_layer( this->dst_layer_, mask_volume );
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "Erode Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "Erode"; 
  }
};


bool ActionErodeFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<ErodeFilterAlgo> algo( new ErodeFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->radius_ = this->radius_.value();

  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_.value(), algo->src_layer_ ) ) )
  {
    return false;   
  }

  if ( this->mask_layer_.value().size() > 0 && this->mask_layer_.value() != "<none>" )
  {
    if ( !( algo->find_layer( this->mask_layer_.value(), algo->mask_layer_ ) ) )
    {
      return false;
    }   
    algo->lock_for_use( algo->mask_layer_ );
  }
  
  algo->invert_mask_ = this->mask_invert_.value();  

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
    algo->create_and_lock_mask_layer_from_layer( algo->src_layer_, algo->dst_layer_ );
  }

  // Return the id of the destination layer.
  result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );

  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionErodeFilter::Dispatch( Core::ActionContextHandle context, 
  std::string target_layer, bool replace, int radius, std::string mask_layer,
  bool mask_invert )
{ 
  // Create a new action
  ActionErodeFilter* action = new ActionErodeFilter;

  // Setup the parameters
  action->target_layer_.value() = target_layer;
  action->replace_.value() = replace;
  action->radius_.value() = radius;
  action->mask_layer_.value() = mask_layer;
  action->mask_invert_.value() = mask_invert;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
