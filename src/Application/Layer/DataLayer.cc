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
#include <limits>

// Boost includes 
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>

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

  DataLayer* layer_;
};

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


DataLayer::DataLayer( const std::string& name, const Core::DataVolumeHandle& volume ) :
  Layer( name, !( volume->is_valid() ) ),
  data_volume_( volume ),
  private_( new DataLayerPrivate )
{
  this->data_volume_->register_data();
  this->private_->layer_ = this;
  this->initialize_states();
}
  
DataLayer::DataLayer( const std::string& state_id ) :
  Layer( "not initialized", state_id ),
  private_( new DataLayerPrivate )
{
  this->private_->layer_ = this;
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

void DataLayer::initialize_states()
{
  // NOTE: This function allows setting of state variables outside of application thread
  this->set_initializing( true ); 

  // == The brightness of the layer ==
  this->add_state( "brightness", brightness_state_, 50.0, 0.0, 100.0, 0.1 );

  // == The contrast of the layer ==
  this->add_state( "contrast", contrast_state_, 0.0, 0.0, 100.0, 0.1 );

  // == Is this volume rendered through the volume renderer ==
  this->add_state( "volume_rendered", volume_rendered_state_, false );
  
  if ( data_volume_ )
  {
    this->generation_state_->set( this->data_volume_->get_generation() );
  }

  this->add_state( "data_type", this->data_type_state_, "unknown" );
  this->add_state( "min", this->min_value_state_, std::numeric_limits< double >::quiet_NaN() );
  this->add_state( "max", this->max_value_state_, std::numeric_limits< double >::quiet_NaN() );

  this->private_->update_data_info();

  this->set_initializing( false );
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
  }

  return true;
} 

bool DataLayer::pre_save_states( Core::StateIO& state_io )
{
  if ( data_volume_ )
  {
    this->generation_state_->set( this->data_volume_->get_generation() );
    
    std::string generation = this->generation_state_->export_to_string() + ".nrrd";
    boost::filesystem::path volume_path = ProjectManager::Instance()->get_project_data_path() /
    generation;
    std::string error;

    bool compress = PreferencesManager::Instance()->compression_state_->get();
    int level = PreferencesManager::Instance()->compression_level_state_->get();
    
    if ( Core::DataVolume::SaveDataVolume( volume_path.string(), 
      this->data_volume_ , error, compress, level ) )
    {
      return true;
    }
    
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}

bool DataLayer::post_load_states( const Core::StateIO& state_io )
{
  if ( this->generation_state_->get() >= 0 )
  {
    std::string generation = this->generation_state_->export_to_string() + ".nrrd";
    boost::filesystem::path volume_path = ProjectManager::Instance()->get_project_data_path() /
    generation;
    std::string error;
    
    if( Core::DataVolume::LoadDataVolume( volume_path, this->data_volume_, error ) )
    {
      this->data_volume_->register_data( this->generation_state_->get() );
      this->private_->update_data_info();
      return true;
    }
    
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
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

} // end namespace Seg3D

