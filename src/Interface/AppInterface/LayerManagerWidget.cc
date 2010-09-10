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


#include <Core/Utils/Log.h>

#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>

//Application Includes
#include <Application/LayerManager/Actions/ActionActivateLayer.h>
#include <Application/LayerManager/LayerManager.h>

//Interface Includes
#include <Interface/AppInterface/LayerManagerWidget.h>

namespace Seg3D
{

LayerManagerWidget::LayerManagerWidget( QWidget* parent ) :
  QScrollArea( parent )
{
  // set some values for the scrollarea widget
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  setContentsMargins( 1, 1, 1, 1 );
  setWidgetResizable( true );

  this->main_ = new QWidget( parent );
  setWidget( this->main_ );
  
  this->group_layout_ = new QVBoxLayout;
  this->group_layout_->setSpacing( 2 );
  this->group_layout_->setContentsMargins( 1, 1, 1, 1 );
  this->group_layout_->setAlignment( Qt::AlignTop );
  
  this->main_->setLayout( this->group_layout_ );
  this->main_->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  this->main_->setAcceptDrops( true );

}
// destructor 
LayerManagerWidget::~LayerManagerWidget()
{
}

// Functions for handling Layer or LayerGroup changes from the LayerManager
void LayerManagerWidget::handle_groups_changed()
{
  std::vector< LayerGroupHandle > group_list;
  LayerManager::Instance()->get_groups( group_list );
  bool group_widget_deleted = false;
  
  this->setUpdatesEnabled( false );
  
  // Remove unused group widgets from layout
  group_widget_map_type::iterator it = this->group_map_.begin();
  while ( it != this->group_map_.end() )
  {
    ( *it ).second->set_drop( false );
    ( *it ).second->seethrough( false );

    bool found = false;
    for ( size_t i = 0; i < group_list.size(); ++i )
    {
      if ( group_list[ i ]->get_group_id() == ( *it ).first )
      {
        found = true;
        break;
      }
    }
    group_widget_map_type::iterator prev = it++;
    if ( !found )
    {
      LayerGroupWidget* group_widget = ( *prev ).second.data();
      this->group_layout_->removeWidget( group_widget );
      group_widget->deleteLater();
      this->group_map_.erase( prev );
    }
  }

  this->setMinimumHeight( 0 );

  // Add the group widgets in order
  for ( size_t i = 0; i < group_list.size(); ++i )
  {
    LayerGroupWidget* group_widget;
    group_widget_map_type::iterator it = this->group_map_.find( 
      group_list[ i ]->get_group_id() );
    if ( it == this->group_map_.end() )
    {
      group_widget = this->make_new_group( group_list[ i ] );
      group_widget->handle_change();
      group_widget->show();
      this->group_map_[ group_list[ i ]->get_group_id() ] =
        LayerGroupWidgetQHandle( group_widget );
    }
    else
    {
      group_widget = ( *it ).second.data();
    }

    this->group_layout_->insertWidget( static_cast< int >( i ), group_widget );
  }

  this->setUpdatesEnabled( true );
}

void LayerManagerWidget::handle_group_internals_change( LayerGroupHandle group )
{
  group_widget_map_type::iterator it = this->group_map_.find( group->get_group_id() );
  if( it != this->group_map_.end() )
  {
    ( *it ).second->handle_change();
  }
  else
  {
    CORE_THROW_LOGICERROR( "Group hasn't been created yet" );
  }
}

LayerGroupWidget* LayerManagerWidget::make_new_group( LayerGroupHandle group )
{
  LayerGroupWidget* new_group = new LayerGroupWidget( this, group );
  
  connect( new_group, SIGNAL( prep_layers_for_drag_and_drop_signal_( bool ) ), 
      this, SLOT( prep_layers_for_drag_and_drop( bool ) ) );
  
  connect( new_group, SIGNAL( prep_groups_for_drag_and_drop_signal_( bool ) ), 
      this, SLOT( prep_groups_for_drag_and_drop( bool ) ) );
  
  connect( new_group, SIGNAL( picked_up_group_size_signal_( int ) ), 
      this, SLOT( notify_picked_up_group_size( int ) ) );
  
  return new_group;
}

void  LayerManagerWidget::set_active_layer( LayerHandle layer )
{
  if( this->active_layer_ )
    this->active_layer_.toStrongRef()->set_active( false );
  
  for( group_widget_map_type::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    LayerWidgetQWeakHandle temp_layer = ( *it ).second->set_active_layer( layer );
      if( temp_layer )
      {
      this->active_layer_ = temp_layer;
      break;
    }
  }
}

// Drag and Drop Functions  
void LayerManagerWidget::prep_layers_for_drag_and_drop( bool move_time )
{
  for( group_widget_map_type::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    ( *it ).second->prep_layers_for_drag_and_drop( move_time );
  }
  
}

void LayerManagerWidget::prep_groups_for_drag_and_drop( bool move_time )
{
  for( group_widget_map_type::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    ( *it ).second->prep_for_animation( move_time );
  }
  
}
  
void LayerManagerWidget::notify_picked_up_group_size( int group_size )
{
  for( group_widget_map_type::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    ( *it ).second->set_picked_up_group_size( group_size );
  }
}

}  // end namespace Seg3D
