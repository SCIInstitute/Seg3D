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
#include <Interface/AppInterface/GroupButtonMenu.h>

//UI Includes
#include "ui_LayerGroupWidget.h"

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
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
//  LayerGroupHandle group_;
  Ui::LayerGroupWidget ui_;
  DropSpaceWidget* drop_space_;
  OverlayWidget* overlay_;
  std::string layer_to_drop_;
  int group_height;
  LayerGroupWidget* drop_group_;
  GroupButtonMenu* button_menu_;

  std::string group_id_;
  int picked_up_group_height_;
  bool group_menus_open_;
  bool picked_up_;
};
  
LayerGroupWidget::LayerGroupWidget( QWidget* parent, LayerGroupHandle group ) :
  QWidget( parent ),
  private_( new LayerGroupWidgetPrivate )
{ 
  this->private_->ui_.setupUi( this );
  this->private_->group_id_ = group->get_group_id();
  this->private_->group_menus_open_ = false;
  this->private_->picked_up_ = false;
  this->private_->button_menu_ = new GroupButtonMenu( this, group );
  this->private_->ui_.tools_and_layers_layout_->addWidget( this->private_->button_menu_ );
  
  // Set up the Drag and Drop
  this->setAcceptDrops( true );

  // set some values of the GUI
  std::string group_name = Core::ExportToString( 
    group->get_grid_transform().get_nx() ) + " x " +
    Core::ExportToString( group->get_grid_transform().get_ny() ) + " x " +
    Core::ExportToString( group->get_grid_transform().get_nz() );
  this->private_->ui_.group_dimensions_label_->setText( QString::fromStdString( group_name ) );

  this->private_->drop_space_ = new DropSpaceWidget( this, 105 );
  this->private_->ui_.verticalLayout_12->insertWidget( 0, this->private_->drop_space_ );
  this->private_->drop_space_->hide();
  
  //  connect the gui signals and slots
  connect( this->private_->ui_.open_button_, SIGNAL( toggled( bool ) ), this, 
    SLOT( show_layers( bool )) );
  
  QtUtils::QtBridge::Connect( this->private_->ui_.open_button_, 
    group->group_widget_expanded_state_ );
  
  connect( this->private_->button_menu_, SIGNAL( delete_pressed() ), this, 
    SLOT( verify_delete() ) );
  connect( this->private_->button_menu_, SIGNAL( duplicate_pressed() ), this,
    SLOT( duplicate_checked_layers() ) );
  connect( this->private_->button_menu_, SIGNAL( duplicate_select_all_pressed( bool ) ), this, 
    SLOT( check_uncheck_for_delete( bool ) ) );
  connect( this->private_->button_menu_, SIGNAL( delete_select_all_pressed( bool ) ), this, 
    SLOT( check_uncheck_for_duplicate( bool ) ) );
  
  connect( this->private_->ui_.group_iso_visibility_button_, SIGNAL( toggled( bool ) ), this, 
    SLOT( set_iso_surface_visibility( bool )) );

  
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


  this->private_->ui_.group_frame_layout_->setAlignment( Qt::AlignTop );
  this->private_->ui_.group_frame_layout_->setSpacing( 1 );
  this->private_->ui_.verticalLayout_13->setAlignment( Qt::AlignTop );

  this->private_->overlay_ = new OverlayWidget( this );
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
  // Exit immediately if they are no longer holding the button the press event isn't valid
  if( ( event->button() != Qt::LeftButton )|| ( event->modifiers() != Qt::ShiftModifier ) )
  { 
    event->setAccepted( true );
    return;
  }

  if( this->private_->group_menus_open_ )
  {
    event->setAccepted( true );
    return;
  }
  QPoint hotSpot = event->pos();

  // Make up some mimedata containing the layer_id of the layer
  QMimeData *mimeData = new QMimeData;
  mimeData->setText( QString::fromStdString( this->get_group_id() ) );

  // Create a drag object and insert the hotspot
  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData );

  // here we add basically a screenshot of the widget
  QImage temp_image( this->private_->ui_.base_->size(), QImage::Format_ARGB32 );
  temp_image.fill( 0 );
  this->private_->ui_.base_->render( &temp_image, QPoint(), this->private_->ui_.base_->rect(), QWidget::DrawChildren );
  drag->setPixmap( QPixmap::fromImage( temp_image ) );
  drag->setHotSpot( hotSpot );

  // Next we hide the LayerWidget that we are going to be dragging.
  this->seethrough( true );

  Q_EMIT prep_groups_for_drag_and_drop_signal_( true );
  Q_EMIT picked_up_group_size_signal_( this->height() - 2 );
  
  // If our drag was successful then we do stuff
  if( ( drag->exec( Qt::MoveAction, Qt::MoveAction ) ) == Qt::MoveAction ) 
  {
    ActionMoveGroupAbove::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->get_group_id(), this->private_->drop_group_->get_group_id() );
  }
  else
  {
    this->seethrough( false );
  }

  this->enable_drop_space( false );
  this->private_->layer_to_drop_ = "";
  
  Q_EMIT prep_groups_for_drag_and_drop_signal_( false );
  event->setAccepted( true );
}

void LayerGroupWidget::set_drop_target( LayerGroupWidget* target_group)
{
  this->private_->drop_group_ = target_group;
}

void LayerGroupWidget::dropEvent( QDropEvent* event )
{
  this->enable_drop_space( false );
  event->setAccepted( true );
  
  std::string drop_item_id = event->mimeData()->text().toStdString();
  
  if( ( this->get_group_id() != drop_item_id ) && ( LayerManager::Instance()->get_layer_group( drop_item_id ) ) ) 
  {
    dynamic_cast< LayerGroupWidget* >( event->source() )->set_drop_target( this ); 
    event->setDropAction( Qt::MoveAction );
    return;

  }

  event->setDropAction( Qt::IgnoreAction );
}

void LayerGroupWidget::dragEnterEvent( QDragEnterEvent* event)
{
  std::string drop_item_id = event->mimeData()->text().toStdString();

  if( ( this->get_group_id() != drop_item_id ) && ( LayerManager::Instance()->get_layer_group( drop_item_id ) ) ) 
  {
    this->enable_drop_space( true );
  }
  event->setAccepted( true );
}

void LayerGroupWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
  CORE_LOG_DEBUG( "Leaving the group" );
  this->enable_drop_space( false );
  event->setAccepted( true );
}

void LayerGroupWidget::seethrough( bool see )
{
  this->set_picked_up( see );

  if( see )
  {
    this->private_->ui_.base_->hide();
  }
  else
  { 
    this->private_->ui_.base_->show();
  }
}

void LayerGroupWidget::enable_drop_space( bool drop )
{
  this->private_->drop_space_->set_height( this->private_->picked_up_group_height_ + 4 );
  
  if( this->private_->picked_up_ )
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
  return this->private_->group_id_;
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
      LayerManager::Instance()->FindLayerGroup( this->private_->group_id_ ) );
    
    this->private_->button_menu_->uncheck_delete_menu_button();
    this->private_->button_menu_->uncheck_delete_button();
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
  this->private_->ui_.group_background_->setStyleSheet( StyleSheet::GROUP_WIDGET_BACKGROUND_ACTIVE_C );
}

void LayerGroupWidget::hide_group()
{
  this->private_->ui_.fake_widget_->hide();
  this->private_->ui_.group_background_->setStyleSheet( StyleSheet::GROUP_WIDGET_BACKGROUND_INACTIVE_C );
}
  
void LayerGroupWidget::resizeEvent( QResizeEvent *event )
{
  this->private_->overlay_->resize( event->size() );
  event->setAccepted( true );
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
  if( this->private_->picked_up_ )
    return;
  
  //this->setUpdatesEnabled( false );
  
  if( move_time )
  {
    this->private_->ui_.group_dummy_->setMinimumSize( this->private_->ui_.base_->size() );
    QImage temp_image( this->private_->ui_.base_->size(), QImage::Format_ARGB32 );
    temp_image.fill( 0 );
    this->private_->ui_.base_->render( &temp_image, QPoint(), this->private_->ui_.base_->rect(), QWidget::DrawChildren );
    this->private_->ui_.group_dummy_->setPixmap( QPixmap::fromImage( temp_image ) );
    
    this->private_->ui_.base_->hide();
    this->private_->ui_.group_dummy_->show();
  }
  else
  {
    this->private_->ui_.group_dummy_->hide();
    this->private_->ui_.base_->show();
  }
  
  //this->setUpdatesEnabled( true );
}
  
void LayerGroupWidget::set_picked_up_group_size( int group_height )
{
  this->private_->picked_up_group_height_ = group_height;
}
  
void LayerGroupWidget::handle_change()
{
  LayerGroupHandle this_group = LayerManager::Instance()->FindLayerGroup( this->private_->group_id_ );
  layer_list_type layer_list = this_group->get_layer_list();
  
  int index = 0;
  bool layer_widget_deleted = false;
  this->setUpdatesEnabled( false );
  
  // First we remove the LayerWidgets that arent needed any more.
  for( std::map< std::string, LayerWidgetQHandle>::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    // we are also going to take the opportunity to turn off all the drag and drop settings
    ( *it ).second->set_picked_up( false );
    ( *it ).second->instant_hide_drop_space();
    ( *it ).second->hide_overlay();
    if( this_group->show_delete_menu_state_->get() || 
       this_group->show_duplicate_menu_state_->get() )
    {
      ( *it ).second->show_selection_checkbox( true );
    }
    
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
      
      connect( new_layer_handle.data(), SIGNAL( selection_box_changed() ),
          this, SLOT( enable_disable_duplicate_button() ) );
      
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
  
void LayerGroupWidget::duplicate_checked_layers()
{
  this->private_->button_menu_->uncheck_delete_button();
  this->private_->button_menu_->uncheck_duplicate_button();
  
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    if( ( *it ).second->get_selected() )
    {
      ActionDuplicateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), 
        ( *it ).second->get_layer_id() );
      ( *it ).second->set_check_selected( false );
      
    }
  }
}

void LayerGroupWidget::check_uncheck_for_delete( bool checked )
{
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    ( *it ).second->set_check_selected( checked );
  }
}
  
void LayerGroupWidget::check_uncheck_for_duplicate( bool checked )
{
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    ( *it ).second->set_check_selected( checked );
  }
} 

void LayerGroupWidget::enable_disable_delete_button()
{
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    if( ( *it ).second->get_selected() )
    {
      this->private_->button_menu_->set_delete_enabled( true );
      return;
    }
  }
  this->private_->button_menu_->set_delete_enabled( false );
}
  
void LayerGroupWidget::enable_disable_duplicate_button()
{
  for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    if( ( *it ).second->get_selected() )
    {
      this->private_->button_menu_->set_duplicate_enabled( true );
      return;
    }
  }
  this->private_->button_menu_->set_duplicate_enabled( false );
}
  
  void LayerGroupWidget::set_iso_surface_visibility( bool visible )
  {
    for( std::map< std::string, LayerWidgetQHandle >::iterator it = this->layer_map_.begin(); 
      it != this->layer_map_.end(); ++it )
    {
      ( *it ).second->set_iso_surface_visibility( visible );
    }
  }

  void LayerGroupWidget::set_picked_up( bool up )
  {
    this->private_->picked_up_ = up; 
  }



}  //end namespace Seg3D
