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

//Boost Includes
#include <boost/lexical_cast.hpp>

//Core Includes - for logging
#include <Core/Utils/Log.h>
#include <Core/State/Actions/ActionSet.h>
//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>
#include <QtUtils/Widgets/QtColorBarWidget.h>

//Interface Includes
#include <Interface/AppInterface/LayerWidget.h>
#include <Interface/AppInterface/StyleSheet.h>
#include <Interface/AppInterface/PushDragButton.h>
#include <Interface/AppInterface/DropSpaceWidget.h>
#include <Interface/AppInterface/OverlayWidget.h>

//UI Includes
#include "ui_LayerWidget.h"

//Application Includes
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionActivateLayer.h>
#include <Application/LayerManager/Actions/ActionMoveLayerAbove.h>
#include <Application/PreferencesManager/PreferencesManager.h>


namespace Seg3D
{

class LayerWidgetPrivate
{
public:
  // Layer
  LayerHandle layer_;
  
  // Icons
  QIcon data_layer_icon_;
  QIcon label_layer_icon_;
  QIcon mask_layer_icon_; 
  
  // UI Designed by the QtDesginer
  Ui::LayerWidget ui_;
  
  // Custom widgets that cannot be added by the Qt Designer
  QtUtils::QtSliderDoubleCombo* opacity_adjuster_;
  QtUtils::QtSliderDoubleCombo* brightness_adjuster_;
  QtUtils::QtSliderDoubleCombo* contrast_adjuster_;
  QtUtils::QtColorBarWidget* color_widget_;
  
  // Drag and drop stuff
  PushDragButton* activate_button_;
  DropSpaceWidget* drop_space_;
  OverlayWidget* overlay_;
  LayerWidget* drop_layer_;

  // Local copy of state information 
  bool active_;
  bool locked_;
  bool picked_up_;
};

LayerWidget::LayerWidget( QFrame* parent, LayerHandle layer ) :
  QWidget( parent ),
  private_( new LayerWidgetPrivate ),
  layer_menus_open_( false ),
  group_menus_open_( false ) 
{
  // Store the layer in the private class
  this->private_->layer_ = layer;
  
  this->private_->active_ = false;
  this->private_->locked_ = false;
  this->private_->picked_up_ = false;
  
  // Store the icons in the private class, so they only need to be generated once
  this->private_->label_layer_icon_.addFile( QString::fromUtf8( ":/Images/LabelMapWhite.png" ),
    QSize(), QIcon::Normal, QIcon::Off );
  this->private_->data_layer_icon_.addFile( QString::fromUtf8( ":/Images/DataWhite.png" ), 
    QSize(), QIcon::Normal, QIcon::Off );
  this->private_->mask_layer_icon_.addFile( QString::fromUtf8( ":/Images/MaskWhite_shadow.png" ), 
    QSize(), QIcon::Normal, QIcon::Off );

  // Update the style sheet of this widget
  this->setObjectName( QString::fromUtf8( "LayerWidget" ) );
  this->setStyleSheet( StyleSheet::LAYERWIDGET_C );

  // Add the Ui children onto the QWidget
  this->private_->ui_.setupUi( this );
  
  this->setUpdatesEnabled( false );
  
  // set some Drag and Drop stuff
  this->setAcceptDrops( true );
  
  // Set the defaults
  // this is a default setting until we can get the name of the layer from the file or by some other means
  this->private_->ui_.label_->setText( QString::fromStdString( 
    this->private_->layer_->name_state_->get() ) );
  this->private_->ui_.label_->setAcceptDrops( false );
  this->private_->ui_.label_->setValidator( new QRegExpValidator( 
    QRegExp( QString::fromStdString( Core::StateName::REGEX_VALIDATOR_C ) ), this ) );

  // hide the toolbars and the selection check box
  // hide the tool bars and the selection checkbox
  this->private_->ui_.color_bar_->hide();
  this->private_->ui_.bright_contrast_bar_->hide();
  this->private_->ui_.checkbox_widget_->hide();
  this->private_->ui_.opacity_bar_->hide();
  this->private_->ui_.progress_bar_bar_->hide();
  this->private_->ui_.border_bar_->hide();

  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );
  
  // add the PushDragButton
  this->private_->activate_button_ = new PushDragButton( this->private_->ui_.typeGradient_ );
  this->private_->activate_button_->setObjectName( QString::fromUtf8( "activate_button_" ) );
  this->private_->activate_button_->setStyleSheet( StyleSheet::LAYER_PUSHDRAGBUTTON_C );
  this->private_->activate_button_->setMinimumHeight( 42 );
  this->private_->activate_button_->setMinimumWidth( 31 );
  this->private_->activate_button_->setIconSize( QSize( 25, 25 ) );
  this->private_->ui_.horizontalLayout_9->addWidget( this->private_->activate_button_ );
  this->private_->activate_button_->setAcceptDrops( false );
  
  // add the DropSpaceWidget
  this->private_->drop_space_ = new DropSpaceWidget( this );
  this->private_->ui_.verticalLayout_3->insertWidget( 0, this->private_->drop_space_ );
  this->private_->drop_space_->hide();
  
  // add the SliderCombo Widgets
  this->private_->opacity_adjuster_ = new QtUtils::QtSliderDoubleCombo( 
    this->private_->ui_.opacity_bar_ );
  this->private_->ui_.verticalLayout_2->addWidget( this->private_->opacity_adjuster_ );
  this->private_->opacity_adjuster_->setObjectName( QString::fromUtf8( "opacity_adjuster_" ) );
  
  this->private_->brightness_adjuster_ = new QtUtils::QtSliderDoubleCombo( 
    this->private_->ui_.bright_contrast_bar_ );
  this->private_->ui_.brightness_h_layout_->addWidget( this->private_->brightness_adjuster_ );
  this->private_->brightness_adjuster_->setObjectName( QString::fromUtf8( "brightness_adjuster_" ) );
  
  this->private_->contrast_adjuster_ = new QtUtils::QtSliderDoubleCombo( 
    this->private_->ui_.bright_contrast_bar_ );
  this->private_->ui_.contrast_h_layout_->addWidget( this->private_->contrast_adjuster_ );
  this->private_->contrast_adjuster_->setObjectName( QString::fromUtf8( "contrast_adjuster_" ) );
  
  this->private_->color_widget_ = new QtUtils::QtColorBarWidget( this->private_->ui_.color_bar_ );
  this->private_->ui_.verticalLayout_5->addWidget( this->private_->color_widget_ );
  this->private_->color_widget_->setObjectName( QString::fromUtf8( "color_widget_" ) );
  
  // connect the GUI signals and slots
  connect( this->private_->ui_.opacity_button_, 
      SIGNAL( clicked( bool ) ), this, 
      SLOT( select_opacity_bar( bool ) ) );
  connect( this->private_->ui_.brightness_contrast_button_, 
      SIGNAL( clicked( bool ) ), this, 
      SLOT( select_brightness_contrast_bar( bool ) ) );
  connect( this->private_->ui_.fill_border_button_, 
      SIGNAL( clicked( bool ) ), this, 
      SLOT( select_border_fill_bar ( bool ) ) );
  connect( this->private_->ui_.color_button_, 
      SIGNAL( clicked( bool ) ), this, 
      SLOT( select_color_bar( bool ) ) );
  connect( this->private_->ui_.lock_button_, 
      SIGNAL( toggled( bool )), this, 
      SLOT( select_visual_lock( bool ) ) );
      
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  
    qpointer_type qpointer( this );
    this->private_->layer_->visual_lock_state_->state_changed_signal_.connect(
      boost::bind( &LayerWidget::UpdateState, qpointer ) );
    this->private_->layer_->menu_state_->state_changed_signal_.connect(
      boost::bind( &LayerWidget::UpdateState, qpointer ) );
    this->private_->layer_->data_state_->state_changed_signal_.connect(
      boost::bind( &LayerWidget::UpdateState, qpointer ) ); 
  
    this->private_->layer_->update_progress_signal_.connect(
      boost::bind( &LayerWidget::UpdateProgress, qpointer, _1 ) );
  
    QtUtils::QtBridge::Connect( this->private_->activate_button_, 
      boost::bind( static_cast<void (*) ( Core::ActionContextHandle, LayerHandle )>( 
      &ActionActivateLayer::Dispatch ), Core::Interface::GetWidgetActionContext(), layer ) );
    
    // make the default connections, for any layer type, to the state engine
    QtUtils::QtBridge::Connect( this->private_->ui_.selection_checkbox_, layer->selected_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.label_, layer->name_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.visibility_button_, layer->visible_state_,
      ViewerManager::Instance()->active_viewer_state_ );
    QtUtils::QtBridge::Connect( this->private_->opacity_adjuster_, layer->opacity_state_ );
    
    
  
    switch( this->get_volume_type() )
    {
      // This if for the Data Layers
      case Core::VolumeType::DATA_E:
        {
          // Update the icons
          this->private_->activate_button_->setIcon(this->private_->data_layer_icon_);
        
          // Hide the buttons that are not needed for this widget
          this->private_->ui_.color_button_->hide();
          this->private_->ui_.compute_iso_surface_button_->hide();
          this->private_->ui_.delete_iso_surface_button_->hide();
          this->private_->ui_.show_iso_surface_button_->hide();
          this->private_->ui_.iso_control_separator_line_->hide();
          this->private_->ui_.fill_border_button_->hide();
          
          // Add the layer specific connections
          DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
          if ( data_layer )
          {
            QtUtils::QtBridge::Connect( this->private_->brightness_adjuster_, 
              data_layer->brightness_state_ );
            QtUtils::QtBridge::Connect( this->private_->contrast_adjuster_, 
              data_layer->contrast_state_ );
          }
        }
        break;
        
      // This is for the Mask Layers  
      case Core::VolumeType::MASK_E:
        {
          // Update the icons
          this->private_->activate_button_->setIcon( this->private_->mask_layer_icon_ );

          // Hide the buttons that are not needed for this widget
          this->private_->ui_.brightness_contrast_button_->hide();
          this->private_->ui_.volume_rendered_button_->hide();
          
          this->connect( this->private_->color_widget_, SIGNAL( color_changed( int ) ), 
            this, SLOT( set_mask_background_color( int ) ) );
            
          MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );  
          if ( mask_layer )
          {
            QtUtils::QtBridge::Connect( this->private_->ui_.show_iso_surface_button_, 
              mask_layer->show_isosurface_state_ );
            QtUtils::QtBridge::Connect( this->private_->ui_.compute_iso_surface_button_,
              boost::bind( &MaskLayer::compute_isosurface, mask_layer ) );
            QtUtils::QtBridge::Connect( this->private_->ui_.delete_iso_surface_button_,
              boost::bind( &MaskLayer::delete_isosurface, mask_layer ) );
            QtUtils::QtBridge::Connect( this->private_->ui_.border_selection_combo_, 
              mask_layer->border_state_ );
            QtUtils::QtBridge::Connect( this->private_->ui_.fill_selection_combo_, 
              mask_layer->fill_state_ );

            QtUtils::QtBridge::Connect( this->private_->color_widget_, 
              mask_layer->color_state_,
              PreferencesManager::Instance()->color_states_ );
            
            //this->private_->ui_.show_iso_surface_button_->setEnabled( false );
            //this->private_->ui_.delete_iso_surface_button_->setEnabled( false );
            
            QtUtils::QtBridge::Enable( this->private_->ui_.show_iso_surface_button_, 
              mask_layer->iso_generated_state_ );
            
            QtUtils::QtBridge::Enable( this->private_->ui_.delete_iso_surface_button_, 
              mask_layer->iso_generated_state_ );
            
            connect( this->private_->ui_.delete_iso_surface_button_, 
              SIGNAL( clicked() ), this, SLOT( uncheck_show_iso_button() ) );
          
            this->set_mask_background_color( mask_layer->color_state_->get() );
          }
        }
        break;
        
      // This is for the Label Layers
      case Core::VolumeType::LABEL_E:
        {
          // Update the icons
          this->private_->activate_button_->setIcon(this->private_->label_layer_icon_);
        }
        break;
        
      default:
        break;
    }   
  }
  
  // Set up the overlay widgets
  this->private_->overlay_ = new OverlayWidget( this );
  this->private_->overlay_->hide(); 
  this->private_->ui_.dummy_widget_->hide();
  
  this->update_widget_state( true );
  this->setUpdatesEnabled( true );
}
  
  
LayerWidget::~LayerWidget()
{
}

void LayerWidget::enable_buttons( bool lock_button, bool other_buttons, bool /*initialize*/ )
{
  this->private_->activate_button_->set_enabled( other_buttons );
  this->private_->ui_.opacity_button_->setEnabled( other_buttons );
  this->private_->ui_.visibility_button_->setEnabled( other_buttons );
  this->private_->ui_.color_button_->setEnabled( other_buttons );
  this->private_->ui_.compute_iso_surface_button_->setEnabled( other_buttons );
  this->private_->ui_.fill_border_button_->setEnabled( other_buttons );
  this->private_->ui_.volume_rendered_button_->setEnabled( other_buttons );
  this->private_->ui_.brightness_contrast_button_->setEnabled( other_buttons );
  this->private_->ui_.label_->setEnabled( other_buttons );
  this->private_->ui_.lock_button_->setEnabled( lock_button );
  
  // for the iso surface buttons on the mask layer
  if( this->get_volume_type() == Core::VolumeType::MASK_E )
  {
    if( other_buttons )
    { 
      MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( this->private_->layer_.get() ); 
      this->private_->ui_.show_iso_surface_button_->setEnabled( 
        mask_layer->iso_generated_state_->get() );
      this->private_->ui_.delete_iso_surface_button_->setEnabled( 
        mask_layer->iso_generated_state_->get() );
    }
    else
    {
      this->private_->ui_.show_iso_surface_button_->setEnabled( other_buttons );
      this->private_->ui_.delete_iso_surface_button_->setEnabled( other_buttons );
    }
  }

}
  
void LayerWidget::uncheck_show_iso_button()
{
  if( this->private_->ui_.show_iso_surface_button_->isChecked() )
  {
    this->private_->ui_.show_iso_surface_button_->setChecked( false );
  }
}

void LayerWidget::set_active_menu( std::string& menu_state, bool override, bool /*initialize*/ )
{
  bool opacity_menu = false;
  bool color_menu = false;
  bool contrast_menu = false;
  bool appearance_menu = false;

  if ( !override )
  {
    if ( menu_state == Layer::NO_MENU_C ) {}
    else if ( menu_state == Layer::OPACITY_MENU_C ) opacity_menu = true;
    else if ( menu_state == Layer::COLOR_MENU_C ) color_menu = true;
    else if ( menu_state == Layer::CONTRAST_MENU_C ) contrast_menu = true;
    else if ( menu_state == Layer::APPEARANCE_MENU_C ) appearance_menu = true;
  }
  
  this->private_->ui_.base_->setUpdatesEnabled( false );
  
  // Update the state of the opacity menu
  if ( opacity_menu )
  {
    if ( this->private_->ui_.opacity_bar_->isHidden() )   
    {
      this->private_->ui_.opacity_bar_->show();
      this->private_->ui_.opacity_button_->blockSignals( true );
      this->private_->ui_.opacity_button_->setChecked( true );
      this->private_->ui_.opacity_button_->blockSignals( false );
    }
  }
  else
  {
    if ( this->private_->ui_.opacity_bar_->isVisible() )    
    {
      this->private_->ui_.opacity_bar_->hide();
      this->private_->ui_.opacity_button_->blockSignals( true );
      this->private_->ui_.opacity_button_->setChecked( false );
      this->private_->ui_.opacity_button_->blockSignals( false );
    }
  }

  // Update the state of the color menu
  if ( color_menu )
  {
    if ( this->private_->ui_.color_bar_->isHidden() )   
    {
      this->private_->ui_.color_bar_->show();
      this->private_->ui_.color_button_->blockSignals( true );
      this->private_->ui_.color_button_->setChecked( true );
      this->private_->ui_.color_button_->blockSignals( false );
    }
  }
  else
  {
    if ( this->private_->ui_.color_bar_->isVisible() )    
    {
      this->private_->ui_.color_bar_->hide();
      this->private_->ui_.color_button_->blockSignals( true );
      this->private_->ui_.color_button_->setChecked( false );
      this->private_->ui_.color_button_->blockSignals( false );
    }
  }

  // Update the state of the contrast/brightness menu
  if ( contrast_menu )
  {
    if ( this->private_->ui_.bright_contrast_bar_->isHidden() )   
    {
      this->private_->ui_.bright_contrast_bar_->show();
      this->private_->ui_.brightness_contrast_button_->blockSignals( true );
      this->private_->ui_.brightness_contrast_button_->setChecked( true );
      this->private_->ui_.brightness_contrast_button_->blockSignals( false );
    }
  }
  else
  {
    if ( this->private_->ui_.bright_contrast_bar_->isVisible() )    
    {
      this->private_->ui_.bright_contrast_bar_->hide();
      this->private_->ui_.brightness_contrast_button_->blockSignals( true );
      this->private_->ui_.brightness_contrast_button_->setChecked( false );
      this->private_->ui_.brightness_contrast_button_->blockSignals( false );
    }
  }

  // Update the state of the contrast/brightness menu
  if ( appearance_menu )
  {
    if ( this->private_->ui_.border_bar_->isHidden() )    
    {
      this->private_->ui_.border_bar_->show();
      this->private_->ui_.fill_border_button_->blockSignals( true );
      this->private_->ui_.fill_border_button_->setChecked( true );
      this->private_->ui_.fill_border_button_->blockSignals( false );
    }
  }
  else
  {
    if ( this->private_->ui_.border_bar_->isVisible() )   
    {
      this->private_->ui_.border_bar_->hide();
      this->private_->ui_.fill_border_button_->blockSignals( true );
      this->private_->ui_.fill_border_button_->setChecked( false );
      this->private_->ui_.fill_border_button_->blockSignals( false );
    }
  }
  this->private_->ui_.base_->setUpdatesEnabled( true );
}

void LayerWidget::update_appearance( bool locked, bool active, bool initialize )
{
  if ( locked == this->private_->locked_ && 
     active == this->private_->active_ && initialize == false ) return;
  
  this->private_->locked_ = locked;
  this->private_->active_ = active;

  // Update the lock button
  this->private_->ui_.lock_button_->blockSignals( true );
  this->private_->ui_.lock_button_->setChecked( locked );
  this->private_->ui_.lock_button_->blockSignals( false );

  // Update the background color inside the icon
  switch( this->get_volume_type() )
  {
    case Core::VolumeType::DATA_E:
    {
      if ( locked )
      {
        this->private_->ui_.type_->setStyleSheet( StyleSheet::LAYER_WIDGET_BACKGROUND_LOCKED_C );
      }
      else if ( active )
      {
        this->private_->ui_.type_->setStyleSheet( StyleSheet::DATA_VOLUME_COLOR_C );      
      }
      else
      {
        this->private_->ui_.type_->setStyleSheet( StyleSheet::DATA_VOLUME_COLOR_C );
      }
    }
    break;
    case Core::VolumeType::MASK_E:
    {
      int color_index =  dynamic_cast< MaskLayer* >( ( LayerManager::Instance()->
        get_layer_by_id( this->get_layer_id() ) ).get() )->color_state_->get();
      
      this->set_mask_background_color( color_index );
    }
    break;
    case Core::VolumeType::LABEL_E:
    {
      if ( locked )
      {
        this->private_->ui_.type_->setStyleSheet( StyleSheet::LAYER_WIDGET_BACKGROUND_LOCKED_C );
      }
      else if ( active )
      {
        this->private_->ui_.type_->setStyleSheet( StyleSheet::LABEL_VOLUME_COLOR_C );
      }
      else
      {
        this->private_->ui_.type_->setStyleSheet( StyleSheet::LABEL_VOLUME_COLOR_C );
      }
    }
    break;
  }

  if ( locked )
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_LOCKED_C );
    this->private_->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_LOCKED_C ); 
  }
  else if( active )
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );  
    this->private_->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_ACTIVE_C );
  }
  else
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
    this->private_->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_INACTIVE_C );
  }
}

void LayerWidget::update_widget_state( bool initialize )
{
  // Step 1 : retrieve the state of the layer
  std::string data_state;
  std::string menu_state;
  bool visual_lock = false;
  bool active_layer = false;
  
  {
    // NOTE:
    // Since state can is changed on the application thread
    // We need to lock the state engine to prevent any change
    // to be made into the state manager.
    Layer::lock_type lock( Layer::GetMutex() );
    data_state = this->private_->layer_->data_state_->get();
    menu_state = this->private_->layer_->menu_state_->get();
    visual_lock = this->private_->layer_->visual_lock_state_->get();
    active_layer = ( LayerManager::Instance()->get_active_layer() == this->private_->layer_ );
  }
  
  CORE_LOG_DEBUG( std::string(" menu state = ") + menu_state );
  CORE_LOG_DEBUG( std::string(" data state = ") + data_state );
  
  if ( data_state == Layer::AVAILABLE_C )
  {
    // Lock buttons if widget is locked
    this->enable_buttons( true, !visual_lock, initialize );
    // If locked no bars can be opened and adjust the option
    // menu to the actual menu that is open in the state manager
    this->set_active_menu( menu_state, visual_lock, initialize );
    
    // Change the color of the widget
    this->update_appearance( visual_lock,  active_layer, initialize );
    
    // Hide the progress bar
    this->show_progress_bar( false );
  }
  else if ( data_state == Layer::CREATING_C )
  {
    // Everything is still locked down
    enable_buttons( false, false, initialize );
    // Menus cannot be opened, so override them and close all of them
    set_active_menu( menu_state, true, initialize );

    // Change the color of the widget
    update_appearance( true,  false, initialize );  

    // Show the progress bar, since we are computing a new layer
    this->show_progress_bar( true );
  }
  else if ( data_state == Layer::PROCESSING_C )
  {
    // PROCESSING_C state
    // Lock buttons if widget is locked
    enable_buttons( true, !visual_lock, initialize );
    // If locked no bars can be opened and adjust the option
    // menu to the actual menu that is open in the state manager    
    set_active_menu( menu_state, visual_lock, initialize );
    
    // Change the color of the widget
    update_appearance( visual_lock,  active_layer, initialize );  
    
    // Show the progress bar, since we are computing a new layer
    this->show_progress_bar( true );
  }
}

std::string LayerWidget::get_layer_id() const
{ 
  return this->private_->layer_->get_layer_id(); 
}

int LayerWidget::get_volume_type() const
{ 
  return this->private_->layer_->type(); 
}

void LayerWidget::set_mask_background_color( int color_index )
{
  Core::Color color = PreferencesManager::Instance()->color_states_[ color_index ]->get();
    
  int r = static_cast< int >( color.r() );
  int g = static_cast< int >( color.g() );
  int b = static_cast< int >( color.b() );

  QString style_sheet = QString::fromUtf8( 
  "background-color: rgb(" ) + QString::number( r ) +
  QString::fromUtf8( ", " ) + QString::number( g ) +
  QString::fromUtf8( ", " ) + QString::number( b ) +
  QString::fromUtf8( ");" );

  this->private_->ui_.type_->setStyleSheet( style_sheet );
}

void LayerWidget::set_mask_background_color_from_preference_change( int color_index )
{
  if( dynamic_cast< MaskLayer* >( this->private_->layer_.get()  )->color_state_->get() != color_index )
    return;

  Core::Color color = PreferencesManager::Instance()->color_states_[ color_index ]->get();

  int r = static_cast< int >( color.r() );
  int g = static_cast< int >( color.g() );
  int b = static_cast< int >( color.b() );
  
  QString style_sheet = QString::fromUtf8( 
  "background-color: rgb(" ) + QString::number( r ) +
  QString::fromUtf8( ", " ) + QString::number( g ) +
  QString::fromUtf8( ", " ) + QString::number( b ) +
  QString::fromUtf8( ");" );

  this->private_->ui_.type_->setStyleSheet( style_sheet );
}

void LayerWidget::set_group_menu_status( bool status )
{
  this->group_menus_open_ = status;
}
  
void LayerWidget::mousePressEvent( QMouseEvent *event )
{
  // Exit immediately if they are no longer holding the button the press event isnt valid
  if( event->button() != Qt::LeftButton )
  { 
    return;
  }
  
  if( ( event->modifiers() != Qt::ControlModifier ) && ( event->modifiers() != Qt::ShiftModifier ) )
  {
    ActionActivateLayer::Dispatch( Core::Interface::GetWidgetActionContext(),
      LayerManager::Instance()->get_layer_by_id( this->get_layer_id() ) ); 
    return;
  }
  
  if( this->group_menus_open_ || this->layer_menus_open_ )
    return;

  QPoint hotSpot = event->pos();
  
  // Make up some mimedata containing the layer_id of the layer
  QMimeData *mimeData = new QMimeData;

  LayerHandle layer = this->private_->layer_;
  switch ( layer->type() ) 
  {
  case Core::VolumeType::DATA_E:
    mimeData->setText( QString::fromStdString( std::string("data|") + layer->get_layer_id() ) );
    break;
  case Core::VolumeType::MASK_E:
    mimeData->setText( QString::fromStdString( std::string("mask|") + layer->get_layer_id() ) );
    break;
  case Core::VolumeType::LABEL_E:
    mimeData->setText( QString::fromStdString( std::string("label|") + layer->get_layer_id() ) );
    break;
  default:
    break;
  }
  
  // Create a drag object and insert the hotspot
  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData );
  
  // here we add basically a screenshot of the widget
  drag->setPixmap( QPixmap::grabWidget( this ));
  drag->setHotSpot( hotSpot );

  // Next we hide the LayerWidget that we are going to be dragging.
  this->parentWidget()->setMinimumHeight( this->parentWidget()->height() );
  this->seethrough( true );

  Q_EMIT prep_for_drag_and_drop( true );
  
  // Finally if our drag was aborted then we reset the layers styles to be visible
  if( ( drag->exec(Qt::MoveAction, Qt::MoveAction) ) != Qt::MoveAction )
  {
    this->seethrough( false );
  }
  // Otherwise we dispatch our move function
  else 
  { 
    ActionMoveLayerAbove::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->get_layer_id(), this->private_->drop_layer_->get_layer_id() );
  }
  Q_EMIT prep_for_drag_and_drop( false );
  
  this->repaint();
  //this->parentWidget()->setMinimumHeight( 0 );
}

void LayerWidget::set_drop_target( LayerWidget* target_layer )
{
  this->private_->drop_layer_ = target_layer;
}

void LayerWidget::dropEvent( QDropEvent* event )
{
  std::vector<std::string> mime_data = 
    Core::SplitString( event->mimeData()->text().toStdString(), "|" );
  if( mime_data.size() < 2 ) return;

  if( this->get_layer_id() == mime_data[ 1 ] )
  {
    event->ignore();
    return;
  }
  
  bool good_to_go = false;
  
  if( !LayerManager::Instance()->check_for_same_group(
    mime_data[ 1 ], this->get_layer_id() ) )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "This move will modify the layer.  "
        "The new size of the layer will be: " )
        + QString::number( this->private_->layer_->get_grid_transform().get_nx() ) 
        + QString::fromUtf8( " x " )
        + QString::number( this->private_->layer_->get_grid_transform().get_ny() ) 
        + QString::fromUtf8( " x " ) 
        + QString::number( this->private_->layer_->get_grid_transform().get_nz() ) );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( QMessageBox::Yes == message_box.exec() )
    {
      good_to_go = true;
    }
  }
  else
  {
    good_to_go = true;
  }
  
  this->enable_drop_space( false );
  
  if( good_to_go )
  {
    dynamic_cast< LayerWidget* >( event->source() )->set_drop_target( this ); 
    event->setDropAction(Qt::MoveAction);
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void LayerWidget::dragEnterEvent( QDragEnterEvent* event)
{
  std::vector<std::string> mime_data = 
    Core::SplitString( event->mimeData()->text().toStdString(), "|" );
  if( mime_data.size() < 2 ) return;

  if ( ( ( this->get_volume_type() == Core::VolumeType::DATA_E ) &&
    ( mime_data[ 0 ] == "data" ) ) ||
    ( ( this->get_volume_type() == Core::VolumeType::MASK_E || 
      this->get_volume_type() == Core::VolumeType::LABEL_E ) &&
    ( mime_data[ 0 ] == "mask" || mime_data[ 0 ] == "label" ) ) && 
    ( this->get_layer_id() != mime_data[ 1 ] ) )
  {
    this->enable_drop_space( true );
    event->setDropAction(Qt::MoveAction);
    event->accept();
  }
  else
  {
    this->enable_drop_space( false );
    event->ignore();
  }
}

void LayerWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->enable_drop_space( false );
  event->accept();
}

void LayerWidget::seethrough( bool see )
{
  this->set_picked_up( see );
  
  if( see )
  {

    this->hide();
  }
  else
  {
    this->show();
  }
  this->repaint();
}

void LayerWidget::set_picked_up( bool picked_up )
{ 
  this->private_->picked_up_ = picked_up; 
}
  
void LayerWidget::enable_drop_space( bool drop )
{
  // First we check to see if it is picked up if so, we set change the way it looks
  if( this->private_->picked_up_ )
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_PICKED_UP_C );  
  }
  // If its not picked up, we set its color to indicate whether or not its a potential drop site
  else if( drop )
  {
    this->private_->overlay_->show();
    this->private_->drop_space_->show();
  }
  else
  {
    this->private_->drop_space_->hide();
    this->private_->overlay_->hide();
  }
  
  this->repaint();
} 

void LayerWidget::set_active( bool active )
{
  this->update_widget_state();
  this->repaint();
}

void LayerWidget::show_selection_checkbox( bool show )
{
  if( show && (!this->private_->ui_.lock_button_->isChecked()) )
  { 
    this->private_->ui_.checkbox_widget_->show();
  }
  else 
  {
    this->private_->ui_.checkbox_widget_->hide();
  }
}

void LayerWidget::select_opacity_bar( bool show )
{
  if ( show )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->private_->layer_->menu_state_, Layer::OPACITY_MENU_C );
  }
  else
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->layer_->menu_state_, Layer::NO_MENU_C );
  }
}

void LayerWidget::select_brightness_contrast_bar( bool show )
{
  if ( show )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->layer_->menu_state_, Layer::CONTRAST_MENU_C );
  }
  else
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->layer_->menu_state_, Layer::NO_MENU_C );
  }
}

void LayerWidget::select_border_fill_bar( bool show )
{
  if ( show )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->layer_->menu_state_, Layer::APPEARANCE_MENU_C );
  }
  else
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->layer_->menu_state_, Layer::NO_MENU_C );
  }
}

void LayerWidget::select_color_bar( bool show )
{
  if ( show )
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->layer_->menu_state_, Layer::COLOR_MENU_C );
  }
  else
  {
    Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
      this->private_->layer_->menu_state_, Layer::NO_MENU_C );
  }
}

void LayerWidget::show_progress_bar( bool show )
{
  if ( show )
  {
    this->private_->ui_.progress_bar_->setValue( 0 );
    this->private_->ui_.progress_bar_bar_->show();
  }
  else 
  {
    this->private_->ui_.progress_bar_bar_->hide();
  }
}

void LayerWidget::update_progress_bar( double progress )
{
  this->private_->ui_.progress_bar_->setValue( static_cast<int>( progress * 100.0 ) );
}


void LayerWidget::select_visual_lock( bool lock )
{
  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
    this->private_->layer_->visual_lock_state_, lock );
}

void LayerWidget::resizeEvent( QResizeEvent *event )
{
  this->private_->overlay_->resize( event->size() );
  event->accept();
}

void LayerWidget::prep_for_animation( bool move_time )
{
  if( move_time )
  {
    this->private_->ui_.dummy_widget_->setMinimumHeight( this->private_->ui_.base_->height() );
    this->private_->ui_.dummy_widget_->setMinimumWidth( this->private_->ui_.base_->width() );
    this->private_->ui_.dummy_widget_->setPixmap( QPixmap::grabWidget( this->private_->ui_.base_ ) );
    this->private_->ui_.base_->hide();
    this->private_->ui_.dummy_widget_->show();
  }
  else
  {
    this->private_->ui_.dummy_widget_->hide();
    this->private_->ui_.base_->show();
  } 
}

void LayerWidget::UpdateState( qpointer_type qpointer )
{
  // Hand it off to the right thread
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &LayerWidget::UpdateState, qpointer) );
    return; 
  }

  // When we are finally on the interface thread run this code:
  if ( qpointer.data() )
  {
    qpointer->setUpdatesEnabled( false );
    qpointer->update_widget_state();
    qpointer->setUpdatesEnabled( true );
    qpointer->repaint();
  }
}

void LayerWidget::UpdateProgress( qpointer_type qpointer, double progress )
{
  // Hand it off to the right thread
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &LayerWidget::UpdateProgress, 
      qpointer, progress) );
    return; 
  }

  // When we are finally on the interface thread run this code:
  if ( qpointer.data() )
  {
    qpointer->update_progress_bar( progress );
    qpointer->update();
  }
}


} //end namespace Seg3D
