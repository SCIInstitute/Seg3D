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
#include <Interface/AppInterface/StyleSheet.h>

//UI Includes
#include "ui_LayerWidget.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/Actions/ActionActivateLayer.h>


namespace Seg3D
{

class LayerWidgetPrivate
{
public:
  Ui::LayerWidget ui_;
  
  SliderDoubleCombo* opacity_adjuster_;
  SliderDoubleCombo* brightness_adjuster_;
  SliderDoubleCombo* contrast_adjuster_;
  
  std::string layer_id_;
  Utils::GridTransform grid_transform_;
  int volume_type_;
  bool active_;
};

LayerWidget::LayerWidget( QFrame* parent, LayerHandle layer ) :
  QWidget( parent ),
  private_( new LayerWidgetPrivate )
{
  
  {// Prepare the icons!!
    this->label_layer_icon_.addFile( QString::fromUtf8( ":/Images/LabelMapWhite.png" ),
                    QSize(), QIcon::Normal, QIcon::Off );
    this->data_layer_icon_.addFile( QString::fromUtf8( ":/Images/DataWhite.png" ), QSize(),
                     QIcon::Normal, QIcon::Off );
  }
  
  // Add the Ui children onto the QWidget
  this->setObjectName(QString::fromUtf8("LayerWidget"));
  this->private_->ui_.setupUi( this );
  
  
  // set some Drag and Drop stuff
  //this->setAcceptDrops( true );
  
  // Set the defaults
  // this is a default setting until we can get the name of the layer from the file or by some other means
  this->private_->ui_.label_->setText( QString::fromStdString( layer->name_state_->get()) );
  this->private_->ui_.label_->setAcceptDrops( false );
  
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

  
  // add the SliderCombo Widgets
  this->private_->opacity_adjuster_ = new SliderDoubleCombo( this->private_->ui_.opacity_bar_ );
  this->private_->ui_.verticalLayout_2->addWidget( this->private_->opacity_adjuster_ );
  this->private_->opacity_adjuster_->setObjectName( QString::fromUtf8( "opacity_adjuster_" ) );
  
  this->private_->brightness_adjuster_ = new SliderDoubleCombo( this->private_->ui_.bright_contrast_bar_ );
  this->private_->ui_.brightness_h_layout_->addWidget( this->private_->brightness_adjuster_ );
  this->private_->brightness_adjuster_->setObjectName( QString::fromUtf8( "brightness_adjuster_" ) );
  
  this->private_->contrast_adjuster_ = new SliderDoubleCombo( this->private_->ui_.bright_contrast_bar_ );
  this->private_->ui_.contrast_h_layout_->addWidget( this->private_->contrast_adjuster_ );
  this->private_->contrast_adjuster_->setObjectName( QString::fromUtf8( "contrast_adjuster_" ) );
  
  // --- set the values for the dropdown menu's using values from the state handles
  // -- set the border selection combo box's values 
  std::vector< std::string > temp_option_list = layer->border_mode_state_->option_list();
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
      this->private_->ui_.border_selection_combo_->addItem( QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.border_selection_combo_->setCurrentIndex(layer->border_mode_state_->index());
  
  // -- set the fill selection combo box's values 
  temp_option_list = layer->fill_mode_state_->option_list();
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
      this->private_->ui_.fill_selection_combo_->addItem( QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.fill_selection_combo_->setCurrentIndex(layer->fill_mode_state_->index());
  
  // set the default values for the slidercombo's
  // set the defaults for the opacity
        double opacity_min = 0.0; 
      double opacity_max = 0.0;
      double opacity_step = 0.0;
      layer->opacity_state_->get_step( opacity_step );
      layer->opacity_state_->get_range( opacity_min, opacity_max );
      this->private_->opacity_adjuster_->setStep( opacity_step );
        this->private_->opacity_adjuster_->setRange( opacity_min, opacity_max );
        this->private_->opacity_adjuster_->setCurrentValue( layer->opacity_state_->get() );


  // connect the GUI signals and slots
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
      SIGNAL( toggled( bool )), this, 
      SLOT( visual_lock( bool )));
      
    QtBridge::Connect( this->private_->ui_.activate_button_, 
    boost::bind( &ActionActivateLayer::Dispatch, layer ) );
  
  
  // make the default connections, for any layer type, to the state engine
  QtBridge::Connect( this->private_->ui_.selection_checkbox_, layer->selected_state_ );
  QtBridge::Connect( this->private_->ui_.lock_button_, layer->lock_state_ );
  QtBridge::Connect( this->private_->opacity_adjuster_, layer->opacity_state_ );
  QtBridge::Connect( this->private_->ui_.label_, layer->name_state_ );
    
  switch( layer->type() )
  {
    // This if for the Data Layers
    case Utils::VolumeType::DATA_E:
      {
        this->private_->ui_.color_button_->hide();
        this->private_->ui_.compute_iso_surface_button_->hide();
        this->private_->ui_.fill_border_button_->hide();
        this->private_->ui_.iso_surface_button_->hide();
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::DATA_VOLUME_COLOR_C );
        this->private_->ui_.activate_button_->setIcon(this->data_layer_icon_);
        
        DataLayer* data_layer = dynamic_cast< DataLayer* >( layer.get() );
        QtBridge::Connect( this->private_->brightness_adjuster_, data_layer->brightness_state_ );
        QtBridge::Connect( this->private_->contrast_adjuster_, data_layer->contrast_state_ );
        
        // set the defaults for the brightness
                double brightness_min = 0.0; 
                double brightness_max = 0.0;
                double brightness_step = 0.0;
                data_layer->brightness_state_->get_step( brightness_step );
                data_layer->brightness_state_->get_range( brightness_min, brightness_max );
                this->private_->brightness_adjuster_->setStep( brightness_step );
                this->private_->brightness_adjuster_->setRange( brightness_min, brightness_max );
                this->private_->brightness_adjuster_->setCurrentValue( data_layer->brightness_state_->get() );
                
                // set the defaults for the contrast
                double contrast_min = 0.0; 
                double contrast_max = 0.0;
                double contrast_step = 0.0;
                data_layer->contrast_state_->get_step( contrast_step );
                data_layer->contrast_state_->get_range( contrast_min, contrast_max );
                this->private_->contrast_adjuster_->setStep( contrast_step );
                this->private_->contrast_adjuster_->setRange( contrast_min, contrast_max );
                this->private_->contrast_adjuster_->setCurrentValue( data_layer->contrast_state_->get() );
        
        // keep track locally of what type we are
        this->private_->volume_type_ = 1;
      }
      break;
    // This is for the Mask Layers  
    case Utils::VolumeType::MASK_E:
      {
        this->private_->ui_.brightness_contrast_button_->hide();
        this->private_->ui_.volume_rendered_button_->hide();
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::MASK_VOLUME_COLOR_C );
        MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );        
        QtBridge::Connect( this->private_->ui_.iso_surface_button_, mask_layer->show_isosurface_state_ );
        QtBridge::Connect( this->private_->ui_.border_selection_combo_, mask_layer->fill_state_ );

          // keep track locally of what type we are
          this->private_->volume_type_ = 2;
      }
      break;
      
    // This is for the Label Layers
    case Utils::VolumeType::LABEL_E:
        {
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::LABEL_VOLUME_COLOR_C );
        
        // keep track locally of what type we are
            this->private_->volume_type_ = 3;
            
        }
      
      break;
      
    default:
      break;
  }
}
  
  
LayerWidget::~LayerWidget()
{
}

int LayerWidget::get_volume_type()
{
  return this->private_->volume_type_;
}

void LayerWidget::set_active( bool active )
{
  // keep track locally if we are an active layer or not so we know what color to revert to if locked
  this->private_->active_ = active;
  
    if( active )
    {
        this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );  
    }
    else
    {
        this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
    }
}

std::string& LayerWidget::get_layer_id()
{
  return this->private_->layer_id_;
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
    this->private_->ui_.header_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_LOCKED_C );
    this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::LAYER_WIDGET_BACKGROUND_LOCKED_C );
    this->private_->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_LOCKED_C );
    this->private_->ui_.activate_button_->setEnabled( false );
    this->private_->ui_.opacity_button_->setEnabled( false );
    this->private_->ui_.visibility_button_->setEnabled( false );
    this->private_->ui_.color_button_->setEnabled( false );
    this->private_->ui_.compute_iso_surface_button_->setEnabled( false );
    this->private_->ui_.iso_surface_button_->setEnabled( false );
    this->private_->ui_.fill_border_button_->setEnabled( false );
    this->private_->ui_.volume_rendered_button_->setEnabled( false );
    this->private_->ui_.brightness_contrast_button_->setEnabled( false );
    this->private_->ui_.label_->setEnabled( false );
    this->private_->ui_.checkbox_widget_->hide();
    this->private_->ui_.color_bar_->hide();
    this->private_->ui_.progress_bar_bar_->hide();
    this->private_->ui_.border_bar_->hide();
    this->private_->ui_.opacity_bar_->hide();
    this->private_->ui_.bright_contrast_bar_->hide();
  }
  else
  {
    switch( this->private_->volume_type_ )
      {
        case Utils::VolumeType::DATA_E:
        {
              this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::DATA_VOLUME_COLOR_C );
      }
        break;
        case Utils::VolumeType::MASK_E:
      {
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::MASK_VOLUME_COLOR_C );
      }
      break;
      case Utils::VolumeType::LABEL_E:
      {
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::LABEL_VOLUME_COLOR_C );
        }
        break;
        
        default:
      break;
    }
    
    if( this->private_->active_ )
    {
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );  
    }
    else
    {
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
    }
    
    this->private_->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_C );
    
    this->private_->ui_.activate_button_->setEnabled( true );
    this->private_->ui_.opacity_button_->setEnabled( true );
    this->private_->ui_.visibility_button_->setEnabled( true );
    this->private_->ui_.color_button_->setEnabled( true );
    this->private_->ui_.compute_iso_surface_button_->setEnabled( true );
    this->private_->ui_.iso_surface_button_->setEnabled( true );
    this->private_->ui_.fill_border_button_->setEnabled( true );
    this->private_->ui_.volume_rendered_button_->setEnabled( true );
    this->private_->ui_.brightness_contrast_button_->setEnabled( true );
    this->private_->ui_.label_->setEnabled( true );
    
  }
}

void LayerWidget::set_drop( bool drop )
{
  if( drop )
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_DROP_C );
    
  }
  else
  {
    if( this->private_->active_ )
    {
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );  
      
    }
    else
    {
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
    }
  }

}

} //end namespace Seg3D

