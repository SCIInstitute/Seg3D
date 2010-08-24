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
  
  // First we remove the LayerGroupWidgets that arent needed any more.
  for( std::map< std::string, LayerGroupWidgetQHandle>::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    ( *it ).second->set_drop( false );
    ( *it ).second->seethrough( false );
    
    bool found = false;
    for( int i = 0; i < static_cast< int >( group_list.size() ); ++i )
    {
      if( ( *it ).first == group_list[ i ]->get_group_id() )
      {
        found = true;
        break;
      }
    }
    if( !found )
    {
      this->group_layout_->removeWidget( ( *it ).second.data() );
      group_widget_deleted = true;
    }
  }

  // then if we need to, we delete the unused LayerGroupWidgets from the group_map_
  if( group_widget_deleted )
  {
    this->cleanup_removed_groups();
  }
  
  this->setMinimumHeight( 0 );
  
  for( size_t i = 0; i < group_list.size(); ++i )
  {
    std::map< std::string, LayerGroupWidgetQHandle >::iterator found_group_widget = 
    this->group_map_.find( group_list[ i ]->get_group_id() );
    if( found_group_widget != this->group_map_.end() )
    {
      this->group_layout_->insertWidget( static_cast< int >( i ), 
        ( *found_group_widget ).second.data() );
    }
  }
  this->setUpdatesEnabled( true );
}

void LayerManagerWidget::cleanup_removed_groups()
{
  for( std::map< std::string, LayerGroupWidgetQHandle>::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    if( this->group_layout_->indexOf( ( *it ).second.data() ) == -1 )
    {
      ( *it ).second->deleteLater();
      this->group_map_.erase( it );
    }
  }
}

void LayerManagerWidget::handle_group_internals_change( LayerGroupHandle group )
{
  std::map< std::string, LayerGroupWidgetQHandle >::iterator found_group_widget = 
  this->group_map_.find( group->get_group_id() );
  if( found_group_widget != this->group_map_.end() )
  {
    ( *found_group_widget ).second->handle_change();
  }
  else
  {
    this->make_new_group( group );
    this->handle_group_internals_change( group );
  }
}

void LayerManagerWidget::make_new_group( LayerGroupHandle group )
{
  LayerGroupWidgetQHandle new_group_handle( new LayerGroupWidget( this, group ) );
  this->group_layout_->addWidget( new_group_handle.data() );
  new_group_handle->show();
  this->group_map_[ group->get_group_id() ] = new_group_handle;
  
  connect( new_group_handle.data(), SIGNAL( prep_layers_for_drag_and_drop_signal_( bool ) ), 
      this, SLOT( prep_layers_for_drag_and_drop( bool ) ) );
  
  connect( new_group_handle.data(), SIGNAL( prep_groups_for_drag_and_drop_signal_( bool ) ), 
      this, SLOT( prep_groups_for_drag_and_drop( bool ) ) );
  
  connect( new_group_handle.data(), SIGNAL( picked_up_group_size_signal_( int ) ), 
      this, SLOT( notify_picked_up_group_size( int ) ) );
  
}

void  LayerManagerWidget::set_active_layer( LayerHandle layer )
{
  if( this->active_layer_ )
    this->active_layer_.toStrongRef()->set_active( false );
  
  for( std::map< std::string, LayerGroupWidgetQHandle>::iterator it = this->group_map_.begin(); 
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
  for( std::map< std::string, LayerGroupWidgetQHandle>::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    ( *it ).second->prep_layers_for_drag_and_drop( move_time );
  }
  
}

void LayerManagerWidget::prep_groups_for_drag_and_drop( bool move_time )
{
  for( std::map< std::string, LayerGroupWidgetQHandle>::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    ( *it ).second->prep_for_animation( move_time );
  }
  
}
  
void LayerManagerWidget::notify_picked_up_group_size( int group_size )
{
  for( std::map< std::string, LayerGroupWidgetQHandle>::iterator it = this->group_map_.begin(); 
    it != this->group_map_.end(); ++it )
  {
    ( *it ).second->set_picked_up_group_size( group_size );
  }
}

}  // end namespace Seg3D
