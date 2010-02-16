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

// Application layer includes
#include <Application/Action/Actions.h>
#include <Application/Action/ActionHistory.h>
#include <Application/Interface/Interface.h>

// Interface bridging includes
#include <Interface/QtInterface/QtBridge.h>

// Interface includes
#include <Interface/AppController/AppController.h>
#include <Interface/AppController/AppControllerContext.h>
#include <Interface/AppController/AppControllerActionHistory.h>
#include <Interface/AppController/AppControllerLogHistory.h>

// The interface from the designer
#include "ui_AppController.h"

namespace Seg3D {

class AppControllerPrivate {
  public:
    // The User Inrterface from the designer
    Ui::F_CONTROLLER ui_;

    // Local classes that are only of interest for this class
    AppControllerActionHistory* action_history_model_;
    AppControllerLogHistory*    log_history_model_;

    // Action context for running the command line
    ActionContextHandle         context_;
};


AppController::AppController(QWidget* parent) : 
  QWidget(parent,Qt::Window),
  private_(new AppControllerPrivate)
{

  // Step 1: Setup the private structure and allocate all the needed structures
  private_->ui_.setupUi(this);
  private_->context_ = ActionContextHandle(new AppControllerContext(this)); 

  // These next two are Qt objects and will be deleted when the parent object is
  // deleted
  private_->action_history_model_ = new AppControllerActionHistory(this);
  private_->log_history_model_ = new AppControllerLogHistory(1000,this);

  // Step 2: Modify the widget
  setWindowTitle(QString("Seg3D Controller"));
  qpointer_type controller(this);

  // Step 3: Get short cuts to all the widgets
  tw_controller_   = private_->ui_.TW_CONTROLLER;
  tb_action_       = private_->ui_.TB_ACTION;
  le_edit_action_  = private_->ui_.LE_EDIT_ACTION;
  l_action_status_ = private_->ui_.L_ACTION_STATUS;
  l_action_usage_  = private_->ui_.L_ACTION_USAGE;
  tv_action_history_ = private_->ui_.TV_ACTION_HISTORY;
  tv_log_history_    = private_->ui_.TV_LOG_HISTORY;
     
  // Step 4: Fix the widget properties
  l_action_status_->setText("");
  l_action_usage_->setText("");
  
  // Set up the model view widgets
  tv_action_history_->setModel(private_->action_history_model_);
  tv_action_history_->setColumnWidth(0,600);
  tv_action_history_->setColumnWidth(1,200);
  tv_action_history_->resizeRowsToContents();
  
  tv_log_history_->setModel(private_->log_history_model_);
  tv_log_history_->setColumnWidth(0,1000);
  tv_log_history_->resizeRowsToContents();

  // Get the list of actions
  ActionFactory::action_list_type action_list;
  ActionFactory::Instance()->action_list(action_list);
  
  QMenu* action_menu = new QMenu(tb_action_);
  
  ActionFactory::action_list_type::iterator it =  action_list.begin();
  ActionFactory::action_list_type::iterator it_end =  action_list.end();
  while (it != it_end)
  {
    QAction* action_item = action_menu->addAction(QString::fromStdString(*it));
    QtBridge::connect(action_item,boost::bind(&AppController::SetActionType,controller,(*it)));
    ++it;
  }
  
  tb_action_->setMenu(action_menu);

  // Step 5: Link the ActionHistory to this widget and have it update 
  // automatically
  
  ActionHistory::Instance()->history_changed_signal_.connect(boost::bind(
    &AppController::UpdateActionHistory,controller));

  Utils::Log::Instance()->post_log_signal_.connect(boost::bind(
    &AppController::UpdateLogHistory,controller,true,_1,_2));

  // Step 6: Qt connections
  // Connect the edit box to the slot that posts the action
  connect(le_edit_action_,SIGNAL(returnPressed()),this,SLOT(post_action()));
}

AppController::~AppController()
{
}

void
AppController::post_action()
{
  // Clear usage string
  l_action_usage_->setText("");
  
  // Post the action string
  std::string action_string = le_edit_action_->text().toStdString();
  std::string action_error;
  std::string action_usage;
  
  ActionHandle action;
  if (!(ActionFactory::CreateAction(action_string,action,
        action_error,action_usage)))
  {
    qpointer_type controller(this);
    AppController::PostActionMessage(controller,action_error);
    AppController::PostActionUsage(controller,action_usage);
  }
  else
  {  
    PostAction(action,private_->context_);
  }
}

void
AppController::post_action_message(std::string message)
{
  l_action_status_->setText(QString::fromStdString(message));
}

void
AppController::post_action_usage(std::string usage)
{
  l_action_usage_->setText(QString::fromStdString(usage));
}

void 
AppController::UpdateActionHistory(qpointer_type controller)
{
  // Ensure that this call gets relayed to the right thread
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::PostEvent(boost::bind(
      &AppController::UpdateActionHistory, controller));
    return;
  }
  
  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->private_->action_history_model_->updateHistory();
    // controller->tv_action_history_->resizeRowsToContents();
    // Auto scroll to last event
    QScrollBar* scrollbar = controller->tv_action_history_->verticalScrollBar();
    if (scrollbar) scrollbar->setValue(scrollbar->maximum());
  }
}


void 
AppController::UpdateLogHistory( qpointer_type controller, bool relay,
                                int message_type, std::string message)
{
  // Ensure that this call gets relayed to the right thread
  if (relay)
  {
    Interface::PostEvent(boost::bind( &AppController::UpdateLogHistory,
      controller, false, message_type, message));
    return;
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->private_->log_history_model_->add_log_entry(message_type,message);
    
    // Auto scroll to last event
    QScrollBar* scrollbar = controller->tv_log_history_->verticalScrollBar();
    if (scrollbar) scrollbar->setValue(scrollbar->maximum());
  }
}


void
AppController::PostActionMessage(qpointer_type controller, std::string message)
{
  // Ensure that this call gets relayed to the right thread
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::PostEvent( boost::bind( &AppController::PostActionMessage,
      controller, message));
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->post_action_message(message);
  }
}

void
AppController::PostActionUsage(qpointer_type controller, std::string usage)
{
  // Ensure that this call gets relayed to the right thread
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::PostEvent( boost::bind( &AppController::PostActionUsage,
      controller, usage));
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->post_action_usage(usage);
  }
}

void
AppController::SetActionType(qpointer_type controller, std::string action_type)
{
  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->le_edit_action_->setText(QString::fromStdString(action_type));
  }
}

} // end namespace Seg3D

