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

  this->main_layout_ = new QVBoxLayout( this->main_ );
  this->main_layout_->setContentsMargins( 1, 1, 1, 1 );
  this->main_layout_->setSpacing( 1 );

  this->group_layout_ = new QVBoxLayout;
  this->group_layout_->setSpacing( 5 );
  this->main_layout_->addLayout( this->group_layout_ );
  this->main_layout_->addStretch();

  this->main_->setLayout( this->main_layout_ );
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
    if ( group_id == ( *i )->get_group_id() ) 
    {
      ( *i )->setUpdatesEnabled( false );
      ( *i )->insert_layer( layer, -1 );
      ( *i )->setUpdatesEnabled( true );
      if( layer->get_active() )
        this->set_active_layer( layer );
      ( *i )->update();
      inserted = true;
      break;
    } 
  }
  if ( !inserted )
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
    if ( group_id == ( *i )->get_group_id() ) 
    { 
      ( *i )->setUpdatesEnabled( false );
      ( *i )->insert_layer( layer, index );
      ( *i )->setUpdatesEnabled( true );
      if( layer->get_active() )
        this->set_active_layer( layer );
      ( *i )->update();
      break;
    } 
  }
}

void LayerManagerWidget::delete_layers( std::vector< LayerHandle > layers )
{
  this->setUpdatesEnabled( false );

  for( int j = 0; j < static_cast< int >(layers.size()); ++j )
  {
    for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
      i  != this->group_list_.end(); i++ )
    {
      
      if( ( *i )->delete_layer( layers[j] ) )
        break;
    }   
  }
  this->setUpdatesEnabled( true );
  this->update();
}

void LayerManagerWidget::delete_layer( LayerHandle layer )
{
  this->setUpdatesEnabled( false );

  for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
    i  != this->group_list_.end(); i++ )
  {
    
    if( ( *i )->delete_layer( layer ) )
      break;
  }   

  this->setUpdatesEnabled( true );
  this->update();
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
    if ( group->get_group_id() == ( *i )->get_group_id() ) 
    {
      ( *i )->deleteLater();
      group_list_.erase( i );
      this->set_active_group( LayerManager::Instance()->get_active_group() );
      return;
    }
  }
}


void  LayerManagerWidget::set_active_layer( LayerHandle layer )
{
  //SCI_LOG_DEBUG( "LayerManagerWidget set_active_layer started" );
    
  if( active_layer_ )
    this->active_layer_.toStrongRef()->set_active( false );
  
    //this->set_active_group( layer->get_layer_group() );   
    
    for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
     i  != this->group_list_.end(); i++ )
  {
    //SCI_LOG_DEBUG( "LayerGroupWidget set_active_layer started" );
    LayerWidgetQWeakHandle temp_layer = ( *i )->set_active_layer( layer );
    //SCI_LOG_DEBUG( "LayerGroupWidget set_active_layer ended" );
      if( temp_layer )
      {
      active_layer_ = temp_layer;
      break;
    }
  }
  
  //SCI_LOG_DEBUG( "LayerManagerWidget set_active_layer ended" );
}


  
void LayerManagerWidget::set_active_group( LayerGroupHandle group )
{
  SCI_LOG_DEBUG( "LayerManagerWidget set_active_group started" );

    for ( QList< LayerGroupWidgetQHandle >::iterator i = this->group_list_.begin(); 
     i  != this->group_list_.end(); i++ )
  {
    if ( group->get_group_id() == ( *i )->get_group_id() ) 
    {
        ( *i )->set_active( true );
    }
      else
      {
          ( *i )->set_active( false );
      }
  }
  
  SCI_LOG_DEBUG( "LayerManagerWidget set_active_group ended" );
}



// Keep this around for when we are ready for it
//void LayerManagerWidget::color_button_clicked()
//{
//  QToolButton *color_button = ::qobject_cast< QToolButton* >( sender() );
//
//  for ( LayerList_type::const_iterator i = this->private_->layer_list_.begin(); i
//      != this->private_->layer_list_.end(); i++ )
//  {
//    if ( ( *i )->color_button_01_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(251,255,74);}" ) );
//    }
//    if ( ( *i )->color_button_02_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(248,188,37);}" ) );
//    }
//    if ( ( *i )->color_button_03_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(249,152,28);}" ) );
//    }
//    if ( ( *i )->color_button_04_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(251,78,15);}" ) );
//    }
//    if ( ( *i )->color_button_05_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(252,21,17);}" ) );
//    }
//    if ( ( *i )->color_button_06_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(166,12,73);}" ) );
//    }
//    if ( ( *i )->color_button_07_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(135,0,172);}" ) );
//    }
//    if ( ( *i )->color_button_08_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(66,0,161);}" ) );
//    }
//    if ( ( *i )->color_button_09_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(45,66,250);}" ) );
//    }
//    if ( ( *i )->color_button_10_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(32,146,204);}" ) );
//    }
//    if ( ( *i )->color_button_11_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(100,177,61);}" ) );
//    }
//    if ( ( *i )->color_button_12_ == color_button )
//    {
//      ( *i )->typeBackground_->setStyleSheet( QString::fromUtf8(
//          "QWidget#typeBackground_{background-color: rgb(205,235,66);}" ) );
//    }
//  }
//} // end color_button_clicked





}  // end namespace Seg3D
