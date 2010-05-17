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
  
  //QVector< ColorButton* > color_buttons_;
  QButtonGroup* color_button_group_;
  QVector< ColorPickerWidget* > color_pickers_;
};

AppPreferences::AppPreferences( QWidget *parent ) :
    QDialog( parent ),
    private_( new AppPreferencesPrivate )
{
    this->private_->ui_.setupUi( this );
  
  // Initialize all the tabs
  this->setup_general_prefs();
  this->setup_layer_prefs();
  this->setup_viewer_prefs();
  this->setup_sidebar_prefs();
  this->setup_interface_controls_prefs();

  // connect the apply button to the save defaults function
  connect( this->private_->ui_.apply_button_, SIGNAL( clicked () ), this, SLOT( save_defaults() ) );
  
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
  // Initialize the QButtonGroup
  this->private_->color_button_group_ = new QButtonGroup( this );
  
  for( int i = 0; i < 12; ++i )
  {
    // Step 1: create new buttons and add them to the button group
    this->private_->color_button_group_->addButton( new ColorButton( 
      this, i, PreferencesManager::Instance()->color_states_[ i ]->get(), 25, 25 ), i );
    this->private_->ui_.color_h_layout_->addWidget( this->private_->color_button_group_->button( i ) );
    
    // Step 2: create new ColorPickerWidgets, hide them, and add them to the appropriate layout
    this->private_->color_pickers_.push_back( new ColorPickerWidget( this ) );
    this->private_->color_pickers_[ i ]->hide();
    this->private_->ui_.color_widget_picker_layout_->addWidget( this->private_->color_pickers_[ i ] );
    
    // Step 3: Connect the ColorPickerWidgets and the ColorButtons to each other and the state engine
    QtBridge::Connect( dynamic_cast< ColorButton* >( this->private_->color_button_group_->button( i ) ), 
      PreferencesManager::Instance()->color_states_[ i ] );
    connect( this->private_->color_pickers_[ i ], SIGNAL( color_set( Core::Color ) ), 
      this->private_->color_button_group_->button( i ), SLOT( set_color( Core::Color ) ) );
    connect( this->private_->color_button_group_->button( i ), 
      SIGNAL( button_clicked( Core::Color, bool ) ), this->private_->color_pickers_[ i ], 
      SLOT( hide_show( Core::Color, bool ) ) );
    connect( this->private_->color_button_group_, SIGNAL( buttonClicked( int ) ), 
      this, SLOT( hide_the_others ( int ) ) );
  }
  
  // setting an arbitrary picker as the default active picker to deal with the first case
  this->active_picker_ = this->private_->color_pickers_[ 0 ];

  
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
  for( int i = 0; i < 12; ++i )
  {
    dynamic_cast< ColorButton* >( this->private_->color_button_group_->button( i ) )->
      set_color( temp_color_list[ i ] );
  }
}

void AppPreferences::hide_the_others( int active )
{
  this->active_picker_->hide();
  this->active_picker_ = this->private_->color_pickers_[ active ];
  this->active_picker_->show();
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

void AppPreferences::save_defaults()
{
  PreferencesManager::Instance()->save_state();
}

void AppPreferences::setup_interface_controls_prefs()
{
  
  //Interface Controls Preferences
  
}
  
  
  
  


} // end namespace Seg3D
