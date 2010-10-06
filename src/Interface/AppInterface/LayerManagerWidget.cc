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


class LayerManagerWidgetPrivate : public boost::noncopyable
{
public:
  // Main widget inside the scroll area
  QWidget* main_;
  
  // Layouts that define where the groups are located within the
  // widget
  QVBoxLayout* main_layout_;
  QVBoxLayout* group_layout_;

  // Mapping of group name to its underlying widget
  typedef std::map< std::string, LayerGroupWidgetQHandle > group_widget_map_type;
  group_widget_map_type group_map_;
};


LayerManagerWidget::LayerManagerWidget( QWidget* parent ) :
  QScrollArea( parent ),
  private_( new LayerManagerWidgetPrivate )
{
  // Customize the settings for the scroll area
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  setContentsMargins( 1, 1, 1, 1 );
  setWidgetResizable( true );

  // Define a new central widget inside the scroll area
  this->private_->main_ = new QWidget( parent );
  setWidget( this->private_->main_ );
  
  // Setup the spacing between the groups
  this->private_->group_layout_ = new QVBoxLayout;
  this->private_->group_layout_->setSpacing( 0 );
  this->private_->group_layout_->setContentsMargins( 1, 1, 1, 1 );
  this->private_->group_layout_->setAlignment( Qt::AlignTop );
  
  // Link the layout to the main widget
  this->private_->main_->setLayout( this->private_->group_layout_ );
  this->private_->main_->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  this->private_->main_->setAcceptDrops( true );

  LayerManager::lock_type lock( LayerManager::Instance()->get_mutex() );
  qpointer_type layer_widget( this );
  
  // Connect the signals from the LayerManager to the GUI
  this->add_connection(LayerManager::Instance()->group_internals_changed_signal_.connect( 
    boost::bind( &LayerManagerWidget::HandleGroupInternalChanged, layer_widget, _1 ) ) );
  
  this->add_connection(LayerManager::Instance()->groups_changed_signal_.connect( 
    boost::bind( &LayerManagerWidget::HandleGroupsChanged, layer_widget ) ) );
  
  // Add any layers that may have been added before the GUI was initialized
  this->handle_groups_changed();
}

LayerManagerWidget::~LayerManagerWidget()
{
  this->disconnect_all();
}

// Functions for handling Layer or LayerGroup changes from the LayerManager
void LayerManagerWidget::handle_groups_changed()
{
  // Get an up to date list from the layermanager
  std::vector< LayerGroupHandle > group_list;
  LayerManager::Instance()->get_groups( group_list );

  // Disable updates to the layermanager widget, while it is being reconfigured
  this->setUpdatesEnabled( false );
  
  // Remove unused group widgets from layout
  
  LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
    this->private_->group_map_.begin();
  while ( it != this->private_->group_map_.end() )
  {
    // remove drag and drop infrastructure in current group widget
    ( *it ).second->seethrough( false );
    ( *it ).second->instant_hide_drop_space();

    // Check whether any group has the name we are looking for
    bool found = false;
    for ( size_t i = 0; i < group_list.size(); ++i )
    {
      if ( group_list[ i ]->get_group_id() == ( *it ).first )
      {
        found = true;
        break;
      }
    }
    
    // Continue to the next one, and keep the old one in case we need to delete it
    LayerManagerWidgetPrivate::group_widget_map_type::iterator prev = it++;
    if ( !found )
    {
      LayerGroupWidget* group_widget = ( *prev ).second.data();
      this->private_->group_layout_->removeWidget( group_widget );
      group_widget->deleteLater();
      this->private_->group_map_.erase( prev );
    }
  }

  this->setMinimumHeight( 0 );

  // Add the group widgets in order
  for ( size_t i = 0; i < group_list.size(); ++i )
  {
    LayerGroupWidget* group_widget;
    LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
      this->private_->group_map_.find( group_list[ i ]->get_group_id() );
      
    // Create new one if needed, or else find the old one
    if ( it == this->private_->group_map_.end() )
    {
      group_widget = this->make_new_group( group_list[ i ] );
      group_widget->handle_change();
      group_widget->show();
      this->private_->group_map_[ group_list[ i ]->get_group_id() ] =
        LayerGroupWidgetQHandle( group_widget );
    }
    else
    {
      group_widget = ( *it ).second.data();
    }

    this->private_->group_layout_->insertWidget( static_cast< int >( i ), group_widget );
  }

  this->setUpdatesEnabled( true );
}

void LayerManagerWidget::handle_group_internals_change( LayerGroupHandle group )
{
  LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
    this->private_->group_map_.find( group->get_group_id() );
    
  if( it != this->private_->group_map_.end() )
  {
    ( *it ).second->handle_change();
  }
  else
  {
    CORE_THROW_LOGICERROR( "Group has not been created yet" );
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
      
  connect( new_group, SIGNAL( picked_up_layer_size_signal_( int ) ), 
    this, SLOT( notify_groups_of_picked_up_layer_size( int ) ) );
  
  return new_group;
}


// Drag and Drop Functions  
void LayerManagerWidget::prep_layers_for_drag_and_drop( bool move_time )
{
  for( LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
    this->private_->group_map_.begin(); it != this->private_->group_map_.end(); ++it )
  {
    ( *it ).second->prep_layers_for_drag_and_drop( move_time );
  }
  
}

void LayerManagerWidget::prep_groups_for_drag_and_drop( bool move_time )
{
  for( LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
    this->private_->group_map_.begin(); it != this->private_->group_map_.end(); ++it )
  {
    ( *it ).second->prep_for_animation( move_time );
  }
  
}
  
void LayerManagerWidget::notify_picked_up_group_size( int group_size )
{
  for( LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
    this->private_->group_map_.begin(); it != this->private_->group_map_.end(); ++it )
  {
    ( *it ).second->set_picked_up_group_size( group_size );
  }
}

void LayerManagerWidget::notify_groups_of_picked_up_layer_size( int layer_size )
{
  for( LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
    this->private_->group_map_.begin(); it != this->private_->group_map_.end(); ++it )
  {
    ( *it ).second->notify_picked_up_layer_size( layer_size );
  }
}

void LayerManagerWidget::HandleGroupInternalChanged( qpointer_type qpointer, 
  LayerGroupHandle &group )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &LayerManagerWidget::HandleGroupInternalChanged, qpointer, group ) );
    return;
  }
  
  if( qpointer.data() ) qpointer->handle_group_internals_change( group );
}
           
void LayerManagerWidget::HandleGroupsChanged( qpointer_type qpointer )
{
   if( !( Core::Interface::IsInterfaceThread() ) )
   {
     Core::Interface::Instance()->post_event( boost::bind( 
      &LayerManagerWidget::HandleGroupsChanged, qpointer ) );
     return;
   }
   
   if( qpointer.data() ) qpointer->handle_groups_changed();
}


}  // end namespace Seg3D
