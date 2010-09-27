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

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Interface Includes
#include <Interface/AppInterface/LayerGroupWidget.h>
#include <Interface/AppInterface/StyleSheet.h>
#include <Interface/AppInterface/DropSpaceWidget.h>
#include <Interface/AppInterface/OverlayWidget.h>
#include <Interface/AppInterface/PushDragButton.h>

//UI Includes
#include "ui_LayerGroupWidget.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerManager/Actions/ActionDeleteLayers.h>
#include <Application/LayerManager/Actions/ActionNewMaskLayer.h>
#include <Application/LayerManager/Actions/ActionMoveGroupAbove.h>
#include <Application/ViewerManager/ViewerManager.h>


namespace Seg3D
{
  
class LayerGroupWidgetPrivate
{
public:
  
  LayerGroupHandle group_;
  
  Ui::LayerGroupWidget ui_;
  
  PushDragButton* activate_button_;
  DropSpaceWidget* drop_space_;
  OverlayWidget* overlay_;

  int group_height;

  QButtonGroup* iso_quality_button_group_;
};
  
LayerGroupWidget::LayerGroupWidget( QWidget* parent, LayerGroupHandle group ) :
  QWidget( parent ),
  private_( new LayerGroupWidgetPrivate ),
  group_menus_open_( false ),
  picked_up_( false ),
  drop_group_set_( false )
{ 
    this->private_->group_ = group;

  this->private_->ui_.setupUi( this );
  
  this->group_id_ = this->private_->group_->get_group_id();
  
  // Set up the Drag and Drop
  this->setAcceptDrops( true );

  // hide the tool bars 
  this->private_->ui_.iso_quality_->hide();
  this->private_->ui_.delete_->hide();
  this->private_->ui_.delete_button_->setEnabled( false );

  //add the PushDragButton
  this->private_->activate_button_ = new PushDragButton( this->private_->ui_.group_header_ );
  this->private_->activate_button_->setObjectName( QString::fromUtf8( "activate_button_" ) );
  this->private_->activate_button_->setStyleSheet( StyleSheet::GROUP_PUSHDRAGBUTTON_C );
  QSizePolicy sizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  sizePolicy.setHeightForWidth( 
    this->private_->activate_button_->sizePolicy().hasHeightForWidth() );
    this->private_->activate_button_->setSizePolicy( sizePolicy );
    this->private_->activate_button_->setMinimumSize( QSize( 152, 21 ) );
    this->private_->activate_button_->setCheckable( false );
    this->private_->activate_button_->setFlat( true );
  this->private_->ui_.horizontalLayout->insertWidget( 1, this->private_->activate_button_ );

  // set some values of the GUI
  std::string group_name = Core::ExportToString( 
    this->private_->group_->get_grid_transform().get_nx() ) + " x " +
    Core::ExportToString( this->private_->group_->get_grid_transform().get_ny() ) + " x " +
    Core::ExportToString( this->private_->group_->get_grid_transform().get_nz() );
  this->private_->activate_button_->setText( QString::fromStdString( group_name ) );

  this->private_->drop_space_ = new DropSpaceWidget( this, 105 );
  this->private_->ui_.verticalLayout_12->insertWidget( 0, this->private_->drop_space_ );
  
  this->private_->drop_space_->hide();
  
  // Add isosurface quality radio buttons to QButtonGroup so that QtButtonGroupConnector can be
  // used to connect the buttons directly to a state variable.
  this->private_->iso_quality_button_group_ = new QButtonGroup( this );
  this->private_->iso_quality_button_group_->setExclusive( true );
  this->private_->iso_quality_button_group_->addButton( this->private_->ui_.radioButton_1_point_0 );
  this->private_->iso_quality_button_group_->addButton( this->private_->ui_.radioButton_point_5 );
  this->private_->iso_quality_button_group_->addButton( this->private_->ui_.radioButton_point_25 );
  this->private_->iso_quality_button_group_->addButton( this->private_->ui_.radioButton_point_125 );
  
  //  connect the gui signals and slots
  connect( this->private_->ui_.open_button_, SIGNAL( toggled( bool ) ), this, 
    SLOT( show_layers( bool )) );
  connect( this->private_->ui_.confirm_delete_checkbox_, SIGNAL( clicked ( bool ) ), this, 
    SLOT( enable_delete_button( bool )) );
  connect( this->private_->ui_.delete_button_, SIGNAL( clicked () ), this, 
    SLOT( uncheck_delete_confirm() ) );
  
  
  //Set the default values for the Group UI and make the connections to the state engine
      // --- GENERAL ---
  QtUtils::QtBridge::Connect( this->private_->ui_.group_iso_button_, 
    group->show_iso_menu_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.group_delete_button_, 
    group->show_delete_menu_state_ );

  QtUtils::QtBridge::Show( this->private_->ui_.iso_quality_, group->show_iso_menu_state_ ); 
  QtUtils::QtBridge::Show( this->private_->ui_.delete_, group->show_delete_menu_state_ );
  
    QtUtils::QtBridge::Connect( this->private_->ui_.group_visibility_button_, 
    this->private_->group_->visibility_state_, 
    ViewerManager::Instance()->active_viewer_state_ );
    QtUtils::QtBridge::Connect( this->private_->ui_.delete_button_, 
    boost::bind( &ActionDeleteLayers::Dispatch, 
    Core::Interface::GetWidgetActionContext(), this->private_->group_ ) );
  QtUtils::QtBridge::Connect( this->private_->ui_.group_new_button_, 
    boost::bind( &ActionNewMaskLayer::Dispatch, 
    Core::Interface::GetWidgetActionContext(), this->private_->group_ ) );

  // --- ISOSURFACE---
  QtUtils::QtBridge::Connect( this->private_->iso_quality_button_group_, 
    this->private_->group_->isosurface_quality_state_ );

  this->setMinimumHeight( 0 );
  this->private_->ui_.group_frame_layout_->setAlignment( Qt::AlignTop );
  this->private_->ui_.verticalLayout_13->setAlignment( Qt::AlignTop );
  this->private_->ui_.verticalLayout_10->setAlignment( Qt::AlignTop );

  this->private_->overlay_ = new OverlayWidget( this );
  this->private_->overlay_->setStyleSheet( 
    QString::fromUtf8( "background-color: rgba( 255, 0, 0, 200 )" ) );
  this->private_->overlay_->hide(); 

  this->private_->ui_.fake_widget_->setMinimumHeight( 0 );
  this->private_->ui_.fake_widget_->hide();
  
  this->private_->ui_.group_dummy_->hide();

  this->private_->group_height = this->private_->ui_.tools_and_layers_widget_->height();
}

LayerGroupWidget::~LayerGroupWidget()
{
}

void LayerGroupWidget::mousePressEvent( QMouseEvent *event )
{
  // Exit immediately if they are no longer holding the button the press event isnt valid
  if( event->button() != Qt::LeftButton )
  { 
    return;
  }

  if( this->group_menus_open_ )
    return;

  QPoint hotSpot = event->pos();

  // Make up some mimedata containing the layer_id of the layer
  QMimeData *mimeData = new QMimeData;
  mimeData->setText(QString::fromStdString( std::string( "group|" ) +  this->get_group_id() ) );

  // Create a drag object and insert the hotspot
  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData );

  // here we add basically a screenshot of the widget
  drag->setPixmap( QPixmap::grabWidget( this ));
  drag->setHotSpot( hotSpot );

  // Next we hide the LayerWidget that we are going to be dragging.
  this->seethrough( true );

  Q_EMIT prep_groups_for_drag_and_drop_signal_( true );
  Q_EMIT picked_up_group_size_signal_( this->height() - 2 );
  
  // Finally if our drag was aborted then we reset the layers styles to be visible
  if( ( ( drag->exec( Qt::MoveAction, Qt::MoveAction ) ) == Qt::MoveAction ) 
     && this->drop_group_set_ )
  {
    ActionMoveGroupAbove::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->get_group_id(), this->drop_group_->get_group_id() );
  }
  else
  {
    this->seethrough( false );
  }

  this->drop_group_set_ = false;
  this->private_->overlay_->hide();
  
  Q_EMIT prep_groups_for_drag_and_drop_signal_( false );
}

void LayerGroupWidget::set_drop_target( LayerGroupWidget* target_group)
{
  this->drop_group_ = target_group;
  this->drop_group_set_ = true;
}

void LayerGroupWidget::dropEvent( QDropEvent* event )
{
  std::vector<std::string> mime_data = 
    Core::SplitString( event->mimeData()->text().toStdString(), "|" );
  
  if( mime_data.size() < 2 ) 
  {
    this->set_drop( false );
    return;
  }

  if( ( this->get_group_id() == mime_data[ 1 ] ) || ( mime_data[ 0 ] != "group" ) )
  {
    this->set_drop( false );
    event->ignore();
    return;
  }

  if( this->group_menus_open_ )
  {
    this->set_drop( false );
    return;
  }

  this->private_->overlay_->hide();

  dynamic_cast< LayerGroupWidget* >( event->source() )->set_drop_target( this ); 
  event->setDropAction(Qt::MoveAction);
  event->accept();
}

void LayerGroupWidget::dragEnterEvent( QDragEnterEvent* event)
{
  this->private_->overlay_->show();

  std::vector<std::string> mime_data = 
    Core::SplitString( event->mimeData()->text().toStdString(), "|" );
  
  if( mime_data.size() < 2 ) return;

  if( ( this->get_group_id() != mime_data[ 0 ] ) && ( mime_data[ 0 ] == "group" ) )
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

void LayerGroupWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->set_drop( false );
  this->private_->overlay_->hide();
}

void LayerGroupWidget::seethrough( bool see )
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
}

void LayerGroupWidget::set_drop( bool drop )
{
  this->private_->drop_space_->set_height( this->picked_up_group_height_ + 4 );
  
  if( this->picked_up_ )
  {
    return;
  }
  
  if( drop )
  {
    this->private_->drop_space_->show();
  }
  else
  {
    this->private_->drop_space_->hide();
  }
} 
  
LayerWidgetQWeakHandle LayerGroupWidget::set_active_layer( LayerHandle layer )
{
  std::map< std::string, LayerWidgetQHandle >::iterator found_layer_widget = 
    this->layer_map_.find( layer->get_layer_id() );
  
  if( found_layer_widget != this->layer_map_.end() )
  {
    ( *found_layer_widget ).second->set_active( true );
    return ( *found_layer_widget ).second;
  }
  
  return LayerWidgetQWeakHandle();
}

const std::string& LayerGroupWidget::get_group_id()
{
  return this->group_id_;
}

void LayerGroupWidget::enable_delete_button( bool enable )
{
  this->private_->ui_.delete_button_->setEnabled( enable );
}

void LayerGroupWidget::uncheck_delete_confirm()
{
    this->private_->ui_.confirm_delete_checkbox_->setChecked( false );
    this->private_->ui_.delete_button_->setEnabled( false );
}

void LayerGroupWidget::show_layers( bool show )
{
  if( show ) 
  {
    if( this->private_->ui_.tools_and_layers_widget_->isHidden() )
    {
      if( this->private_->ui_.fake_widget_->isHidden() )
      {
        this->private_->ui_.fake_widget_->setMinimumHeight( 0 );
        this->private_->ui_.fake_widget_->setFixedHeight( 0 );
        this->private_->ui_.fake_widget_->show();
      }
      
      QPropertyAnimation *animation = new QPropertyAnimation( 
        this->private_->ui_.fake_widget_, "minimumHeight" );
      animation->setDuration( 500 );
      animation->setEndValue( this->private_->group_height );
      animation->setEasingCurve( QEasingCurve::OutQuad );
      connect( animation, SIGNAL( finished() ), this, SLOT( show_group() ) );
      animation->start();
    }
  }
  else
  {
    this->private_->group_height = this->private_->ui_.tools_and_layers_widget_->height();
    this->private_->ui_.tools_and_layers_widget_->hide();
    this->private_->ui_.fake_widget_->setMinimumHeight( 0 );
  }
}
  
void LayerGroupWidget::show_group()
{
  this->private_->ui_.fake_widget_->hide();
  this->private_->ui_.tools_and_layers_widget_->show();
}

void LayerGroupWidget::hide_group()
{
  this->private_->ui_.fake_widget_->hide();
}
  
void LayerGroupWidget::resizeEvent( QResizeEvent *event )
{
  this->private_->overlay_->resize( event->size() );
  event->accept();
}
  
void LayerGroupWidget::prep_layers_for_drag_and_drop( bool move_time )
{
  for( std::map< std::string, LayerWidgetQHandle>::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    ( *it ).second->prep_for_animation( move_time );
  } 
}

void LayerGroupWidget::prep_for_animation( bool move_time )
{
  if( this->picked_up_ )
    return;
  
  if( move_time )
  {
    this->private_->ui_.group_dummy_->setMinimumHeight( this->private_->ui_.base_->height() );
    this->private_->ui_.group_dummy_->setMinimumWidth( this->private_->ui_.base_->width() );
    this->private_->ui_.group_dummy_->setPixmap( QPixmap::grabWidget( this->private_->ui_.base_ ) );
    this->private_->ui_.group_dummy_->setStyleSheet( QString::fromUtf8( 
      "QLabel#group_dummy {"
      " background-color: yellow; }" ) );
    
    this->private_->ui_.base_->hide();
    this->private_->ui_.group_dummy_->show();
  }
  else
  {
    this->private_->ui_.group_dummy_->hide();
    this->private_->ui_.base_->show();
  }
}
  
void LayerGroupWidget::set_picked_up_group_size( int group_height )
{
  this->picked_up_group_height_ = group_height;
}
  
void LayerGroupWidget::handle_change()
{
  layer_list_type layer_list = this->private_->group_->get_layer_list();
  int index = 0;
  bool layer_widget_deleted = false;
  
  this->setUpdatesEnabled( false );
  
  // First we remove the LayerWidgets that arent needed any more.
  for( std::map< std::string, LayerWidgetQHandle>::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    // we are also going to take the opportunity to turn off all the drag and drop settings
    ( *it ).second->seethrough( false );
    ( *it ).second->enable_drop_space( false );
    
    bool found = false;
    for( layer_list_type::iterator i = layer_list.begin(); i != layer_list.end(); ++i )
    {
      if( ( *it ).first == ( *i )->get_layer_id() )
      {
        found = true;
        break;
      }
    }
    if( !found )
    {
      this->private_->ui_.group_frame_layout_->removeWidget( ( *it ).second.data() );
      layer_widget_deleted = true;
    }
  }

  // then we need to, we delete the unused LayerWidgets from the layer_map_
  if( layer_widget_deleted )
  {
    this->cleanup_removed_widgets();
  }
  
  this->private_->ui_.group_frame_->setMinimumHeight( 0 );
  
  for( layer_list_type::iterator i = layer_list.begin(); i != layer_list.end(); ++i )
  {
    std::map< std::string, LayerWidgetQHandle >::iterator found_layer_widget = this->layer_map_.find( ( *i )->get_layer_id() );
    if( found_layer_widget != this->layer_map_.end() )
    {
      this->private_->ui_.group_frame_layout_->insertWidget( index, ( *found_layer_widget ).second.data() );
    }
    else
    {
      LayerWidgetQHandle new_layer_handle( new LayerWidget(this->private_->ui_.group_frame_, ( *i ) ) );
      this->private_->ui_.group_frame_layout_->insertWidget( index, new_layer_handle.data() );
      this->layer_map_[ ( *i )->get_layer_id() ] = new_layer_handle;
      
      connect( new_layer_handle.data(), SIGNAL( prep_for_drag_and_drop( bool ) ), 
        this, SIGNAL( prep_layers_for_drag_and_drop_signal_( bool ) ) );
      
    }
    index++;
  }
  
  this->setUpdatesEnabled( true );
}

void LayerGroupWidget::cleanup_removed_widgets()
{
  std::map< std::string, LayerWidgetQHandle>::iterator it = this->layer_map_.begin();
  while ( it != this->layer_map_.end() )
  {
    std::map< std::string, LayerWidgetQHandle>::iterator prev = it++;
    if( this->private_->ui_.group_frame_layout_->indexOf( ( *prev ).second.data() ) == -1 )
    {
      ( *prev ).second->deleteLater();
      this->layer_map_.erase( prev );
    }
  }
}
  
}  //end namespace Seg3D
