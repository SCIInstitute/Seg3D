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
#include <Core/Utils/AtomicCounter.h>
#include <Core/Utils/StringUtil.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

Layer::Layer( const std::string& name, size_t version_number ) :
  StateHandler( "layer", version_number, true )
{ 
  this->initialize_states( name );
  this->is_moving_ = false;
    
  // == Resource locking ==
  resource_lock_ = Core::ResourceLockHandle( new Core::ResourceLock( get_statehandler_id() ) );
}

Layer::Layer( const std::string& name, size_t version_number, const std::string& state_id ) :
  StateHandler( state_id, version_number, false )
{
  this->initialize_states( name );
  this->is_moving_ = false;

  // == Resource locking ==
  resource_lock_ = Core::ResourceLockHandle( new Core::ResourceLock( get_statehandler_id() ) );
}
  
Layer::~Layer()
{
  // Disconnect all current connections
  this->disconnect_all();
}

LayerGroupHandle Layer::get_layer_group() 
{ 
  return this->layer_group_.lock(); 
}

Core::ResourceLockHandle Layer::get_resource_lock()
{
  return this->resource_lock_;
}

  
void Layer::set_layer_group( LayerGroupWeakHandle layer_group )
{
  this->layer_group_ = layer_group;
}
    
// bool Layer::get_active()
// {
//  return this->active_state_->get();
// }
//  
// void Layer::set_active( bool active )
// {
//  this->active_state_->set( active );
// }

std::string Layer::get_layer_id() const
{
  return this->get_statehandler_id();
}
  
std::string Layer::get_layer_name() const
{
  return this->name_state_->get();
}

Layer::mutex_type& Layer::GetMutex()
{
  return Core::StateEngine::GetMutex();
}

void Layer::initialize_states( const std::string& name )
{
  // Step (1) : Build the layer specific state variables

  // == The name of the layer ==
  this->add_state( "name", name_state_, name );

  // == Visibility information for this layer per viewer ==
  size_t num_viewers = ViewerManager::Instance()->number_of_viewers();
  this->visible_state_.resize( num_viewers );

  for ( size_t j = 0; j < visible_state_.size(); j++ )
  {
    std::string key = std::string( "visible" ) + Core::ExportToString( j );
    this->add_state( key, visible_state_[ j ], true );
  }

  // == The state of the lock ==
  this->add_state( "lock", lock_state_, false );

  // == The opacity of the layer ==
  this->add_state( "opacity", opacity_state_, PreferencesManager::Instance()->default_layer_opacity_state_->get(), 0.0, 1.0, 0.01 );

  // == Selected by the LayerGroup ==
  this->add_state( "selected", selected_state_, false );

//  // == Selected by the LayerGroup ==
//  this->add_state( "active", active_state_, false );

  // == Which of the submenus is being editted ==
  this->add_state( "edit_mode", edit_mode_state_, "none", "none|opacity|color|contrast|appearance" );

  this->add_state( "generation", this->generation_state_, -1 );
}




} // end namespace Seg3D

