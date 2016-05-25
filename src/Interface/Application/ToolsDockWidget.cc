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

// STL includes
#include <sstream>
#include <iostream>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Utils/QtPointer.h>
#include <QtUtils/Bridge/QtBridge.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/ToolManager/Actions/ActionCloseTool.h>
#include <Application/ToolManager/Actions/ActionActivateTool.h>

// Interface includes
#include <Interface/Application/ToolsDockWidget.h>

namespace Seg3D
{

ToolsDockWidget::ToolsDockWidget( QWidget *parent ) :
  QtUtils::QtCustomDockWidget( parent )
{
  // Update the title and where this window can be docked
  setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  setWindowTitle( "Tools/Filters" );

  this->setMinimumSize( QSize( 300, 300 ) );
  this->tool_dock_widget_contents_ = new QWidget();
  this->tool_dock_widget_contents_->setObjectName( QString::fromUtf8( "tool_dock_" ) );
  
  // Create a new ToolBoxWidget that encapsulates all the tool widgets
  this->v_layout_ = new QVBoxLayout( this->tool_dock_widget_contents_ );
  this->v_layout_->setSpacing( 0 );
  this->v_layout_->setContentsMargins( 0, 0, 0, 0 );
  this->v_layout_->setObjectName( QString::fromUtf8( "v_layout_" ) );
  this->v_layout_->setAlignment( Qt::AlignTop );
  
  this->disable_tools_widget_ = new QWidget( this->tool_dock_widget_contents_ );
  this->disable_tools_widget_->setObjectName( QString::fromUtf8( "disable_tools_widget_" ) );
  this->disable_tools_widget_->setMinimumSize( QSize( 0, 32 ) );
  this->disable_tools_widget_->setMaximumSize( QSize( 16777215, 32 ) );
  
  this->h_layout_ = new QHBoxLayout( this->disable_tools_widget_ );
  this->h_layout_->setSpacing( 0 );
  this->h_layout_->setContentsMargins( 0, 0, 0, 0 );
  this->h_layout_->setObjectName( QString::fromUtf8( "h_layout_" ) );
  
  this->disable_tools_button_ = new QPushButton( this->disable_tools_widget_ );
  this->disable_tools_button_->setObjectName( QString::fromUtf8( "disable_tools_button_" ) );
  this->disable_tools_button_->setText( QString::fromUtf8( "Disable Tools" ) );
  this->disable_tools_button_->setCheckable( true );
  
  this->h_layout_->addWidget( this->disable_tools_button_ );
  
  this->v_layout_->addWidget( this->disable_tools_widget_ );
  
  this->toolbox_ = new ToolBoxWidget( this );
  this->v_layout_->addWidget( this->toolbox_ );
  
  // This functionality is not required currently so we will hide it
  this->disable_tools_widget_->hide();
  
  setWidget( this->tool_dock_widget_contents_ );
  
  
  QtUtils::QtBridge::Connect( this->disable_tools_button_, ToolManager::Instance()->disable_tools_state_ );
  QtUtils::QtBridge::Enable( this->toolbox_, ToolManager::Instance()->disable_tools_state_, true );
  

  // NOTE: Ensure no changes are made to list while the GUI is hooked up
  // This needs to be done atomically, otherwise we may miss a message
  // opening or closing a new tool as this is not run on the application
  // thread. This ensures that no modifications are made and the application
  // thread is waiting until this is done.
  ToolManager::lock_type lock( ToolManager::Instance()->get_mutex() );

  // Forward the open tool:
  // This function binds a set of functions together: (1) it post a new function
  // message, it will check whether the pointer still exists to the current
  // QObject and then execute the function on the interface thread.

  qpointer_type dock_widget( this );

  add_connection( ToolManager::Instance()->open_tool_signal_.connect( boost::bind(
      &ToolsDockWidget::HandleOpenTool, dock_widget, _1 ) ) );

  add_connection( ToolManager::Instance()->close_tool_signal_.connect( boost::bind(
      &ToolsDockWidget::HandleCloseTool, dock_widget, _1 ) ) );

  add_connection( ToolManager::Instance()->activate_tool_signal_.connect( boost::bind(
      &ToolsDockWidget::HandleActivateTool, dock_widget, _1 ) ) );

  this->add_connection( Core::Application::Instance()->reset_signal_.connect( boost::bind(
    &ToolsDockWidget::HandleReset, dock_widget ) ) );

  ToolManager::tool_list_type tool_list = ToolManager::Instance()->tool_list();
  std::string active_toolid = ToolManager::Instance()->active_toolid();

  ToolManager::tool_list_type::iterator it = tool_list.begin();
  ToolManager::tool_list_type::iterator it_end = tool_list.end();
  while ( it != it_end )
  {
    // Call open_tool for each tool in the list
    open_tool( ( *it ).second );
    ++it;
  }

  ToolManager::tool_list_type::iterator active_it = tool_list.find( active_toolid );

  // Set the active tool
  if( active_it != it_end )
  {
    activate_tool( ( *active_it ).second );
  }
}

ToolsDockWidget::~ToolsDockWidget()
{
  this->disconnect_all();
}

void ToolsDockWidget::open_tool( ToolHandle& tool )
{
  // Step (1) : Find the widget class in the ToolFactory
  ToolInterface *tool_interface;

  // NOTE: The reason that the tool interface is upcasted to the real type, is
  // because the ToolFactory only stores pointers to the base classes
  ToolFactory::Instance()->create_toolinterface( tool->get_name(), tool_interface );
  ToolWidget *widget = dynamic_cast< ToolWidget* > ( tool_interface );

  if( widget == 0 )
  {
    CORE_THROW_LOGICERROR("A ToolInterface cannot be up casted to a ToolWidget pointer");
  }

  // Step (2) : Instantiate the widget and add the tool to the toolbox
  widget->create_widget( this, tool );

  std::string label = tool->tool_name();
  toolbox_->add_tool( widget, QString::fromStdString( label ), 
    boost::bind( &ActionCloseTool::Dispatch, Core::Interface::GetWidgetActionContext(), 
      tool->toolid() ),
      boost::bind( &ActionActivateTool::Dispatch, Core::Interface::GetWidgetActionContext(),
      tool->toolid() ), tool->get_url() );

  // Step (3) : Add the tool to the list
  tool_widget_list_[ tool->toolid() ] = widget;

  // Step (4) : If the dock widget was hidden (somebody closed the window),
  // reopen the window

  if( this->isHidden() )
  {
    this->show();
  }
  this->raise();
}

void ToolsDockWidget::close_tool( ToolHandle& tool )
{
  // Step (1): Find the widget in the list
  tool_widget_list_type::iterator it = tool_widget_list_.find( tool->toolid() );
  if( it == tool_widget_list_.end() )
  {
    CORE_LOG_ERROR(std::string("widget with toolid '")+
      tool->toolid()+"' does not exist");
    return;
  }
  ToolWidget *widget = ( *it ).second;

  // Step (2): Remove this widget from the widget
  toolbox_->remove_tool( toolbox_->index_of( widget ) );

  // Step (3): Remove the widget from the toollist
  tool_widget_list_.erase( tool->toolid() );

  // Step (4): Schedule object to be destroyed by Qt
  //widget->deleteLater();
}

void ToolsDockWidget::activate_tool( ToolHandle& tool )
{
  // Step (1): Find the widget
  tool_widget_list_type::iterator it = tool_widget_list_.find( tool->toolid() );
  if( it == tool_widget_list_.end() )
  {
    CORE_LOG_ERROR(std::string("widget with toolid '")+
      tool->toolid()+"' does not exist");
    return;
  }
  ToolWidget *widget = ( *it ).second;

  // Step (2): Set the active tool if it is not active already
  if( widget != toolbox_->get_active_tool() )
  {
    toolbox_->set_active_tool( widget );
  }
}

void ToolsDockWidget::clear()
{
  tool_widget_list_type::iterator it = this->tool_widget_list_.begin();
  while ( it != this->tool_widget_list_.end() )
  {
    this->toolbox_->remove_tool( this->toolbox_->index_of( ( *it ).second ) );
    ++it;
  }

  this->tool_widget_list_.clear();
}

void ToolsDockWidget::HandleOpenTool( qpointer_type qpointer, ToolHandle tool )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &ToolsDockWidget::HandleOpenTool, qpointer,
        tool ) );
    return;
  }

  if( qpointer.data() ) qpointer->open_tool( tool );
}

void ToolsDockWidget::HandleCloseTool( qpointer_type qpointer, ToolHandle tool )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &ToolsDockWidget::HandleCloseTool,
        qpointer, tool ) );
    return;
  }

  if( qpointer.data() ) qpointer->close_tool( tool );
}

void ToolsDockWidget::HandleActivateTool( qpointer_type qpointer, ToolHandle tool )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &ToolsDockWidget::HandleActivateTool,
        qpointer, tool ) );
    return;
  }

  if( qpointer.data() ) qpointer->activate_tool( tool );
}

void ToolsDockWidget::HandleReset( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &ToolsDockWidget::clear, qpointer.data() ) ) );
}

} // end namespace Seg3D
