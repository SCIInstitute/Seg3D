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
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Core/Application/Application.h>


namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( PreferencesManager );

PreferencesManager::PreferencesManager() :
  StateHandler( "PreferencesManager", false )
{ 
  // Initialize the local config directory path
  Core::Application::Instance()->get_config_directory( local_config_path_ );
  local_config_path_ = local_config_path_ / "user_prefs.cfg";

  if(  initialize_default_colors() )
    this->initialize_states();

  // this is just for testing
  this->save_preferences_to_file();
  
}

PreferencesManager::~PreferencesManager()
{
}

Core::Color PreferencesManager::get_color( int index ) const
{
  assert( index >= 0 && index < this->color_states_.size() );
  return this->color_states_[ index ]->get();
}

void PreferencesManager::initialize_states()
{
  //General Preferences
  add_state( "project_path", project_path_state_, "" );
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
  
  add_state( "color_01", color_01_state_, this->default_colors_[0] );
  this->color_states_.push_back( color_01_state_ );
  add_state( "color_02", color_02_state_, this->default_colors_[1] );
  this->color_states_.push_back( color_02_state_ );
  add_state( "color_03", color_03_state_, this->default_colors_[2] );
  this->color_states_.push_back( color_03_state_ );
  add_state( "color_04", color_04_state_, this->default_colors_[3] );
  this->color_states_.push_back( color_04_state_ );
  add_state( "color_05", color_05_state_, this->default_colors_[4] );
  this->color_states_.push_back( color_05_state_ );
  add_state( "color_06", color_06_state_, this->default_colors_[5] );
  this->color_states_.push_back( color_06_state_ );
  add_state( "color_07", color_07_state_, this->default_colors_[6] );
  this->color_states_.push_back( color_07_state_ );
  add_state( "color_08", color_08_state_, this->default_colors_[7] );
  this->color_states_.push_back( color_08_state_ );
  add_state( "color_09", color_09_state_, this->default_colors_[8] );
  this->color_states_.push_back( color_09_state_ );
  add_state( "color_10", color_10_state_, this->default_colors_[9] );
  this->color_states_.push_back( color_10_state_ );
  add_state( "color_11", color_11_state_, this->default_colors_[10] );
  this->color_states_.push_back( color_11_state_ );
  add_state( "color_12", color_12_state_, this->default_colors_[11] );
  this->color_states_.push_back( color_12_state_ );
  
  //Interface Controls Preferences
  
  //Sidebars Preferences
  add_state( "show_tools_bar", show_tools_bar_state_, true );
  add_state( "show_layermanager_bar", show_layermanager_bar_state_, true );
  add_state( "show_projectmanager_bar", show_projectmanager_bar_state_, false );
  add_state( "show_measurement_bar", show_measurement_bar_state_, false );
  add_state( "show_history_bar", show_history_bar_state_, false );
  
}

bool PreferencesManager::save_preferences_to_file()
{
  std::ofstream user_prefs( local_config_path_.string().c_str() );

  //TODO: add xml encoder
    // this is temporary test code to write data to the file
    for( size_t i = 0; i < this->color_states_.size(); ++i )
    {
      user_prefs << this->color_states_[ i ]->export_to_string() << std::endl;
    }
  user_prefs.close();
  return true;
}

bool PreferencesManager::load_preferences_from_file()
{
  std::ifstream user_prefs( local_config_path_.string().c_str() );
  return true;
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
