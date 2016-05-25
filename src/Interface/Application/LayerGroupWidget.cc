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

// Qt includes
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QtCore/QMimeData>
#include <QtGui/QDrag>

//Core Includes - for logging
#include <Core/Utils/Log.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Application Includes
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionDeleteLayers.h>
#include <Application/Layer/Actions/ActionNewMaskLayer.h>
#include <Application/Layer/Actions/ActionDuplicateLayer.h>
#include <Application/Layer/Actions/ActionMoveGroup.h>

//Interface Includes
#include <Interface/Application/LayerWidget.h>
#include <Interface/Application/LayerGroupWidget.h>
#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/DropSpaceWidget.h>
#include <Interface/Application/OverlayWidget.h>
#include <Interface/Application/PushDragButton.h>
#include <Interface/Application/GroupButtonMenu.h>

//UI Includes
#include "ui_LayerGroupWidget.h"

namespace Seg3D
{

typedef std::map< std::string, LayerWidget* > LayerWidgetMap;

//////////////////////////////////////////////////////////////////////////
// Class LayerGroupWidgetPrivate
//////////////////////////////////////////////////////////////////////////

class LayerGroupWidgetPrivate : public QObject
{
  // -- Constructor --
public:
  LayerGroupWidgetPrivate( LayerGroupWidget* parent ) :
    QObject( parent ),
    parent_( parent )
  {}

  // -- Helper functions --
public:
  // GET_SELECTED_LAYER_IDS:
  // Get the IDs of all the selected layers.
  void get_selected_layer_ids( std::vector< std::string >& layers );

  // CREATE_LAYER_WIDGET:
  // Create a LayerWidget for the given layer.
  LayerWidget* create_layer_widget( LayerHandle layer );

  // SEETHROUGH:
  // function that puts the group widget into a state that makes it look "picked up"
  void seethrough( bool see );

  // SET_DROP_TARGET:
  // this function stores a local copy of the widget that is going to be dropped onto
  // in the LayerGroupWidget that is being dropped
  void set_drop_target( LayerGroupWidget* target_group );

  // ENABLE_DROP_SPACE:
  // this function give the user the impression that a group is available for dropping onto by
  // opening up a space for dropping
  void enable_drop_space( bool drop );

  // GET_GROUP_ID:
  // function that returns a string containing the groups id
  const std::string &get_group_id();

public:
  LayerGroupWidget* parent_;
  Ui::LayerGroupWidget ui_;
  DropSpaceWidget* drop_space_;
  OverlayWidget* overlay_;
  int group_height;
  LayerGroupWidget* drop_group_;
  GroupButtonMenu* button_menu_;

  LayerGroupHandle layer_group_;
  int picked_up_group_height_;
  bool group_menus_open_;
  bool picked_up_;

  LayerWidgetMap layer_map_;    
};

void LayerGroupWidgetPrivate::get_selected_layer_ids( std::vector< std::string >& layers )
{
  for( LayerWidgetMap::iterator it = this->layer_map_.begin(); 
    it != this->layer_map_.end(); ++it )
  {
    if( it->second->is_selected() )
    {
      std::string layer_id;
      Core::ImportFromString( it->first, layer_id );
      layers.push_back( layer_id );
    }
  }
}

LayerWidget* LayerGroupWidgetPrivate::create_layer_widget( LayerHandle layer )
{
  LayerWidget* layer_widget = new LayerWidget( this->ui_.group_frame_, layer );

  QObject::connect( layer_widget, SIGNAL( prep_for_drag_and_drop( bool ) ), 
    this->parent_, SIGNAL( prep_layers_for_drag_and_drop_signal_( bool ) ) );
  QObject::connect( layer_widget, SIGNAL( layer_size_signal_( int ) ), 
    this->parent_, SIGNAL( picked_up_layer_size_signal_( int ) ) );
  QObject::connect( layer_widget, SIGNAL( selection_box_changed() ),
    this->parent_, SLOT( enable_disable_delete_button() ) );
  QObject::connect( layer_widget, SIGNAL( selection_box_changed() ),
    this->parent_, SLOT( enable_disable_duplicate_button() ) );

  return layer_widget;
}

void LayerGroupWidgetPrivate::seethrough( bool see )
{
  this->picked_up_ = see;

  if( see )
  {
    this->ui_.base_->hide();
  }
  else
  { 
    this->ui_.base_->show();
  }
}

void LayerGroupWidgetPrivate::set_drop_target( LayerGroupWidget* target_group)
{
  this->drop_group_ = target_group;
}

void LayerGroupWidgetPrivate::enable_drop_space( bool drop )
{
  this->drop_space_->set_height( this->picked_up_group_height_ + 4 );

  if( this->picked_up_ ) return;

  if( drop )
  {
    this->drop_space_->show();
    this->overlay_->show();
  }
  else
  {
    this->drop_space_->hide();
    this->overlay_->hide();
  }
} 

const std::string& LayerGroupWidgetPrivate::get_group_id()
{
  return this->layer_group_->get_group_id();
}

//////////////////////////////////////////////////////////////////////////
// Class LayerGroupWidget
//////////////////////////////////////////////////////////////////////////

LayerGroupWidget::LayerGroupWidget( QWidget* parent, LayerGroupHandle group ) :
  QWidget( parent )
{
  this->private_ = new LayerGroupWidgetPrivate( this );
  this->private_->ui_.setupUi( this );
  this->private_->layer_group_ = group;
  this->private_->group_menus_open_ = false;
  this->private_->picked_up_ = false;
  this->private_->button_menu_ = new GroupButtonMenu( this, group );
  this->private_->ui_.tools_and_layers_layout_->addWidget( this->private_->button_menu_ );
  
  // Set up the Drag and Drop
  this->setAcceptDrops( true );

  // set some values of the GUI
  Core::GridTransform trans = group->get_grid_transform();
  std::string group_name = Core::ExportToString( trans.get_nx() ) + " x " +
    Core::ExportToString( trans.get_ny() ) + " x " +
    Core::ExportToString( trans.get_nz() );
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
  
  // Set the icons for the group visibility button
  QIcon none_visible_icon;
  none_visible_icon.addFile( QString::fromUtf8( ":/Palette_Icons/visibility_01.png" ), QSize(), QIcon::Normal );
  QIcon some_visible_icon;
  some_visible_icon.addFile( QString::fromUtf8( ":/Palette_Icons/visibility_01.png" ), QSize(), QIcon::Normal );
  QIcon all_visible_icon;
  all_visible_icon.addFile( QString::fromUtf8( ":/Palette_Icons/visibility_01.png" ), QSize(), QIcon::Normal );
  
  this->private_->ui_.group_visibility_button_->set_icons( none_visible_icon, 
    some_visible_icon, all_visible_icon );
  this->private_->ui_.group_visibility_button_->set_stylesheets( StyleSheet::TRISTATE_UNCHECKED_C,
                                                                 StyleSheet::TRISTATE_INTERMEDIATE_C,
                                                                 StyleSheet::TRISTATE_CHECKED_C );
    
  QtUtils::QtBridge::Connect( this->private_->ui_.group_visibility_button_, 
    group->layers_visible_state_ );
  
  QIcon no_iso_visible_icon;
  no_iso_visible_icon.addFile( QString::fromUtf8( ":/Palette_Icons/iso_01.png" ), QSize(), QIcon::Normal );
  QIcon some_iso_visible_icon;
  some_iso_visible_icon.addFile( QString::fromUtf8( ":/Palette_Icons/iso_01.png" ), QSize(), QIcon::Normal );
  QIcon all_iso_visible_icon;
  all_iso_visible_icon.addFile( QString::fromUtf8( ":/Palette_Icons/iso_01.png" ), QSize(), QIcon::Normal );
  
  this->private_->ui_.group_iso_visibility_button_->set_icons( no_iso_visible_icon, 
    some_iso_visible_icon, all_iso_visible_icon );
  this->private_->ui_.group_iso_visibility_button_->set_stylesheets( StyleSheet::TRISTATE_UNCHECKED_C,
                                                                     StyleSheet::TRISTATE_INTERMEDIATE_C,
                                                                     StyleSheet::TRISTATE_CHECKED_C );

  QtUtils::QtBridge::Connect( this->private_->ui_.group_iso_visibility_button_, 
    group->layers_iso_visible_state_ );

  this->private_->ui_.group_frame_layout_->setAlignment( Qt::AlignTop );
  this->private_->ui_.group_frame_layout_->setSpacing( 0 );
  this->private_->ui_.verticalLayout_13->setAlignment( Qt::AlignTop );

  this->private_->overlay_ = new OverlayWidget( this );
  this->private_->overlay_->hide();
  
  this->private_->ui_.fake_widget_->setMinimumHeight( 0 );
  this->private_->ui_.fake_widget_->hide();
  
  this->private_->ui_.group_dummy_->hide();
  
  this->private_->ui_.group_background_->setStyleSheet( StyleSheet::GROUP_WIDGET_BACKGROUND_ACTIVE_C );

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

  // Make up some mime data containing the layer_id of the layer
  QMimeData *mimeData = new QMimeData;
  mimeData->setText( QString::fromStdString( this->private_->get_group_id() ) );

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
  this->private_->seethrough( true );

  Q_EMIT prep_groups_for_drag_and_drop_signal_( true );
  Q_EMIT picked_up_group_size_signal_( this->height() - 2 );
  
  // If our drag was successful then we do stuff
  if( ( drag->exec( Qt::MoveAction, Qt::MoveAction ) ) == Qt::MoveAction ) 
  {
    ActionMoveGroup::Dispatch( Core::Interface::GetWidgetActionContext(), 
      this->private_->get_group_id(), this->private_->drop_group_->private_->get_group_id() );
  }

  this->private_->seethrough( false );
  
  Q_EMIT prep_groups_for_drag_and_drop_signal_( false );
  event->setAccepted( true );
}

void LayerGroupWidget::dropEvent( QDropEvent* event )
{
  this->private_->enable_drop_space( false );
  event->setAccepted( true );
  
  LayerGroupWidget* source_widget = dynamic_cast< LayerGroupWidget* >( event->source() );
  if( source_widget != NULL && source_widget != this )
  {
    source_widget->private_->set_drop_target( this ); 
    event->setDropAction( Qt::MoveAction );
    return;
  }
  event->setDropAction( Qt::IgnoreAction );
}

void LayerGroupWidget::dragEnterEvent( QDragEnterEvent* event)
{
  LayerGroupWidget* source_widget = dynamic_cast< LayerGroupWidget* >( event->source() );
  if( source_widget != NULL && source_widget != this )
  {
    this->private_->enable_drop_space( true );
  }
  event->setAccepted( true );
}

void LayerGroupWidget::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->private_->enable_drop_space( false );
  event->setAccepted( true );
}

void LayerGroupWidget::verify_delete()
{

  // Check whether the users wants to save and whether the user wants to quit
  int ret = QMessageBox::warning( this, "Delete Warning",
    "Are you sure you want to delete these layers?",
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No  );
        
  if( ret == QMessageBox::Yes )
  {
    std::vector< std::string > layers;
    this->private_->get_selected_layer_ids( layers );
    
    this->private_->button_menu_->blockSignals( true );
    this->private_->button_menu_->uncheck_delete_menu_button();
    this->private_->button_menu_->uncheck_delete_button();
    this->private_->button_menu_->blockSignals( false );
    
    ActionDeleteLayers::Dispatch( Core::Interface::GetWidgetActionContext(), 
      layers );
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
    this->private_->ui_.group_background_->setStyleSheet( StyleSheet::GROUP_WIDGET_BACKGROUND_INACTIVE_C );

  }
}
  
void LayerGroupWidget::show_group()
{
  this->private_->ui_.fake_widget_->hide();
  this->private_->ui_.tools_and_layers_widget_->show();
  this->private_->ui_.group_background_->setStyleSheet( StyleSheet::GROUP_WIDGET_BACKGROUND_ACTIVE_C );
}
  
void LayerGroupWidget::resizeEvent( QResizeEvent *event )
{
  this->private_->overlay_->resize( event->size() );
  event->setAccepted( true );
}
  
void LayerGroupWidget::prep_layers_for_drag_and_drop( bool move_time )
{
  for( LayerWidgetMap::iterator it = this->private_->layer_map_.begin(); 
    it != this->private_->layer_map_.end(); ++it )
  {
    ( *it ).second->prep_for_animation( move_time );
  } 
}

void LayerGroupWidget::prep_for_animation( bool move_time )
{
  if( this->private_->picked_up_ )
    return;
  
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
}
  
void LayerGroupWidget::set_picked_up_group_size( int group_height )
{
  this->private_->picked_up_group_height_ = group_height;
}
  
void LayerGroupWidget::handle_change()
{ 
  // Whether the group menu is currently shown
  bool show_menu = this->private_->layer_group_->show_delete_menu_state_->get() || 
    this->private_->layer_group_->show_duplicate_menu_state_->get();

  // Get a list of all the layers
  LayerVector layers;
  this->private_->layer_group_->get_layers( layers );

  // Make a copy of the current widgets map
  LayerWidgetMap tmp_map = this->private_->layer_map_;
  // Clear the original map
  this->private_->layer_map_.clear();

  // Disable UI updates
  this->setUpdatesEnabled( false );

  // Loop through all the layers and put their corresponding widgets in
  // the right order. Create new widgets when necessary.
  for ( size_t i = 0; i < layers.size(); ++i )
  {
    // Look for an existing widget for the layer. If found, remove it from the 
    // temporary map. Otherwise, create a new widget.
    LayerWidget* widget;
    std::string layer_id = layers[ i ]->get_layer_id();
    LayerWidgetMap::iterator it = tmp_map.find( layer_id );
    if ( it != tmp_map.end() )
    {
      widget = it->second;
      tmp_map.erase( it );

      // Instantly hide any still visible drop space
      widget->instant_hide_drop_space();

      // Clear selection checkbox
      widget->set_selected( false );
      if( show_menu )
      {
        widget->show_selection_checkbox( true );
      }
    }
    else
    {
      widget = this->private_->create_layer_widget( layers[ i ] );
    }
    
    // Put the widget in the layout
    this->private_->ui_.group_frame_layout_->insertWidget( static_cast< int >( i ), widget );
    // Add the widget to the map
    this->private_->layer_map_[ layer_id ] = widget;
  }
  
  // For anything left in the temporary map, they are no longer needed.
  // Remove them from the layout and delete them.
  for ( LayerWidgetMap::iterator it = tmp_map.begin(); it != tmp_map.end(); ++it )
  {
    LayerWidget* layer_widget = it->second;
    this->private_->ui_.group_frame_layout_->removeWidget( layer_widget );
    layer_widget->deleteLater();
  }
  tmp_map.clear();

  // Squeeze the layout
  this->private_->ui_.group_frame_->setMinimumHeight( 0 );
  // Enable UI updates
  this->setUpdatesEnabled( true );
}

void LayerGroupWidget::instant_hide_drop_space()
{
  this->private_->drop_space_->instant_hide();
}

void LayerGroupWidget::notify_picked_up_layer_size( int layer_size )
{
  for( LayerWidgetMap::iterator it = this->private_->layer_map_.begin(); 
    it != this->private_->layer_map_.end(); ++it )
  {
    ( *it ).second->set_picked_up_layer_size( layer_size );
  }
}
  
void LayerGroupWidget::duplicate_checked_layers()
{
  this->private_->button_menu_->blockSignals( true );
  this->private_->button_menu_->uncheck_delete_button();
  this->private_->button_menu_->uncheck_duplicate_button();
  this->private_->button_menu_->blockSignals( false );
  
  for( LayerWidgetMap::iterator it = this->private_->layer_map_.begin(); 
    it != this->private_->layer_map_.end(); ++it )
  {
    if( ( *it ).second->is_selected() )
    {
      ActionDuplicateLayer::Dispatch( Core::Interface::GetWidgetActionContext(), 
        ( *it ).second->get_layer_id() );
    }
  }
}

void LayerGroupWidget::check_uncheck_for_delete( bool checked )
{
  for( LayerWidgetMap::iterator it = this->private_->layer_map_.begin(); 
    it != this->private_->layer_map_.end(); ++it )
  {
    ( *it ).second->set_selected( checked );
  }
}
  
void LayerGroupWidget::check_uncheck_for_duplicate( bool checked )
{
  for( LayerWidgetMap::iterator it = this->private_->layer_map_.begin(); 
    it != this->private_->layer_map_.end(); ++it )
  {
    ( *it ).second->set_selected( checked );
  }
} 

void LayerGroupWidget::enable_disable_delete_button()
{
  for( LayerWidgetMap::iterator it = this->private_->layer_map_.begin(); 
    it != this->private_->layer_map_.end(); ++it )
  {
    if( ( *it ).second->is_selected() )
    {
      this->private_->button_menu_->set_delete_enabled( true );
      return;
    }
  }
  this->private_->button_menu_->set_delete_enabled( false );
}
  
void LayerGroupWidget::enable_disable_duplicate_button()
{
  for( LayerWidgetMap::iterator it = this->private_->layer_map_.begin(); 
    it != this->private_->layer_map_.end(); ++it )
  {
    if( ( *it ).second->is_selected() )
    {
      this->private_->button_menu_->set_duplicate_enabled( true );
      return;
    }
  }
  this->private_->button_menu_->set_duplicate_enabled( false );
}

}  //end namespace Seg3D
