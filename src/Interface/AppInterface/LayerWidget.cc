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
#include <Utils/Core/Log.h>

//Interface Includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/LayerWidget.h>

//UI Includes
#include "ui_LayerWidget.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>


namespace Seg3D
{

class LayerWidgetPrivate
{
public:
  Ui::LayerWidget ui_;
  
  SliderSpinComboDouble* opacity_adjuster_;
  SliderSpinComboDouble* brightness_adjuster_;
  SliderSpinComboDouble* contrast_adjuster_;
  
  std::string layer_id_;
  Utils::GridTransform grid_transform_;
};

LayerWidget::LayerWidget( QFrame* parent, LayerHandle layer ) :
  private_( new LayerWidgetPrivate )
{
  
  {// Prepare the icons!!
    this->label_layer_icon_.addFile( QString::fromUtf8( ":/Images/LabelMapWhite.png" ),
                    QSize(), QIcon::Normal, QIcon::Off );
    this->data_layer_icon_.addFile( QString::fromUtf8( ":/Images/DataWhite.png" ), QSize(),
                     QIcon::Normal, QIcon::Off );
  }
  
  // Set the parent of the widget for keeping track of it in Qt
  this->setParent( parent );
  
  // Add the Ui children onto the QWidget
  this->private_->ui_.setupUi( this );
  
  // Set the defaults
  // this is a default setting until we can get the name of the layer from the file or by some other means
  this->private_->ui_.label_->setText( QString::fromStdString( layer->get_layer_id() ) );
  
  // here we set the unique layer_id_ of the layer
  this->private_->layer_id_ = layer->get_layer_id();
  this->private_->grid_transform_ = layer->get_grid_transform();
  
  // hide the toolbars and the selection check box
  // hide the tool bars and the selection checkbox
  this->private_->ui_.color_bar_->hide();
  this->private_->ui_.bright_contrast_bar_->hide();
  this->private_->ui_.checkbox_widget_->hide();
  this->private_->ui_.opacity_bar_->hide();
  this->private_->ui_.progress_bar_bar_->hide();
  this->private_->ui_.border_bar_->hide();
  //this->private_->ui_.dimensions_->hide();
  
  // add the SliderSpinCombo Widgets
  this->private_->opacity_adjuster_ = new SliderSpinComboDouble( this->private_->ui_.opacity_bar_ );
  this->private_->ui_.verticalLayout_2->addWidget( this->private_->opacity_adjuster_ );
  this->private_->opacity_adjuster_->setObjectName( QString::fromUtf8( "opacity_adjuster_" ) );
  
  this->private_->brightness_adjuster_ = new SliderSpinComboDouble( this->private_->ui_.bright_contrast_bar_ );
  this->private_->ui_.brightness_h_layout_->addWidget( this->private_->brightness_adjuster_ );
  this->private_->brightness_adjuster_->setObjectName( QString::fromUtf8( "brightness_adjuster_" ) );
  
  this->private_->contrast_adjuster_ = new SliderSpinComboDouble( this->private_->ui_.bright_contrast_bar_ );
  this->private_->ui_.contrast_h_layout_->addWidget( this->private_->contrast_adjuster_ );
  this->private_->contrast_adjuster_->setObjectName( QString::fromUtf8( "contrast_adjuster_" ) );
  
  //--Temporary-- eventually we will set these with the state engine
  this->private_->ui_.border_selection_combo_->addItem( QString::fromUtf8( "No Border" ) );
  this->private_->ui_.border_selection_combo_->addItem( QString::fromUtf8( "Thin Border" ) );
  this->private_->ui_.border_selection_combo_->addItem( QString::fromUtf8( "Thick Border" ) );
  
  // connect the signals and slots
  connect( this->private_->ui_.opacity_button_, 
      SIGNAL( clicked( bool )), this, 
      SLOT( show_opacity_bar( bool )));
  connect( this->private_->ui_.brightness_contrast_button_, 
      SIGNAL( clicked( bool )), this, 
      SLOT( show_brightness_contrast_bar( bool )));
  connect( this->private_->ui_.fill_border_button_, 
      SIGNAL( clicked( bool )), this, 
      SLOT( show_border_fill_bar ( bool )));
  connect( this->private_->ui_.color_button_, 
      SIGNAL( clicked( bool )), this, 
      SLOT( show_color_bar( bool )));
  
  // this is only for testing until we have this hooked up through the state manager
  connect( this->private_->ui_.lock_button_, 
      SIGNAL( clicked( bool )), this, 
      SLOT( visual_lock( bool )));
  
  
  // make the default connections, for any layer type, to the state engine
  QtBridge::Connect( this->private_->ui_.selection_checkbox_, layer->selected_state_ );
  QtBridge::Connect( this->private_->ui_.lock_button_, layer->lock_state_ );
  QtBridge::Connect( this->private_->opacity_adjuster_, layer->opacity_state_ );
  
  switch( layer->type() )
  {
    // This if for the Data Layers
    case Utils::VolumeType::DATA_E:
      {
        this->private_->ui_.color_button_->hide();
        this->private_->ui_.compute_iso_surface_button_->hide();
        this->private_->ui_.fill_border_button_->hide();
        this->private_->ui_.iso_surface_button_->hide();
        this->private_->ui_.typeBackground_->setStyleSheet(QString::fromUtf8("QWidget#typeBackground_{ background-color: rgb(166, 12, 73); }"));
        this->private_->ui_.colorChooseButton_->setIcon(this->data_layer_icon_);
        
        DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
        if ( data_layer == 0 ) SCI_THROW_LOGICERROR("Pointer is empty");
        QtBridge::Connect( this->private_->brightness_adjuster_, data_layer->brightness_state_ );
        QtBridge::Connect( this->private_->contrast_adjuster_, data_layer->contrast_state_ );
      }
      break;
    // This is for the Mask Layers  
    case Utils::VolumeType::MASK_E:
      {
        this->private_->ui_.brightness_contrast_button_->hide();
        this->private_->ui_.volume_rendered_button_->hide();
        
        MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );
        if ( mask_layer == 0 ) SCI_THROW_LOGICERROR("Pointer is empty");
        
        QtBridge::Connect( this->private_->ui_.iso_surface_button_, mask_layer->show_isosurface_state_ );
        QtBridge::Connect( this->private_->ui_.border_selection_combo_, mask_layer->fill_state_ );
      }
      break;
      
    // This is for the Label Layers
    case 3:
      
      break;
      
    default:
      break;
  }
}
  
  
LayerWidget::~LayerWidget()
{
}
  
void LayerWidget::show_selection_checkbox( bool show )
{
  if( show )
  { 
    this->private_->ui_.checkbox_widget_->show();
  }
  else 
  {
    this->private_->ui_.checkbox_widget_->hide();
    this->private_->ui_.selection_checkbox_->setChecked( false );
  }
}

void LayerWidget::show_opacity_bar( bool show )
{
  if ( show )
  {
    this->private_->ui_.opacity_bar_->show();
    
    this->private_->ui_.bright_contrast_bar_->hide();
    this->private_->ui_.brightness_contrast_button_->setChecked( false );
    
    this->private_->ui_.color_bar_->hide();
    this->private_->ui_.color_button_->setChecked( false );
    
    this->private_->ui_.border_bar_->hide();
    this->private_->ui_.fill_border_button_->setChecked( false );

  }
  else
  {
    this->private_->ui_.opacity_bar_->hide();
  }
}

void LayerWidget::show_brightness_contrast_bar( bool show )
{
  if ( show )
  {
    this->private_->ui_.bright_contrast_bar_->show();
    
    this->private_->ui_.opacity_bar_->hide();
    this->private_->ui_.opacity_button_->setChecked( false );
    
    this->private_->ui_.color_bar_->hide();
    this->private_->ui_.color_button_->setChecked( false );
    
    this->private_->ui_.border_bar_->hide();
    this->private_->ui_.fill_border_button_->setChecked( false );

  }
  else
  {
    this->private_->ui_.bright_contrast_bar_->hide();
  }
}

void LayerWidget::show_border_fill_bar( bool show )
{
  if ( show )
  {
    this->private_->ui_.border_bar_->show();
    
    this->private_->ui_.opacity_bar_->hide();
    this->private_->ui_.opacity_button_->setChecked( false );
    
    this->private_->ui_.color_bar_->hide();
    this->private_->ui_.color_button_->setChecked( false );
    
    this->private_->ui_.bright_contrast_bar_->hide();
    this->private_->ui_.brightness_contrast_button_->setChecked( false );
    
  }
  else
  {
    this->private_->ui_.border_bar_->hide();
  }
}

void LayerWidget::show_color_bar( bool show )
{
  if ( show )
  {
    this->private_->ui_.color_bar_->show();
    
    this->private_->ui_.opacity_bar_->hide();
    this->private_->ui_.opacity_button_->setChecked( false );
    
    this->private_->ui_.border_bar_->hide();
    this->private_->ui_.fill_border_button_->setChecked( false );
    
    this->private_->ui_.bright_contrast_bar_->hide();
    this->private_->ui_.brightness_contrast_button_->setChecked( false );
    
  }
  else
  {
    this->private_->ui_.color_bar_->hide();
  }
}

void LayerWidget::show_progress_bar( bool show )
{
  if ( show )
  {
    this->private_->ui_.progress_bar_bar_->show();
  }
  else 
  {
    this->private_->ui_.progress_bar_bar_->hide();
  }
}

void LayerWidget::visual_lock( bool lock )
{
  if ( lock )
  {
    this->private_->ui_.header_->setStyleSheet(
      QString::fromUtf8(
         "QWidget#header_{"
         "background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:0.960227, stop:0 rgba(201, 201, 201, 255), stop:0.155779 rgba(208, 208, 208, 255), stop:1 rgba(184, 184, 184, 255));"
         "border-radius: 6px;}" ) );
    this->private_->ui_.typeBackground_->setStyleSheet( 
      QString::fromUtf8(
         "QWidget#typeBackground_{"
         "background-color: gray;"
         "border: 1px solid rgb(141, 141, 141);"
         "border-radius: 4px;}" ) );
    
    this->private_->ui_.label_->setStyleSheet( 
      QString::fromUtf8(
         "QLineEdit#label_{background-color:rgb(208, 208, 208); color: gray;}" ) );
    this->private_->ui_.colorChooseButton_->setEnabled( false );
    this->private_->ui_.opacity_button_->setEnabled( false );
    this->private_->ui_.visibility_button_->setEnabled( false );
    this->private_->ui_.color_button_->setEnabled( false );
    this->private_->ui_.compute_iso_surface_button_->setEnabled( false );
    this->private_->ui_.iso_surface_button_->setEnabled( false );
    this->private_->ui_.fill_border_button_->setEnabled( false );
    this->private_->ui_.volume_rendered_button_->setEnabled( false );
    this->private_->ui_.brightness_contrast_button_->setEnabled( false );
    this->private_->ui_.checkbox_widget_->hide();
    this->private_->ui_.color_bar_->hide();
    this->private_->ui_.progress_bar_bar_->hide();
    this->private_->ui_.border_bar_->hide();
    this->private_->ui_.opacity_bar_->hide();
    this->private_->ui_.bright_contrast_bar_->hide();
    
  }
  else
  {
    this->private_->ui_.header_->setStyleSheet(
      QString::fromUtf8(
         "QWidget#header_{"
         "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.512563 rgba(255, 255, 255, 0));}" ) );
    this->private_->ui_.typeBackground_->setStyleSheet( 
      QString::fromUtf8(
          "QWidget#typeBackground_{"
          "background-color: rgb(255, 128, 0);"
          "border: 1px solid rgb(141, 141, 141);"
          "border-radius: 4px;}" ) );
    this->private_->ui_.label_->setStyleSheet( 
      QString::fromUtf8( 
          "QLineEdit#label_{"
          "text-align: left;"
          "color: black;"
          "margin-right: 3px;"
          "background-color: rgb(243, 243, 243);}" ) );
    
    this->private_->ui_.colorChooseButton_->setEnabled( true );
    this->private_->ui_.opacity_button_->setEnabled( true );
    this->private_->ui_.visibility_button_->setEnabled( true );
    this->private_->ui_.color_button_->setEnabled( true );
    this->private_->ui_.compute_iso_surface_button_->setEnabled( true );
    this->private_->ui_.iso_surface_button_->setEnabled( true );
    this->private_->ui_.fill_border_button_->setEnabled( true );
    this->private_->ui_.volume_rendered_button_->setEnabled( true );
    this->private_->ui_.brightness_contrast_button_->setEnabled( true );
    
  }
}
  
} //end namespace Seg3D

