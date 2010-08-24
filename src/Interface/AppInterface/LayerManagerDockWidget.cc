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
#include <sstream>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>

// Application Includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LabelLayer.h>

// Interface includes
#include <Interface/AppInterface/LayerManagerDockWidget.h>


namespace Seg3D
{

LayerManagerDockWidget::LayerManagerDockWidget( QWidget *parent ) :
  QDockWidget( "LayerManagerDockWidget", parent )
{
  this->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  this->setWindowTitle( "Layer Manager" );

  this->resize( 280, 640 );
  QSizePolicy sizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  setSizePolicy( sizePolicy );
  this->setMinimumSize( QSize( 280, 313 ) );
    this->setMaximumSize( QSize( 524287, 524287 ) );
  
  this->layer_manager_widget_ = QSharedPointer< LayerManagerWidget > ( new LayerManagerWidget( this ) );
  this->setWidget( layer_manager_widget_.data() );
  
  LayerManager::lock_type lock( LayerManager::Instance()->get_mutex() );
  
  qpointer_type layer_dock_widget( this );
  
  
  // Connect the signals from the LayerManager to the GUI
  
  add_connection(LayerManager::Instance()->group_internals_changed_signal_.connect( boost::bind(
    &LayerManagerDockWidget::HandleGroupInternalChanged, layer_dock_widget, _1 ) ) );
  
  add_connection(LayerManager::Instance()->groups_changed_signal_.connect( boost::bind(
    &LayerManagerDockWidget::HandleGroupsChanged, layer_dock_widget ) ) );
        
  add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect( boost::bind(
    &LayerManagerDockWidget::HandleActivateLayer, layer_dock_widget, _1 ) ) );
  
  // Add any layers that may have been added before the GUI was initialized
  std::vector< LayerGroupHandle > temp_groups_vectors;
  LayerManager::Instance()->get_groups( temp_groups_vectors );
  
  for( size_t i = 0; i <  temp_groups_vectors.size(); ++i)
  {
      this->group_internals_changed_ui( temp_groups_vectors[ i ] );
  }

}

LayerManagerDockWidget::~LayerManagerDockWidget()
{
  this->disconnect_all();
}

void LayerManagerDockWidget::activate_layer_ui( LayerHandle &layer )
{
    this->layer_manager_widget_->set_active_layer( layer );
}
           
void LayerManagerDockWidget::groups_changed_ui()
{
   this->layer_manager_widget_->handle_groups_changed();
} 

void LayerManagerDockWidget::group_internals_changed_ui( LayerGroupHandle &group )
{
  this->layer_manager_widget_->handle_group_internals_change( group );
}

  void LayerManagerDockWidget::HandleActivateLayer( qpointer_type qpointer, LayerHandle &layer )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleActivateLayer,
        qpointer, layer ) );
    return;
  }

  CORE_LOG_DEBUG( "HandleActivateLayer started" );
  if( qpointer.data() ) qpointer->activate_layer_ui( layer );
  CORE_LOG_DEBUG( "HandleActivateLayer done" );
}

void LayerManagerDockWidget::HandleGroupInternalChanged( qpointer_type qpointer, LayerGroupHandle &group )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( 
      &LayerManagerDockWidget::HandleGroupInternalChanged, qpointer, group ) );
    return;
  }
  
  CORE_LOG_DEBUG( "HandleGroupInternalChanged started" );
  if( qpointer.data() ) qpointer->group_internals_changed_ui( group );
  CORE_LOG_DEBUG( "HandleGroupInternalChanged done" );
}
           
void LayerManagerDockWidget::HandleGroupsChanged( qpointer_type qpointer )
{
   if( !( Core::Interface::IsInterfaceThread() ) )
   {
     Core::Interface::Instance()->post_event( boost::bind( 
      &LayerManagerDockWidget::HandleGroupsChanged, qpointer ) );
     return;
   }
   
   CORE_LOG_DEBUG( "HandleGroupsChanged started" );
   if( qpointer.data() ) qpointer->groups_changed_ui();
   CORE_LOG_DEBUG( "HandleGroupsChanged done" );
}

}  // end namespace Seg3D
