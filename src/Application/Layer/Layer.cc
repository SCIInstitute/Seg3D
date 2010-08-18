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
#include <Core/Utils/AtomicCounter.h>
#include <Core/Utils/StringUtil.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

const std::string Layer::CREATING_C( "creating" );
const std::string Layer::PROCESSING_C( "processing" );
const std::string Layer::AVAILABLE_C( "available" );
const std::string Layer::IN_USE_C( "inuse" );

const std::string Layer::NO_MENU_C( "none" );
const std::string Layer::OPACITY_MENU_C( "opacity" );
const std::string Layer::COLOR_MENU_C( "color" );
const std::string Layer::CONTRAST_MENU_C( "contrast" );
const std::string Layer::APPEARANCE_MENU_C( "appearance" );

Layer::Layer( const std::string& name, bool creating ) :
  StateHandler( "layer",  true )
{ 
  this->initialize_states( name, creating );
}

Layer::Layer( const std::string& name, const std::string& state_id, bool creating ) :
  StateHandler( state_id, false )
{
  this->initialize_states( name, creating );
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
  
void Layer::set_layer_group( LayerGroupWeakHandle layer_group )
{
  this->layer_group_ = layer_group;
}

std::string Layer::get_layer_id() const
{
  return this->get_statehandler_id();
}
  
std::string Layer::get_layer_name() const
{
  return this->name_state_->get();
}
  
Core::DataBlock::generation_type Layer::get_generation() const
{
  return this->generation_state_->get();
}

Layer::mutex_type& Layer::GetMutex()
{
  return Core::StateEngine::GetMutex();
}

void Layer::initialize_states( const std::string& name, bool creating )
{
  // Step (1) : Build the layer specific state variables

  // == The name of the layer ==
  this->add_state( "name", name_state_, name );

  // == Visibility information for this layer per viewer ==
  this->visible_state_.resize( ViewerManager::Instance()->number_of_viewers() );
  for ( size_t j = 0; j < this->visible_state_.size(); j++ )
  {
    this->add_state( std::string( "visible" ) + Core::ExportToString( j ), 
      this->visible_state_[ j ], true );
  }

  // == The state of the lock ==
  this->add_state( "visual_lock", visual_lock_state_, false );

  // == The opacity of the layer ==
  this->add_state( "opacity", opacity_state_, 
    PreferencesManager::Instance()->default_layer_opacity_state_->get(), 0.0, 1.0, 0.01 );

  // == Selected by the LayerGroup ==
  this->add_state( "selected", selected_state_, false );

  // == Which of the sub-menus is being edited ==
  this->add_state( "menu", menu_state_, NO_MENU_C, NO_MENU_C + "|" + OPACITY_MENU_C + 
    "|" + COLOR_MENU_C + "|" + CONTRAST_MENU_C + "|" + APPEARANCE_MENU_C );

  // == The generation number of the data, or -1 if there is no data =
  this->add_state( "generation", this->generation_state_, -1 );

  // == The last action that was run on this layer ==
  this->add_state( "last_action", this->last_action_state_, "" );
  
  // == The layer state indicating whether data is bein processed ==
  this->add_state( "data", this->data_state_,  creating ? CREATING_C : AVAILABLE_C  , 
    AVAILABLE_C + "|" + CREATING_C + "|" + PROCESSING_C + "|" + IN_USE_C );
}

bool Layer::post_save_states( Core::StateIO& state_io )
{
  TiXmlElement* layer_element = state_io.get_current_element();
  assert( this->get_statehandler_id() == layer_element->Value() );
  std::string layer_type;
  switch ( this->type() )
  {
  case Core::VolumeType::DATA_E:
    layer_type = "data";
    break;
  case Core::VolumeType::MASK_E:
    layer_type = "mask";
    break;
  case Core::VolumeType::LABEL_E:
    layer_type = "label";
    break;
  default:
    CORE_LOG_ERROR( "Unknow layer type" );
    assert( false );
  }
  
  layer_element->SetAttribute( "type", layer_type );
  return true;
}


} // end namespace Seg3D

