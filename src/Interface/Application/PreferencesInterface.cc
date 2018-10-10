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

// boost includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// Core Includes
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h> 

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/Tool/ToolFactory.h>


// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Utils/QtPointer.h>
#include <QButtonGroup>

// Interface includes
#include <Interface/Application/PreferencesInterface.h>
#include <Interface/Application/StyleSheet.h>

// Resource includes
#include <Resources/QtResources.h>

// The interface from the designer
#include "ui_PreferencesInterface.h"

namespace Seg3D
{

class PreferencesInterfacePrivate
{
public:
    Ui::PreferencesInterface ui_;
  QButtonGroup* color_button_group_;
  QVector< ColorPickerWidget* > color_pickers_;
};

PreferencesInterface::PreferencesInterface( QWidget *parent ) :
    QtUtils::QtCustomDialog( parent ),
    private_( new PreferencesInterfacePrivate )
{
  InitQtResources();

    this->private_->ui_.setupUi( this );
    this->private_->ui_.prefs_tabs_->setStyleSheet( StyleSheet::PREFERENCES_C );
  // Update the title of the dialog
  std::string title = std::string( "Preferences - "  )
    + Core::Application::GetApplicationNameAndVersion();
  this->setWindowTitle( QString::fromStdString( title ) );
  
  QIcon icon = windowIcon();
  Qt::WindowFlags flags = windowFlags();
  Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & ( ~helpFlag );
  this->setWindowFlags( flags );
  this->setWindowIcon( icon );

  // Initialize all the tabs
  this->setup_layer_prefs();
  this->setup_general_prefs();
  this->setup_viewer_prefs();
  this->setup_sidebar_prefs();
//  this->setup_interface_controls_prefs();
  this->setup_large_volume_prefs();

  //Hide the interface controls since they arent connected yet
  this->private_->ui_.prefs_tabs_->removeTab( 3 );
  
  //This will ensure that the first tab, the general settings tab, will always be active by default
  this->private_->ui_.prefs_tabs_->setCurrentIndex( 0 );
}

PreferencesInterface::~PreferencesInterface()
{
  this->disconnect_all();
}

void PreferencesInterface::change_project_directory()
{
  QString path = QFileDialog::getExistingDirectory ( this, tr( "Directory" ), 
    project_directory_.path() );
    if ( path.isNull() == false )
    {
        project_directory_.setPath( path );
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
      PreferencesManager::Instance()->project_path_state_, 
      project_directory_.absolutePath().toStdString() );
  }
}

void PreferencesInterface::setup_general_prefs()
{
  //Set Layers Preferences
  this->project_directory_.setPath( QString::fromStdString( PreferencesManager::Instance()->
    project_path_state_->export_to_string() ) );

  this->private_->ui_.path_->setText( QString::fromStdString( PreferencesManager::Instance()->
    project_path_state_->export_to_string() ) );

  //Connect General Preferences
  QtUtils::QtBridge::Connect( this->private_->ui_.path_, 
    PreferencesManager::Instance()->project_path_state_ );

  connect( this->private_->ui_.change_directory_button_, SIGNAL( clicked() ), 
    this, SLOT( change_project_directory() ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.full_screen_on_startup_checkbox_, 
    PreferencesManager::Instance()->full_screen_on_startup_state_ );
  
  this->private_->ui_.auto_save_checkbox_->setChecked( 
    PreferencesManager::Instance()->auto_save_state_->get() );
  connect( this->private_->ui_.auto_save_checkbox_, SIGNAL( toggled( bool ) ),
    this, SLOT( set_autosave_checked_state( bool ) ) );
  
  add_connection( PreferencesManager::Instance()->auto_save_state_->
    value_changed_signal_.connect( boost::bind( 
    &PreferencesInterface::HandleAutosaveStateChanged, qpointer_type( this ), _1 ) ) );
  
  QtUtils::QtBridge::Connect( this->private_->ui_.smart_save_checkbox_,
    PreferencesManager::Instance()->smart_save_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.auto_save_timer_adjuster_,
    PreferencesManager::Instance()->auto_save_time_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.advanced_visibility_checkbox_,
    PreferencesManager::Instance()->advanced_visibility_settings_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.reverse_slice_navigation_checkbox_,
    PreferencesManager::Instance()->reverse_slice_navigation_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.cursor_visibility_checkbox_,
    PreferencesManager::Instance()->paint_cursor_invisibility_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.zero_based_slice_numbers_,
    PreferencesManager::Instance()->zero_based_slice_numbers_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.status_sci_notation_,
    PreferencesManager::Instance()->status_sci_notation_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.active_layer_navigation_,
    PreferencesManager::Instance()->active_layer_navigation_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.export_dicom_headers_,
    PreferencesManager::Instance()->export_dicom_headers_state_ );  
  QtUtils::QtBridge::Connect( this->private_->ui_.export_nrrd0005_,
    PreferencesManager::Instance()->export_nrrd0005_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_compression_checkbox_,
    PreferencesManager::Instance()->compression_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.compression_adjuster_,
    PreferencesManager::Instance()->compression_level_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.slice_step_multiplier_spinbox_,
    PreferencesManager::Instance()->slice_step_multiplier_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.axis_combobox_,
    PreferencesManager::Instance()->axis_labels_option_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.x_lineedit_,
    PreferencesManager::Instance()->x_axis_label_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.y_lineedit_,
    PreferencesManager::Instance()->y_axis_label_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.z_lineedit_,
    PreferencesManager::Instance()->z_axis_label_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.enable_undo_,
    PreferencesManager::Instance()->enable_undo_state_ ); 
  QtUtils::QtBridge::Connect( this->private_->ui_.percent_of_memory_,
    PreferencesManager::Instance()->percent_of_memory_state_ );

  QtUtils::QtBridge::Enable( this->private_->ui_.x_lineedit_, 
    PreferencesManager::Instance()->axis_labels_option_state_,
    boost::lambda::bind( &Core::StateLabeledOption::get, 
    PreferencesManager::Instance()->axis_labels_option_state_.get() ) == "custom" );
  QtUtils::QtBridge::Enable( this->private_->ui_.y_lineedit_, 
    PreferencesManager::Instance()->axis_labels_option_state_,
    boost::lambda::bind( &Core::StateLabeledOption::get, 
    PreferencesManager::Instance()->axis_labels_option_state_.get() ) == "custom" );
  QtUtils::QtBridge::Enable( this->private_->ui_.z_lineedit_, 
    PreferencesManager::Instance()->axis_labels_option_state_,
    boost::lambda::bind( &Core::StateLabeledOption::get, 
    PreferencesManager::Instance()->axis_labels_option_state_.get() ) == "custom" );

  QtUtils::QtBridge::Enable( this->private_->ui_.compression_adjuster_, 
    PreferencesManager::Instance()->compression_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.embed_input_files_,
    PreferencesManager::Instance()->embed_input_files_state_ );

  QtUtils::QtBridge::Connect( this->private_->ui_.generate_osx_project_bundle_,
    PreferencesManager::Instance()->generate_osx_project_bundle_state_ );

#ifndef __APPLE__
  this->private_->ui_.generate_osx_project_bundle_->hide();
#endif  
  
  this->private_->ui_.compression_adjuster_->set_description( "Compression" );
  this->private_->ui_.auto_save_timer_adjuster_->set_description( "Frequency (minutes)" );
  this->private_->ui_.percent_of_memory_->set_description( "Undo/Redo buffer size" );
  this->private_->ui_.opacity_adjuster_->set_description( "Default layer opacity" );

}

void PreferencesInterface::setup_layer_prefs()
{
  // Initialize the QButtonGroup
  this->private_->color_button_group_ = new QButtonGroup( this );
  
  for( int i = 0; i < 12; ++i )
  {
    // Step 1: create new buttons and add them to the button group
    this->private_->color_button_group_->addButton( new QtUtils::QtColorButton( 
      this, i, PreferencesManager::Instance()->color_states_[ i ]->get() ), i );
    this->private_->ui_.color_h_layout_->addWidget( this->private_->
      color_button_group_->button( i ) );
    
    // Step 2: create new ColorPickerWidgets, hide them, and add them to the appropriate layout
    this->private_->color_pickers_.push_back( new ColorPickerWidget( this ) );
    this->private_->color_pickers_[ i ]->hide();
    this->private_->ui_.color_widget_picker_layout_->addWidget( this->private_->
      color_pickers_[ i ] );
    

    // Step 3: Connect the ColorPickerWidgets and the ColorButtons to each other and the state engine
    QtUtils::QtBridge::Connect( dynamic_cast< QtUtils::QtColorButton* >( this->private_->color_button_group_->button( i ) ), 
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
  this->private_->color_button_group_->button( 0 )->click();
  
  //Set Layers Preferences
  //Connect Layers Preferences
  QtUtils::QtBridge::Connect( this->private_->ui_.opacity_adjuster_, 
    PreferencesManager::Instance()->default_layer_opacity_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.mask_fill_combobox_, 
    PreferencesManager::Instance()->default_mask_fill_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.mask_border_combobox_, 
    PreferencesManager::Instance()->default_mask_border_state_ );
  
  this->connect( this->private_->ui_.revert_to_defaults_button_, SIGNAL( clicked() ), 
    this, SLOT( set_buttons_to_default_colors () ) );
}
  
void PreferencesInterface::setup_viewer_prefs()
{
  //Connect Viewer Preferences

  QtUtils::QtBridge::Connect( this->private_->ui_.viewer_mode_combobox_, 
    PreferencesManager::Instance()->default_viewer_mode_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.background_color_combobox_, 
    PreferencesManager::Instance()->background_color_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.grid_size_spinbox_, 
    PreferencesManager::Instance()->grid_size_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.show_slice_numbers_checkbox_, 
    PreferencesManager::Instance()->show_slice_number_state_ );
}

void PreferencesInterface::setup_sidebar_prefs()
{
  // Set Sidebars Preferences
  this->private_->ui_.tools_filters_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_tools_bar_state_->get() );
  this->private_->ui_.layer_manager_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_layermanager_bar_state_->get() );
  this->private_->ui_.project_manager_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_projectmanager_bar_state_->get() );
  this->private_->ui_.volume_view_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_rendering_bar_state_->get() );
  this->private_->ui_.provenance_checkbox_->setChecked( 
    PreferencesManager::Instance()->show_provenance_bar_state_->get() );
  
  
  
//  this->private_->ui_.history_checkbox_->setChecked( 
//    PreferencesManager::Instance()->show_history_bar_state_->get() );
  
  // Connect Sidebars Preferences
  QtUtils::QtBridge::Connect( this->private_->ui_.tools_filters_checkbox_,
    PreferencesManager::Instance()->show_tools_bar_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.layer_manager_checkbox_, 
    PreferencesManager::Instance()->show_layermanager_bar_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.project_manager_checkbox_, 
    PreferencesManager::Instance()->show_projectmanager_bar_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.volume_view_checkbox_, 
    PreferencesManager::Instance()->show_rendering_bar_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.provenance_checkbox_, 
    PreferencesManager::Instance()->show_provenance_bar_state_ );
  
//  QtUtils::QtBridge::Connect( this->private_->ui_.history_checkbox_, 
//    PreferencesManager::Instance()->show_history_bar_state_ );
  
  
  ToolMenuList tool_menus;
  ToolFactory::Instance()->list_menus( tool_menus );
  
  for( size_t i = 0; i < tool_menus.size(); ++i )
  {
    QWidget* tools_tab = new QWidget();
    
    QVBoxLayout* verticalLayout = new QVBoxLayout( tools_tab );
    verticalLayout->setSpacing( 0 );
    verticalLayout->setContentsMargins( 11, 11, 11, 11 );
    verticalLayout->setContentsMargins( 4, 5, 4, 4 );
    
    QListWidget* tools_list = new QListWidget( tools_tab );
    tools_list->setEditTriggers( QAbstractItemView::NoEditTriggers );
    tools_list->setAlternatingRowColors( true );
    tools_list->setSelectionMode( QAbstractItemView::MultiSelection );
    QtUtils::QtBridge::Connect( tools_list,
      ToolFactory::Instance()->startup_tools_state_[ tool_menus[ i ] ] );
    
    verticalLayout->addWidget( tools_list );
      
    QWidget* widget = new QWidget( tools_tab );
    widget->setMinimumSize( QSize( 0, 28 ) );
    QHBoxLayout* horizontalLayout = new QHBoxLayout( widget );
    horizontalLayout->setSpacing( 0 );
    horizontalLayout->setContentsMargins( 4, 4, 4, 4 );
    QLabel* tools_label = new QLabel( widget );
    tools_label->setText( QString::fromStdString( "Select which " +
      Core::StringToLower( tool_menus[ i ] ) + " are active on startup." ) );
    horizontalLayout->addWidget( tools_label );

    verticalLayout->addWidget( widget );

    this->private_->ui_.tools_tab_widget_->addTab( tools_tab, 
      QString::fromStdString( tool_menus[ i ] ) );
  }
  
}

// TODO: feasible to set up?
//void PreferencesInterface::setup_interface_controls_prefs()
//{
//  //Interface Controls Preferences  
//}

void PreferencesInterface::setup_large_volume_prefs()
{
  this->private_->ui_.large_volume_checkbox_->setChecked( PreferencesManager::Instance()->enable_large_volume_state_->get() );
  QtUtils::QtBridge::Connect( this->private_->ui_.large_volume_checkbox_, PreferencesManager::Instance()->enable_large_volume_state_ );
}

void PreferencesInterface::set_autosave_checked_state( bool state )
{
  Core::ActionSet::Dispatch( Core::Interface::GetKeyboardActionContext(),
    PreferencesManager::Instance()->auto_save_state_, state );
}
  
void PreferencesInterface::set_autosave_checkbox( bool state )
{
  this->private_->ui_.auto_save_checkbox_->blockSignals( true );
  this->private_->ui_.auto_save_checkbox_->setChecked( state );
  this->private_->ui_.auto_save_checkbox_->blockSignals( false );
}

void PreferencesInterface::HandleAutosaveStateChanged( qpointer_type qpointer, bool state )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &PreferencesInterface::set_autosave_checkbox, qpointer.data(), state ) ) );
}

void PreferencesInterface::set_buttons_to_default_colors()
{
  std::vector< Core::Color > temp_color_list = PreferencesManager::Instance()->
  get_default_colors();
  for( int i = 0; i < 12; ++i )
  {
    dynamic_cast< QtUtils::QtColorButton* >( this->private_->color_button_group_->button( i ) )->
    set_color( temp_color_list[ i ] );
  }
}

void PreferencesInterface::hide_the_others( int active )
{
  this->active_picker_->hide();
  this->active_picker_ = this->private_->color_pickers_[ active ];
  this->active_picker_->show();
}

} // end namespace Seg3D
