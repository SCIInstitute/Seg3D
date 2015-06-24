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
#include <Core/Utils/AtomicCounter.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Log.h>
#include <Core/Volume/MaskVolume.h>

// Application includes
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/ProjectManager.h>


namespace Seg3D
{

class MaskLayerPrivate 
{
  // -- internal functions --
public:
  void initialize_states();
  void handle_mask_data_changed();
  void handle_isosurface_update_progress( double progress );
  void update_mask_info();

  // Extra private state information
  // NOTE: This used for saving the bit that is used in a mask to a session file. As the state
  // variables are read first, this will allow for reconstructing which data block and which bit
  // need to be loaded.
  Core::StateIntHandle   bit_state_;

  // Information about two components not included in the state manager.
  Core::MaskVolumeHandle mask_volume_;
  Core::IsosurfaceHandle isosurface_;

  MaskLayer * layer_;
};

// counter for generating new colors for each new mask
size_t ColorCount = 0;
boost::mutex ColorCountMutex;

void MaskLayerPrivate::initialize_states()
{
  // == Color of the layer ==

  {
    boost::mutex::scoped_lock lock( ColorCountMutex );
    this->layer_->add_state( "color", this->layer_->color_state_, static_cast< int >( ColorCount % 
      PreferencesManager::Instance()->color_states_.size() ) );

    ColorCount++;
  }

  // == What border is used ==
  this->layer_->add_state( "border", this->layer_->border_state_, PreferencesManager::Instance()->
    default_mask_border_state_->export_to_string(), PreferencesManager::Instance()->
    default_mask_border_state_->export_list_to_string() );

  // == How is the segmentation filled in ==
  this->layer_->add_state( "fill", this->layer_->fill_state_, PreferencesManager::Instance()->
    default_mask_fill_state_->export_to_string(), PreferencesManager::Instance()->
    default_mask_fill_state_->export_list_to_string() );

  // == Whether the isosurface is shown in the volume display ==
  this->layer_->add_state( "isosurface", this->layer_->show_isosurface_state_, false );

  // == Internal information for keeping track of which bit we are using ==
  this->layer_->add_state( "bit", this->bit_state_, 0 );

  // == Keep track of whether the isosurface has been generated
  this->layer_->add_state( "iso_generated", this->layer_->iso_generated_state_, false );

  this->layer_->add_state( "isosurface_area", this->layer_->isosurface_area_state_, 0.0 );
  this->layer_->isosurface_area_state_->set_session_priority( Core::StateBase::DO_NOT_LOAD_E );

  // == Keep track of the calculated volume and put it in the UI
  this->layer_->add_state( "calculated_volume", this->layer_->calculated_volume_state_, "N/A" );

  // == Keep track of the calculated volume and put it in the UI
  this->layer_->add_state( "counted_pixels", this->layer_->counted_pixels_state_, "N/A" );

  this->layer_->add_state( "min", this->layer_->min_value_state_, std::numeric_limits< double >::quiet_NaN() );
  this->layer_->add_state( "max", this->layer_->max_value_state_, std::numeric_limits< double >::quiet_NaN() );

  this->update_mask_info();
}

void MaskLayerPrivate::handle_mask_data_changed()
{
  this->layer_->calculated_volume_state_->set( "N/A" );
  this->layer_->counted_pixels_state_->set( "N/A" );
  this->layer_->layer_updated_signal_();
}

void MaskLayerPrivate::handle_isosurface_update_progress( double progress )
{
  this->layer_->update_progress_signal_( progress );
}

void MaskLayerPrivate::update_mask_info()
{
  this->layer_->centering_state_->set( 
    this->layer_->get_grid_transform().get_originally_node_centered() ? "node" : "cell" );

  if (! this->mask_volume_ ||
      ! this->mask_volume_->is_valid() )
  {
    this->layer_->min_value_state_->set( std::numeric_limits< double >::quiet_NaN() );
    this->layer_->max_value_state_->set( std::numeric_limits< double >::quiet_NaN() );
    return;
  }
  
  this->layer_->min_value_state_->set( this->mask_volume_->get_min() );
  this->layer_->max_value_state_->set( this->mask_volume_->get_max() );
}

MaskLayer::MaskLayer( const std::string& name, const Core::MaskVolumeHandle& volume ) :
  Layer( name, !( volume->is_valid() ) ), 
  private_( new MaskLayerPrivate )
{
  this->private_->mask_volume_ = volume;
  this->private_->mask_volume_->register_data();
  this->private_->layer_ = this;
  
  this->private_->initialize_states();
  
  if (  volume->get_mask_data_block() )
  {
    this->private_->bit_state_->set( static_cast< int >( volume->get_mask_data_block()->get_mask_bit() ) );
    this->add_connection( this->private_->mask_volume_->get_mask_data_block()->mask_updated_signal_.
      connect( boost::bind( &MaskLayerPrivate::handle_mask_data_changed, this->private_ ) ) );
  }
}

MaskLayer::MaskLayer( const std::string& state_id ) :
  Layer( "not_initialized", state_id ),
  private_( new MaskLayerPrivate )
{
  this->private_->layer_ = this;
  this->private_->initialize_states();
}

MaskLayer::~MaskLayer()
{
  // Disconnect all current connections
  this->disconnect_all();

  if ( this->private_->mask_volume_ )
  {
    this->private_->mask_volume_->unregister_data();
  }
}

Core::GridTransform MaskLayer::get_grid_transform() const 
{ 
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->private_->mask_volume_ )
  {
    return this->private_->mask_volume_->get_grid_transform(); 
  }
  else
  {
    return Core::GridTransform();
  }
}

void MaskLayer::set_grid_transform( const Core::GridTransform& grid_transform, 
  bool preserve_centering )
{
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->private_->mask_volume_ )
  {
    this->private_->mask_volume_->set_grid_transform( grid_transform, preserve_centering ); 
  }
}

Core::DataType MaskLayer::get_data_type() const
{
  return Core::DataType::UCHAR_E;
}

Core::MaskVolumeHandle MaskLayer::get_mask_volume() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  return this->private_->mask_volume_;
}

bool MaskLayer::has_valid_data() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->private_->mask_volume_ )
  {
    return this->private_->mask_volume_->is_valid();
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

    if ( this->private_->mask_volume_ )
    {
      // Unregister the old volume
      this->private_->mask_volume_->unregister_data();
    }
    
    // delete the isosurface
    this->private_->isosurface_.reset();

    this->disconnect_all();

    this->private_->mask_volume_ = volume;
    if ( this->private_->mask_volume_ )
    {
      // Register the new volume
      this->private_->mask_volume_->register_data();
      this->generation_state_->set( this->private_->mask_volume_->get_generation() );
      this->private_->bit_state_->set( static_cast< int >( 
        volume->get_mask_data_block()->get_mask_bit() ) );

      this->add_connection( this->private_->mask_volume_->get_mask_data_block()->mask_updated_signal_.
        connect( boost::bind( &MaskLayerPrivate::handle_mask_data_changed, this->private_ ) ) );
    }

    this->private_->update_mask_info();
  }

  return true;
}

bool MaskLayer::pre_save_states( Core::StateIO& state_io )
{
  long long generation_number = this->get_mask_volume()->get_generation();
  this->generation_state_->set( generation_number );

  // Add the number to the project so it can be recorded into the session database
  ProjectManager::Instance()->get_current_project()->add_generation_number( generation_number );
  
  boost::filesystem::path data_file = ProjectManager::Instance()->get_current_project()->
    get_project_data_path() / ( this->generation_state_->export_to_string() + ".nrrd" );
  if ( boost::filesystem::exists( data_file ) )
  {
    // File has already been saved
    return true;
  }
  
  bool compress = PreferencesManager::Instance()->compression_state_->get();
  int level = PreferencesManager::Instance()->compression_level_state_->get();

  Core::DataBlockHandle data_block = this->get_mask_volume()->
    get_mask_data_block()->get_data_block();
  Core::NrrdDataHandle nrrd( new Core::NrrdData( data_block, this->get_grid_transform() ) );

  std::string error;
  Core::DataBlock::shared_lock_type slock( data_block->get_mutex() );
  if ( !Core::NrrdData::SaveNrrd( data_file.string(), nrrd, error, compress, level ) )
  {
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}

bool MaskLayer::post_load_states( const Core::StateIO& state_io )
{
  Core::DataBlock::generation_type generation = this->generation_state_->get();
  unsigned int bit = static_cast< unsigned int >( this->private_->bit_state_->get() );
  Core::MaskDataBlockHandle mask_data_block;
  Core::GridTransform grid_transform;
  bool success = Core::MaskDataBlockManager::Instance()->
    create( generation, bit, grid_transform, mask_data_block );
  if ( !success )
  {
    Core::DataVolumeHandle data_volume;
    boost::filesystem::path volume_path = ProjectManager::Instance()->get_current_project()->
      get_project_data_path() / ( this->generation_state_->export_to_string() + ".nrrd" );
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
    this->private_->mask_volume_ = Core::MaskVolumeHandle( new Core::MaskVolume( 
      grid_transform, mask_data_block ) );
    this->add_connection( this->private_->mask_volume_->get_mask_data_block()->mask_updated_signal_.
      connect( boost::bind( &MaskLayerPrivate::handle_mask_data_changed, this->private_ ) ) );
    this->private_->update_mask_info();
  }

  // If the layer didn't have a valid provenance ID, generate one
  if ( this->provenance_id_state_->get() < 0 )
  {
    this->provenance_id_state_->set( GenerateProvenanceID() );
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
    if ( this->private_->mask_volume_ )
    {
      this->private_->mask_volume_->unregister_data();
      Core::MaskVolume::CreateInvalidMask( this->private_->mask_volume_->get_grid_transform(),
        this->private_->mask_volume_ );
    }
    this->private_->isosurface_.reset();
    this->iso_generated_state_->set( false );
  }

  // Remove all the connections
  this->disconnect_all();
}

Core::IsosurfaceHandle MaskLayer::get_isosurface()
{
  lock_type lock( Layer::GetMutex() );
  return this->private_->isosurface_;
}

void MaskLayer::compute_isosurface( double quality_factor, bool capping_enabled )
{
  if ( !Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &MaskLayer::compute_isosurface, 
      this, quality_factor, capping_enabled ) );
    return;
  }
  
  Core::IsosurfaceHandle iso = this->private_->isosurface_;
  if ( !iso )
  {
    iso.reset( new Core::Isosurface( this->private_->mask_volume_ ) );
    this->add_connection( iso->update_progress_signal_.connect(
      boost::bind( &MaskLayerPrivate::handle_isosurface_update_progress, this->private_, _1 ) ) );
  }
  
  // Set data state to processing so that progress bar is displayed
  this->data_state_->set( Layer::PROCESSING_C );

  this->reset_abort();
  iso->compute( quality_factor, capping_enabled, boost::bind( &Layer::check_abort, this ) );

  this->data_state_->set( Layer::AVAILABLE_C );
  this->isosurface_area_state_->set( iso->surface_area() );

  if ( !this->private_->isosurface_)
  {
    lock_type lock( Layer::GetMutex() );
    this->private_->isosurface_ = iso;
  }
  
  if ( this->show_isosurface_state_->get() )
  {
    this->isosurface_updated_signal_();
  }

  this->iso_generated_state_->set( true );
  LayerManager::Instance()->mask_layer_isosurface_created_signal_();
}

void MaskLayer::delete_isosurface()
{
  if ( ! Core::Application::IsApplicationThread() )
  {
    Core::Application::PostEvent( boost::bind( &MaskLayer::delete_isosurface, this ) );
    return;
  }

  {
    lock_type lock( Layer::GetMutex() );
    this->private_->isosurface_.reset();
  }

  if ( this->show_isosurface_state_->get() )
  {
    this->isosurface_updated_signal_();
  } 
  
  this->iso_generated_state_->set( false );

  LayerManager::Instance()->mask_layer_isosurface_deleted_signal_();
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
  if ( this->private_->mask_volume_ && this->private_->mask_volume_->is_valid() )
  {
    return this->private_->mask_volume_->get_byte_size();
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
  
  return MaskLayerHandle( new MaskLayer( "Copy_" + this->get_layer_name(), mask_volume ) );
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
