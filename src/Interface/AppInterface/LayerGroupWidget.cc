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

// Qt includes
#include <QtGui/QMessageBox>
#include <QtCore/QPropertyAnimation>

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
#include <Application/LayerManager/LayerManager.h>
#include <Application/LayerManager/Actions/ActionDeleteLayers.h>
#include <Application/LayerManager/Actions/ActionNewMaskLayer.h>
#include <Application/LayerManager/Actions/ActionDuplicateLayer.h>
#include <Application/LayerManager/Actions/ActionMoveGroupAbove.h>
#include <Application/LayerManager/Actions/ActionMoveLayerBelow.h>
#include <Application/ViewerManager/ViewerManager.h>

namespace Seg3D
{
  
class LayerGroupWidgetPrivate
{
public:
  
  LayerGroupHandle group_;
  
  Ui::LayerGroupWidget ui_;
  DropSpaceWidget* drop_space_;
  DropSpaceWidget* layer_slot_;
  OverlayWidget* overlay_;
  OverlayWidget* button_overlay_;
  
  std::string layer_to_drop_;

  int group_height;
  LayerGroupWidget* drop_group_;
  bool drop_group_set_;

  QButtonGroup* iso_quality_button_group_;
};
  
LayerGroupWidget::LayerGroupWidget( QWidget* parent, LayerGroupHandle group ) :
  QWidget( parent ),
  private_( new LayerGroupWidgetPrivate ),
  group_menus_open_( false ),
  picked_up_( false )
{ 
    this->private_->group_ = group;

  this->private_->ui_.setupUi( this );
  
  this->group_id_ = this->private_->group_->get_group_id();
  
  this->private_->drop_group_set_ = false;
  
  // Set up the Drag and Drop
  this->setAcceptDrops( true );

  // hide the tool bars 
  this->private_->ui_.iso_quality_->hide();
  this->private_->ui_.delete_->hide();
  
  this->private_->ui_.delete_button_->setEnabled( false );

  // set some values of the GUI
  std::string group_name = Core::ExportToString( 
    this->private_->group_->get_grid_transform().get_nx() ) + " x " +
    Core::ExportToString( this->private_->group_->get_grid_transform().get_ny() ) + " x " +
    Core::ExportToString( this->private_->group_->get_grid_transform().get_nz() );
  this->private_->ui_.group_dimensions_label_->setText( QString::fromStdString( group_name ) );

  this->private_->drop_space_ = new DropSpaceWidget( this, 105 );
  this->private_->ui_.verticalLayout_12->insertWidget( 0, this->private_->drop_space_ );
  this->private_->drop_space_->hide();
  
  this->private_->layer_slot_ = new DropSpaceWidget( this );
  this->private_->ui_.verticalLayout->insertWidget( 0, this->private_->layer_slot_ );
  this->private_->layer_slot_->hide();
  
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
  connect( this->private_->ui_.delete_button_, SIGNAL( clicked() ), this, 
    SLOT( verify_delete() ) );
  connect( this->private_->ui_.select_all_button_, SIGNAL( released() ), this, 
    SLOT( hide_show_checkboxes() ) );
  
  // Set the icons for the group visibility button
  QIcon none_visible_icon;
  none_visible_icon.addFile( QString::fromUtf8( ":/Images/VisibleOff.png" ), QSize(), QIcon::Normal );
  QIcon some_visible_icon;
  some_visible_icon.addFile( QString::fromUtf8( ":/Images/VisibleGray.png" ), QSize(), QIcon::Normal );
  QIcon all_visible_icon;
  all_visible_icon.addFile( QString::fromUtf8( ":/Images/VisibleWhite.png" ), QSize(), QIcon::Normal );
  this->private_->ui_.group_visibility_button_->set_icons( none_visible_icon, 
    some_visible_icon, all_visible_icon );
  QtUtils::QtBridge::Connect( this->private_->ui_.group_visibility_button_, 
    group->layers_visible_state_ );

  //Set the default values for the Group UI and make the connections to the state engine
      // --- GENERAL ---
  QtUtils::QtBridge::Connect( this->private_->ui_.group_iso_button_, 
    group->show_iso_menu_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.group_delete_button_, 
    group->show_delete_menu_state_ );

  QtUtils::QtBridge::Show( this->private_->ui_.iso_quality_, group->show_iso_menu_state_ ); 
  QtUtils::QtBridge::Show( this->private_->ui_.delete_, group->show_delete_menu_state_ );
  
  QtUtils::QtBridge::Connect( this->private_->ui_.group_new_button_, 
    boost::bind( &ActionNewMaskLayer::Dispatch, 
    Core::Interface::GetWidgetActionContext(), this->private_->group_->get_group_id() ) );

  QtUtils::QtBridge::Connect( this->private_->ui_.duplicate_layer_button_,
    boost::bind( &ActionDuplicateLayer::Dispatch, Core::Interface::GetWidgetActionContext() ) );

  // --- ISOSURFACE---
  QtUtils::QtBridge::Connect( this->private_->iso_quality_button_group_, 
    this->private_->group_->isosurface_quality_state_ );

  this->private_->ui_.group_frame_layout_->setAlignment( Qt::AlignTop );
  this->private_->ui_.group_frame_layout_->setSpacing( 1 );
  this->private_->ui_.verticalLayout_13->setAlignment( Qt::AlignTop );
  this->private_->ui_.verticalLayout_10->setAlignment( Qt::AlignTop );

  this->private_->overlay_ = new OverlayWidget( this );
  this->private_->overlay_->hide();
  
  this->private_->button_overlay_ = new OverlayWidget( this->private_->ui_.widget );  
  this->private_->button_overlay_->hide();

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
  mimeData->setText( QString::fromStdString( this->get_group_id() ) );

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
  
  // If our drag was successful then we do stuff
  if( ( ( drag->exec( Qt::MoveAction, Qt::MoveAction ) ) == Qt::MoveAction ) 
    && ( this->private_->drop_group_set_ ) )
  {
    ActionMoveGroupAbove::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->get_group_id(), this->private_->drop_group_->get_group_id() );
  }
  else
  {
    this->seethrough( false );
  }

  this->private_->drop_group_set_ = false;
  this->enable_drop_space( false );
  this->private_->layer_to_drop_ = "";
  
  Q_EMIT prep_groups_for_drag_and_drop_signal_( false );
}

void LayerGroupWidget::set_drop_target( LayerGroupWidget* target_group)
{
  this->private_->drop_group_ = target_group;
  this->private_->drop_group_set_ = true;
}

void LayerGroupWidget::dropEvent( QDropEvent* event )
{
  this->enable_drop_space( false );
  this->private_->button_overlay_->hide();
  
  std::string drop_item_id = event->mimeData()->text().toStdString();
  
  if( ( this->get_group_id() != drop_item_id ) && ( LayerManager::Instance()->get_layer_group( drop_item_id ) ) ) 
  {
    dynamic_cast< LayerGroupWidget* >( event->source() )->set_drop_target( this ); 
    event->setDropAction( Qt::MoveAction );
    event->accept();
    return;
  }
  else if ( LayerManager::Instance()->get_layer_by_name( drop_item_id ) )
  {
    ActionMoveLayerBelow::Dispatch( Core::Interface::GetWidgetActionContext(), 
      drop_item_id, this->get_group_id() );
    this->private_->layer_slot_->instant_hide();
    event->setDropAction( Qt::MoveAction );
    event->accept();
    return;
  }
  event->ignore();
}

void LayerGroupWidget::dragEnterEvent( QDragEnterEvent* event)
{
  std::string drop_item_id = event->mimeData()->text().toStdString();

  if( ( this->get_group_id() != drop_item_id ) && ( LayerManager::Instance()->get_layer_group( drop_item_id ) ) ) 
  {
    this->enable_drop_space( true );
    event->setDropAction( Qt::MoveAction );
    event->accept();
    return;
  }
  else if ( ( LayerManager::Instance()->get_layer_by_name( drop_item_id ) ) && (
    ( this->private_->ui_.buttons_ == this->childAt( event->pos() ) ) || 
    ( this->private_->button_overlay_ == this->childAt( event->pos() ) ) ) )
  {
    this->private_->layer_slot_->show();
    this->private_->button_overlay_->show();
    event->accept();
    return;
  }

  this->enable_drop_space( false );
  this->private_->button_overlay_->hide();
  event->ignore();

}

void LayerGroupWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->enable_drop_space( false );
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

void LayerGroupWidget::enable_drop_space( bool drop )
{
  this->private_->drop_space_->set_height( this->picked_up_group_height_ + 4 );
  
  if( this->picked_up_ )
  {
    return;
  }
  
  if( drop )
  {
    this->private_->drop_space_->show();
    this->private_->overlay_->show();
  }
  else
  {
    this->private_->layer_slot_->hide();
    this->private_->drop_space_->hide();
    this->private_->overlay_->hide();
    
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

void LayerGroupWidget::verify_delete()
{

  // Check whether the users wants to save and whether the user wants to quit
  int ret = QMessageBox::warning( this, "Delete Warning",
    "Are you sure, you want to delete these layers? This action cannot be undone.",
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No  );
        
  if( ret == QMessageBox::Yes )
  {
    ActionDeleteLayers::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->private_->group_ );
    this->private_->ui_.group_delete_button_->setChecked( false );
    this->private_->ui_.select_all_button_->setChecked( false );
  }
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
  this->private_->button_overlay_->resize( this->private_->ui_.widget->size() );
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
    ( *it ).second->instant_hide_drop_space();
    ( *it ).second->hide_overlay();
    
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
        
      connect( new_layer_handle.data(), SIGNAL( layer_size_signal_( int ) ), 
        this, SIGNAL( picked_up_layer_size_signal_( int ) ) );
        
      connect( new_layer_handle.data(), SIGNAL( selection_box_changed() ),
        this, SLOT( enable_disable_delete_button() ) );
      
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

void LayerGroupWidget::instant_hide_drop_space()
{
  this->private_->drop_space_->instant_hide();
}

void LayerGroupWidget::notify_picked_up_layer_size( int layer_size )
{
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    ( *it ).second->set_picked_up_layer_size( layer_size );
  }
}

void LayerGroupWidget::hide_show_checkboxes()
{
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    ( *it ).second->set_check_selected( this->private_->ui_.select_all_button_->isChecked() );
  }
}

void LayerGroupWidget::enable_disable_delete_button()
{
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    if( ( *it ).second->get_selected() )
    {
      this->private_->ui_.delete_button_->setEnabled( true );
      return;
    }
  }
  this->private_->ui_.delete_button_->setEnabled( false );
}


}  //end namespace Seg3D
