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

// Interface includes
#include <Interface/QtInterface/QtBridge.h>

#include <Interface/AppController/AppController.h>
#include <Interface/AppController/AppControllerContext.h>
#include <Interface/AppController/AppControllerActionHistory.h>
#include <Interface/AppController/AppControllerLogHistory.h>

// The interface from the designer
#include "ui_AppController.h"

namespace Seg3D {

class AppControllerPrivate {
  public:
    Ui::F_CONTROLLER ui_;

    // Local classes that are only of interest for this class
    AppControllerActionHistory* action_history_model_;
    AppControllerLogHistory*    log_history_model_;
    AppControllerContextHandle  context_;
};


AppController::AppController(QWidget* parent) : 
  QWidget(parent,Qt::Window),
  private_(new AppControllerPrivate)
{

  // Step 1: Setup the private structure and allocate all the needed structures
  private_->ui_.setupUi(this);
  private_->context_ = AppControllerContextHandle(new AppControllerContext(this)); 
  private_->action_history_model_ = new AppControllerActionHistory(this);
  private_->log_history_model_ = new AppControllerLogHistory(this);

  // Step 2: Modify the widget
  setWindowTitle(QString("Seg3D Controller"));
  QPointer<AppController> controller(this);


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
  
  tv_action_history_->setModel(private_->action_history_model_);
  tv_action_history_->setColumnWidth(0,600);
  tv_action_history_->setColumnWidth(1,200);
  tv_action_history_->resizeRowsToContents();
//  tv_action_history_->resizeColumnsToContents();
  
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
  
  ActionHistory::Instance()->history_changed_signal.connect(boost::bind(
    &AppController::UpdateActionHistory,true,controller));

  Utils::LogHistory::Instance()->history_changed_signal.connect(boost::bind(
    &AppController::UpdateLogHistory,true,controller));

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
  std::string stringaction = le_edit_action_->text().toStdString();
  PostAction(stringaction,ActionContextHandle(private_->context_));
}

void
AppController::post_message(std::string message)
{
  l_action_status_->setText(QString::fromStdString(message));
}

void
AppController::post_usage(std::string usage)
{
  l_action_usage_->setText(QString::fromStdString(usage));
}

void 
AppController::UpdateActionHistory(bool relay,QPointer<AppController> controller)
{
  // Ensure that this call gets relayed to the right thread
  if (relay)
  {
    PostInterface(boost::bind(&AppController::UpdateActionHistory,false,controller));
    return;
  }
  
  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->private_->action_history_model_->updateHistory();
    controller->tv_action_history_->resizeRowsToContents();
    // Auto scroll to last event
    QScrollBar* scrollbar = controller->tv_action_history_->verticalScrollBar();
    if (scrollbar) scrollbar->setValue(scrollbar->maximum());

  }
}


void 
AppController::UpdateLogHistory(bool relay,QPointer<AppController> controller)
{
  // Ensure that this call gets relayed to the right thread
  if (relay)
  {
    PostInterface(boost::bind(&AppController::UpdateLogHistory,false,controller));
    return;
  }
  
  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
//    controller->private_->log_history_model_->updateHistory();
//    controller->tv_log_history_->resizeRowsToContents();

    // Auto scroll to last event
    QScrollBar* scrollbar = controller->tv_log_history_->verticalScrollBar();
    if (scrollbar) scrollbar->setValue(scrollbar->maximum());
  }
}


void
AppController::PostMessage(QPointer<AppController> controller,
                           std::string message)
{
  // Ensure that this call gets relayed to the right thread
  if (!(Interface::IsInterfaceThread()))
  {
    PostInterface(boost::bind(&AppController::PostMessage,controller,message));
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->post_message(message);
  }
}

void
AppController::PostUsage(QPointer<AppController> controller,
                         std::string usage)
{
  // Ensure that this call gets relayed to the right thread
  if (!(Interface::IsInterfaceThread()))
  {
    PostInterface(boost::bind(&AppController::PostUsage,controller,usage));
  }

  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->post_usage(usage);
  }
}

void
AppController::SetActionType(QPointer<AppController> controller,
                             std::string action_type)
{
  // Protect controller pointer, so we do not execute if controller does not
  // exist anymore
  if (controller.data())
  {
    controller->le_edit_action_->setText(QString::fromStdString(action_type));
  }
}



} // end namespace Seg3D

