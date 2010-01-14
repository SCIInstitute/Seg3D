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

// Application includes
#include <Application/Tool/ToolManager.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Interface/Interface.h>

#include <Application/Tool/Actions/ActionOpenTool.h>
#include <Application/Tool/Actions/ActionCloseTool.h>
#include <Application/Tool/Actions/ActionActivateTool.h>

// Interface includes
#include <Interface/AppInterface/ToolsDockWidget.h>

namespace Seg3D  {


ToolsDockWidget::ToolsDockWidget(QWidget *parent) :
    QDockWidget(parent)
{
  setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
  setWindowTitle("Tools/Plugins");

  toolbox_ = new ToolBoxWidget(this);
  setWidget(toolbox_);


  // Ensure that the application does not change any of the tools while
  // the user interface is being built
  ToolManager::Instance()->lock_tool_list();
  
  // Forward the open tool:
  // This function binds a set of functions together: (1) it post a new function
  // message, it will check whether the pointer still exists to the current 
  // QObject and then execute the function on the interface thread.

  QPointer<ToolsDockWidget> dock_widget(this);

  // Ensure no changes are made to list while the GUI is hooked up
  // This needs to be done atomically, otherwise we may miss a message
  // opening or closing a new tool
  
  ToolManager::Instance()->lock_tool_list();

  open_tool_connection_ =
    ToolManager::Instance()->open_tool_signal.connect(
        boost::bind(&ToolsDockWidget::HandleOpenTool,dock_widget,_1));

  close_tool_connection_ =
    ToolManager::Instance()->close_tool_signal.connect(
        boost::bind(&ToolsDockWidget::HandleCloseTool,dock_widget,_1));

  activate_tool_connection_ =
    ToolManager::Instance()->activate_tool_signal.connect(
        boost::bind(&ToolsDockWidget::HandleActivateTool,dock_widget,_1));

  
  ToolManager::tool_list_type tool_list = ToolManager::Instance()->tool_list();
  std::string active_toolid = ToolManager::Instance()->active_toolid();

  ToolManager::tool_list_type::iterator it = tool_list.begin();
  ToolManager::tool_list_type::iterator it_end = tool_list.end();

  ToolManager::Instance()->unlock_tool_list();

  while (it != it_end)
  {
    // Call open_tool for each tool in the list
    open_tool((*it).second);
    ++it;
  }
  
  ToolManager::tool_list_type::iterator active_it = tool_list.find(active_toolid);

  // Set the active tool
  if (active_it != it_end)
  {
    activate_tool((*active_it).second);
  } 

  // Connect the signal that the active tool was changed to dispatching
  // an action in the application.
  connect(toolbox_,SIGNAL(currentChanged(int)),this,SLOT(tool_changed(int)));
  
  // Now the tool list is up to date we can release the lock on the ToolManager
  ToolManager::Instance()->unlock_tool_list();
}
  
ToolsDockWidget::~ToolsDockWidget()
{
  open_tool_connection_.disconnect();
  close_tool_connection_.disconnect();
  activate_tool_connection_.disconnect();
}

void
ToolsDockWidget::open_tool(ToolHandle tool)
{
  // Step (1) : find the widget class in the ToolFactory
  ToolInterface *interface;
  
  ToolFactory::Instance()->create_toolinterface(tool->type(),interface);
  ToolWidget *widget = dynamic_cast<ToolWidget*>(interface);
  
  if (widget == 0)
  {
    SCI_THROW_LOGICERROR("A ToolInterface cannot be up casted to a ToolWidget pointer");
  }
  
  // Step (2) : instantiate the widget
  widget->create_widget(this,tool);

  toolbox_->add_tool(widget,QString::fromStdString(tool->menu_name()
    +" "+Utils::to_string(tool->toolid_number())), 
    boost::bind(&ActionCloseTool::Dispatch, tool->toolid()),
    boost::bind(&ActionActivateTool::Dispatch, tool->toolid()));

  tool_widget_list_[tool->toolid()] = widget;
  
  if (isHidden()) show(); 
  raise();
}

void
ToolsDockWidget::close_tool(ToolHandle tool)
{
  // Find the widget
  tool_widget_list_type::iterator it = tool_widget_list_.find(tool->toolid());
  
  if (it == tool_widget_list_.end()) 
  {
    SCI_LOG_ERROR(std::string("widget with toolid '")+tool->toolid()+"' does not exist");
    return;
  } 
  
  ToolWidget *widget = (*it).second;
  
  // Remove this widget from the widget
  toolbox_->remove_tool(toolbox_->index_of(widget));
  
  // Remove the pointer from widget 
  tool_widget_list_.erase(tool->toolid());
  
  // Schedule object to be destroyed by Qt
  widget->deleteLater();
}

void
ToolsDockWidget::activate_tool(ToolHandle tool)
{
  // Find the widget
  tool_widget_list_type::iterator it = tool_widget_list_.find(tool->toolid());
  if (it == tool_widget_list_.end()) 
  {
    SCI_LOG_ERROR(std::string("widget with toolid '")+tool->toolid()+"' does not exist");
    return;
  } 
  ToolWidget *widget = (*it).second;
  
  if (widget != toolbox_->get_active_tool())
  {
    toolbox_->set_active_tool(widget);  
  }
}

void
ToolsDockWidget::tool_changed(int index)
{
  if (index >= 0)
  {
    ToolWidget *widget = static_cast<ToolWidget*>(toolbox_->get_tool_at(index));
    ActionActivateTool::Dispatch(widget->toolid());
  }
}


void
ToolsDockWidget::HandleOpenTool(QPointer<ToolsDockWidget> tools_widget,ToolHandle tool)
{
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::Instance()->post_event(boost::bind(&ToolsDockWidget::HandleOpenTool,tools_widget,tool));
    return;
  }

  if (tools_widget.data()) tools_widget->open_tool(tool);
}

void
ToolsDockWidget::HandleCloseTool(QPointer<ToolsDockWidget> tools_widget,ToolHandle tool)
{
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::Instance()->post_event(boost::bind(&ToolsDockWidget::HandleCloseTool,tools_widget,tool));
    return;
  }
  
  if (tools_widget.data()) tools_widget->close_tool(tool);
}

void
ToolsDockWidget::HandleActivateTool(QPointer<ToolsDockWidget> tools_widget,ToolHandle tool)
{
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::Instance()->post_event(boost::bind(&ToolsDockWidget::HandleActivateTool,tools_widget,tool));
    return;
  }
  
  if (tools_widget.data()) tools_widget->activate_tool(tool);
}

} // end namespace
