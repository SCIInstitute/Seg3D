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
#include <Interface/AppInterface/LayerGroupWidget.h>

//UI Includes
#include "ui_LayerGroupWidget.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/Actions/ActionDeleteLayers.h>
#include <Application/LayerManager/Actions/ActionNewMaskLayer.h>
#include <Application/LayerManager/Actions/ActionInsertLayerAbove.h>



namespace Seg3D
{
  
class LayerGroupWidgetPrivate
{
public:
  Ui::LayerGroupWidget ui_;
  
  SliderDoubleCombo* center_x_adjuster_crop;
  SliderDoubleCombo* center_y_adjuster_crop;
  SliderDoubleCombo* center_z_adjuster_crop;
  
  SliderDoubleCombo* size_width_adjuster_crop;
  SliderDoubleCombo* size_height_adjuster_crop;
  SliderDoubleCombo* size_depth_adjuster_crop;
  
    SliderDoubleCombo* scale_adjuster;
  
  std::string group_id_;
  Utils::GridTransform grid_transform_;
  int current_height;
  int current_width;
  int current_depth;
  
};
  
LayerGroupWidget::LayerGroupWidget( QWidget* parent, LayerHandle layer ) :
  private_( new LayerGroupWidgetPrivate )
{
    LayerGroupHandle group = layer->get_layer_group();

  this->setParent( parent );
  
  this->private_->ui_.setupUi( this );
  
  this->private_->group_id_ = group->get_group_id();
  
  // Set up the Drag and Drop
  this->setAcceptDrops( true );
  //this->dragged_layer_ = " ";

  // set some values of the GUI

  //this->private_->ui_.activate_button_->setText( QString::fromStdString( group->get_grid_transform().get_as_string() ) );
  std::string group_name = Utils::to_string( group->get_grid_transform().get_nx() ) + " x " +
    Utils::to_string( group->get_grid_transform().get_ny() ) + " x " +
    Utils::to_string( group->get_grid_transform().get_nz() );
  this->private_->ui_.activate_button_->setText( QString::fromStdString( group_name ) );

  // hide the tool bars 
  this->private_->ui_.roi_->hide();
  this->private_->ui_.resample_->hide();
  this->private_->ui_.flip_rotate_->hide();
  this->private_->ui_.transform_->hide();
  this->private_->ui_.delete_->hide();
  this->private_->ui_.delete_button_->setEnabled( false );

  //this->private_->ui_.open_button_->setChecked( true );
  
  // add the slider spinner combo's for the crop
  this->private_->center_x_adjuster_crop = new SliderDoubleCombo( this->private_->ui_.widget );
  this->private_->ui_.horizontalLayout_11->addWidget( this->private_->center_x_adjuster_crop );
  this->private_->center_x_adjuster_crop->setObjectName( QString::fromUtf8( "center_x_adjuster_crop" ) );
  
  this->private_->center_y_adjuster_crop = new SliderDoubleCombo( this->private_->ui_.widget_2 );
  this->private_->ui_.horizontalLayout_12->addWidget( this->private_->center_y_adjuster_crop );
  this->private_->center_y_adjuster_crop->setObjectName( QString::fromUtf8( "center_y_adjuster_crop" ) );
  
  this->private_->center_z_adjuster_crop = new SliderDoubleCombo( this->private_->ui_.widget_3 );
  this->private_->ui_.horizontalLayout_14->addWidget( this->private_->center_z_adjuster_crop );
  this->private_->center_z_adjuster_crop->setObjectName( QString::fromUtf8( "center_z_adjuster_crop" ) );
  
  this->private_->size_height_adjuster_crop = new SliderDoubleCombo( this->private_->ui_.widget_4 );
  this->private_->ui_.horizontalLayout_7->addWidget( this->private_->size_height_adjuster_crop );
  this->private_->size_height_adjuster_crop->setObjectName( QString::fromUtf8( "size_height_adjuster_crop" ) );
  
  this->private_->size_width_adjuster_crop = new SliderDoubleCombo( this->private_->ui_.widget_5 );
  this->private_->ui_.horizontalLayout_9->addWidget( this->private_->size_width_adjuster_crop );
  this->private_->size_width_adjuster_crop->setObjectName( QString::fromUtf8( "size_width_adjuster_crop" ) );
  
  this->private_->size_depth_adjuster_crop = new SliderDoubleCombo( this->private_->ui_.widget_6 );
  this->private_->ui_.horizontalLayout_10->addWidget( this->private_->size_depth_adjuster_crop );
  this->private_->size_depth_adjuster_crop->setObjectName( QString::fromUtf8( "size_depth_adjuster_crop" ) );
  
  this->private_->scale_adjuster = new SliderDoubleCombo( this->private_->ui_.widget_7 );
  this->private_->ui_.horizontalLayout_15->addWidget( this->private_->scale_adjuster );
  this->private_->scale_adjuster->setObjectName( QString::fromUtf8( "scale_adjuster" ) );
  
  // set some local values for the current size
  this->private_->current_width = static_cast<int>( group->get_grid_transform().get_nx() );
  this->private_->current_height = static_cast<int>( group->get_grid_transform().get_ny() );
  this->private_->current_depth = static_cast<int>( group->get_grid_transform().get_nz() );
  
  //  connect the gui signals and slots
    connect( this->private_->scale_adjuster, SIGNAL( valueAdjusted( double ) ), this, SLOT( adjust_new_size_labels( double )) );
  connect( this->private_->ui_.open_button_, SIGNAL( toggled( bool ) ), this, SLOT( show_layers( bool )) );
  connect( this->private_->ui_.group_resample_button_, SIGNAL( clicked( bool ) ), this, SLOT( show_resample( bool )) );
  connect( this->private_->ui_.group_crop_button_, SIGNAL( clicked( bool ) ), this, SLOT( show_crop( bool )) );
  connect( this->private_->ui_.group_transform_button_, SIGNAL( clicked( bool ) ), this, SLOT( show_transform( bool )) );
  connect( this->private_->ui_.group_flip_rotate_button_, SIGNAL( clicked ( bool ) ), this, SLOT( show_flip_rotate( bool )) );
  connect( this->private_->ui_.group_delete_button_, SIGNAL( clicked ( bool ) ), this, SLOT( show_delete( bool )) );
  connect( this->private_->ui_.confirm_delete_checkbox_, SIGNAL( clicked ( bool ) ), this, SLOT( enable_delete_button( bool )) );
  connect( this->private_->ui_.delete_button_, SIGNAL( clicked () ), this, SLOT( uncheck_delete_confirm() ) );
  
  // Add all current layer to the new group
  this->add_layer( layer );

  
  //Set the defaulf values for the Group UI and make the connections to the state engine
      // --- GENERAL ---
      this->private_->ui_.open_button_->setChecked( group->show_layers_state_.get() );
      this->private_->ui_.group_visibility_button_->setChecked(group->visibility_state_.get() );
      
      QtBridge::Connect( this->private_->ui_.open_button_, group->show_layers_state_ );
      QtBridge::Connect( this->private_->ui_.group_visibility_button_, group->visibility_state_ );
      QtBridge::Connect( this->private_->ui_.delete_button_, boost::bind( &ActionDeleteLayers::Dispatch, group ) );
      QtBridge::Connect( this->private_->ui_.group_new_button_, boost::bind( &ActionNewMaskLayer::Dispatch, group ) );
  
  
      // --- RESAMPLE ---
      // = set the default values
      this->private_->ui_.x_axis_label_current_->setText( QString::fromUtf8("X: ") + 
      QString::number(group->get_grid_transform().get_nx()) );
      this->private_->ui_.y_axis_label_current_->setText( QString::fromUtf8("Y: ") + 
      QString::number(group->get_grid_transform().get_ny()) );
      this->private_->ui_.z_axis_label_current_->setText( QString::fromUtf8("Z: ") + 
      QString::number(group->get_grid_transform().get_nz()) );
      
      this->private_->ui_.x_axis_label_new_->setText( QString::fromUtf8("X: ") + 
      QString::number(group->get_grid_transform().get_nx()) );
      this->private_->ui_.y_axis_label_new_->setText( QString::fromUtf8("Y: ") + 
      QString::number(group->get_grid_transform().get_ny()) );
      this->private_->ui_.z_axis_label_new_->setText( QString::fromUtf8("Z: ") + 
      QString::number(group->get_grid_transform().get_nz()) );
      
      QFont font;
        font.setPointSize(8);
        this->private_->ui_.x_axis_label_current_->setFont(font);
        this->private_->ui_.y_axis_label_current_->setFont(font);
        this->private_->ui_.z_axis_label_current_->setFont(font);
        
        font.setBold(true);
        this->private_->ui_.x_axis_label_new_->setFont(font);
        this->private_->ui_.y_axis_label_new_->setFont(font);
        this->private_->ui_.z_axis_label_new_->setFont(font);
        
        this->private_->ui_.resample_replace_checkBox_->setChecked( 
      group->resample_replace_state_->get() );
        
        // set the defaults for the upper threshold
        double resample_min = 0.0; 
      double resample_max = 0.0;
      double resample_step = 0.0;
      group->resample_factor_state_->get_step( resample_step );
      group->resample_factor_state_->get_range( resample_min, resample_max );
      this->private_->scale_adjuster->setStep( resample_step );
        this->private_->scale_adjuster->setRange( resample_min, resample_max );
        this->private_->scale_adjuster->setCurrentValue( group->resample_factor_state_->get() );
        
         // = make the connections
        QtBridge::Connect( this->private_->scale_adjuster, group->resample_factor_state_ );
      QtBridge::Connect( this->private_->ui_.resample_replace_checkBox_, 
      group->resample_replace_state_ );
        

        // --- CROP ---
        // = set the default values
        this->private_->size_width_adjuster_crop->setRange( 0, 
      group->get_grid_transform().get_nx() );
        this->private_->size_height_adjuster_crop->setRange( 0, 
      group->get_grid_transform().get_ny() );
        this->private_->size_depth_adjuster_crop->setRange( 0, 
      group->get_grid_transform().get_nz() );
        
        this->private_->size_width_adjuster_crop->setCurrentValue( 
      group->get_grid_transform().get_nx() );
        this->private_->size_height_adjuster_crop->setCurrentValue( 
      group->get_grid_transform().get_ny() );
        this->private_->size_depth_adjuster_crop->setCurrentValue( 
      group->get_grid_transform().get_nz() );

        this->private_->center_x_adjuster_crop->setRange( 0, group->get_grid_transform().get_nx() );
        this->private_->center_y_adjuster_crop->setRange( 0, group->get_grid_transform().get_ny() );
        this->private_->center_z_adjuster_crop->setRange( 0, group->get_grid_transform().get_nz() );
        
        this->private_->center_x_adjuster_crop->setCurrentValue( 0 );
        this->private_->center_y_adjuster_crop->setCurrentValue( 0 );
        this->private_->center_z_adjuster_crop->setCurrentValue( 0 );
        
        this->private_->ui_.crop_replace_checkBox_->setChecked( group->crop_replace_state_->get() );
        
        // = make the connections
        QtBridge::Connect( this->private_->size_width_adjuster_crop, group->crop_size_width_state_ );
        QtBridge::Connect( this->private_->size_height_adjuster_crop, group->crop_size_height_state_ );
        QtBridge::Connect( this->private_->size_depth_adjuster_crop, group->crop_size_depth_state_ );
        
        QtBridge::Connect( this->private_->center_x_adjuster_crop, group->crop_center_x_state_ );
        QtBridge::Connect( this->private_->center_y_adjuster_crop, group->crop_center_y_state_ );
        QtBridge::Connect( this->private_->center_z_adjuster_crop, group->crop_center_z_state_ );
        
        QtBridge::Connect( this->private_->ui_.crop_replace_checkBox_, group->crop_replace_state_ );
        
        
        // --- TRANSFORM ---
        // = set the default values
        this->private_->ui_.spacing_x_spinbox_->setValue( group->get_grid_transform().get_nx() );
        this->private_->ui_.spacing_y_spinbox_->setValue( group->get_grid_transform().get_ny() );
        this->private_->ui_.spacing_z_spinbox_->setValue( group->get_grid_transform().get_nz() );
        
        this->private_->ui_.transform_replace_checkBox_->setChecked( group->resample_replace_state_->get() );
        
        // = make the connections
      QtBridge::Connect( this->private_->ui_.origin_x_spinbox_, group->transform_origin_x_state_ );
      QtBridge::Connect( this->private_->ui_.origin_y_spinbox_, group->transform_origin_y_state_ );
      QtBridge::Connect( this->private_->ui_.origin_z_spinbox_, group->transform_origin_z_state_ );
      
      QtBridge::Connect( this->private_->ui_.spacing_x_spinbox_, group->transform_spacing_x_state_ );
      QtBridge::Connect( this->private_->ui_.spacing_y_spinbox_, group->transform_spacing_y_state_ );
      QtBridge::Connect( this->private_->ui_.spacing_z_spinbox_, group->transform_spacing_z_state_ );
        
        QtBridge::Connect( this->private_->ui_.transform_replace_checkBox_, group->transform_replace_state_ );
        
        

}
  
LayerGroupWidget::~LayerGroupWidget()
{
}
  
void LayerGroupWidget::add_layer( LayerHandle layer )
{
  LayerWidget_handle new_layer_handle( new LayerWidget(this->private_->ui_.group_frame_, layer ) );
  this->private_->ui_.group_frame_layout_->addWidget( new_layer_handle.data() );
  new_layer_handle->set_active( layer->get_active() );
  this->layer_list_.push_back( new_layer_handle );
}

void LayerGroupWidget::delete_layer( LayerHandle layer )
{
    for( QVector< LayerWidget_handle >::iterator i = layer_list_.begin(); i != 
        layer_list_.end(); ++i)
  {
        if( layer->get_layer_id() == ( *i )->get_layer_id() )
      {
          ( *i )->deleteLater();
          layer_list_.erase( i );
          this->repaint();
          return;
      }
  }    
}


void LayerGroupWidget::set_active_layer( LayerHandle layer )
{
    for( int i = 0; i < layer_list_.size(); ++i)
  {
      if( layer->get_layer_id() == layer_list_[i]->get_layer_id() )
      {
          layer_list_[i]->set_active( true );
      }
      else
      {
          layer_list_[i]->set_active( false );
      }
  }
  this->repaint();
}

void  LayerGroupWidget::set_active( bool active )
{
    if( active )
    {
        this->private_->ui_.base_->setStyleSheet( QString::fromUtf8(
                    "QWidget#base_{"
                  "background-color: rgb(150, 150, 150);\n"
                  "border-radius:6px;\n"
                  "border: 1px solid rgba(80, 80, 80, 255);\n"
                  "color: rgb(90,90,90);\n"
                  "}\n"));
                  
    this->private_->ui_.group_background_->setStyleSheet(  QString::fromUtf8(  
                    "QWidget#group_background_{\n"
                    "background-color: rgb(255, 128, 0);\n"
                    "border-radius: 6px;\n"
                    "color: white;\n"
                    "}\n"));
                    
    this->private_->ui_.activate_button_->setStyleSheet(  QString::fromUtf8(
                      "QPushButton#activate_button_{\n"
                      "background-color:rgba(0, 0, 0, 0);\n"
                      "border-color: rgba(0, 0, 0, 0);\n"
                      "border: none;\n"
                      "height: 24px;\n"
                      "text-align: left;\n"
                      "color: white;\n"
                      "margin: 0 0 0 0;\n"
                      "padding: 0 0 0 0;\n"
                      "}\n"));  
    }
    else
    {
        this->private_->ui_.base_->setStyleSheet(QString::fromUtf8(
                    "QWidget#base_{"
                  "background-color: rgb(110, 110, 110);\n"
                  "border-radius:6px;\n"
                  "border: 1px solid rgb(80, 80, 80);\n"
                  "}\n"));
        this->private_->ui_.group_background_->setStyleSheet(  QString::fromUtf8(  
                    "QWidget#group_background_{\n"
                    "background-color: rgb(210, 210, 210);\n"
                    "border-radius: 6px;\n"
                    "}\n"));
                    
      this->private_->ui_.activate_button_->setStyleSheet(  QString::fromUtf8(
                      "QPushButton#activate_button_{\n"
                      "background-color:rgba(0, 0, 0, 0);\n"
                      "border-color: rgba(0, 0, 0, 0);\n"
                      "border: none;\n"
                      "height: 24px;\n"
                      "text-align: left;\n"
                      "color: rgb( 90, 90, 90 );\n"
                      "margin: 0 0 0 0;\n"
                      "padding: 0 0 0 0;\n"
                      "}\n"));
                      
      
                    
    }
}


std::string& LayerGroupWidget::get_group_id()
{
  return this->private_->group_id_;
}

LayerWidget_handle LayerGroupWidget::check_for_layer( const std::string &layer )
{
  for( int i = 0; i < static_cast< int >( this->layer_list_.size() ); ++i )
  {
    if( this->layer_list_[i]->get_layer_id() == layer )
    {
      return this->layer_list_[i];
    }
  }
  return LayerWidget_handle();
}
  
  
void LayerGroupWidget::show_selection_checkboxes( bool show )
{
  for( int i = 0; i < layer_list_.size(); ++i)
  {
    layer_list_[i]->show_selection_checkbox( show );
  }
}

void LayerGroupWidget::adjust_new_size_labels( double scale_factor )
{
    this->private_->ui_.x_axis_label_new_->setText( QString::fromUtf8("X: ") + QString::number(this->private_->current_width * scale_factor ) );
  this->private_->ui_.y_axis_label_new_->setText( QString::fromUtf8("Y: ") + QString::number(this->private_->current_height * scale_factor ) );
  this->private_->ui_.z_axis_label_new_->setText( QString::fromUtf8("Z: ") + QString::number(this->private_->current_depth * scale_factor ) );
}
  
  
void LayerGroupWidget::enable_delete_button( bool enable )
{
  this->private_->ui_.delete_button_->setEnabled( enable );
}

void LayerGroupWidget::uncheck_delete_confirm()
{
    this->private_->ui_.confirm_delete_checkbox_->setChecked( false );
    this->private_->ui_.delete_button_->setEnabled( false );
    show_delete( false );
}

void LayerGroupWidget::show_layers( bool show )
{
  if (show) 
  {
    this->private_->ui_.group_frame_->show();
    this->private_->ui_.group_tools_->show();
  }
  else
  {
    this->private_->ui_.group_frame_->hide();
    this->private_->ui_.group_tools_->hide();
  }
}


void LayerGroupWidget::show_resample( bool show )
{
  if (show) 
  {
    this->private_->ui_.resample_->show();
    
    this->private_->ui_.roi_->hide();
    this->private_->ui_.group_crop_button_->setChecked( false );
    
    this->private_->ui_.delete_->hide();
    this->private_->ui_.group_delete_button_->setChecked( false );
    
    this->private_->ui_.flip_rotate_->hide();
    this->private_->ui_.group_flip_rotate_button_->setChecked( false );
    
    this->private_->ui_.transform_->hide();
    this->private_->ui_.group_transform_button_->setChecked( false );
    
  }
  else
  {
    this->private_->ui_.resample_->hide();
  }
  show_selection_checkboxes( show );
}

void LayerGroupWidget::show_transform( bool show )
{
  if (show) 
  {
    this->private_->ui_.transform_->show();
    
    this->private_->ui_.roi_->hide();
    this->private_->ui_.group_crop_button_->setChecked( false );
    
    this->private_->ui_.delete_->hide();
    this->private_->ui_.group_delete_button_->setChecked( false );
    
    this->private_->ui_.flip_rotate_->hide();
    this->private_->ui_.group_flip_rotate_button_->setChecked( false );
    
    this->private_->ui_.resample_->hide();
    this->private_->ui_.group_resample_button_->setChecked( false );
  }
  else
  {
    this->private_->ui_.transform_->hide();
  }
  show_selection_checkboxes( show );
}

void LayerGroupWidget::show_crop( bool show )
{
  if (show) 
  {
    this->private_->ui_.roi_->show();
    
    this->private_->ui_.transform_->hide();
    this->private_->ui_.group_transform_button_->setChecked( false );
    
    this->private_->ui_.delete_->hide();
    this->private_->ui_.group_delete_button_->setChecked( false );
    
    this->private_->ui_.flip_rotate_->hide();
    this->private_->ui_.group_flip_rotate_button_->setChecked( false );
    
    this->private_->ui_.resample_->hide();
    this->private_->ui_.group_resample_button_->setChecked( false );
  }
  else
  {
    this->private_->ui_.roi_->hide();
  }
  show_selection_checkboxes( show );
}

void LayerGroupWidget::show_flip_rotate( bool show )
{
  if (show) 
  {
    this->private_->ui_.flip_rotate_->show();
    
    this->private_->ui_.transform_->hide();
    this->private_->ui_.group_transform_button_->setChecked( false );
    
    this->private_->ui_.delete_->hide();
    this->private_->ui_.group_delete_button_->setChecked( false );
    
    this->private_->ui_.roi_->hide();
    this->private_->ui_.group_crop_button_->setChecked( false );
    
    this->private_->ui_.resample_->hide();
    this->private_->ui_.group_resample_button_->setChecked( false );
  }
  else
  {
    this->private_->ui_.flip_rotate_->hide();
  }
  show_selection_checkboxes( show );
}

void LayerGroupWidget::show_delete( bool show )
{
  if (show) 
  {
    this->private_->ui_.delete_->show();
    
    this->private_->ui_.transform_->hide();
    this->private_->ui_.group_transform_button_->setChecked( false );
    
    this->private_->ui_.flip_rotate_->hide();
    this->private_->ui_.group_flip_rotate_button_->setChecked( false );
    
    this->private_->ui_.roi_->hide();
    this->private_->ui_.group_crop_button_->setChecked( false );
    
    this->private_->ui_.resample_->hide();
    this->private_->ui_.group_resample_button_->setChecked( false );
  }
  else
  {
    this->private_->ui_.delete_->hide();
    this->private_->ui_.group_delete_button_->setChecked( false );
  }
  show_selection_checkboxes( show );
}


void LayerGroupWidget::mousePressEvent(QMouseEvent *event)
{

  // First we try and get a LayerWidget_handle from the location
  LayerWidget_handle layer_to_drag = validate_location( event->pos());
  
  // Exit immediately if they are no longer holding the button the press event isnt valid
  if ( ( event->button() != Qt::LeftButton ) ||  !layer_to_drag )
  { 
    return;
  }
  
  // Calculate the location on the widget for the mouse to be holding
  // We have to account for the offset of the header in the GroupLayerWidget
  QPoint hotSpot = event->pos() - QPoint( 2, 25 ) - layer_to_drag->pos();
  
  // Create some Item data. - THIS IS CURRENTLY NOT REALLY BEING USED
  QByteArray itemData;
  itemData = ( QString::fromStdString( layer_to_drag->get_layer_id() ) ).toAscii();
  QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  dataStream << QPoint(hotSpot);

  // Make up some mimedata containing the layer_id of the layer
  QMimeData *mimeData = new QMimeData;
  mimeData->setData( "layer_id", itemData );
  mimeData->setText( QString::fromStdString( layer_to_drag->get_layer_id() ) );
  
  // Create a drag object and insert the hotspot, and mimedata
  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData );
  // here we add basically a screenshot of the widget
  drag->setPixmap( QPixmap::grabWidget( layer_to_drag.data()  ));
  drag->setHotSpot( hotSpot );

  // Next we hide the LayerWidget that we are going to be dragging.
  layer_to_drag->hide();
  
  // Here we either close the layer if we are successful with our drag, 
  //  or show it again if we aren't
  if (drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction) == Qt::MoveAction)
  { 
    layer_to_drag->close();
  }
  else
  {
    layer_to_drag->show();
    
    // Set the style sheets back to what they should be
    for( int i = 0; i < static_cast< int >( this->layer_list_.size() ); ++i )
    {           
      {
        this->layer_list_[i]->set_drop( false );
      }
    }
  }
}


void LayerGroupWidget::dragEnterEvent(QDragEnterEvent* event)
{
  LayerWidget_handle potential_drop_site = validate_location( event->pos());
  bool found_valid_layer = false;
  
  // If its a valid dropsite then we color the LayerWidget appropriately
  if ( potential_drop_site  && event->mimeData()->hasFormat("layer_id") ) 
  { 
    for( int i = 0; i < static_cast< int >( this->layer_list_.size() ); ++i )
    {
      if( this->layer_list_[i] == potential_drop_site ) 
      {
        //TODO: we need to add logic to prevent certain illegal drag and drop.
        this->layer_list_[i]->set_drop( true );
        found_valid_layer = true;
      }
      else
      {
        this->layer_list_[i]->set_drop( false );
      }
    }
    if( found_valid_layer )
    {
      event->acceptProposedAction();
      return;
    }
  }
  
  // If they aren't dragging a valid widget we ignore it
  event->ignore();
}

void LayerGroupWidget::dragLeaveEvent( QDragLeaveEvent * event )
{
  for( int i = 0; i < static_cast< int >( this->layer_list_.size() ); ++i )
  {
    this->layer_list_[i]->set_drop( false );
  }

}


void LayerGroupWidget::dropEvent(QDropEvent* event)
{
  // First we validate the location
  LayerWidget_handle drop_site = validate_location( event->pos());
  
  if ( drop_site)
  {
    // Here we see if the LayerWidget they are dragging is from the current layer
    LayerWidget_handle layer = this->check_for_layer( event->mimeData()->text().toStdString() );

    // if it's not we ask them if they want to resample the layer they are dragging
    if ( !layer )
    {
      // Make a messagebox that gets their resample response
      QMessageBox confirm_resample_messagebox;
      confirm_resample_messagebox.setText( QString::fromUtf8( 
        "\nMoving the file will resample the layer to " )
        + QString::number( this->private_->current_height ) 
        + QString::fromUtf8( " x " ) 
        + QString::number( this->private_->current_width )
        + QString::fromUtf8( " x " )
        + QString::number( this->private_->current_depth )
        + QString::fromUtf8( ".\t" ) );

      confirm_resample_messagebox.setInformativeText( QString::fromUtf8( 
        "Do you really want to do this?" ) );
      confirm_resample_messagebox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
      confirm_resample_messagebox.setDefaultButton( QMessageBox::Cancel );

      switch( confirm_resample_messagebox.exec() )
      {
        // If they do want to resample, we dispatch the move function
        // TODO: we need to actually resample the layer
        case QMessageBox::Yes:
          ActionInsertLayerAbove::Dispatch( event->mimeData()->text().toStdString(), 
            drop_site->get_layer_id() );
          break;

        default:
          // Anything else we just make sure that none of the layers are colored as drop
          //  zones.
          for( int i = 0; i < static_cast< int >( this->layer_list_.size() ); ++i )
          {           
            {
              this->layer_list_[i]->set_drop( false );
            }
          }
          break;
      }
    }
    else
    {
      // If they are just reordering the layers within the group we just do it.
      ActionInsertLayerAbove::Dispatch( event->mimeData()->text().toStdString(), 
        drop_site->get_layer_id() );
    }
  }
}



LayerWidget_handle LayerGroupWidget::validate_location(const QPoint &point )
{
  // because the header counts as part of the group we need to account for its offset
  QPoint header_difference = QPoint(2, 25);
  // we create a temporary rectangle to represent the size of the widget
  QRect rectangle;
  
  // Check all the layers in the list and see if our point is inside
  for( int i = 0; i < static_cast< int >( this->layer_list_.size() ); ++i )
  {
    rectangle = this->layer_list_[i]->geometry();
    rectangle.setTopLeft( rectangle.topLeft() + header_difference );
    rectangle.setTopRight( rectangle.topRight() + header_difference );
    rectangle.setBottomLeft( rectangle.bottomLeft() + header_difference );
    rectangle.setBottomRight( rectangle.bottomRight() + header_difference );
    if( rectangle.contains( point ) )
    {
      return this->layer_list_[i];
    }
  }
  return LayerWidget_handle();
}
  
}  //end namespace Seg3D






