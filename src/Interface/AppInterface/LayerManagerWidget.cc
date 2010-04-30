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


#include <Utils/Core/Log.h>

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


void LayerManagerWidget::insert_layer( LayerHandle layer )
{
  std::string group_id = layer->get_layer_group()->get_group_id();
  bool inserted = false;

  for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
    i  != this->group_list_.end(); i++ )
  {
    if( group_id == ( *i )->get_group_id() ) 
    {
      ( *i )->setUpdatesEnabled( false );
      ( *i )->insert_layer( layer, -1 );
      if( layer->get_active() )
        this->set_active_layer( layer );
      ( *i )->setUpdatesEnabled( true );
      ( *i )->repaint();
      inserted = true;
      break;
    } 
  }
  if( !inserted )
  {
    make_new_group( layer );
  }
}

void LayerManagerWidget::insert_layer( LayerHandle layer, int index )
{
  std::string group_id = layer->get_layer_group()->get_group_id();

  for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
    i  != this->group_list_.end(); i++ )
  {
    if( group_id == ( *i )->get_group_id() ) 
    { 
      ( *i )->setUpdatesEnabled( false );
      ( *i )->insert_layer( layer, index );
      if( layer->get_active() )
        this->set_active_layer( layer );
      ( *i )->setUpdatesEnabled( true );
      ( *i )->repaint();
      break;
    } 
  }
}

void LayerManagerWidget::move_group( std::string group_id, int new_index )
{
  int current_index = 0;

  for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
      i  != this->group_list_.end(); i++ )
  {
    if( ( *i )->get_group_id() == group_id )
    {
      LayerGroupWidgetQHandle group_handle( *i ); 
      this->group_layout_->removeWidget( group_handle.data() );
      this->group_layout_->insertWidget( new_index, group_handle.data() );
      break; 
    }
    current_index++;
  }
  
  this->group_list_.move( current_index, new_index );

  
  
}

void LayerManagerWidget::delete_layers( std::vector< LayerHandle > layers )
{
  for( int j = 0; j < static_cast< int >(layers.size()); ++j )
  {
    for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
      i  != this->group_list_.end(); i++ )
    {
      
      if( ( *i )->delete_layer( layers[j] ) )
        break;
    }   
  }
}

void LayerManagerWidget::delete_layer( LayerHandle layer )
{
  for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
    i  != this->group_list_.end(); i++ )
  {
    
    if( ( *i )->delete_layer( layer ) )
      break;
  }   
}



void LayerManagerWidget::make_new_group( LayerHandle layer )
{
    LayerGroupWidgetQHandle new_group_handle( new LayerGroupWidget( this->main_, layer ) );
  this->group_layout_->addWidget( new_group_handle.data() );
  new_group_handle->show();
  this->group_list_.push_back( new_group_handle );
}


  
void LayerManagerWidget::delete_group( LayerGroupHandle group )
{
  for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
     i  != this->group_list_.end(); i++ )
  {
    if( group->get_group_id() == ( *i )->get_group_id() ) 
    {
      ( *i )->deleteLater();
      group_list_.erase( i );
      return;
    }
  }
}


void  LayerManagerWidget::set_active_layer( LayerHandle layer )
{
  if( active_layer_ )
    this->active_layer_.toStrongRef()->set_active( false );
  
    //this->set_active_group( layer->get_layer_group() );   
    
    for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
     i  != this->group_list_.end(); i++ )
  {
    LayerWidgetQWeakHandle temp_layer = ( *i )->set_active_layer( layer );
      if( temp_layer )
      {
      active_layer_ = temp_layer;
      break;
    }
  }
}

}  // end namespace Seg3D
