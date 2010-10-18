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

// QT includes
#include <QtGui>

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Action/ActionHistory.h>
#include <Core/Interface/Interface.h>

// Interface bridging includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/AppController/AppController.h>
#include <Interface/AppController/AppControllerContext.h>
#include <Interface/AppController/AppControllerActionHistory.h>
#include <Interface/AppController/AppControllerStateEngine.h>
#include <Interface/AppController/AppControllerLogHistory.h>

// The interface from the designer
#include "ui_AppController.h"

namespace Seg3D
{

class AppControllerPrivate
{
public:
  // The User Inrterface from the designer
  Ui::F_CONTROLLER ui_;

  // Local classes that are only of interest for this class
  AppControllerActionHistory* action_history_model_;
  AppControllerStateEngine* state_engine_model_;
  AppControllerLogHistory* log_history_model_;

  // Action context for running the command line
  Core::ActionContextHandle context_;
};

AppController::AppController( QWidget* parent ) :
  QDialog( parent ), 
  private_( new AppControllerPrivate )
{

  // Step 1: Setup the private structure and allocate all the needed structures
  private_->ui_.setupUi( this );
  private_->context_ = Core::ActionContextHandle( new AppControllerContext( this ) );
  
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
  private_->action_history_model_ = new AppControllerActionHistory( this );
  private_->state_engine_model_ = new AppControllerStateEngine( this );
  private_->log_history_model_ = new AppControllerLogHistory( 1000, this );

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

  // Get the list of actions
  std::vector<std::string> action_list;
  Core::ActionFactory::Instance()->action_list( action_list );

  QMenu* action_menu = new QMenu( this->private_->ui_.TB_ACTION );

  std::vector<std::string>::iterator it = action_list.begin();
  std::vector<std::string>::iterator it_end = action_list.end();
  
  while ( it != it_end )
  {
    QAction* action_item = action_menu->addAction( QString::fromStdString( *it ) );
    QtUtils::QtBridge::Connect( action_item, boost::bind( &AppController::SetActionType, 
      controller, ( *it ) ) );
    ++it;
  }

  this->private_->ui_.TB_ACTION->setMenu( action_menu );

  // Step 5: Link the ActionHistory/StateEngine/EventLog to this widget and have it update 
  // automatically using the signal/slot system

  this->add_connection( Core::ActionHistory::Instance()->history_changed_signal_.connect( 
    boost::bind( &AppController::UpdateActionHistory, controller ) ) );

  this->add_connection( Core::StateEngine::Instance()->state_changed_signal_.connect( 
    boost::bind( &AppController::UpdateStateEngine, controller ) ) );
  
  this->add_connection( Core::Log::Instance()->post_log_signal_.connect( 
    boost::bind( &AppController::UpdateLogHistory, controller, true, _1, _2 ) ) );

  // Step 6: Qt connections
  // Connect the edit box to the slot that posts the action
  this->connect( this->private_->ui_.LE_EDIT_ACTION, SIGNAL( returnPressed() ), this, SLOT( post_action() ) );
}

AppController::~AppController()
{
}

void AppController::closeEvent( QCloseEvent* event )
{
  QWidget::closeEvent( event );
  if ( event->isAccepted() )
  {
    this->disconnect_all();
  }
}

void AppController::post_action()
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
    AppController::PostActionMessage( controller, action_error );
    AppController::PostActionUsage( controller, action_usage );
  }
  else
  {
    Core::ActionDispatcher::PostAction( action, private_->context_ );
  }
}

void AppController::post_action_message( std::string message )
{
  this->private_->ui_.L_ACTION_STATUS->setText( QString::fromStdString( message ) );
}

void AppController::post_action_usage( std::string usage )
{
  this->private_->ui_.L_ACTION_USAGE->setText( QString::fromStdString( usage ) );
}

void AppController::UpdateActionHistory( qpointer_type controller )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( &AppController::UpdateActionHistory, controller ) );
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

void AppController::UpdateStateEngine( qpointer_type controller )
{ 
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( &AppController::UpdateStateEngine, controller ) );
    return;
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->private_->state_engine_model_->update();
  }
}


void AppController::UpdateLogHistory( qpointer_type controller, bool relay, int message_type,
    std::string message )
{
  // Ensure that this call gets relayed to the right thread
  if ( relay )
  {
    Core::Interface::PostEvent( boost::bind( &AppController::UpdateLogHistory, controller, false,
        message_type, message ) );
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

void AppController::PostActionMessage( qpointer_type controller, std::string message )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( &AppController::PostActionMessage, controller, message ) );
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->post_action_message( message );
  }
}

void AppController::PostActionUsage( qpointer_type controller, std::string usage )
{
  // Ensure that this call gets relayed to the right thread
  if ( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::PostEvent( boost::bind( &AppController::PostActionUsage, controller, usage ) );
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->post_action_usage( usage );
  }
}

void AppController::SetActionType( qpointer_type controller, std::string action_type )
{
  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if ( controller.data() )
  {
    controller->private_->ui_.LE_EDIT_ACTION->setText( QString::fromStdString( action_type ) );
  }
}

} // end namespace Seg3D

