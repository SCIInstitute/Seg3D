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

#include <Utils/Core/Log.h>
#include <boost/lexical_cast.hpp>

// Application Includes
#include <Application/LayerManager/LayerManager.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LabelLayer.h>
#include <Application/Interface/Interface.h>

// Interface includes
#include <Interface/AppInterface/LayerManagerDockWidget.h>


namespace Seg3D
{

LayerManagerDockWidget::LayerManagerDockWidget( QWidget *parent ) :
  QDockWidget( "LayerManagerDockWidget" )
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
  
  add_connection( LayerManager::Instance()->layer_inserted_signal_.connect( boost::bind(
          &LayerManagerDockWidget::HandleInsertLayer, layer_dock_widget, _1 ) ) );
          
  add_connection( LayerManager::Instance()->layer_inserted_at_signal_.connect( boost::bind(
    &LayerManagerDockWidget::HandleInsertLayerAt, layer_dock_widget, _1, _2 ) ) );
          
  add_connection( LayerManager::Instance()->layer_deleted_signal_.connect( boost::bind(
          &LayerManagerDockWidget::HandleDeleteLayer, layer_dock_widget, _1 ) ) );
          
  add_connection( LayerManager::Instance()->layers_deleted_signal_.connect( boost::bind(
    &LayerManagerDockWidget::HandleDeleteLayers, layer_dock_widget, _1 ) ) );
          
  add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect( boost::bind(
          &LayerManagerDockWidget::HandleActivateLayer, layer_dock_widget, _1 ) ) );
  
  add_connection( LayerManager::Instance()->group_deleted_signal_.connect( boost::bind(
                  &LayerManagerDockWidget::HandleGroupDeleted, layer_dock_widget, _1 ) ) );
                  
  add_connection( LayerManager::Instance()->group_changed_signal_.connect( boost::bind(
          &LayerManagerDockWidget::HandleGroupChanged, layer_dock_widget, _1 ) ) );
          
  std::vector< LayerHandle > temporary_layerhandle_vector;
  LayerManager::Instance()->get_layers( temporary_layerhandle_vector );
  
  for( size_t i = 0; i <  temporary_layerhandle_vector.size(); ++i)
  {
      insert_layer_ui( temporary_layerhandle_vector[i] );
  }
  
}

LayerManagerDockWidget::~LayerManagerDockWidget()
{

}

void LayerManagerDockWidget::insert_layer_ui( LayerHandle &layer )
{
  layer_manager_widget_->insert_layer( layer );
}
void LayerManagerDockWidget::insert_layer_at_ui( LayerHandle &layer, int index )
{
  layer_manager_widget_->insert_layer( layer, index );
}

void LayerManagerDockWidget::delete_layer_ui( LayerHandle &layer )
{
  layer_manager_widget_->delete_layer( layer );
}

void LayerManagerDockWidget::delete_layers_ui( std::vector< LayerHandle > layers )
{
  layer_manager_widget_->delete_layers( layers );
}


void LayerManagerDockWidget::activate_layer_ui( LayerHandle &layer )
{
    layer_manager_widget_->set_active_layer( layer );
}

void LayerManagerDockWidget::delete_group_ui( LayerGroupHandle &group )
{
  layer_manager_widget_->delete_group( group );
}

void LayerManagerDockWidget::refresh_group_ui( LayerGroupHandle &group )
{
  layer_manager_widget_->refresh_group( group );
}
  
void LayerManagerDockWidget::HandleInsertLayer( qpointer_type qpointer, LayerHandle layer )
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleInsertLayer,
                             qpointer, layer ) );
    return;
  }
  
  SCI_LOG_DEBUG( "HandleInsertLayer started" );
  if ( qpointer.data() ) qpointer->insert_layer_ui( layer );
  SCI_LOG_DEBUG( "HandleInsertLayer done" );
}

void LayerManagerDockWidget::HandleInsertLayerAt( qpointer_type qpointer, LayerHandle layer, int index )
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleInsertLayerAt,
      qpointer, layer, index ) );
    return;
  }

  SCI_LOG_DEBUG( "HandleInsertLayerAt started" );
  if ( qpointer.data() ) qpointer->insert_layer_at_ui( layer, index );
  SCI_LOG_DEBUG( "HandleInsertLayerAt done" );
}

void LayerManagerDockWidget::HandleDeleteLayer( qpointer_type qpointer, LayerHandle layer )
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleDeleteLayer,
                             qpointer, layer ) );
    return;
  }
  
  if ( qpointer.data() ) qpointer->delete_layer_ui( layer );
}

void LayerManagerDockWidget::HandleDeleteLayers( qpointer_type qpointer, std::vector< LayerHandle > layers)
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleDeleteLayers,
      qpointer, layers ) );
    return;
  }
  
  SCI_LOG_DEBUG( "HandleDeleteLayer started" );
  if ( qpointer.data() ) qpointer->delete_layers_ui( layers );
  SCI_LOG_DEBUG( "HandleDeleteLayer done" );

}



void LayerManagerDockWidget::HandleActivateLayer( qpointer_type qpointer, LayerHandle layer )
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleActivateLayer,
        qpointer, layer ) );
    return;
  }

  SCI_LOG_DEBUG( "HandleActivateLayer started" );
  if ( qpointer.data() ) qpointer->activate_layer_ui( layer );
  SCI_LOG_DEBUG( "HandleActivateLayer done" );
}

void LayerManagerDockWidget::HandleGroupDeleted( qpointer_type qpointer, LayerGroupHandle group )
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleGroupDeleted,
                             qpointer, group ) );
    return;
  }
  
  SCI_LOG_DEBUG( "HandleGroupDeleted started" );
  if ( qpointer.data() ) qpointer->delete_group_ui( group );
  SCI_LOG_DEBUG( "HandleGroupDeleted done" );
}

void LayerManagerDockWidget::HandleGroupChanged( qpointer_type qpointer, LayerGroupHandle group )
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::HandleGroupChanged,
      qpointer, group ) );
    return;
  }

  SCI_LOG_DEBUG( "HandleGroupChanged started" );
  if ( qpointer.data() ) qpointer->refresh_group_ui( group );
  SCI_LOG_DEBUG( "HandleGroupChanged done" );
}


}  // end namespace Seg3D
