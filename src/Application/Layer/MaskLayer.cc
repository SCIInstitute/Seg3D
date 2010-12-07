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

// STL includes

// Boost includes 

// Core includes
#include <Core/Application/Application.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Volume/MaskVolume.h>
#include <Core/Utils/StringUtil.h>


// Application includes
#include <Application/Layer/MaskLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/ProjectManager.h>


namespace Seg3D
{

MaskLayer::MaskLayer( const std::string& name, const Core::MaskVolumeHandle& volume ) :
  Layer( name, !( volume->is_valid() ) ), 
  loading_( false ),
  mask_volume_( volume )
{
  this->mask_volume_->register_data();
  this->initialize_states();
  
  if (  volume->get_mask_data_block() )
  {
    this->bit_state_->set( static_cast< int >( volume->get_mask_data_block()->get_mask_bit() ) );
    this->add_connection( this->mask_volume_->get_mask_data_block()->mask_updated_signal_.
      connect( boost::bind( &MaskLayer::handle_mask_data_changed, this ) ) );
  }
}

MaskLayer::MaskLayer( const std::string& state_id ) :
  Layer( "not_initialized", state_id ),
  loading_( false )
{
  this->initialize_states();
}

MaskLayer::~MaskLayer()
{
  // Disconnect all current connections
  this->disconnect_all();

  if ( this->mask_volume_ )
  {
    this->mask_volume_->unregister_data();
  }
}

Core::GridTransform MaskLayer::get_grid_transform() const 
{ 
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->mask_volume_ )
  {
    return this->mask_volume_->get_grid_transform(); 
  }
  else
  {
    return Core::GridTransform();
  }
}

Core::DataType MaskLayer::get_data_type() const
{
  return Core::DataType::UCHAR_E;
}

Core::MaskVolumeHandle MaskLayer::get_mask_volume() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  return this->mask_volume_;
}

bool MaskLayer::has_valid_data() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->mask_volume_ )
  {
    return this->mask_volume_->is_valid();
  }
  else
  {
    return false;
  }
}

Core::VolumeHandle MaskLayer::get_volume() const
{
  return this->get_mask_volume();
}

bool MaskLayer::set_mask_volume( Core::MaskVolumeHandle volume )
{
  ASSERT_IS_APPLICATION_THREAD();

  // Only insert the volume if the layer is still valid
  if ( !this->is_valid() )  return false;

  {
    Layer::lock_type lock( Layer::GetMutex() );

    if ( this->mask_volume_ )
    {
      // Unregister the old volume
      this->mask_volume_->unregister_data();
    }
    
    this->disconnect_all();

    this->mask_volume_ = volume;
    if ( this->mask_volume_ )
    {
      // Register the new volume
      this->mask_volume_->register_data();
      this->generation_state_->set( this->mask_volume_->get_generation() );
      this->bit_state_->set( static_cast< int >( 
        volume->get_mask_data_block()->get_mask_bit() ) );

      this->add_connection( this->mask_volume_->get_mask_data_block()->mask_updated_signal_.
        connect( boost::bind( &MaskLayer::handle_mask_data_changed, this ) ) );
    }
  }

  return true;
}

// counter for generating new colors for each new mask
size_t ColorCount = 0;
boost::mutex ColorCountMutex;

void MaskLayer::initialize_states()
{
    // == Color of the layer ==

  {
    boost::mutex::scoped_lock lock( ColorCountMutex );
    this->add_state( "color", color_state_, static_cast< int >( ColorCount % 
      PreferencesManager::Instance()->color_states_.size() ) );

    ColorCount++;
  }
  
    // == What border is used ==
    this->add_state( "border", this->border_state_, PreferencesManager::Instance()->
    default_mask_border_state_->export_to_string(), PreferencesManager::Instance()->
    default_mask_border_state_->export_list_to_string() );

    // == How is the segmentation filled in ==
    this->add_state( "fill", this->fill_state_, PreferencesManager::Instance()->
    default_mask_fill_state_->export_to_string(), PreferencesManager::Instance()->
    default_mask_fill_state_->export_list_to_string() );

    // == Whether the isosurface is shown in the volume display ==
    this->add_state( "isosurface", this->show_isosurface_state_, false );

  // == Internal information for keeping track of which bit we are using ==
  this->add_state( "bit", this->bit_state_, 0 );
  
  // == Keep track of whether the iso surface has been generated
  this->add_state( "iso_generated", this->iso_generated_state_, false );
  
  // == Keep track of the calculated volume and put it in the UI
  this->add_state( "calculated_volume", this->calculated_volume_state_, "N/A" );
  
  // == Keep track of the calculated volume and put it in the UI
  this->add_state( "counted_pixels", this->counted_pixels_state_, "N/A" );
}

bool MaskLayer::pre_save_states( Core::StateIO& state_io )
{
  this->generation_state_->set( static_cast< int >( this->get_mask_volume()->get_generation() ) );
  return true;
}

bool MaskLayer::post_load_states( const Core::StateIO& state_io )
{
  Core::DataBlock::generation_type generation = this->generation_state_->get();
  unsigned int bit = static_cast< unsigned int >( this->bit_state_->get() );
  Core::MaskDataBlockHandle mask_data_block;
  Core::GridTransform grid_transform;
  bool success = Core::MaskDataBlockManager::Instance()->
    create( generation, bit, grid_transform, mask_data_block );
  if ( !success )
  {
    Core::DataVolumeHandle data_volume;
    boost::filesystem::path volume_path = ProjectManager::Instance()->get_project_data_path() /
      ( this->generation_state_->export_to_string() + ".nrrd" );
    std::string error;

    if( Core::DataVolume::LoadDataVolume( volume_path, data_volume, error ) )
    {
      data_volume->register_data( generation );
      Core::MaskDataBlockManager::Instance()->register_data_block( 
        data_volume->get_data_block(), data_volume->get_grid_transform() );
      success = Core::MaskDataBlockManager::Instance()->
        create( generation, bit, grid_transform, mask_data_block );
    }
  }

  if ( success )
  {
    this->mask_volume_ = Core::MaskVolumeHandle( new Core::MaskVolume( 
      grid_transform, mask_data_block ) );
    this->add_connection( this->mask_volume_->get_mask_data_block()->mask_updated_signal_.
      connect( boost::bind( &MaskLayer::handle_mask_data_changed, this ) ) );
  }
  
  return success;
}

void MaskLayer::clean_up()
{
  // Abort any filter still using this layer
  this->abort_signal_();
  
  // Clean up the data that is still associated with this layer
  {
    Layer::lock_type lock( Layer::GetMutex() );
    if ( this->mask_volume_ )
    {
      this->mask_volume_->unregister_data();
      Core::MaskVolume::CreateInvalidMask( this->mask_volume_->get_grid_transform(),
        this->mask_volume_ );
    }
  }

  // Remove all the connections
  this->disconnect_all();
}

void MaskLayer::handle_mask_data_changed()
{
  this->calculated_volume_state_->set( "N/A" );
  this->counted_pixels_state_->set( "N/A" );
  this->layer_updated_signal_();
}

void MaskLayer::handle_isosurface_update_progress( double progress )
{
  this->update_progress_signal_( progress );
}

Core::IsosurfaceHandle MaskLayer::get_isosurface()
{
  lock_type lock( Layer::GetMutex() );
  return this->isosurface_;
}

void MaskLayer::compute_isosurface( double quality_factor )
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &MaskLayer::compute_isosurface, 
      this, quality_factor ) );
    return;
  }
  
  Core::IsosurfaceHandle iso = this->isosurface_;
  if ( !iso )
  {
    iso.reset( new Core::Isosurface( this->mask_volume_ ) );
    this->add_connection( iso->update_progress_signal_.connect(
      boost::bind( &MaskLayer::handle_isosurface_update_progress, this, _1 ) ) );
  }
  
  // Set data state to processing so that progress bar is displayed
  this->data_state_->set( Layer::PROCESSING_C );

  this->reset_abort();
  iso->compute( quality_factor, boost::bind( &Layer::check_abort, this ) );

  this->data_state_->set( Layer::AVAILABLE_C );

  if ( !this->isosurface_ )
  {
    lock_type lock( Layer::GetMutex() );
    this->isosurface_ = iso;
  }
  
  if ( this->show_isosurface_state_->get() )
  {
    this->isosurface_updated_signal_();
  }
  
  // now that we are done, we are going to set the proper 
  this->iso_generated_state_->set( true );
  if( !this->loading_ )
  {
    this->show_isosurface_state_->set( true );
  }
}

void MaskLayer::delete_isosurface()
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &MaskLayer::delete_isosurface, this ) );
    return;
  }

  {
    lock_type lock( Layer::GetMutex() );
    this->isosurface_.reset();
  }

  if ( this->show_isosurface_state_->get() )
  {
    this->isosurface_updated_signal_();
  } 
  
  this->iso_generated_state_->set( false );
}

void MaskLayer::calculate_volume()
{
  Core::MaskDataBlockHandle mask_block = this->get_mask_volume()->
    get_mask_data_block();
    
  size_t voxel_count = 0;
  for( size_t j = 0; j < mask_block->get_size(); ++j )
  {
    if( mask_block->get_mask_at( j ) )
    {
      voxel_count++;
    }
  }
  
  double calculated_mask_volume = ( this->get_grid_transform().spacing_x() * 
    this->get_grid_transform().spacing_y() * this->get_grid_transform().spacing_z() )
    * voxel_count;
    
  this->calculated_volume_state_->set( Core::ExportToString( calculated_mask_volume, size_t( 3 ) ) );
  this->counted_pixels_state_->set( Core::ExportToString( voxel_count ) );  
}

size_t MaskLayer::get_byte_size() const
{
  Layer::lock_type lock( Layer::GetMutex() );
  if ( this->mask_volume_ && this->mask_volume_->is_valid() )
  {
    return this->mask_volume_->get_byte_size();
  }
  return 0;
}

LayerHandle MaskLayer::duplicate() const
{
  LayerHandle layer;

  Core::MaskVolumeHandle mask_volume;
  if ( !( Core::MaskVolume::DuplicateMask( this->get_mask_volume(), mask_volume ) ) )
  {
    // NOTE: return an empty handle
    return layer;
  }
  
  return MaskLayerHandle( new MaskLayer( "Copy-" + this->get_layer_name(), mask_volume ) );
}


size_t MaskLayer::GetColorCount()
{
  boost::mutex::scoped_lock lock( ColorCountMutex );
  return ColorCount;
}

void MaskLayer::SetColorCount( size_t count )
{
  boost::mutex::scoped_lock lock( ColorCountMutex );
  ColorCount = count;
}

} // end namespace Seg3D
