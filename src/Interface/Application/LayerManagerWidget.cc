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

// STD includes
#include <sstream>
#include <iostream>

// Boost includes
#include <boost/lexical_cast.hpp>

// Qt Includes
#include <QtGui/QVBoxLayout>

// Core includes
#include <Core/Utils/Log.h>

// QtUtils includes
#include <QtUtils/Utils/QtPointer.h>

//Application Includes
#include <Application/LayerManager/Actions/ActionActivateLayer.h>
#include <Application/LayerManager/LayerManager.h>

//Interface Includes
#include <Interface/Application/LayerManagerWidget.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class LayerManagerWidgetPrivate
//////////////////////////////////////////////////////////////////////////

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

  bool loading_states_;
  // -- static functions for callbacks into this widget --
public:
  typedef QPointer< LayerManagerWidget > qpointer_type;

  // HANDLEGROUPINTERNALCHANGED:
  static void HandleGroupInternalChanged( qpointer_type qpointer, LayerGroupHandle &group );

  // HANDLEGROUPSCHANGED:
  static void HandleGroupsChanged( qpointer_type qpointer );

  static void PreLoadStates( qpointer_type qpointer );

  static void PostLoadStates( qpointer_type qpointer );

  static void HandleReset( qpointer_type qpointer );
};

void LayerManagerWidgetPrivate::HandleGroupInternalChanged( qpointer_type qpointer, 
                          LayerGroupHandle &group )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &LayerManagerWidget::handle_group_internals_change, qpointer.data(), group ) ) );
}

void LayerManagerWidgetPrivate::HandleGroupsChanged( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind( 
    &LayerManagerWidget::handle_groups_changed, qpointer.data() ) ) );
}

void LayerManagerWidgetPrivate::PreLoadStates( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &LayerManagerWidget::pre_load_states, qpointer.data() ) ) );
}

void LayerManagerWidgetPrivate::PostLoadStates( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &LayerManagerWidget::post_load_states, qpointer.data() ) ) );
}

void LayerManagerWidgetPrivate::HandleReset( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &LayerManagerWidget::reset, qpointer.data() ) ) );
}

//////////////////////////////////////////////////////////////////////////
// Class LayerManagerWidget
//////////////////////////////////////////////////////////////////////////

LayerManagerWidget::LayerManagerWidget( QWidget* parent ) :
  QScrollArea( parent ),
  private_( new LayerManagerWidgetPrivate )
{
  this->private_->loading_states_ = false;

  // Customize the settings for the scroll area
  this->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  this->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  this->setContentsMargins( 1,1,1,1 );
  this->setWidgetResizable( true );
  
  this->setFrameShape( QFrame::NoFrame );
  this->setFrameShadow( QFrame::Plain );
  this->setLineWidth( 0 );

  // Define a new central widget inside the scroll area
  this->private_->main_ = new QWidget( parent );
  this->setWidget( this->private_->main_ );
  
  // Setup the spacing between the groups
  this->private_->group_layout_ = new QVBoxLayout( this->private_->main_ );
  this->private_->group_layout_->setSpacing( 2 );
  this->private_->group_layout_->setContentsMargins( 1,1,1,1 );
  this->private_->group_layout_->setAlignment( Qt::AlignTop );
  
  this->private_->main_->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  this->private_->main_->setAcceptDrops( true );

  LayerManager::lock_type lock( LayerManager::Instance()->get_mutex() );
  LayerManagerWidgetPrivate::qpointer_type layer_widget( this );
  
  // Connect the signals from the LayerManager to the GUI
  this->add_connection(LayerManager::Instance()->group_internals_changed_signal_.connect( 
    boost::bind( &LayerManagerWidgetPrivate::HandleGroupInternalChanged, layer_widget, _1 ) ) );
  
  this->add_connection(LayerManager::Instance()->groups_changed_signal_.connect( 
    boost::bind( &LayerManagerWidgetPrivate::HandleGroupsChanged, layer_widget ) ) );

  this->add_connection( Core::StateEngine::Instance()->pre_load_states_signal_.connect( 
    boost::bind( &LayerManagerWidgetPrivate::PreLoadStates, layer_widget ) ) );
  this->add_connection( Core::StateEngine::Instance()->post_load_states_signal_.connect( 
    boost::bind( &LayerManagerWidgetPrivate::PostLoadStates, layer_widget ) ) );
  this->add_connection( Core::Application::Instance()->reset_signal_.connect(
    boost::bind( &LayerManagerWidgetPrivate::HandleReset, layer_widget ) ) );
  
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
  if ( this->private_->loading_states_ )
  {
    return;
  }
  
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
  if ( this->private_->loading_states_ )
  {
    return;
  }
  
  LayerManagerWidgetPrivate::group_widget_map_type::iterator it = 
    this->private_->group_map_.find( group->get_group_id() );
    
  if( it != this->private_->group_map_.end() )
  {
    ( *it ).second->handle_change();
  }
  else
  {
    CORE_LOG_ERROR( "Group has not been created yet or has been deleted" );
  }
}

void LayerManagerWidget::pre_load_states()
{
  assert( this->private_->group_map_.empty() );

  this->private_->loading_states_ = true;
}

void LayerManagerWidget::post_load_states()
{
  this->private_->loading_states_ = false;
  this->handle_groups_changed();
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
    LayerGroupHandle group = 
      LayerManager::Instance()->get_group_by_id( ( *it ).second->get_group_id() );
  
    if( group && group->group_widget_expanded_state_->get() )
    { 
      ( *it ).second->prep_layers_for_drag_and_drop( move_time );
    }
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

void LayerManagerWidget::reset()
{
  QLayoutItem* group_item;
  while ( ( group_item = this->private_->group_layout_->takeAt( 0 ) ) != 0 )
  {
    delete group_item;
  }
  this->private_->group_map_.clear();
}

}  // end namespace Seg3D
