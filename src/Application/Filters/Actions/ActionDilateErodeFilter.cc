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
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/LayerFilter.h>
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
  // Make sure that the sandbox exists
  if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

  // Check for layer existence and type information
  if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
    Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;
  
  // Check for layer availability 
  if ( ! LayerManager::CheckLayerAvailability( this->target_layer_, 
    this->replace_, context, this->sandbox_ ) ) return false;

  // Check for layer existence and type information
  if ( ( this->mask_layer_ != "" ) && ( this->mask_layer_ != "<none>" ) )
  {
    if ( ! LayerManager::CheckLayerExistenceAndType( this->mask_layer_, 
      Core::VolumeType::MASK_E, context, this->sandbox_ ) ) return false;
    
    if ( ! LayerManager::CheckLayerSize( this->mask_layer_, this->target_layer_,
      context, this->sandbox_ ) ) return false;
    
    // Check for layer availability 
    if ( ! LayerManager::CheckLayerAvailability( this->mask_layer_, false, 
      context, this->sandbox_ ) ) return false;
  }
    
  // If the number of iterations is lower than one, we cannot run the filter
  if( this->dilate_radius_ < 0 )
  {
    context->report_error( "The radius needs to be larger than or equal to zero." );
    return false;
  }

  if( this->dilate_radius_ > 254 )
  {
    context->report_error( "The radius is too large." );
    return false;
  }

  if( this->erode_radius_ < 0 )
  {
    context->report_error( "The radius needs to be larger than or equal to zero." );
    return false;
  }

  if( this->erode_radius_ > 254 )
  {
    context->report_error( "The radius is too large." );
    return false;
  }

  if ( this->slice_type_ != Core::SliceType::AXIAL_E &&
    this->slice_type_ != Core::SliceType::CORONAL_E &&
    this->slice_type_ != Core::SliceType::SAGITTAL_E )
  {
    context->report_error( "Invalid slice type" );
    return false;
  }
    
  // Validation successful
  return true;
}

// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.

class DilateErodeFilterAlgo : public LayerFilter
{

public:
  LayerHandle src_layer_;
  LayerHandle mask_layer_;
  LayerHandle dst_layer_;

  int dilate_radius_;
  int erode_radius_;

  bool invert_mask_;
  
  bool only2d_;
  int slice_type_;
  
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
    Core::DataBlock::index_type nxy = nx * ny;
    Core::DataBlock::index_type size = input_data_block->get_size();

    unsigned char* data = reinterpret_cast<unsigned char*>( input_data_block->get_data() );
  
    std::vector< std::vector<Core::DataBlock::index_type> > neighbors;
    
    Core::SliceType slice_type = static_cast<Core::SliceType::enum_type>( this->slice_type_ );
    try
    {
      neighbors.resize( 0x40 );
      
      for ( size_t k = 0; k < neighbors.size(); k++ )
      {
        if ( !( this->only2d_ ) || ( slice_type != Core::SliceType::SAGITTAL_E ) )
        {
          if ( ! ( k & 0x1 ) ) neighbors[ k ].push_back( -1 );
          if ( ! ( k & 0x2 ) ) neighbors[ k ].push_back( 1 );
        }
        
        if ( !( this->only2d_ ) || ( slice_type != Core::SliceType::CORONAL_E ) )
        {
          if ( ! ( k & 0x4 ) ) neighbors[ k ].push_back( -nx );
          if ( ! ( k & 0x8 ) ) neighbors[ k ].push_back( nx );
        }
        
        if ( !( this->only2d_  ) || ( slice_type != Core::SliceType::AXIAL_E ) )
        {
          if ( ! ( k & 0x10 ) ) neighbors[ k ].push_back( -nxy );
          if ( ! ( k & 0x20 ) ) neighbors[ k ].push_back( nxy );
        }
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
            if ( data[ j ] == 0 && ( mask_data[ j ] & mask_value ) ) data[ j ] = 255;
          }       
        }
        else
        {
          for ( Core::DataBlock::index_type j = 0; j < size; j++ )
          {
            if ( data[ j ] == 0 && !( mask_data[ j ] & mask_value ) ) data[ j ] = 255;
          }
        }
      }
    }
  
    // Detect Edge
    float current_progress = 0.0f;
    float progress_multiplier = 0.1f / static_cast<float>( nz );

    size_t k = 0;
    int border = 0;   
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
          if ( data[ k ] == 0 || data[ k ] == 255 ) continue;
          
          int border_x = border;
          if ( x == 0 ) border_x |= 0x1;
          if ( x == nx - 1 ) border_x |= 0x2;

          const std::vector<Core::DataBlock::index_type>& neigh = neighbors[ border_x ];
          for ( size_t m = 0; m < neigh.size(); m++ )
          {
            Core::DataBlock::index_type index = k + neigh[ m ];
            if ( data[ index ] == 0 || data[ index ] == 255 ) data[ k ] = 2;
          }
        }
      }
      
      float progress = static_cast<float>( z ) * progress_multiplier;
    
      if ( current_progress + 0.01f < progress )
      {
        current_progress = progress;
        this->dst_layer_->update_progress( current_progress );
        if ( this->check_abort() ) return;
      }
    }
    
    // Build pattern
    Core::DataBlock::index_type xr = this->dilate_radius_;
    Core::DataBlock::index_type yr = this->dilate_radius_;
    Core::DataBlock::index_type zr = this->dilate_radius_;
    
    if ( this->only2d_ )
    {
      if ( this->slice_type_ == Core::SliceType::AXIAL_E ) zr = 0;
      if ( this->slice_type_ == Core::SliceType::CORONAL_E ) yr = 0;
      if ( this->slice_type_ == Core::SliceType::SAGITTAL_E ) xr = 0;
    }
    
    Core::DataBlock::index_type xs = 2 * xr + 1;
    Core::DataBlock::index_type ys = 2 * yr + 1;
    Core::DataBlock::index_type zs = 2 * zr + 1;

    Core::DataBlockHandle pattern = 
      Core::StdDataBlock::New( xs, ys, zs, Core::DataType::UCHAR_E );
    
    if ( !pattern )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }   
    
    unsigned char* pdata = reinterpret_cast<unsigned char *>( pattern->get_data() );
    
    k = 0;
    for ( Core::DataBlock::index_type z = -zr; z <= zr; z++ )
    {
      for ( Core::DataBlock::index_type y = -yr; y <= yr; y++ )
      {
        for ( Core::DataBlock::index_type x = -xr; x <= xr; x++, k++ )
        {
          if ( sqrt( static_cast<double>( x*x + y*y + z*z ) ) <= this->dilate_radius_ ) 
          {
            pdata[ k ] = 1;
          }
          else
          {
            pdata[ k ] = 0;
          } 
        }
      }
    }
    
    // Fill in dilation
    
    progress_multiplier = 0.4f / static_cast<float>( nz );    
    
    Core::DataBlock::index_type idx = 0;
    for ( Core::DataBlock::index_type z = 0; z < nz; z++ )
    {     
      for ( Core::DataBlock::index_type y = 0; y < ny; y++ )
      {
        for ( Core::DataBlock::index_type x = 0; x < nx; x++, idx++ )
        {
          if ( data[ idx ] != 2 ) continue;
          
          Core::DataBlock::index_type idx2 = 0;
          for ( Core::DataBlock::index_type zz = -zr; zz <= zr; zz++ )
          {
            for ( Core::DataBlock::index_type yy = -yr; yy <= yr; yy++ )
            {
              for ( Core::DataBlock::index_type xx = -xr; xx <= xr; xx++, idx2++ )
              {         
                if ( pdata[ idx2 ] )
                {
                  Core::DataBlock::index_type px = x - xx;
                  if ( px < 0 || px >= nx ) continue;
                  Core::DataBlock::index_type py = y - yy;
                  if ( py < 0 || py >= ny ) continue;
                  Core::DataBlock::index_type pz = z - zz;
                  if ( pz < 0 || pz >= nz ) continue;
                  Core::DataBlock::index_type fidx = px + py * nx + pz * nxy;
                  if ( data[ fidx ] == 0 )
                  {
                    data[ fidx ] = 1;
                  }
                }
              }
            }
          }
        }
      }
        
      float progress = static_cast<float>( z ) * progress_multiplier + 0.1f;
      
      if ( current_progress + 0.01f < progress )
      {
        current_progress = progress;
        this->dst_layer_->update_progress( current_progress );
        if ( this->check_abort() ) return;
      }
    }

    if ( this->mask_layer_ )
    {
      for ( Core::DataBlock::index_type j = 0; j < size; j++ )
      {
        if ( data[ j ] == 255 ) data[ j ] = 0;
        if ( data[ j ] == 2 ) data[ j ] = 1;
      }
    }
    else
    {
      for ( Core::DataBlock::index_type j = 0; j < size; j++ )
      {
        if ( data[ j ] == 2 ) data[ j ] = 1;
      }   
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
      
      
    // Detect Edge
    progress_multiplier = 0.1f / static_cast<float>( nz );

    k = 0;
    border = 0;   
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
          if ( data[ k ] == 0 || data[ k ] == 2 ) continue;
          
          int border_x = border;
          if ( x == 0 ) border_x |= 0x1;
          if ( x == nx - 1 ) border_x |= 0x2;

          const std::vector<Core::DataBlock::index_type>& neigh = neighbors[ border_x ];
          for ( size_t m = 0; m < neigh.size(); m++ )
          {
            Core::DataBlock::index_type index = k + neigh[ m ];
            if ( data[ index ] == 0 ) data[ index ] = 2;
          }
        }
      }
      
      float progress = static_cast<float>( z ) * progress_multiplier + 0.5f;
    
      if ( current_progress + 0.01f < progress )
      {
        current_progress = progress;
        this->dst_layer_->update_progress( current_progress );
        if ( this->check_abort() ) return;
      }
    }
    
    // Build pattern
    xr = this->erode_radius_;
    yr = this->erode_radius_;
    zr = this->erode_radius_;
    
    if ( this->only2d_ )
    {
      if ( this->slice_type_ == Core::SliceType::AXIAL_E ) zr = 0;
      if ( this->slice_type_ == Core::SliceType::CORONAL_E ) yr = 0;
      if ( this->slice_type_ == Core::SliceType::SAGITTAL_E ) xr = 0;
    }
    
    xs = 2 * xr + 1;
    ys = 2 * yr + 1;
    zs = 2 * zr + 1;

    pattern = Core::StdDataBlock::New( xs, ys, zs, Core::DataType::UCHAR_E );
    
    if ( !pattern )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }   
    
    pdata = reinterpret_cast<unsigned char *>( pattern->get_data() );
    
    k = 0;
    for ( Core::DataBlock::index_type z = -zr; z <= zr; z++ )
    {
      for ( Core::DataBlock::index_type y = -yr; y <= yr; y++ )
      {
        for ( Core::DataBlock::index_type x = -xr; x <= xr; x++, k++ )
        {
          if ( sqrt( static_cast<double>( x*x + y*y + z*z ) ) <= this->erode_radius_ ) 
          {
            pdata[ k ] = 1;
          }
          else
          {
            pdata[ k ] = 0;
          } 
        }
      }
    }     
      

    // Fill in erosion
    
    progress_multiplier = 0.4f / static_cast<float>( nz );    
    
    idx = 0;
    for ( Core::DataBlock::index_type z = 0; z < nz; z++ )
    {     
      for ( Core::DataBlock::index_type y = 0; y < ny; y++ )
      {
        for ( Core::DataBlock::index_type x = 0; x < nx; x++, idx++ )
        {
          if ( data[ idx ] != 2 ) continue;
          
          Core::DataBlock::index_type idx2 = 0;
          for ( Core::DataBlock::index_type zz = -zr; zz <= zr; zz++ )
          {
            for ( Core::DataBlock::index_type yy = -yr; yy <= yr; yy++ )
            {
              for ( Core::DataBlock::index_type xx = -xr; xx <= xr; xx++, idx2++ )
              {         
                if ( pdata[ idx2 ] )
                {
                  Core::DataBlock::index_type px = x - xx;
                  if ( px < 0 || px >= nx ) continue;
                  Core::DataBlock::index_type py = y - yy;
                  if ( py < 0 || py >= ny ) continue;
                  Core::DataBlock::index_type pz = z - zz;
                  if ( pz < 0 || pz >= nz ) continue;
                  Core::DataBlock::index_type fidx = px + py * nx + pz * nxy;
                  if ( data[ fidx ] == 1 )
                  {
                    data[ fidx ] = 0;
                  }
                }
              }
            }
          }
        }
      }
        
      float progress = static_cast<float>( z ) * progress_multiplier + 0.9f;
      
      if ( current_progress + 0.01f < progress )
      {
        current_progress = progress;
        this->dst_layer_->update_progress( current_progress );
        if ( this->check_abort() ) return;
      }
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
      this->report_error("Could not allocate enough memory.");
      return;
    }

    Core::MaskVolumeHandle mask_volume( new Core::MaskVolume( 
      this->src_layer_->get_grid_transform(), output_mask ) );
      
    if ( !mask_volume )
    {
      this->report_error("Could not allocate enough memory.");
      return;
    }
            
    this->dispatch_insert_mask_volume_into_layer( this->dst_layer_, mask_volume );
  }
  
  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "FastDilateErode Filter";
  }

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name, 
  // when a new layer is generated. 
  virtual std::string get_layer_prefix() const
  {
    return "FastDilateErode"; 
  }
};

bool ActionDilateErodeFilter::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
  // Create algorithm
  boost::shared_ptr<DilateErodeFilterAlgo> algo( new DilateErodeFilterAlgo );

  // Copy the parameters over to the algorithm that runs the filter
  algo->set_sandbox( this->sandbox_ );
  algo->dilate_radius_ = this->dilate_radius_;
  algo->erode_radius_ = this->erode_radius_;

  algo->only2d_ = this->only2d_;
  algo->slice_type_ = this->slice_type_;
  
  // Find the handle to the layer
  if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
  {
    return false;
  }

  if ( this->mask_layer_.size() > 0 && this->mask_layer_ != "<none>" )
  {
    if ( !( algo->find_layer( this->mask_layer_, algo->mask_layer_ ) ) )
    {
      return false;
    }   
    algo->lock_for_use( algo->mask_layer_ );
  }
  
  algo->invert_mask_ = this->mask_invert_;  

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

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );
  
  // Start the filter.
  Core::Runnable::Start( algo );

  return true;
}

void ActionDilateErodeFilter::Dispatch( Core::ActionContextHandle context, 
  std::string target_layer, bool replace, int dilate_radius, int erode_radius, 
  std::string mask_layer, bool mask_invert, bool only2d, int slice_type  )
{ 
  // Create a new action
  ActionDilateErodeFilter* action = new ActionDilateErodeFilter;

  // Setup the parameters
  action->target_layer_ = target_layer;
  action->replace_ = replace;
  action->dilate_radius_ = dilate_radius;
  action->erode_radius_ = erode_radius;
  action->mask_layer_ = mask_layer;
  action->mask_invert_ = mask_invert;
  action->only2d_ = only2d;
  action->slice_type_ = slice_type;
    
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}
  
} // end namespace Seg3D
