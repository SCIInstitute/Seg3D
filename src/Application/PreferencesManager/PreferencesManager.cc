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
#include <vector>

#include <boost/filesystem.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/State/StateIO.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class PreferencesManagerPrivate
//////////////////////////////////////////////////////////////////////////
  
class PreferencesManagerPrivate
{
public:
  void handle_axis_labels_option_changed( std::string option );

  std::vector< Core::Color > default_colors_;
  boost::filesystem::path local_config_path_;
  PreferencesManager* pm_;
};

void PreferencesManagerPrivate::handle_axis_labels_option_changed( std::string option )
{
  if ( option == "sca" )
  {
    this->pm_->x_axis_label_state_->set( "Sagittal" );
    this->pm_->y_axis_label_state_->set( "Coronal" );
    this->pm_->z_axis_label_state_->set( "Axial" );
  }
  else if ( option == "sct" )
  {
    this->pm_->x_axis_label_state_->set( "Sagittal" );
    this->pm_->y_axis_label_state_->set( "Coronal" );
    this->pm_->z_axis_label_state_->set( "Transverse" );
  }
  else if ( option == "xyz" )
  {
    this->pm_->x_axis_label_state_->set( "X Axis" );
    this->pm_->y_axis_label_state_->set( "Y Axis" );
    this->pm_->z_axis_label_state_->set( "Z Axis" );
  }
  else
  {
    this->pm_->x_axis_label_state_->set( "X Axis" );
    this->pm_->y_axis_label_state_->set( "Y Axis" );
    this->pm_->z_axis_label_state_->set( "Z Axis" );
  }
}

//////////////////////////////////////////////////////////////////////////
// Class PreferencesManager
//////////////////////////////////////////////////////////////////////////

CORE_SINGLETON_IMPLEMENTATION( PreferencesManager );

PreferencesManager::PreferencesManager() :
  StateHandler( "preferences", false ),
  private_( new PreferencesManagerPrivate )
{ 
  this->private_->pm_ = this;
  this->set_initializing( true );

  // Initialize the local config directory path
  Core::Application::Instance()->get_config_directory( this->private_->local_config_path_ );

  // TODO: this is stupid - what happens if initializing colors fails???
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
  if ( state_io.import_from_file( this->private_->local_config_path_ / "preferences.xml" ) )
  {
    this->load_states( state_io );
  }
}

void PreferencesManager::save_state()
{
  Core::StateIO state_io;
  state_io.initialize();
  this->save_states( state_io );
  state_io.export_to_file( this->private_->local_config_path_ / "preferences.xml" );
}

Core::Color PreferencesManager::get_color( int index ) const
{
  // TODO: assert in production code - bad!!!
  assert( index >= 0 && index < static_cast< int >( this->color_states_.size() ) );
  return this->color_states_[ index ]->get();
}

Core::Color PreferencesManager::get_color( const Core::Color& color ) const
{
  throw "todo";
}

void PreferencesManager::initialize_states()
{
  boost::filesystem::path user_path;
  if (Core::Application::Instance()->get_user_directory( user_path ) )
  {
    user_path = user_path / ( Core::Application::GetApplicationName() + "-Projects" );
  }
  else
  {
    user_path = boost::filesystem::current_path();
  }
  boost::filesystem::path desktop_path;
  Core::Application::Instance()->get_user_desktop_directory( desktop_path );

  //General Preferences
  this->add_state( "project_path", this->project_path_state_, user_path.string() );
  this->add_state( "export_path", this->export_path_state_, desktop_path.string() );
  
  this->add_state( "full_screen_on_startup", this->full_screen_on_startup_state_, false );
  this->add_state( "auto_save", this->auto_save_state_, true );
  this->add_state( "auto_save_time", this->auto_save_time_state_, 10, 1, 120, 1 );
  this->add_state( "smart_save", this->smart_save_state_, true );
  this->add_state( "advanced_visibility_settings", this->advanced_visibility_settings_state_, false );
  this->add_state( "paint_cursor_visibility", this->paint_cursor_invisibility_state_, false );
  this->add_state( "compression", this->compression_state_, true );
  this->add_state( "compression_level", this->compression_level_state_, 2, 0, 9, 1 );
  this->add_state( "slice_step_multiplier", this->slice_step_multiplier_state_, 8 );
  this->add_state( "add_dicom_headers", this->export_dicom_headers_state_, true );
  this->add_state( "use_nrrd0005_format", this->export_nrrd0005_state_, true );
  
  this->add_state( "axis_labels_option", this->axis_labels_option_state_, "sca", 
    "sca=Sagittal/Coronal/Axial|sct=Sagittal/Coronal/Transverse|"
    "xyz=X Axis/Y Axis/Z Axis|custom=Custom" );
  this->axis_labels_option_state_->set_session_priority( Core::StateBase::DEFAULT_LOAD_E + 1 );

  this->add_state( "x_axis_label", this->x_axis_label_state_, "Sagittal" );
  this->add_state( "y_axis_label", this->y_axis_label_state_, "Coronal" );
  this->add_state( "z_axis_label", this->z_axis_label_state_, "Axial" );
  
  this->add_state( "enable_undo", this->enable_undo_state_, true );
  
  double percent_of_memory = 0.15;
  if ( sizeof( void* ) == 4 ) percent_of_memory = 0.05;
  
  this->add_state( "percent_of_memory", this->percent_of_memory_state_, 
    percent_of_memory, 0.0, 0.5, 0.01 );

  this->add_state( "embed_input_files_state", this->embed_input_files_state_, true );
  this->add_state( "generate_osx_project_bundle_state", this->generate_osx_project_bundle_state_, true );

  this->add_state( "reverse_slice_navigation", this->reverse_slice_navigation_state_, false );
  this->add_state( "zero_based_slice_numbers", this->zero_based_slice_numbers_state_, false );
  this->add_state( "active_layer_navigation", this->active_layer_navigation_state_, true );
  this->add_state( "status_sci_notation", this->status_sci_notation_state_, false );
  
  //Viewer Preferences
  this->add_state( "default_viewer_mode", this->default_viewer_mode_state_, "1and3", 
    "single|1and1|1and2|1and3|2and2|2and3|3and3" );
  this->add_state( "grid_size", this->grid_size_state_, 50, 10, 500, 5 );
  this->add_state( "background_color", this->background_color_state_, "darkgray", 
    "black=Black|darkgray=Dark Gray|gray=Gray|lightgray=Light Gray|white=White" );
  this->add_state( "show_slice_number", this->show_slice_number_state_, true );
  
  //Layers Preferences
  this->add_state( "default_layer_opacity", this->default_layer_opacity_state_, 1.0, 0.0, 1.0, 0.01 );
  this->add_state( "default_mask_fill", this->default_mask_fill_state_, "striped", "none|striped|solid" );
  this->add_state( "default_mask_border", this->default_mask_border_state_, "thick", "none|thin|thick" );
    
  this->color_states_.resize( 12 );
  for ( size_t j = 0; j < 12; j++ )
  {
    std::string stateid = std::string( "color_" ) + Core::ExportToString( j );
    this->add_state( stateid, this->color_states_[ j ], this->private_->default_colors_[ j ] );
  }
  
  //Interface Controls Preferences
  
  //Sidebars Preferences
  this->add_state( "show_tools_bar", this->show_tools_bar_state_, true );
  this->add_state( "show_layermanager_bar", this->show_layermanager_bar_state_, true );
  this->add_state( "show_projectmanager_bar", this->show_projectmanager_bar_state_, true );
  this->add_state( "show_provenance_bar", this->show_provenance_bar_state_, false );
  this->add_state( "show_rendering_bar", this->show_rendering_bar_state_, false );

  this->add_state( "enable_large_volume", this->enable_large_volume_state_, false );

  this->add_connection( this->axis_labels_option_state_->value_changed_signal_.connect(
    boost::bind( &PreferencesManagerPrivate::handle_axis_labels_option_changed, 
    this->private_, _2 ) ) );
}


bool PreferencesManager::initialize_default_colors()
{
  this->private_->default_colors_.clear();
  this->private_->default_colors_.push_back( Core::Color( 255, 175, 78 ) );
  this->private_->default_colors_.push_back( Core::Color( 116, 255, 122 ) );
  this->private_->default_colors_.push_back( Core::Color( 143, 214, 255 ) );
  this->private_->default_colors_.push_back( Core::Color( 255, 0, 0 ) );
  
  this->private_->default_colors_.push_back( Core::Color( 255, 233, 0 ) );
  this->private_->default_colors_.push_back( Core::Color( 0, 0, 255 ) );
  this->private_->default_colors_.push_back( Core::Color( 112, 181, 66 ) );
  this->private_->default_colors_.push_back( Core::Color( 255, 94, 122 ) );
  
  this->private_->default_colors_.push_back( Core::Color( 255, 255, 165 ) );
  this->private_->default_colors_.push_back( Core::Color( 108, 0, 212 ) );
  this->private_->default_colors_.push_back( Core::Color( 194, 118, 0 ) );
  this->private_->default_colors_.push_back( Core::Color( 159, 143, 255 ) );
  
  return true;
}

Core::Color PreferencesManager::get_background_color() const
{
  static Core::Color bkg_colors_s[] = 
  { 
    Core::Color( 0.0f, 0.0f, 0.0f ), Core::Color( 0.278f, 0.278f, 0.290f ),
    Core::Color( 0.416f, 0.416f, 0.439f ), Core::Color( 0.886f, 0.875f, 0.925f ),
    Core::Color( 1.0f, 1.0f, 1.0f )
  };

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  return bkg_colors_s[ this->background_color_state_->index() ];
}

const std::vector< Core::Color >& PreferencesManager::get_default_colors() const
{
  return this->private_->default_colors_;
}




} // end namespace seg3D
