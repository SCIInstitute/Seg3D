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

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>

// Interface includes
#include <Interface/AppPreferences/AppPreferences.h>
#include <Interface/QtInterface/QtBridge.h>

// The interface from the designer
#include "ui_AppPreferences.h"

namespace Seg3D
{

class AppPreferencesPrivate
{
public:
    Ui::AppPreferences ui_;
  SliderDoubleCombo* opacity_adjuster_;
};

AppPreferences::AppPreferences( QWidget *parent ) :
    QDialog( parent ),
    private_( new AppPreferencesPrivate )
{
    private_->ui_.setupUi( this );
  
  // Initialize all the tabs
  this->setup_general_prefs();
  this->setup_layer_prefs();
  this->setup_viewer_prefs();
  this->setup_sidebar_prefs();
  this->setup_interface_controls_prefs();

}

AppPreferences::~AppPreferences()
{
}

void AppPreferences::setup_general_prefs()
{
//Set Layers Preferences
  this->private_->ui_.consolidate_project_checkbox_->setChecked( 
    PreferencesManager::Instance()->considate_project_state_->get() );
  this->private_->ui_.full_screen_on_startup_checkbox_->setChecked(  
    PreferencesManager::Instance()->full_screen_on_startup_state_->get() );
  
//Connect General Preferences 
  QtBridge::Connect( this->private_->ui_.consolidate_project_checkbox_, 
    PreferencesManager::Instance()->considate_project_state_ );
  QtBridge::Connect( this->private_->ui_.full_screen_on_startup_checkbox_, 
    PreferencesManager::Instance()->full_screen_on_startup_state_ );
}

void AppPreferences::setup_layer_prefs()
{
  
  for( int i = 0; i < 12; ++i )
  {
    // Step 1: create new ColorButtons, and set some default sizes
    this->color_buttons_.push_back( new ColorButton( this, i ) );
    this->color_buttons_[ i ]->setMinimumSize( 25, 25 );
    this->color_buttons_[ i ]->setMaximumSize( 25, 25 );
    
    // Step 2: set the default colors for the ColorButtons and connect them to the 
    // PreferencesManager's color states
    this->color_buttons_[ i ]->set_color( PreferencesManager::Instance()->color_states_[ i ]->get() );
    QtBridge::Connect( this->color_buttons_[ i ], PreferencesManager::Instance()->color_states_[ i ] );
    
    // Step 3: create new ColorPickerWidgets, hide them, and add them to the appropriate layout
    this->color_pickers_.push_back( new ColorPickerWidget( this ) );
    this->color_pickers_[ i ]->hide();
    this->private_->ui_.color_widget_picker_layout_->addWidget( this->color_pickers_[ i ] );
    
    // Step 4: Connect the ColorPickerWidgets and the ColorButtons
    connect( this->color_pickers_[ i ], SIGNAL( color_set( Core::Color ) ), 
        this->color_buttons_[ i ], SLOT( set_color( Core::Color ) ) );
    connect( this->color_buttons_[ i ], SIGNAL( clicked( Core::Color, bool ) ), 
        this->color_pickers_[ i ], SLOT( hide_show( Core::Color, bool ) ) );
    connect( this->color_buttons_[ i ], SIGNAL( clicked( int ) ), 
        this, SLOT( hide_the_others ( int ) ) );
  }
  
  // We need to add the ColorButton's to the appropriate layouts
  this->private_->ui_.color_1_layout_->addWidget( this->color_buttons_[ 0 ] );
  this->private_->ui_.color_2_layout_->addWidget( this->color_buttons_[ 1 ] );
  this->private_->ui_.color_3_layout_->addWidget( this->color_buttons_[ 2 ] );
  this->private_->ui_.color_4_layout_->addWidget( this->color_buttons_[ 3 ] );
  this->private_->ui_.color_5_layout_->addWidget( this->color_buttons_[ 4 ] );
  this->private_->ui_.color_6_layout_->addWidget( this->color_buttons_[ 5 ] );
  this->private_->ui_.color_7_layout_->addWidget( this->color_buttons_[ 6 ] );
  this->private_->ui_.color_8_layout_->addWidget( this->color_buttons_[ 7 ] );
  this->private_->ui_.color_9_layout_->addWidget( this->color_buttons_[ 8 ] );
  this->private_->ui_.color_10_layout_->addWidget( this->color_buttons_[ 9 ] );
  this->private_->ui_.color_11_layout_->addWidget( this->color_buttons_[ 10 ] );
  this->private_->ui_.color_12_layout_->addWidget( this->color_buttons_[ 11 ] );
  
  
//Set Layers Preferences
  // Add SliderSpin Combo for adjusting the default opacity of the layers
  this->private_->opacity_adjuster_ = new SliderDoubleCombo( this );
  this->private_->ui_.verticalLayout_2->addWidget( this->private_->opacity_adjuster_ );
  this->private_->opacity_adjuster_->setObjectName( QString::fromUtf8( "opacity_adjuster_" ) );
  
  // set the default values for the slidercombo's
  // set the defaults for the opacity
  double opacity_min = 0.0; 
  double opacity_max = 0.0;
  double opacity_step = 0.0;
  PreferencesManager::Instance()->default_layer_opacity_state_->get_step( opacity_step );
  PreferencesManager::Instance()->default_layer_opacity_state_->get_range( opacity_min, opacity_max );
  this->private_->opacity_adjuster_->setStep( opacity_step );
  this->private_->opacity_adjuster_->setRange( opacity_min, opacity_max );
  this->private_->opacity_adjuster_->setCurrentValue( 
    PreferencesManager::Instance()->default_layer_opacity_state_->get() );
  
  
  // --- set the values for the dropdown menu's using values from the state handles
  // -- set the border selection combo box's values 
  std::vector< std::string > temp_option_list = 
    PreferencesManager::Instance()->default_mask_border_state_->option_list();
  
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
    this->private_->ui_.mask_border_combobox_->addItem( 
      QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.mask_border_combobox_->setCurrentIndex( 
    PreferencesManager::Instance()->default_mask_border_state_->index() );
  
  // -- set the fill selection combo box's values 
  temp_option_list = PreferencesManager::Instance()->default_mask_fill_state_->option_list();
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
    this->private_->ui_.mask_fill_combobox_->addItem( 
      QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.mask_fill_combobox_->setCurrentIndex( 
    PreferencesManager::Instance()->default_mask_fill_state_->index() );
  
//Connect Layers Preferences
  QtBridge::Connect( this->private_->opacity_adjuster_, 
    PreferencesManager::Instance()->default_layer_opacity_state_ );
  QtBridge::Connect( this->private_->ui_.mask_fill_combobox_, 
    PreferencesManager::Instance()->default_mask_fill_state_ );
  QtBridge::Connect( this->private_->ui_.mask_border_combobox_, 
    PreferencesManager::Instance()->default_mask_border_state_ );
  
  connect( this->private_->ui_.revert_to_defaults_button_, SIGNAL( clicked() ), 
    this, SLOT( set_buttons_to_default_colors () ) );
}
  
void AppPreferences::set_buttons_to_default_colors()
{
  std::vector< Core::Color > temp_color_list = PreferencesManager::Instance()->get_default_colors();
  for( int i = 0; i < this->color_buttons_.size(); ++i )
  {
    this->color_buttons_[ i ]->set_color( temp_color_list[ i ] );
  }
}

void AppPreferences::hide_the_others( int active )
{
  for( int i = 0; i < this->color_pickers_.size(); ++i )
  {
    if( i != active) 
    {
      this->color_pickers_[ i ]->hide();
      this->color_buttons_[ i ]->setChecked( false );
    }
  }
}

void AppPreferences::setup_viewer_prefs()
{
//Set Viewer Preferences
  // --- set the values for the dropdown menu's using values from the state handles
  // -- set the border selection combo box's values 
  std::vector< std::string > temp_option_list = 
  PreferencesManager::Instance()->default_viewer_mode_state_->option_list();
  
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
    this->private_->ui_.viewer_mode_combobox_->addItem( 
      QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.viewer_mode_combobox_->setCurrentIndex( 
    PreferencesManager::Instance()->default_viewer_mode_state_->index() );
  
  // -- set the fill selection combo box's values 
  temp_option_list = PreferencesManager::Instance()->background_color_state_->option_list();
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
    this->private_->ui_.background_color_combobox_->addItem( 
      QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.background_color_combobox_->setCurrentIndex( 
    PreferencesManager::Instance()->background_color_state_->index() );
  
  // -- set the fill selection combo box's values 
  temp_option_list = PreferencesManager::Instance()->naming_convention_state_->option_list();
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
    this->private_->ui_.naming_convention_combobox_->addItem( 
      QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.naming_convention_combobox_->setCurrentIndex( 
    PreferencesManager::Instance()->naming_convention_state_->index() );
  
  this->private_->ui_.grid_size_spinbox_->setValue(
    PreferencesManager::Instance()->grid_size_state_->get() );
  
  this->private_->ui_.show_slice_numbers_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_slice_number_state_->get() );
  
  
//Connect Viewer Preferences
  QtBridge::Connect( this->private_->ui_.viewer_mode_combobox_, 
    PreferencesManager::Instance()->default_viewer_mode_state_ );
  QtBridge::Connect( this->private_->ui_.background_color_combobox_, 
    PreferencesManager::Instance()->background_color_state_ );
  QtBridge::Connect( this->private_->ui_.grid_size_spinbox_, 
    PreferencesManager::Instance()->grid_size_state_ );
  QtBridge::Connect( this->private_->ui_.naming_convention_combobox_, 
    PreferencesManager::Instance()->naming_convention_state_ );
  QtBridge::Connect( this->private_->ui_.show_slice_numbers_checkbox_, 
    PreferencesManager::Instance()->show_slice_number_state_ );

}

void AppPreferences::setup_sidebar_prefs()
{
//Set Sidebars Preferences
  this->private_->ui_.tools_filters_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_tools_bar_state_->get() );
  this->private_->ui_.layer_manager_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_layermanager_bar_state_->get() );
  this->private_->ui_.project_manager_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_projectmanager_bar_state_->get() );
  this->private_->ui_.measurement_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_measurement_bar_state_->get() );
  this->private_->ui_.history_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_history_bar_state_->get() );
  
//Connect Sidebars Preferences
  QtBridge::Connect( this->private_->ui_.tools_filters_checkbox_, 
    PreferencesManager::Instance()->show_tools_bar_state_ );
  QtBridge::Connect( this->private_->ui_.layer_manager_checkbox_, 
    PreferencesManager::Instance()->show_layermanager_bar_state_ );
  QtBridge::Connect( this->private_->ui_.project_manager_checkbox_, 
    PreferencesManager::Instance()->show_projectmanager_bar_state_ );
  QtBridge::Connect( this->private_->ui_.measurement_checkbox_, 
    PreferencesManager::Instance()->show_measurement_bar_state_ );
  QtBridge::Connect( this->private_->ui_.history_checkbox_, 
    PreferencesManager::Instance()->show_history_bar_state_ );
  
}

void AppPreferences::setup_interface_controls_prefs()
{
  
  //Interface Controls Preferences
  
}
  
  
  
  


} // end namespace Seg3D
