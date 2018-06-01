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

// STL includes
#include <limits>

// Boost includes 
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>
#include <Core/Utils/ScopedCounter.h>
#include <Core/Utils/Log.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

class DataLayerPrivate
{
public:
  void update_data_info();
  void update_display_value_range();

  void handle_contrast_brightness_changed();
  void handle_display_value_range_changed();

  DataLayer* layer_;
  size_t signal_block_count_;
};

// counter for generating new colors for each new data layer
size_t data_ColorCount = 0;
boost::mutex data_ColorCountMutex;

void DataLayer::initialize_states()
{
	// NOTE: This function allows setting of state variables outside of application thread
	this->set_initializing(true);

	// == Color of the layer ==

	{
		boost::mutex::scoped_lock lock(data_ColorCountMutex);
		this->private_->layer_->add_state("color", this->private_->layer_->color_state_, 
			static_cast<int>(data_ColorCount % PreferencesManager::Instance()->color_states_.size()));

		data_ColorCount++;
	}

	// == The brightness of the layer ==
	this->add_state("brightness", brightness_state_, 50.0, 0.0, 100.0, 0.1);

	// == The contrast of the layer ==
	this->add_state("contrast", contrast_state_, 0.0, 0.0, 100.0, 0.1);

	this->add_state("display_min", this->display_min_value_state_, 0.0, 0.0, 1.0, 1.0);
	this->display_min_value_state_->set_session_priority(Core::StateBase::DO_NOT_LOAD_E);
	this->add_state("display_max", this->display_max_value_state_, 1.0, 0.0, 1.0, 1.0);
	this->display_max_value_state_->set_session_priority(Core::StateBase::DO_NOT_LOAD_E);

	this->add_state("adjust_minmax", this->adjust_display_min_max_state_, false);
    this->add_state("pick_color", this->pick_color_state_, false);

	// == Is this volume rendered through the volume renderer ==
	this->add_state("volume_rendered", volume_rendered_state_, false);

	if (data_volume_)
	{
		this->generation_state_->set(this->data_volume_->get_generation());
	}

	this->add_state("data_type", this->data_type_state_, "unknown");
	this->add_state("min", this->min_value_state_, std::numeric_limits< double >::quiet_NaN());
	this->add_state("max", this->max_value_state_, std::numeric_limits< double >::quiet_NaN());

	this->add_connection(this->contrast_state_->state_changed_signal_.connect(boost::bind(
		&DataLayerPrivate::handle_contrast_brightness_changed, this->private_)));
	this->add_connection(this->brightness_state_->state_changed_signal_.connect(boost::bind(
		&DataLayerPrivate::handle_contrast_brightness_changed, this->private_)));
	this->add_connection(this->display_min_value_state_->state_changed_signal_.connect(boost::bind(
		&DataLayerPrivate::handle_display_value_range_changed, this->private_)));
	this->add_connection(this->display_max_value_state_->state_changed_signal_.connect(boost::bind(
		&DataLayerPrivate::handle_display_value_range_changed, this->private_)));

	this->private_->update_data_info();

	this->set_initializing(false);
}

void DataLayerPrivate::update_data_info()
{
  if ( !this->layer_->data_volume_ ||
    !this->layer_->data_volume_->is_valid() )
  {
    this->layer_->data_type_state_->set( "unknown" );
    this->layer_->min_value_state_->set( std::numeric_limits< double >::quiet_NaN() );
    this->layer_->max_value_state_->set( std::numeric_limits< double >::quiet_NaN() );
    return;
  }
  
  this->layer_->centering_state_->set( 
    this->layer_->get_grid_transform().get_originally_node_centered() ? "node" : "cell" );

  switch ( this->layer_->get_data_type() )
  {
  case Core::DataType::CHAR_E:
    this->layer_->data_type_state_->set( "char" );
    break;
  case Core::DataType::UCHAR_E:
    this->layer_->data_type_state_->set( "unsigned char" );
    break;
  case Core::DataType::SHORT_E:
    this->layer_->data_type_state_->set( "short" );
    break;
  case Core::DataType::USHORT_E:
    this->layer_->data_type_state_->set( "unsigned short" );
    break;
  case Core::DataType::INT_E:
    this->layer_->data_type_state_->set( "int" );
    break;
  case Core::DataType::UINT_E:
    this->layer_->data_type_state_->set( "unsigned int" );
    break;
  case Core::DataType::FLOAT_E:
    this->layer_->data_type_state_->set( "float" );
    break;
  case Core::DataType::DOUBLE_E:
    this->layer_->data_type_state_->set( "double" );
    break;
  }
  
  this->layer_->min_value_state_->set( this->layer_->data_volume_->get_min() );
  this->layer_->max_value_state_->set( this->layer_->data_volume_->get_max() );
}

void DataLayerPrivate::update_display_value_range()
{
  if ( !this->layer_->has_valid_data() )  return;
  
  {
    Core::ScopedCounter signal_block( this->signal_block_count_ );
    double min_val = this->layer_->get_data_volume()->get_min();
    double max_val = this->layer_->get_data_volume()->get_max();
    this->layer_->display_min_value_state_->set_range( min_val, max_val );
    this->layer_->display_max_value_state_->set_range( min_val, max_val );
  }

  this->handle_contrast_brightness_changed();
}

void DataLayerPrivate::handle_contrast_brightness_changed()
{
  if ( this->signal_block_count_ > 0 || !this->layer_->has_valid_data() )
  {
    return;
  }
  
  Core::ScopedCounter signal_block( this->signal_block_count_ );

  // Convert contrast to range ( 0, 1 ] and brightness to [ 0, 2 ]
  double contrast = ( 1 - this->layer_->contrast_state_->get() / 101 );
  double brightness = this->layer_->brightness_state_->get() / 50.0;

  double min_val, max_val;
  this->layer_->display_min_value_state_->get_range( min_val, max_val );
  double mid_val = max_val - brightness * 0.5 * ( max_val - min_val );
  double window_size = ( max_val - min_val ) * contrast;
  this->layer_->display_min_value_state_->set( mid_val - window_size * 0.5 );
  this->layer_->display_max_value_state_->set( mid_val + window_size * 0.5 );
}

void DataLayerPrivate::handle_display_value_range_changed()
{
  if ( this->signal_block_count_ > 0 || !this->layer_->has_valid_data() )
  {
    return;
  }

  Core::ScopedCounter signal_block( this->signal_block_count_ );

  double min_val, max_val;
  this->layer_->display_min_value_state_->get_range( min_val, max_val );
  double display_min = this->layer_->display_min_value_state_->get();
  double display_max = this->layer_->display_max_value_state_->get();
  if ( display_min > display_max )
  {
    std::swap( display_min, display_max );
  }
  
  double contrast = 1.0 - ( display_max - display_min ) / ( max_val - min_val );
  double brightness = ( max_val * 2 - display_min - display_max ) / ( max_val - min_val );
  this->layer_->contrast_state_->set( contrast * 100 );
  this->layer_->brightness_state_->set( brightness * 50 );
}


DataLayer::DataLayer( const std::string& name, const Core::DataVolumeHandle& volume ) :
  Layer( name, !( volume->is_valid() ) ),
  data_volume_( volume ),
  private_( new DataLayerPrivate )
{
  this->data_volume_->register_data();
  this->private_->layer_ = this;
  this->private_->signal_block_count_ = 0;
  this->initialize_states();
  this->private_->update_display_value_range();
}
  
DataLayer::DataLayer( const std::string& state_id ) :
  Layer( "not initialized", state_id ),
  private_( new DataLayerPrivate )
{
  this->private_->layer_ = this;
  this->private_->signal_block_count_ = 0;
  this->initialize_states();
}

DataLayer::~DataLayer()
{
  // Disconnect all current connections
  this->disconnect_all();
  if ( this->data_volume_ )
  {
    this->data_volume_->unregister_data();
  }
}

Core::GridTransform DataLayer::get_grid_transform() const 
{ 
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->data_volume_ )
  {
    return this->data_volume_->get_grid_transform(); 
  }
  else
  {
    return Core::GridTransform();
  }
}

void DataLayer::set_grid_transform( const Core::GridTransform& grid_transform, 
  bool preserve_centering )
{
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->data_volume_ )
  {
    this->data_volume_->set_grid_transform( grid_transform, preserve_centering ); 
  }
}

Core::DataType DataLayer::get_data_type() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->data_volume_ )
  {
    return this->data_volume_->get_data_type();
  }
  
  return Core::DataType::UNKNOWN_E;
}

Core::DataVolumeHandle DataLayer::get_data_volume() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  return this->data_volume_;
}

size_t DataLayer::get_byte_size() const
{
  Layer::lock_type lock( Layer::GetMutex() );
  if ( this->data_volume_ && this->data_volume_->is_valid() )
  {
    return this->data_volume_->get_byte_size();
  }
  return 0;
}

bool DataLayer::has_valid_data() const
{
  Layer::lock_type lock( Layer::GetMutex() );

  if ( this->data_volume_ )
  {
    return this->data_volume_->is_valid();
  }
  else
  {
    return false;
  }
}

Core::VolumeHandle DataLayer::get_volume() const
{
  return this->get_data_volume();
}

bool DataLayer::set_data_volume( Core::DataVolumeHandle data_volume )
{ 
  ASSERT_IS_APPLICATION_THREAD();

  // Only insert the volume if the layer is still valid
  if ( !this->is_valid() )  return false;
  
  {
    Layer::lock_type lock( Layer::GetMutex() );

    if ( this->data_volume_ )
    {
      // Unregister the old volume
      this->data_volume_->unregister_data();
    }
    
    this->data_volume_ = data_volume; 

    if ( this->data_volume_ )
    {
      // Register the new volume
      this->data_volume_->register_data();
      this->generation_state_->set( this->data_volume_->get_generation() );
    }

    this->private_->update_data_info();
    this->private_->update_display_value_range();
  }

  return true;
} 

bool DataLayer::pre_save_states( Core::StateIO& state_io )
{
  if ( this->data_volume_ )
  {
    long long generation_number = this->data_volume_->get_generation();
    this->generation_state_->set( generation_number );

    // Add the number to the project so it can be recorded into the session database
    ProjectManager::Instance()->get_current_project()->add_generation_number( generation_number );
    
    std::string data_file_name = this->generation_state_->export_to_string() + ".nrrd";
    boost::filesystem::path full_data_file_name = ProjectManager::Instance()->
      get_current_project()->get_project_data_path() / data_file_name;
    
    if ( boost::filesystem::exists( full_data_file_name ) )
    {
      // File has already been saved
      return true;
    }
      
    bool compress = PreferencesManager::Instance()->compression_state_->get();
    int level = PreferencesManager::Instance()->compression_level_state_->get();
    
    std::string error;
    if ( ! Core::DataVolume::SaveDataVolume( full_data_file_name.string(), this->data_volume_, 
      error, compress, level ) )
    {
      CORE_LOG_ERROR( error );
      return false;   
    }

    return true;
  }
  
  return true;
}

bool DataLayer::post_load_states( const Core::StateIO& state_io )
{
  if ( this->generation_state_->get() >= 0 )
  {
    std::string generation = this->generation_state_->export_to_string() + ".nrrd";
    boost::filesystem::path volume_path = ProjectManager::Instance()->get_current_project()->
      get_project_data_path() / generation;
    std::string error;
    
    if( Core::DataVolume::LoadDataVolume( volume_path, this->data_volume_, error ) )
    {
      this->data_volume_->register_data( this->generation_state_->get() );
      this->private_->update_data_info();
      this->private_->update_display_value_range();

      // If the layer didn't have a valid provenance ID, generate one
      if ( this->provenance_id_state_->get() < 0 )
      {
        this->provenance_id_state_->set( GenerateProvenanceID() );
      }

      return true;
    }
    CORE_LOG_ERROR( error );
  }

  return false;
}
  
void DataLayer::clean_up()
{
  // Abort any filter still using this layer
  this->abort_signal_();
  
  // Clean up the data that is still associated with this layer
  {
    Layer::lock_type lock( Layer::GetMutex() );
    if ( this->data_volume_ ) 
    {
      this->data_volume_->unregister_data();
      Core::DataVolume::CreateInvalidData( this->data_volume_->get_grid_transform(), 
        this->data_volume_ );
    }
  }
  
  // Remove all the connections
  this->disconnect_all();   
}

LayerHandle DataLayer::duplicate() const
{
  LayerHandle layer;

  Core::DataVolumeHandle data_volume;
  if ( !( Core::DataVolume::DuplicateVolume( this->get_data_volume(), data_volume ) ) )
  {
    // NOTE: return an empty handle
    return layer;
  }
  
  return DataLayerHandle( new DataLayer( "Copy_" + this->get_layer_name(), data_volume ) );

}

size_t DataLayer::GetColorCount()
{
	boost::mutex::scoped_lock lock(data_ColorCountMutex);
	return data_ColorCount;
}

void DataLayer::SetColorCount(size_t count)
{
	boost::mutex::scoped_lock lock(data_ColorCountMutex);
	data_ColorCount = count;
}

} // end namespace Seg3D

