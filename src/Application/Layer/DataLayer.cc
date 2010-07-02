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

// Application includes
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/Layer/DataLayer.h>

namespace Seg3D
{

const size_t DataLayer::VERSION_NUMBER_C = 1;

DataLayer::DataLayer( const std::string& name, const Core::DataVolumeHandle& volume ) :
  Layer( name, VERSION_NUMBER_C ),
  data_volume_( volume )
{
  this->initialize_states();
  
}
  
DataLayer::DataLayer( const std::string& state_id ) :
  Layer( "not_initialized", VERSION_NUMBER_C, state_id )
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

  // Step (1) : Build the layer specific state variables

  // == The brightness of the layer ==
  add_state( "brightness", brightness_state_, 50.0, 0.0, 100.0, 0.1 );

  // == The contrast of the layer ==
  add_state( "contrast", contrast_state_, 0.0, 0.0, 100.0, 0.1 );

  // == Is this volume rendered through the volume renderer ==
  add_state( "volume_rendered", volume_rendered_state_, false );

}

  
bool DataLayer::pre_save_states()
{
  this->generation_state_->set( static_cast< int >( this->data_volume_->get_generation() ) );
  
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

bool DataLayer::post_load_states()
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
  
void DataLayer::clean_up()
{
  this->data_volume_.reset();
}


} // end namespace Seg3D

