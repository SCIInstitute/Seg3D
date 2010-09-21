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
#include <Application/Filters/Actions/ActionDilateErodeFilter.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, DilateErodeFilter )

namespace Seg3D
{

bool ActionDilateErodeFilter::validate( Core::ActionContextHandle& context )
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
    
  // If the number of iterations is lower than one, we cannot run the filter
  if( this->dilate_radius_.value() < 0 )
  {
    context->report_error( "The radius needs to be larger than or equal to zero." );
    return false;
  }

  if( this->erode_radius_.value() < 0 )
  {
    context->report_error( "The radius needs to be larger than or equal to zero." );
    return false;
  }

  
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class DilateErodeFilterAlgo : public BaseFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle dst_layer_;

  int dilate_radius_;
  int erode_radius_;

public:
  // RUN:
  // Implemtation of run of the Runnable base class, this function is called when the thread
  // is launched.

  virtual void run()
  {
    MaskLayerHandle input_mask = boost::dynamic_pointer_cast<MaskLayer>( this->src_layer_ );
    Core::MaskVolumeHandle input_volume = input_mask->get_mask_volume();
    Core::DataBlockHandle input_data_block;
    
    if ( ! ( Core::MaskDataBlockManager::Convert( input_volume->get_mask_data_block(), 
      input_data_block, Core::DataType::UCHAR_E ) ) )
    {
      StatusBar::SetMessage( Core::LogMessageType::ERROR_E, "DilateErodeFilter failed." );
      return;
    }       
    
    Core::DataBlock::index_type nx = input_data_block->get_nx();
    Core::DataBlock::index_type ny = input_data_block->get_ny();
    Core::DataBlock::index_type nz = input_data_block->get_nz();
    unsigned char* data = reinterpret_cast<unsigned char*>( input_data_block->get_data() );
  
    std::vector< std::vector<Core::DataBlock::index_type> > neighbors( 0x40 );
    for ( size_t k = 0; k < neighbors.size(); k++ )
    {
      if ( ! ( k & 0x1 ) ) neighbors[ k ].push_back( -1 );
      if ( ! ( k & 0x2 ) ) neighbors[ k ].push_back( 1 );

      if ( ! ( k & 0x4 ) ) neighbors[ k ].push_back( -nx );
      if ( ! ( k & 0x8 ) ) neighbors[ k ].push_back( nx );

      if ( ! ( k & 0x10 ) ) neighbors[ k ].push_back( -nx * ny );
      if ( ! ( k & 0x20 ) ) neighbors[ k ].push_back( nx * ny );
    }
  
    unsigned char current_label = 2;
    unsigned char previous_label = 1;
    float current_progress = 0.0f;
    float progress_multiplier = 1.0f / static_cast<float>( ( this->dilate_radius_ +
      this->erode_radius_ ) * nz );
    
    for ( int i = 0; i < this->dilate_radius_; i++)
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
            if ( data[ k ] != previous_label ) continue;
            
            int border_x = border;
            if ( x == 0 ) border_x |= 0x1;
            if ( x == nx - 1 ) border_x |= 0x2;

            const std::vector<Core::DataBlock::index_type>& neigh = neighbors[ border_x ];
            for ( size_t m = 0; m < neigh.size(); m++ )
            {
              Core::DataBlock::index_type index = k + neigh[ m ];
              if ( data[ index ] == 0 ) data[ index ] = current_label;
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
    
    Core::DataBlock::index_type data_size = input_data_block->get_size();
    for ( Core::DataBlock::index_type k = 0; k < data_size;  k++ )
    {
      if ( data[ k ] ) data[ k ] = 1;
    }

    current_label = 2;
    previous_label = 1;
    
    for ( int i = 0; i < this->erode_radius_; i++)
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
              if ( data[ index ] == 1  || data[ index ] == current_label ) continue;
              data[ k ] = current_label;
            }
          }
        }
        
        float progress = static_cast<float>( ( this->dilate_radius_ + i ) *  nz + z ) * progress_multiplier;
      
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
    
    Core::MaskDataBlockHandle output_mask;

    if (!( Core::MaskDataBlockManager::ConvertLabel( input_data_block, 
      this->src_layer_->get_grid_transform(), output_mask, 1.0 ) ) )
    {
      StatusBar::SetMessage( Core::LogMessageType::ERROR_E, "DilateErodeFilter failed." );
      return;
    }

    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
      this->src_layer_->get_grid_transform(), output_mask ) );
      
    this->dispatch_insert_mask_volume_into_layer( this->dst_layer_, mask_volume, true );
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "DilateErode";
  }
};


bool ActionDilateErodeFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<DilateErodeFilterAlgo> algo( new DilateErodeFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->dilate_radius_ = this->dilate_radius_.value();
  algo->erode_radius_ = this->erode_radius_.value();

  // Find the handle to the layer
  algo->find_layer( this->target_layer_.value(), algo->src_layer_ );

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

void ActionDilateErodeFilter::Dispatch( Core::ActionContextHandle context, 
  std::string target_layer, bool replace, int dilate_radius, int erode_radius )
{ 
  // Create a new action
  ActionDilateErodeFilter* action = new ActionDilateErodeFilter;

  // Setup the parameters
  action->target_layer_.value() = target_layer;
  action->replace_.value() = replace;
  action->dilate_radius_.value() = dilate_radius;
  action->erode_radius_.value() = erode_radius;

  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
