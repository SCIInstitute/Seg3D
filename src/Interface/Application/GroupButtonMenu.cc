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

//Boost Includes
#include <boost/lexical_cast.hpp>

// Qt includes
#include <QMessageBox>
#include <QButtonGroup>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QMimeData>

//Core Includes - for logging
#include <Core/Utils/Log.h>

//QtUtils Includes
#include <QtUtils/Bridge/QtBridge.h>

//Interface Includes
#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/GroupButtonMenu.h>
#include <Interface/Application/DropSpaceWidget.h>
#include <Interface/Application/OverlayWidget.h>
#include <Interface/Application/LayerWidget.h>

//Application Includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionNewMaskLayer.h>

//UI Includes
#include "ui_GroupButtonMenu.h"

namespace Seg3D
{
  
class GroupButtonMenuPrivate
{
public:
  Ui::GroupButtonMenu ui_;
  DropSpaceWidget* layer_slot_;
  OverlayWidget* overlay_;
  QButtonGroup* iso_quality_button_group_;
  std::string group_id_;
  LayerGroupHandle group_;
};
  
GroupButtonMenu::GroupButtonMenu( QWidget* parent, LayerGroupHandle group ) :
  QWidget( parent ),
  private_( new GroupButtonMenuPrivate )
{ 
  this->private_->ui_.setupUi( this );

  this->setProperty(StyleSheet::PALETTE_BACKGROUND_PROPERTY_C, true);
  this->private_->ui_.background_->setStyleSheet(StyleSheet::GROUP_MENUBAR_C);
  
  // Set up the Drag and Drop
  this->setAcceptDrops( true );

  this->private_->ui_.facade_->hide();
  this->private_->group_id_ = group->get_group_id();
  this->private_->group_ = group;

  this->private_->ui_.delete_button_->setEnabled( false );
  this->private_->ui_.duplicate_button_->setEnabled( false );
  
  this->private_->layer_slot_ = new DropSpaceWidget( this );
  this->private_->ui_.verticalLayout->insertWidget( 0, this->private_->layer_slot_ );
  this->private_->layer_slot_->hide();
  
  // Add isosurface quality radio buttons to QButtonGroup so that QtButtonGroupConnector can be
  // used to connect the buttons directly to a state variable.
  this->private_->iso_quality_button_group_ = new QButtonGroup( this );
  this->private_->iso_quality_button_group_->setExclusive( true );
  this->private_->iso_quality_button_group_->addButton( 
    this->private_->ui_.radioButton_1_point_0 );
  this->private_->iso_quality_button_group_->addButton( 
    this->private_->ui_.radioButton_point_5 );
  this->private_->iso_quality_button_group_->addButton( 
    this->private_->ui_.radioButton_point_25 );
  this->private_->iso_quality_button_group_->addButton( 
    this->private_->ui_.radioButton_point_125 );

  connect( this->private_->ui_.delete_button_, SIGNAL( clicked() ),
    this, SIGNAL( delete_pressed() ) );
  connect( this->private_->ui_.duplicate_button_, SIGNAL( clicked() ),
    this, SIGNAL( duplicate_pressed() ) );
  connect( this->private_->ui_.select_all_button_, SIGNAL( toggled( bool ) ),
    this, SIGNAL( delete_select_all_pressed( bool ) ) );
  connect( this->private_->ui_.select_all_for_duplication_button_, SIGNAL( toggled( bool ) ),
    this, SIGNAL( duplicate_select_all_pressed( bool ) ) );
    
  connect( this->private_->ui_.group_delete_button_, SIGNAL( toggled( bool ) ), 
    this, SLOT(  handle_delete_menu_changed( bool ) ) );
  connect( this->private_->ui_.duplicate_layer_button_, SIGNAL( toggled( bool ) ), 
    this, SLOT(  handle_duplicate_menu_changed( bool ) ) );

  //Set the default values for the Group UI and make the connections to the state engine
      // --- GENERAL ---
  QtUtils::QtBridge::Connect( this->private_->ui_.group_iso_button_, 
    group->show_iso_menu_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.group_delete_button_, 
    group->show_delete_menu_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.duplicate_layer_button_, 
    group->show_duplicate_menu_state_ );
    
  QtUtils::QtBridge::Show( this->private_->ui_.iso_quality_, group->show_iso_menu_state_ ); 
  QtUtils::QtBridge::Show( this->private_->ui_.iso_capping_, group->show_iso_menu_state_ ); 
  QtUtils::QtBridge::Show( this->private_->ui_.delete_, group->show_delete_menu_state_ );
  QtUtils::QtBridge::Show( this->private_->ui_.duplicate_layers_,
    group->show_duplicate_menu_state_ );
  
  /// TODO: At some point for none bricked volumes we should allow data to be bricked on the fly
  /// when displaying them, until that time 4096 is the largest texture OpenGL will render\
  /// If data is beyond that disable adding new layers of this resolution
  Core::GridTransform gt =  this->private_->group_->get_grid_transform();
  bool beyond_render_capabilities = ( gt.get_nx() > 4096 ) || ( gt.get_ny() > 4096) || ( gt.get_nz() > 4096);

  if ( beyond_render_capabilities ) {
    this->private_->ui_.group_new_mask_button_->setDisabled( true );
    this->private_->ui_.group_new_mask_button_->setToolTip( "Adding mask is disable as layer is too large" );
  }

  QtUtils::QtBridge::Connect( this->private_->ui_.group_new_mask_button_, 
    boost::bind( &ActionNewMaskLayer::Dispatch, 
    Core::Interface::GetWidgetActionContext(), this->private_->group_id_ ) );

  // --- ISOSURFACE---
  QtUtils::QtBridge::Connect( this->private_->iso_quality_button_group_, 
    group->isosurface_quality_state_ );
  QtUtils::QtBridge::Connect( this->private_->ui_.iso_capping_checkbox_, 
    group->isosurface_capping_enabled_state_ );

  this->private_->ui_.verticalLayout_10->setAlignment( Qt::AlignTop );
  
  this->private_->overlay_ = new OverlayWidget( this ); 
  this->private_->overlay_->hide();
  
}

GroupButtonMenu::~GroupButtonMenu()
{
}

LayerGroupHandle GroupButtonMenu::get_group() const
{
  return this->private_->group_;
}

void GroupButtonMenu::dropEvent( QDropEvent* event )
{
  this->enable_drop_space( false );
  event->setAccepted( true );
    
  std::string layer_name_ = event->mimeData()->text().toStdString();
  
  if ( LayerManager::Instance()->find_layer_by_name( layer_name_ ) )
  {
    LayerWidget* layer_widget = dynamic_cast< LayerWidget* >( event->source() );
    if ( layer_widget )
  {
      layer_widget->set_drop_group( this ); 
      event->setDropAction( Qt::MoveAction );
      return;
  }
  }

  event->setDropAction( Qt::IgnoreAction );
}

void GroupButtonMenu::dragEnterEvent( QDragEnterEvent* event)
{
  if( LayerManager::Instance()->find_layer_by_name( event->mimeData()->text().toStdString() ) )
  {
    this->enable_drop_space( true );
    event->setAccepted( true );
    return;
  }
  event->ignore();
}

void GroupButtonMenu::dragLeaveEvent( QDragLeaveEvent* event )
{
  this->enable_drop_space( false );
  event->setAccepted( true );
}

void GroupButtonMenu::enable_drop_space( bool drop )
{
  if( drop )
  {
    this->prep_for_animation( true );
    this->private_->overlay_->show();
    this->private_->layer_slot_->show();
  }
  else
  {
    this->prep_for_animation( false );
    this->private_->layer_slot_->hide();
    this->private_->overlay_->hide();
  }
} 

  
void GroupButtonMenu::resizeEvent( QResizeEvent *event )
{
  this->private_->overlay_->resize( event->size() );
  event->setAccepted( true );
}
  

void GroupButtonMenu::prep_for_animation( bool move_time )
{
  if( move_time )
  {
    this->private_->ui_.facade_->setMinimumSize( this->private_->ui_.group_tools_->size() );
    this->private_->ui_.facade_->setPixmap( this->private_->ui_.group_tools_->grab() );
    this->private_->ui_.group_tools_->hide();
    this->private_->ui_.facade_->show();
  }
  else
  {
    this->private_->ui_.facade_->hide();
    this->private_->ui_.group_tools_->show();
  }
}
  
void GroupButtonMenu::uncheck_delete_button()
{
  this->private_->ui_.select_all_button_->setChecked( true );
  this->private_->ui_.select_all_button_->setChecked( false );
}

void GroupButtonMenu::uncheck_duplicate_button()
{
  this->private_->ui_.select_all_for_duplication_button_->setChecked( true );
  this->private_->ui_.select_all_for_duplication_button_->setChecked( false );
}

void GroupButtonMenu::set_delete_enabled( bool enabled )
{
  this->private_->ui_.delete_button_->setEnabled( enabled );
}

void GroupButtonMenu::set_duplicate_enabled( bool enabled )
{
  this->private_->ui_.duplicate_button_->setEnabled( enabled );
}

void GroupButtonMenu::uncheck_delete_menu_button()
{
  this->private_->ui_.group_delete_button_->setChecked( false );
}

void GroupButtonMenu::handle_delete_menu_changed( bool visibility_status )
{
  if( !visibility_status ) 
  { 
    this->uncheck_delete_button();
  }
}

void GroupButtonMenu::handle_duplicate_menu_changed( bool visibility_status )
{
  if( !visibility_status ) 
  {
    this->uncheck_duplicate_button();
  }
}

}  //end namespace Seg3D
