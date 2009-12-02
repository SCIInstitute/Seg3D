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

#include <QMenuBar.h>

//  Application includes
#include <Application/Tool/ToolManager.h>
#include <Application/Tool/ToolFactory.h>

// Interface includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/AppMenu.h>


namespace Seg3D {


AppMenu::AppMenu(QMainWindow* parent) :
 QObject(parent)
{
  // Obtain the menubar from the main widget
  QMenuBar* menubar = parent->menuBar();

  // menus
  QMenu* file_menu = menubar->addMenu(tr("&File"));
  QMenu* edit_menu = menubar->addMenu(tr("&Edit"));
  QMenu* view_menu = menubar->addMenu("View");
  QMenu* tool_menu = menubar->addMenu("Tools");
  QMenu* filter_menu = menubar->addMenu("Filters");
  QMenu* window_menu = menubar->addMenu("Window");
  
  create_file_menu(file_menu);
  create_edit_menu(edit_menu);
  create_view_menu(view_menu);

  create_tool_menu(tool_menu);
  create_filter_menu(filter_menu);

  create_window_menu(window_menu);
}

AppMenu::~AppMenu()
{
}


void 
AppMenu::create_file_menu(QMenu* qmenu)
{
  QAction* qaction;
  qaction = qmenu->addAction(tr("&Open Project"));
  qaction->setShortcut(QKeySequence::Open);
  qaction->setToolTip(tr("Open a new project"));
  // Need to setup signal/slot part here
  
  qaction = qmenu->addAction(tr("&Quit"));
  qaction->setShortcut(tr("Ctrl+Q"));
  qaction->setToolTip(tr("Open a file."));
  connect(qaction, SIGNAL(triggered()), this->parent(), SLOT(close()));
}

void
AppMenu::create_edit_menu(QMenu* qmenu)
{
}

void
AppMenu::create_view_menu(QMenu* qmenu)
{
}

void 
AppMenu::create_tool_menu(QMenu* qmenu)
{
  ToolFactory::tool_list_type tool_types_list;
  ToolFactory::instance()->list_tool_types(tool_types_list,Tool::TOOL_E);
  
  ToolFactory::tool_list_type::const_iterator it = tool_types_list.begin();
  ToolFactory::tool_list_type::const_iterator it_end = tool_types_list.end();
  
  while(it != it_end)
  {
    // Add menu option to open tool
    QAction* qaction = qmenu->addAction(QString((*it).c_str()));
    // Connect the action with dispatching a command in the ToolManager
    QtBridge::connect(qaction,boost::bind(&ToolManager::dispatch_opentool,
                                          ToolManager::instance(),
                                          (*it)));
    ++it;
  }
}


void 
AppMenu::create_filter_menu(QMenu* qmenu)
{
  ToolFactory::tool_list_type tool_types_list;

  ToolFactory::instance()->list_tool_types(tool_types_list,Tool::DATATODATA_FILTER_E);  
  {
    ToolFactory::tool_list_type::const_iterator it = tool_types_list.begin();
    ToolFactory::tool_list_type::const_iterator it_end = tool_types_list.end();  
    while(it != it_end)
    {
      // Add menu option to open tool
      QAction* qaction = qmenu->addAction(QString((*it).c_str()));
      // Connect the action with dispatching a command in the ToolManager
      QtBridge::connect(qaction,boost::bind(&ToolManager::dispatch_opentool,
                                            ToolManager::instance(),
                                            (*it)));
      ++it;
    }
  }
  
  qmenu->addSeparator();

  ToolFactory::instance()->list_tool_types(tool_types_list,Tool::DATATOMASK_FILTER_E);  
  {
    ToolFactory::tool_list_type::const_iterator it = tool_types_list.begin();
    ToolFactory::tool_list_type::const_iterator it_end = tool_types_list.end();  
    while(it != it_end)
    {
      // Add menu option to open tool
      QAction* qaction = qmenu->addAction(QString((*it).c_str()));
      // Connect the action with dispatching a command in the ToolManager
      QtBridge::connect(qaction,boost::bind(&ToolManager::dispatch_opentool,
                                            ToolManager::instance(),
                                            (*it)));
      ++it;
    }
  }

  qmenu->addSeparator();

  ToolFactory::instance()->list_tool_types(tool_types_list,Tool::MASKTOMASK_FILTER_E);  
  {
    ToolFactory::tool_list_type::const_iterator it = tool_types_list.begin();
    ToolFactory::tool_list_type::const_iterator it_end = tool_types_list.end();  
    while(it != it_end)
    {
      // Add menu option to open tool
      QAction* qaction = qmenu->addAction(QString((*it).c_str()));
      // Connect the action with dispatching a command in the ToolManager
      QtBridge::connect(qaction,boost::bind(&ToolManager::dispatch_opentool,
                                            ToolManager::instance(),
                                            (*it)));
      ++it;
    }
  }
}

void
AppMenu::create_window_menu(QMenu* qmenu)
{
}


} // end namespace
