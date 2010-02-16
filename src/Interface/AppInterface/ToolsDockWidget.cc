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
 
// STL includes
#include <sstream>
#include <iostream>

// Utils includes
#include <Utils/Core/Log.h>

// Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/Interface/Interface.h>

#include <Application/ToolManager/ToolManager.h>
#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/ToolManager/Actions/ActionCloseTool.h>
#include <Application/ToolManager/Actions/ActionActivateTool.h>

// Interface includes
#include <Interface/AppInterface/ToolsDockWidget.h>

namespace Seg3D  {

ToolsDockWidget::ToolsDockWidget(QWidget *parent) :
    QDockWidget(parent)
{
  // Update the title and where this window can be docked
  setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
  setWindowTitle("Tools/Plugins");

  // Create a new ToolBoxWidget that encapsulates all the tool widgets
  toolbox_ = new ToolBoxWidget(this);
  setWidget(toolbox_);

  // NOTE: Ensure no changes are made to list while the GUI is hooked up
  // This needs to be done atomically, otherwise we may miss a message
  // opening or closing a new tool as this is not run on the application
  // thread. This ensures that no modifications are made and the application
  // thread is waiting until this is done. 
  ToolManager::lock_type lock(ToolManager::Instance()->get_mutex());
  
  // Forward the open tool:
  // This function binds a set of functions together: (1) it post a new function
  // message, it will check whether the pointer still exists to the current 
  // QObject and then execute the function on the interface thread.

  qpointer_type dock_widget(this);

  add_connection( ToolManager::Instance()->open_tool_signal_.connect(
        boost::bind(&ToolsDockWidget::HandleOpenTool,dock_widget,_1)));

  add_connection( ToolManager::Instance()->close_tool_signal_.connect(
        boost::bind(&ToolsDockWidget::HandleCloseTool,dock_widget,_1)));

  add_connection( ToolManager::Instance()->activate_tool_signal_.connect(
        boost::bind(&ToolsDockWidget::HandleActivateTool,dock_widget,_1)));

  ToolManager::tool_list_type tool_list = ToolManager::Instance()->tool_list();
  std::string active_toolid = ToolManager::Instance()->active_toolid();

  ToolManager::tool_list_type::iterator it = tool_list.begin();
  ToolManager::tool_list_type::iterator it_end = tool_list.end();
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
}


ToolsDockWidget::~ToolsDockWidget()
{
  disconnect_all();
}


void
ToolsDockWidget::open_tool(ToolHandle& tool)
{
  // Step (1) : Find the widget class in the ToolFactory
  ToolInterface *tool_interface;
  
  // NOTE: The reason that the tool interface is upcasted to the real type, is
  // because the ToolFactory only stores pointers to the base classes
  ToolFactory::Instance()->create_toolinterface(tool->type(),tool_interface);
  ToolWidget *widget = dynamic_cast<ToolWidget*>(tool_interface);
  
  if (widget == 0)
  {
    SCI_THROW_LOGICERROR("A ToolInterface cannot be up casted to a ToolWidget pointer");
  }
  
  // Step (2) : Instantiate the widget and add the tool to the toolbox
  widget->create_widget(this,tool);

  toolbox_->add_tool(widget,QString::fromStdString(tool->menu_name()
    +" "+Utils::to_string(tool->toolid_number())), 
    boost::bind(&ActionCloseTool::Dispatch, tool->toolid()),
    boost::bind(&ActionActivateTool::Dispatch, tool->toolid()),
    tool->url());

  // Step (3) : Add the tool to the list
  tool_widget_list_[tool->toolid()] = widget;
  
  // Step (4) : If the dock widget was hidden (somebody closed the window),
  // reopen the window
  
  if (isHidden()) { show(); }
  raise();
}


void
ToolsDockWidget::close_tool(ToolHandle& tool)
{
  // Step (1): Find the widget in the list
  tool_widget_list_type::iterator it = tool_widget_list_.find(tool->toolid());
  if (it == tool_widget_list_.end()) 
  {
    SCI_LOG_ERROR(std::string("widget with toolid '")+
                  tool->toolid()+"' does not exist");
    return;
  }
  ToolWidget *widget = (*it).second;
  
  // Step (2): Remove this widget from the widget
  toolbox_->remove_tool(toolbox_->index_of(widget));
  
  // Step (3): Remove the widget from the toollist
  tool_widget_list_.erase(tool->toolid());
  
  // Step (4): Schedule object to be destroyed by Qt
  //widget->deleteLater();
}


void
ToolsDockWidget::activate_tool(ToolHandle& tool)
{
  // Step (1): Find the widget
  tool_widget_list_type::iterator it = tool_widget_list_.find(tool->toolid());
  if (it == tool_widget_list_.end()) 
  {
    SCI_LOG_ERROR(std::string("widget with toolid '")+
                  tool->toolid()+"' does not exist");
    return;
  } 
  ToolWidget *widget = (*it).second;
  
  // Step (2): Set the active tool if it is not active already
  if (widget != toolbox_->get_active_tool())
  {
    toolbox_->set_active_tool(widget);  
  }
}


void
ToolsDockWidget::HandleOpenTool(qpointer_type qpointer, ToolHandle tool)
{
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::Instance()->post_event(boost::bind(&ToolsDockWidget::HandleOpenTool,qpointer,tool));
    return;
  }

  if (qpointer.data()) qpointer->open_tool(tool);
}


void
ToolsDockWidget::HandleCloseTool(qpointer_type qpointer, ToolHandle tool)
{
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::Instance()->post_event(boost::bind(&ToolsDockWidget::HandleCloseTool,qpointer,tool));
    return;
  }
  
  if (qpointer.data()) qpointer->close_tool(tool);
}


void
ToolsDockWidget::HandleActivateTool(qpointer_type qpointer, ToolHandle tool)
{
  if (!(Interface::IsInterfaceThread()))
  {
    Interface::Instance()->post_event(boost::bind(&ToolsDockWidget::HandleActivateTool,qpointer,tool));
    return;
  }
  
  if (qpointer.data()) qpointer->activate_tool(tool);
}

} // end namespace
