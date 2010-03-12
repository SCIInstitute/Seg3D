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

#include <Application/LayerManager/Actions/ActionCloneLayer.h>
#include <Application/LayerManager/Actions/ActionLayer.h>
#include <Application/LayerManager/Actions/ActionLayerFromFile.h>
#include <Application/LayerManager/Actions/ActionNewMaskLayer.h>
#include <Application/LayerManager/Actions/ActionRemoveLayer.h>

// Interface includes
#include <Interface/AppInterface/LayerManagerDockWidget.h>


namespace Seg3D
{

LayerManagerDockWidget::LayerManagerDockWidget( QWidget *parent ) :
  QDockWidget( "LayerManagerDockWidget" )
{
  setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  setWindowTitle( "Layer Manager" );

  resize( 280, 640 );
  layer_manager_widget_ = new LayerManagerWidget( this );
  setWidget( layer_manager_widget_ );
  
  LayerManager::lock_type lock( LayerManager::Instance()->get_mutex() );
  
  qpointer_type layer_dock_widget( this );
  
  add_connection( LayerManager::Instance()->group_layers_changed_signal_.connect( boost::bind(
          &LayerManagerDockWidget::handle_insert_layer, layer_dock_widget, _1 ) ) );
  

  std::vector< LayerGroupHandle > temporary_layergrouphandle_vector;
  LayerManager::Instance()->return_group_vector( temporary_layergrouphandle_vector );
  
  for( size_t i = 0; i <  temporary_layergrouphandle_vector.size(); ++i)
  {
    process_group_ui( temporary_layergrouphandle_vector[i] );
  }
  
}

LayerManagerDockWidget::~LayerManagerDockWidget()
{

}

void LayerManagerDockWidget::process_group_ui( LayerGroupHandle &group )
{
  layer_manager_widget_->process_group( group );
}
  
  
void LayerManagerDockWidget::insert_layer( LayerHandle &layer ) 
{
  //layer_manager_widget_->new_layer( layer );
  //layer_manager_widget_->new_layer( layer );
}

void LayerManagerDockWidget::new_group()
{
  //TODO implement new group function
}
void LayerManagerDockWidget::close_group()
{
  //TODO implement close group function
}
void LayerManagerDockWidget::layer_from_file()
{
  //TODO implement open data layer function
}
void LayerManagerDockWidget::clone_layer( LayerHandle& layer )
{
  //TODO implement clone layer function
}
void LayerManagerDockWidget::new_mask_layer()
{
  //TODO implement new mask layer function
}
void LayerManagerDockWidget::remove_layer( LayerHandle& layer )
{
  //TODO implement remove layer function
}
  
void LayerManagerDockWidget::insert_above_layer( LayerHandle& below_layer, LayerHandle& above_layer )
{
  //TODO implement insert layer above function
}
  
void LayerManagerDockWidget::handle_insert_layer( qpointer_type qpointer, LayerGroupHandle &group )
{
  if ( !( Interface::IsInterfaceThread() ) )
  {
    Interface::Instance()->post_event( boost::bind( &LayerManagerDockWidget::handle_insert_layer,
                             qpointer, group ) );
    return;
  }
  
  if ( qpointer.data() ) qpointer->process_group_ui( group );
}

}  // end namespace Seg3D
