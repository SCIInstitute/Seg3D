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
#include <vector>
#include <fstream>

// Boost Includes
#include <boost/lexical_cast.hpp>

// TinyXML includes
#include <Externals/tinyxml/tinyxml.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Core/Application/Application.h>


namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( PreferencesManager );

PreferencesManager::PreferencesManager() :
  StateHandler( "preferences", false )
{ 
  // Initialize the local config directory path
  Core::Application::Instance()->get_config_directory( local_config_path_ );
  local_config_path_ = local_config_path_ / "user_prefs.cfg";

  if(  initialize_default_colors() )
    this->initialize_states();

  // After we initialize the states, we then load the saved preferences from file.
  load_states( local_config_path_ );
  
}

PreferencesManager::~PreferencesManager()
{
}

void PreferencesManager::save_state()
{
  save_states( local_config_path_ );
}

Core::Color PreferencesManager::get_color( int index ) const
{
  assert( index >= 0 && index < this->color_states_.size() );
  return this->color_states_[ index ]->get();
}

void PreferencesManager::initialize_states()
{
  //General Preferences
  add_state( "project_path", project_path_state_, local_config_path_.string() );
  add_state( "considate_project", considate_project_state_, false );
  add_state( "full_screen_on_startup", full_screen_on_startup_state_, false );
  
  //Viewer Preferences
  add_state( "default_viewer_mode", default_viewer_mode_state_, "one and three", "one|one and one|one and two|one and three|two and two|two and three|three and three" );
  add_state( "grid_size", grid_size_state_, 20 );
  add_state( "background_color", background_color_state_, "black", "black|lightgray|darkgray|gray|fuchsia" );
  add_state( "show_slice_number", show_slice_number_state_, true );
  add_state( "naming_convention", naming_convention_state_, "sagital/axial/coronal", "sagital/axial/coronal|x/y/z" );
  
  //Layers Preferences
  add_state( "default_layer_opacity", default_layer_opacity_state_, 1.0, 0.0, 1.0, 0.01 );
  add_state( "default_mask_fill", default_mask_fill_state_, "striped", "none|striped|solid" );
  add_state( "default_mask_border", default_mask_border_state_, "thick", "none|thin|thick" );
  //add_state( "colors", colors_state_, this->default_colors_ ); 
  
  
  color_states_.resize( 12 );
  
  for ( size_t j = 0; j < 12; j++ )
  {
    std::string stateid = std::string( "color_" ) + Core::ExportToString( j );
    this->add_state( stateid, color_states_[ j ], this->default_colors_[ j ] );
  }
  
  //Interface Controls Preferences
  
  //Sidebars Preferences
  add_state( "show_tools_bar", show_tools_bar_state_, true );
  add_state( "show_layermanager_bar", show_layermanager_bar_state_, true );
  add_state( "show_projectmanager_bar", show_projectmanager_bar_state_, false );
  add_state( "show_measurement_bar", show_measurement_bar_state_, false );
  add_state( "show_history_bar", show_history_bar_state_, false );
  
}


bool PreferencesManager::initialize_default_colors()
{
  this->default_colors_.push_back( Core::Color( 251, 255, 74 ) );
  this->default_colors_.push_back( Core::Color( 248, 188, 37 ) );
  this->default_colors_.push_back( Core::Color( 248, 152, 28 ) );
  this->default_colors_.push_back( Core::Color( 251, 78, 15 ) );
  
  this->default_colors_.push_back( Core::Color( 252, 21, 17 ) );
  this->default_colors_.push_back( Core::Color( 166, 12, 73 ) );
  this->default_colors_.push_back( Core::Color( 135, 0, 172 ) );
  this->default_colors_.push_back( Core::Color( 66, 0, 161 ) );
  
  this->default_colors_.push_back( Core::Color( 45, 66, 250 ) );
  this->default_colors_.push_back( Core::Color( 32, 146, 204 ) );
  this->default_colors_.push_back( Core::Color( 100, 177, 61 ) );
  this->default_colors_.push_back( Core::Color( 205, 235, 66 ) );
  
  return true;
}
  
} // end namespace seg3D
