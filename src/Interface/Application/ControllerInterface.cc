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

// QT includes
#include <QMenu>
#include <QScrollBar>

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Action/ActionHistory.h>
#include <Core/Interface/Interface.h>

// Applications
#include <Application/UndoBuffer/UndoBuffer.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/Application/ControllerInterface.h>
#include <Interface/Application/ControllerContext.h>
#include <Interface/Application/ControllerActionHistory.h>
#include <Interface/Application/ControllerStateEngine.h>
#include <Interface/Application/ControllerLogHistory.h>
#include <Interface/Application/ControllerUndoBuffer.h>
#include <Interface/Application/ControllerRedoBuffer.h>

// The interface from the designer
#include "ui_ControllerInterface.h"

namespace Seg3D
{

class ControllerInterfacePrivate
{
public:
  // The User Inrterface from the designer
  Ui::F_CONTROLLER ui_;

  // Local classes that are only of interest for this class
  ControllerActionHistory* action_history_model_;
  ControllerStateEngine* state_engine_model_;
  ControllerLogHistory* log_history_model_;
  ControllerUndoBuffer* undo_buffer_model_;
  ControllerRedoBuffer* redo_buffer_model_;

  // Action context for running the command line
  Core::ActionContextHandle context_;
};

ControllerInterface::ControllerInterface( QWidget* parent ) :
  QtUtils::QtCustomDialog( parent ), 
  private_( new ControllerInterfacePrivate )
{
  // Step 1: Setup the private structure and allocate all the needed structures
  private_->ui_.setupUi( this );
  private_->context_ = Core::ActionContextHandle( new ControllerContext( this ) );
  
  // Update the title of the dialog
  std::string title = std::string( "Controller - "  )
    + Core::Application::GetApplicationNameAndVersion();
  this->setWindowTitle( QString::fromStdString( title ) );
  
  // Step 1.5: Remove the help button and set the icon because removing the button can occasionaly
  // cause problems with it
  QIcon icon = windowIcon();
  Qt::WindowFlags flags = windowFlags();
  Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & ( ~helpFlag );
  this->setWindowFlags( flags );
  this->setWindowIcon( icon );

  // These next two are Qt objects and will be deleted when the parent object is
  // deleted
  this->private_->action_history_model_ = new ControllerActionHistory( this );
  this->private_->state_engine_model_ = new ControllerStateEngine( this );
  this->private_->log_history_model_ = new ControllerLogHistory( 1000, this );
  this->private_->undo_buffer_model_ = new ControllerUndoBuffer( this );
  this->private_->redo_buffer_model_ = new ControllerRedoBuffer( this );

  // Step 2: Modify the widget
  qpointer_type controller( this );

  // Step 4: Fix the widget properties
  this->private_->ui_.L_ACTION_STATUS->setText( "" );
  this->private_->ui_.L_ACTION_USAGE->setText( "" );

  // Set up the model view widgets
  this->private_->ui_.TV_ACTION_HISTORY->setModel( private_->action_history_model_ );
  this->private_->ui_.TV_ACTION_HISTORY->setColumnWidth( 0, 600 );
  this->private_->ui_.TV_ACTION_HISTORY->setColumnWidth( 1, 200 );
  this->private_->ui_.TV_ACTION_HISTORY->resizeRowsToContents();

  this->private_->ui_.TV_STATE_ENGINE->setModel(private_->state_engine_model_ );
  this->private_->ui_.TV_STATE_ENGINE->setColumnWidth( 0, 500 );
  this->private_->ui_.TV_STATE_ENGINE->setColumnWidth( 1, 500 );
  this->private_->ui_.TV_STATE_ENGINE->resizeRowsToContents();

  this->private_->ui_.TV_LOG_HISTORY->setModel( private_->log_history_model_ );
  this->private_->ui_.TV_LOG_HISTORY->setColumnWidth( 0, 1000 );
  this->private_->ui_.TV_LOG_HISTORY->resizeRowsToContents();

  this->private_->ui_.TV_UNDOBUFFER->setModel( this->private_->undo_buffer_model_ );
  this->private_->ui_.TV_REDOBUFFER->setModel( this->private_->redo_buffer_model_ );
  this->private_->ui_.TV_REDOBUFFER->setColumnWidth( 0, 600 );
  this->private_->ui_.TV_REDOBUFFER->setColumnWidth( 1, 200 );
  this->private_->ui_.TV_UNDOBUFFER->setColumnWidth( 0, 600 );
  this->private_->ui_.TV_UNDOBUFFER->setColumnWidth( 1, 200 );
  this->private_->ui_.TV_UNDOBUFFER->resizeRowsToContents();
  this->private_->ui_.TV_REDOBUFFER->resizeRowsToContents();
  
  this->private_->ui_.TW_CONTROLLER->setCurrentIndex( 0 );


  // Get the list of actions
  std::vector<std::string> action_list;
  Core::ActionFactory::Instance()->action_list( action_list );

  QMenu* action_menu = new QMenu( this->private_->ui_.TB_ACTION );

  std::vector<std::string>::iterator it = action_list.begin();
  std::vector<std::string>::iterator it_end = action_list.end();
  
  while ( it != it_end )
  {
    QAction* action_item = action_menu->addAction( QString::fromStdString( *it ) );
    QtUtils::QtBridge::Connect( action_item, boost::bind( &ControllerInterface::SetActionType, 
      controller, ( *it ) ) );
    ++it;
  }

  this->private_->ui_.TB_ACTION->setMenu( action_menu );

  // Step 5: Link the ActionHistory/StateEngine/EventLog to this widget and have it update 
  // automatically using the signal/slot system

  this->add_connection( Core::ActionHistory::Instance()->history_changed_signal_.connect( 
    boost::bind( &ControllerInterface::UpdateActionHistory, controller ) ) );

  this->add_connection( Core::StateEngine::Instance()->state_changed_signal_.connect( 
    boost::bind( &ControllerInterface::UpdateStateEngine, controller ) ) );
  
  this->add_connection( Core::Log::Instance()->post_log_signal_.connect( 
    boost::bind( &ControllerInterface::UpdateLogHistory, controller, true, _1, _2 ) ) );

  this->add_connection( UndoBuffer::Instance()->buffer_changed_signal_.connect(
    boost::bind( &ControllerInterface::UpdateUndoBuffer, controller ) ) );

  // Step 6: Qt connections
  // Connect the edit box to the slot that posts the action
  this->connect( this->private_->ui_.LE_EDIT_ACTION, SIGNAL( returnPressed() ), this, SLOT( post_action() ) );
}

ControllerInterface::~ControllerInterface()
{
  this->disconnect_all();
}


void ControllerInterface::post_action()
{
  // Clear usage string
  this->private_->ui_.L_ACTION_USAGE->setText( "" );

  // Post the action string
  std::string action_string = this->private_->ui_.LE_EDIT_ACTION->text().toStdString();
  std::string action_error;
  std::string action_usage;

  Core::ActionHandle action;
  if ( !( Core::ActionFactory::CreateAction( action_string, action, action_error, action_usage ) ) )
  {
    qpointer_type controller( this );
    ControllerInterface::PostActionMessage( controller, action_error );
    ControllerInterface::PostActionUsage( controller, action_usage );
  }
  else
  {
    Core::ActionDispatcher::PostAction( action, private_->context_ );
  }
}

void ControllerInterface::post_action_message( std::string message )
{
  this->private_->ui_.L_ACTION_STATUS->setText( QString::fromStdString( message ) );
}

void ControllerInterface::post_action_usage( std::string usage )
{
  this->private_->ui_.L_ACTION_USAGE->setText( QString::fromStdString( usage ) );
}

void ControllerInterface::UpdateActionHistory( qpointer_type controller )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &ControllerInterface::UpdateActionHistory, controller ) );
    return;
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->private_->action_history_model_->update();
    // Auto scroll to last action in the list
    QScrollBar* scrollbar = controller->private_->ui_.TV_ACTION_HISTORY->verticalScrollBar();
    if ( scrollbar ) scrollbar->setValue( scrollbar->maximum() );
  }
}

void ControllerInterface::UpdateStateEngine( qpointer_type controller )
{ 
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &ControllerInterface::UpdateStateEngine, controller ) );
    return;
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->private_->state_engine_model_->update();
  }
}

void ControllerInterface::UpdateUndoBuffer( qpointer_type controller )
{ 
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( 
      &ControllerInterface::UpdateUndoBuffer, controller ) );
    return;
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->private_->undo_buffer_model_->update();
    controller->private_->redo_buffer_model_->update();
  }
}

void ControllerInterface::UpdateLogHistory( qpointer_type controller, bool relay, int message_type,
    std::string message )
{
  // Ensure that this call gets relayed to the right thread
  if ( relay )
  {
    Core::Interface::PostEvent( boost::bind( 
      &ControllerInterface::UpdateLogHistory, controller, false, message_type, message ) );
    return;
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->private_->log_history_model_->add_log_entry( message_type, message );

    // Auto scroll to last event in the list
    QScrollBar* scrollbar = controller->private_->ui_.TV_LOG_HISTORY->verticalScrollBar();
    if ( scrollbar ) scrollbar->setValue( scrollbar->maximum() );
  }
}

void ControllerInterface::PostActionMessage( qpointer_type controller, std::string message )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( &ControllerInterface::PostActionMessage, 
      controller, message ) );
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->post_action_message( message );
  }
}

void ControllerInterface::PostActionUsage( qpointer_type controller, std::string usage )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( &ControllerInterface::PostActionUsage, 
      controller, usage ) );
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->post_action_usage( usage );
  }
}

void ControllerInterface::SetActionType( qpointer_type controller, std::string action_type )
{
  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->private_->ui_.LE_EDIT_ACTION->setText( QString::fromStdString( action_type ) );
  }
}

} // end namespace Seg3D
