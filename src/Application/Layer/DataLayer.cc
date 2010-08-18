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
#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Layer/DataLayer.h>

namespace Seg3D
{

DataLayer::DataLayer( const std::string& name, const Core::DataVolumeHandle& volume ) :
  Layer( name, !( volume->is_valid() ) ),
  data_volume_( volume )
{
  this->initialize_states();
}
  
DataLayer::DataLayer( const std::string& state_id ) :
  Layer( "not initialized", state_id )
{
  this->initialize_states();
}


DataLayer::~DataLayer()
{
  // Disconnect all current connections
  disconnect_all();
}

void DataLayer::initialize_states()
{
  // NOTE: This function allows setting of state variables outside of application thread
  this->set_initializing( true ); 

  // == The brightness of the layer ==
  add_state( "brightness", brightness_state_, 50.0, 0.0, 100.0, 0.1 );

  // == The contrast of the layer ==
  add_state( "contrast", contrast_state_, 0.0, 0.0, 100.0, 0.1 );

  // == Is this volume rendered through the volume renderer ==
  add_state( "volume_rendered", volume_rendered_state_, false );
  
  if ( data_volume_ )
  {
    this->generation_state_->set( this->data_volume_->get_generation() );
  }
  
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

Core::DataVolumeHandle DataLayer::get_data_volume()
{
  Layer::lock_type lock( Layer::GetMutex() );

  return this->data_volume_;
}

bool DataLayer::is_valid() const
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

void DataLayer::set_data_volume( Core::DataVolumeHandle data_volume )
{ 
  Layer::lock_type lock( Layer::GetMutex() );
  
  this->data_volume_ = data_volume; 

  if ( this->data_volume_ )
  {
    this->generation_state_->set( this->data_volume_->get_generation() );
  }

  this->update_volume_signal_();
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
    
    if ( Core::DataVolume::SaveDataVolume( volume_path.string(), this->data_volume_ , error ) )
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
      return true;
    }
    
    CORE_LOG_ERROR( error );
    return false;
  }
  
  return true;
}
  
void DataLayer::clean_up()
{
  this->data_volume_.reset();
}


} // end namespace Seg3D

