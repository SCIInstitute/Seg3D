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
#include <Interface/AppInterface/PushDragButton.h>
#include <Interface/AppInterface/DropSpaceWidget.h>
#include <Interface/AppInterface/OverlayWidget.h>
#include <Interface/AppInterface/ColorBarWidget.h>

//UI Includes
#include "ui_LayerWidget.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionActivateLayer.h>
#include <Application/LayerManager/Actions/ActionMoveLayerAbove.h>


namespace Seg3D
{

class LayerWidgetPrivate
{
public:
  Ui::LayerWidget ui_;
  SliderDoubleCombo* opacity_adjuster_;
  SliderDoubleCombo* brightness_adjuster_;
  SliderDoubleCombo* contrast_adjuster_;
  PushDragButton* activate_button_;
  DropSpaceWidget* drop_space_;
  OverlayWidget* overlay_;
  ColorBarWidget* color_widget_;
};

LayerWidget::LayerWidget( QFrame* parent, LayerHandle layer ) :
  QWidget( parent ),
  private_( new LayerWidgetPrivate ),
  layer_id_( layer->get_layer_id() ),
  grid_transform_( layer->get_grid_transform() ),
  active_( false ),
  picked_up_( false ),
  group_menus_open_( false ),
  layer_menus_open_( false ),
  volume_type_( layer->type() )
{
  
  
  {// Prepare the icons!!
    this->label_layer_icon_.addFile( QString::fromUtf8( ":/Images/LabelMapWhite.png" ),
                    QSize(), QIcon::Normal, QIcon::Off );
    this->data_layer_icon_.addFile( QString::fromUtf8( ":/Images/DataWhite.png" ), QSize(),
                     QIcon::Normal, QIcon::Off );
    this->mask_layer_icon_.addFile( QString::fromUtf8( ":/Images/MaskWhite_shadow.png" ), QSize(),
      QIcon::Normal, QIcon::Off );
  }

  // Add the Ui children onto the QWidget
  this->setObjectName( QString::fromUtf8( "LayerWidget" ) );
  this->setStyleSheet( StyleSheet::LAYERWIDGET_C );
  this->private_->ui_.setupUi( this );
  
  this->setUpdatesEnabled( false );
  
  // set some Drag and Drop stuff
  this->setAcceptDrops( true );
  
  // Set the defaults
  // this is a default setting until we can get the name of the layer from the file or by some other means
  this->private_->ui_.label_->setText( QString::fromStdString( layer->name_state_->get() ) );
  this->private_->ui_.label_->setAcceptDrops( false );

  // hide the toolbars and the selection check box
  // hide the tool bars and the selection checkbox
  this->private_->ui_.color_bar_->hide();
  this->private_->ui_.bright_contrast_bar_->hide();
  this->private_->ui_.checkbox_widget_->hide();
  this->private_->ui_.horizontalLayout_2->setAlignment( Qt::AlignHCenter );
  this->private_->ui_.opacity_bar_->hide();
  this->private_->ui_.progress_bar_bar_->hide();
  this->private_->ui_.border_bar_->hide();

  
  // add the PushDragButton
  this->private_->activate_button_ = new PushDragButton( this->private_->ui_.typeGradient_ );
  this->private_->activate_button_->setObjectName( QString::fromUtf8( "activate_button_" ) );
  this->private_->activate_button_->setStyleSheet( StyleSheet::PUSHDRAGBUTTON_C );
  this->private_->activate_button_->setMinimumHeight( 42 );
  this->private_->activate_button_->setMinimumWidth( 31 );
  this->private_->activate_button_->setIconSize( QSize( 25, 25 ) );
  this->private_->ui_.horizontalLayout_9->addWidget( this->private_->activate_button_ );
  this->private_->activate_button_->setAcceptDrops( false );
  this->private_->ui_.base_->setAcceptDrops( false );
  this->private_->ui_.header_->setAcceptDrops( false );
  this->private_->ui_.widget->setAcceptDrops( false );
  this->private_->ui_.widget_2->setAcceptDrops( false );
  this->private_->ui_.typeBackground_->setAcceptDrops( false );
  this->private_->ui_.typeGradient_->setAcceptDrops( false );
  
  // add the DropSpaceWidget
  this->private_->drop_space_ = new DropSpaceWidget( this );
  this->private_->ui_.verticalLayout_3->insertWidget( 0, this->private_->drop_space_ );
  
  this->private_->drop_space_->hide();
  
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
  
  this->private_->color_widget_ = new ColorBarWidget( this->private_->ui_.color_bar_ );
  this->private_->ui_.verticalLayout_5->addWidget( this->private_->color_widget_ );
  this->private_->color_widget_->setObjectName( QString::fromUtf8( "color_widget_" ) );
  
  
  // --- set the values for the dropdown menu's using values from the state handles
  // -- set the border selection combo box's values 
  std::vector< std::string > temp_option_list = layer->border_mode_state_->option_list();
  for( size_t i = 0; i < temp_option_list.size(); i++)
  {   
      this->private_->ui_.border_selection_combo_->addItem( QString::fromStdString( temp_option_list[i] ) );
  }
  // Set it's default value
  this->private_->ui_.border_selection_combo_->setCurrentIndex( layer->border_mode_state_->index() );
  
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
      SIGNAL( clicked( bool ) ), this, 
      SLOT( show_opacity_bar( bool ) ) );
  connect( this->private_->ui_.brightness_contrast_button_, 
      SIGNAL( clicked( bool ) ), this, 
      SLOT( show_brightness_contrast_bar( bool ) ) );
  connect( this->private_->ui_.fill_border_button_, 
      SIGNAL( clicked( bool ) ), this, 
      SLOT( show_border_fill_bar ( bool ) ) );
  connect( this->private_->ui_.color_button_, 
      SIGNAL( clicked( bool ) ), this, 
      SLOT( show_color_bar( bool ) ) );
  
  // this is only for testing until we have this hooked up through the state manager
  connect( this->private_->ui_.lock_button_, 
      SIGNAL( toggled( bool )), this, 
      SLOT( visual_lock( bool )));
      
    QtBridge::Connect( this->private_->activate_button_, 
    boost::bind( &ActionActivateLayer::Dispatch, layer ) );
  
  // make the default connections, for any layer type, to the state engine
  QtBridge::Connect( this->private_->ui_.selection_checkbox_, layer->selected_state_ );
  QtBridge::Connect( this->private_->ui_.lock_button_, layer->lock_state_ );
  QtBridge::Connect( this->private_->opacity_adjuster_, layer->opacity_state_ );
  QtBridge::Connect( this->private_->ui_.label_, layer->name_state_ );
    
  switch( this->volume_type_ )
  {
    // This if for the Data Layers
    case Utils::VolumeType::DATA_E:
      {
        this->private_->ui_.color_button_->hide();
        this->private_->ui_.compute_iso_surface_button_->hide();
        this->private_->ui_.fill_border_button_->hide();
        this->private_->ui_.iso_surface_button_->hide();
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::DATA_VOLUME_COLOR_C );
        this->private_->activate_button_->setIcon(this->data_layer_icon_);
        
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

      }
      break;
    // This is for the Mask Layers  
    case Utils::VolumeType::MASK_E:
      {
        this->private_->ui_.brightness_contrast_button_->hide();
        this->private_->ui_.volume_rendered_button_->hide();
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::MASK_VOLUME_COLOR_C );
        this->private_->activate_button_->setIcon( this->mask_layer_icon_ );
        MaskLayer* mask_layer = dynamic_cast< MaskLayer* >( layer.get() );  
        this->private_->color_widget_->set_color_index( mask_layer->color_state_->get() );
        QtBridge::Connect( this->private_->ui_.iso_surface_button_, mask_layer->show_isosurface_state_ );
        QtBridge::Connect( this->private_->ui_.border_selection_combo_, mask_layer->fill_state_ );
        QtBridge::Connect( this->private_->color_widget_, mask_layer->color_state_ );

      }
      break;
      
    // This is for the Label Layers
    case Utils::VolumeType::LABEL_E:
        {
        this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::LABEL_VOLUME_COLOR_C );
        this->private_->activate_button_->setIcon(this->label_layer_icon_);

        }
      break;
      
    default:
      break;
  }
  
  this->private_->overlay_ = new OverlayWidget( this );
  this->private_->overlay_->hide(); 
  
  this->setUpdatesEnabled( true );
}
  
  
LayerWidget::~LayerWidget()
{
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
  
  if( this->group_menus_open_ || this->layer_menus_open_ )
    return;

  QPoint hotSpot = event->pos();
  
  // Make up some mimedata containing the layer_id of the layer
  QMimeData *mimeData = new QMimeData;

  switch ( this->get_volume_type() ) 
  {
  case Utils::VolumeType::DATA_E:
    mimeData->setText( QString::fromStdString( std::string("data|") + this->get_layer_id() ) );
    break;
  case Utils::VolumeType::MASK_E:
    mimeData->setText( QString::fromStdString( std::string("mask|") + this->get_layer_id() ) );
    break;
  case Utils::VolumeType::LABEL_E:
    mimeData->setText( QString::fromStdString( std::string("label|") + this->get_layer_id() ) );
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
  
  // Finally if our drag was aborted then we reset the layers styles to be visible
  if( ( drag->exec(Qt::MoveAction, Qt::MoveAction) ) != Qt::MoveAction )
  {
    this->seethrough( false );
  }
  // Otherwise we dispatch our move function
  else 
  { 
    ActionMoveLayerAbove::Dispatch( this->get_layer_id(), this->drop_layer_->get_layer_id() );
  }
  this->parentWidget()->setMinimumHeight( 0 );
}

void LayerWidget::set_drop_target( LayerWidget* target_layer )
{
  this->drop_layer_ = target_layer;
}
  

void LayerWidget::dropEvent( QDropEvent* event )
{
  std::vector<std::string> mime_data = 
    Utils::SplitString( event->mimeData()->text().toStdString(), "|" );
  if( mime_data.size() < 2 ) return;

  if( this->get_layer_id() == mime_data[ 1 ] )
  {
    event->ignore();
    return;
  }
  
  if( this->group_menus_open_ || this->layer_menus_open_ )
    return;
  
  bool good_to_go = false;
  
  if( !LayerManager::Instance()->check_for_same_group(
    mime_data[ 1 ], this->get_layer_id() ) )
  {
    QMessageBox message_box;
    message_box.setText( QString::fromUtf8( "This move will modify the layer.  "
        "The new size of the layer will be: " )
        + QString::number( this->grid_transform_.get_nx() ) + QString::fromUtf8( " x " )
        + QString::number( this->grid_transform_.get_ny() ) + QString::fromUtf8( " x " ) 
        + QString::number( this->grid_transform_.get_nz() ) );
    message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
    message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    message_box.setDefaultButton( QMessageBox::No );
    if( message_box.exec() )
    {
      good_to_go = true;
    }
  }
  else
  {
    good_to_go = true;
  }
  
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
  this->setUpdatesEnabled( false );
  this->set_drop( false );
  this->private_->overlay_->hide();
  this->setUpdatesEnabled( true );
  this->repaint();

}

void LayerWidget::dragEnterEvent( QDragEnterEvent* event)
{
  this->private_->overlay_->show();
  
  std::vector<std::string> mime_data = 
    Utils::SplitString( event->mimeData()->text().toStdString(), "|" );
  if( mime_data.size() < 2 ) return;

  if( ( ( ( this->get_volume_type() == Utils::VolumeType::DATA_E ) &&
    ( mime_data[ 0 ] == "data" ) ) ||
    ( ( this->get_volume_type() == Utils::VolumeType::MASK_E ) &&
    ( mime_data[ 0 ] =="mask" ) ) ||
    ( ( this->get_volume_type() == Utils::VolumeType::LABEL_E ) &&
    ( mime_data[ 0 ] == "label") ) ) && 
    ( this->get_layer_id() != mime_data[ 1 ] ) )
  {
    this->set_drop( true );
    event->setDropAction(Qt::MoveAction);
    event->accept();
  }
  else
  {
    this->set_drop( false );
    this->private_->overlay_->hide();
    event->ignore();
  }
}

void LayerWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->set_drop( false );
  this->private_->overlay_->hide();
}
  


void LayerWidget::seethrough( bool see )
{
  this->set_picked_up( see );
  
  this->setUpdatesEnabled( false );
  if( see )
  {
// We enable some animation if it is a windows system
#if defined( _WIN32 )
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_PICKED_UP_C );
    this->private_->ui_.header_->hide();
    this->private_->ui_.border_bar_->hide();
    this->private_->ui_.bright_contrast_bar_->hide();
    this->private_->ui_.color_bar_->hide();
    this->private_->ui_.opacity_bar_->hide();
// Otherwise we make things simpler
#else
    this->hide();
#endif
  }
  else
  {
#if defined( _WIN32 )
    this->private_->ui_.header_->show();
    if( this->active_ ) 
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );
    else
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
#else
    this->show();
#endif;
  }
  this->setUpdatesEnabled( true );
  this->repaint();
}
  
void LayerWidget::set_drop( bool drop )
{
  // First we check to see if it is picked up if so, we set change the way it looks
  if( this->picked_up_ )
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_PICKED_UP_C );  
  }
  // If its not picked up, we set its color to indicate whether or not its a potential drop site
  else if( drop )
  {
    this->private_->drop_space_->show();
  }
  else
  {
    this->private_->drop_space_->hide();
  }
  this->repaint();
} 

  
  

void LayerWidget::set_active( bool active )
{
  // keep track locally if we are an active layer or not so we know what color to revert to if locked
  this->active_ = active;
  
  if( active )
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );  
  }
  else
  {
    this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
  }
  this->repaint();
}

  
void LayerWidget::show_selection_checkbox( bool show )
{
  //this->layer_menus_open_ = show;
  if( show && (!this->private_->ui_.lock_button_->isChecked()) )
  { 
    this->private_->ui_.checkbox_widget_->show();
  }
  else 
  {
    this->private_->ui_.checkbox_widget_->hide();
  }
  this->repaint();
}

void LayerWidget::show_opacity_bar( bool show )
{
  //this->layer_menus_open_ = show;
  if( show )
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
  this->repaint();
}

void LayerWidget::show_brightness_contrast_bar( bool show )
{
  //this->layer_menus_open_ = show;
  if( show )
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
  this->repaint();
}

void LayerWidget::show_border_fill_bar( bool show )
{
  //this->layer_menus_open_ = show;
  if( show )
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
  this->repaint();
}

void LayerWidget::show_color_bar( bool show )
{
  //this->layer_menus_open_ = show;
  if( show )
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
  this->repaint();
}

void LayerWidget::show_progress_bar( bool show )
{
  //this->layer_menus_open_ = show;
  if( show )
  {
    this->private_->ui_.progress_bar_bar_->show();
  }
  else 
  {
    this->private_->ui_.progress_bar_bar_->hide();
  }
  this->repaint();
}

void LayerWidget::visual_lock( bool lock )
{
  //this->layer_menus_open_ = lock;
  if( lock )
  {
    this->private_->ui_.header_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_LOCKED_C );
    this->private_->ui_.typeBackground_->setStyleSheet( StyleSheet::LAYER_WIDGET_BACKGROUND_LOCKED_C );
    this->private_->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_LOCKED_C );
    this->private_->activate_button_->setEnabled( false );
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
    switch( this->volume_type_ )
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
    
    if( this->active_ )
    {
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_ACTIVE_C );  
    }
    else
    {
      this->private_->ui_.base_->setStyleSheet( StyleSheet::LAYER_WIDGET_BASE_INACTIVE_C );
    }
    
    this->private_->ui_.label_->setStyleSheet( StyleSheet::LAYER_WIDGET_LABEL_C );
    this->private_->activate_button_->setEnabled( true );
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
  this->repaint();
}

void LayerWidget::resizeEvent( QResizeEvent *event )
{
  this->private_->overlay_->resize( event->size() );
  event->accept();
}

  
} //end namespace Seg3D

