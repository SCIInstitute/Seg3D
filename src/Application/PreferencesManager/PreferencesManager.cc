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

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>


namespace Seg3D
{

CORE_SINGLETON_IMPLEMENTATION( PreferencesManager );

PreferencesManager::PreferencesManager() :
  StateHandler( "preferences", false )
{ 
  this->set_initializing( true );

  // Initialize the local config directory path
  Core::Application::Instance()->get_config_directory( this->local_config_path_ );

  if( initialize_default_colors() )
    this->initialize_states();

  // After we initialize the states, we then load the saved preferences from file.
  this->initialize();
  this->set_initializing( false );
}

PreferencesManager::~PreferencesManager()
{
}

void PreferencesManager::initialize()
{
  Core::StateIO state_io;
  if ( state_io.import_from_file( this->local_config_path_ / "preferences.xml" ) )
  {
    this->load_states( state_io );
  }
}

void PreferencesManager::save_state()
{
  Core::StateIO state_io;
  state_io.initialize( "Seg3D2" );
  this->save_states( state_io );
  state_io.export_to_file( this->local_config_path_ / "preferences.xml" );
}

Core::Color PreferencesManager::get_color( int index ) const
{
  assert( index >= 0 && index < static_cast< int >( this->color_states_.size() ) );
  return this->color_states_[ index ]->get();
}

void PreferencesManager::initialize_states()
{
  boost::filesystem::path user_path;
  Core::Application::Instance()->get_user_directory( user_path );
  user_path = user_path / "Seg3D-Projects";

  //General Preferences
  add_state( "project_path", project_path_state_, user_path.string() );
  add_state( "full_screen_on_startup", full_screen_on_startup_state_, false );
  add_state( "auto_save", auto_save_state_, true );
  add_state( "auto_save_time", auto_save_time_state_, 15, 1, 120, 1 );
  add_state( "smart_save", smart_save_state_, true );

  
  //Viewer Preferences
  add_state( "default_viewer_mode", default_viewer_mode_state_, "1and3", 
    "single|1and1|1and2|1and3|2and2|2and3|3and3" );
  add_state( "grid_size", grid_size_state_, 20 );
  add_state( "background_color", background_color_state_, "black", 
    "black|lightgray|darkgray|gray|fuchsia" );
  add_state( "show_slice_number", show_slice_number_state_, true );
  
  //Layers Preferences
  add_state( "default_layer_opacity", default_layer_opacity_state_, 1.0, 0.0, 1.0, 0.01 );
  add_state( "default_mask_fill", default_mask_fill_state_, "striped", "none|striped|solid" );
  add_state( "default_mask_border", default_mask_border_state_, "thick", "none|thin|thick" );
    
  this->color_states_.resize( 12 );
  for ( size_t j = 0; j < 12; j++ )
  {
    std::string stateid = std::string( "color_" ) + Core::ExportToString( j );
    this->add_state( stateid, color_states_[ j ], this->default_colors_[ j ] );
  }
  
  //Interface Controls Preferences
  
  //Sidebars Preferences
  add_state( "show_tools_bar", show_tools_bar_state_, true );
  add_state( "show_layermanager_bar", show_layermanager_bar_state_, true );
  add_state( "show_projectmanager_bar", show_projectmanager_bar_state_, true );
  add_state( "show_measurement_bar", show_measurement_bar_state_, false );
  add_state( "show_history_bar", show_history_bar_state_, false );
  
}


bool PreferencesManager::initialize_default_colors()
{
  this->default_colors_.push_back( Core::Color( 106, 22, 102 ) );
  this->default_colors_.push_back( Core::Color( 198, 0, 34 ) );
  this->default_colors_.push_back( Core::Color( 210, 70, 38 ) );
  this->default_colors_.push_back( Core::Color( 221, 120, 47 ) );
  
  this->default_colors_.push_back( Core::Color( 237, 177, 53 ) );
  this->default_colors_.push_back( Core::Color( 241, 235, 69 ) );
  this->default_colors_.push_back( Core::Color( 112, 181, 66 ) );
  this->default_colors_.push_back( Core::Color( 21, 143, 67 ) );
  
  this->default_colors_.push_back( Core::Color( 33, 131, 130 ) );
  this->default_colors_.push_back( Core::Color( 41, 93, 153 ) );
  this->default_colors_.push_back( Core::Color( 28, 60, 116 ) );
  this->default_colors_.push_back( Core::Color( 35, 31, 98 ) );
  
  return true;
}
  
} // end namespace seg3D
