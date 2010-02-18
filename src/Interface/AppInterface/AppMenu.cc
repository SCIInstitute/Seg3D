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

#include <QMenuBar>


//  Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/InterfaceManager/Actions/ActionShowWindow.h>

#include <Application/State/State.h>
#include <Application/State/Actions/ActionSet.h>
#include <Application/ViewerManager/ViewerManager.h>

// Interface includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/AppMenu.h>
#include <Interface/AppInterface/AppInterface.h>
#include <Interface/AppInterface/ViewerInterface.h>


namespace Seg3D {


AppMenu::AppMenu(QMainWindow* parent, ViewerInterface* view_ptr) :
 QObject(parent)
{
  // Obtain the menubar from the main widget
  QMenuBar* menubar = parent->menuBar();
  
  viewer_pointer_ = view_ptr;

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
  QAction* qaction;
  
  // Full Screen Window
  qaction = qmenu->addAction( "Full Screen" );
  qaction->setShortcut( tr("Ctrl+F") );
  qaction->setToolTip( tr("Toggle the view between full screen and normal") );
  qaction->setCheckable( true );
  QtBridge::connect( qaction, 
                     InterfaceManager::Instance()->full_screen_state_ );
 
  qmenu->addSeparator();
 
  qaction = qmenu->addAction( tr("Only One Viewer") );
  qaction->setShortcut( tr("ALT+0") );
  qaction->setToolTip( tr("Set the view to one large view") );
  QtBridge::connect( qaction,
    boost::bind( &ActionSet::Dispatch<StateOptionHandle,std::string>,
      ViewerManager::Instance()->layout_state_, "single"));

  qaction = qmenu->addAction( tr("One and One") );
  qaction->setShortcut( tr("ALT+1") );
  qaction->setToolTip( tr("Set the view to two large views") );
  QtBridge::connect( qaction,
    boost::bind( &ActionSet::Dispatch<StateOptionHandle,std::string>,
      ViewerManager::Instance()->layout_state_, "1and1"));
  
  qaction = qmenu->addAction( tr("One and Two") );
  qaction->setShortcut( tr("ALT+2") );
  qaction->setToolTip( tr("Set the view one large and two smaller views") );
  QtBridge::connect( qaction,
    boost::bind( &ActionSet::Dispatch<StateOptionHandle,std::string>,
      ViewerManager::Instance()->layout_state_, "1and2"));

  qaction = qmenu->addAction( tr("One and Three") );
  qaction->setShortcut( tr("ALT+3") );
  qaction->setToolTip( tr("Set the view one large and three smaller views") );
  QtBridge::connect( qaction,
    boost::bind( &ActionSet::Dispatch<StateOptionHandle,std::string>,
      ViewerManager::Instance()->layout_state_, "1and3"));
  
  qaction = qmenu->addAction( tr("Two and Two") );
  qaction->setShortcut( tr("ALT+4") );
  qaction->setToolTip( tr("Set the view one large and three smaller views") );
  QtBridge::connect( qaction,
    boost::bind( &ActionSet::Dispatch<StateOptionHandle,std::string>,
      ViewerManager::Instance()->layout_state_, "2and2"));
  
  qaction = qmenu->addAction( tr("Two and Three") );
  qaction->setShortcut( tr("ALT+5") );
  qaction->setToolTip( tr("Set the view two larger and three smaller views") );
  QtBridge::connect( qaction,
    boost::bind( &ActionSet::Dispatch<StateOptionHandle,std::string>,
      ViewerManager::Instance()->layout_state_, "2and3"));
  
  qaction = qmenu->addAction( tr("Three and Three") );
  qaction->setShortcut( tr("ALT+6") );
  qaction->setToolTip( tr("Set the view to 6 equally sized views") );
  QtBridge::connect( qaction,
    boost::bind( &ActionSet::Dispatch<StateOptionHandle,std::string>,
      ViewerManager::Instance()->layout_state_, "3and3"));
}
 

void 
AppMenu::create_tool_menu(QMenu* qmenu)
{
  ToolFactory::tool_list_type tool_types_list;
  
  ToolFactory::Instance()->list_tool_types_with_interface(tool_types_list,
                                                                Tool::TOOL_E);  
  ToolFactory::tool_list_type::const_iterator it = tool_types_list.begin();
  ToolFactory::tool_list_type::const_iterator it_end = tool_types_list.end();
  
  QAction* qaction;
  while(it != it_end)
  {
    // Add menu option to open tool
    qaction = qmenu->addAction(QString::fromStdString((*it)->menu_name()));
    qaction->setShortcut(QString::fromStdString((*it)->shortcut_key()));
    
    // Connect the action with dispatching a command in the ToolManager
    QtBridge::connect(qaction,
                      boost::bind(&ActionOpenTool::Dispatch,(*it)->type()));
    ++it;
  }
}

void 
AppMenu::create_filter_menu(QMenu* qmenu)
{
  ToolFactory::tool_list_type tool_types_list;
  ToolFactory::tool_list_type::const_iterator it;
  ToolFactory::tool_list_type::const_iterator it_end;
  
  
  ToolFactory::Instance()->list_tool_types_with_interface(tool_types_list,
                                            Tool::DATATODATA_E|Tool::FILTER_E);  
  it = tool_types_list.begin();
  it_end = tool_types_list.end();
  QAction* qaction;
  
  while(it != it_end)
  {
    // Add menu option to open tool
    qaction = qmenu->addAction(QString::fromStdString((*it)->menu_name()));
    // Connect the action with dispatching a command in the ToolManager
    QtBridge::connect(qaction,
                      boost::bind(&ActionOpenTool::Dispatch,(*it)->type()));
    ++it;
  }
  
  qmenu->addSeparator();

  ToolFactory::Instance()->list_tool_types_with_interface(tool_types_list,
                                            Tool::DATATOMASK_E|Tool::FILTER_E);  
  it = tool_types_list.begin();
  it_end = tool_types_list.end();  
  while(it != it_end)
  {
    // Add menu option to open tool
    qaction = qmenu->addAction(QString::fromStdString((*it)->menu_name()));
    // Connect the action with dispatching a command in the ToolManager
    QtBridge::connect(qaction,
                      boost::bind(&ActionOpenTool::Dispatch,(*it)->type()));
    ++it;
  }

  qmenu->addSeparator();

  ToolFactory::Instance()->list_tool_types_with_interface(tool_types_list,
                                            Tool::MASKTOMASK_E|Tool::FILTER_E);  
  it = tool_types_list.begin();
  it_end = tool_types_list.end();  
  while(it != it_end)
  {
    // Add menu option to open tool
    qaction = qmenu->addAction(QString::fromStdString((*it)->menu_name()));
    // Connect the action with dispatching a command in the ToolManager
    QtBridge::connect(qaction,
                      boost::bind(&ActionOpenTool::Dispatch,(*it)->type()));
    ++it;
  }
}

void
AppMenu::create_window_menu(QMenu* qmenu)
{
  QAction* qaction = 0;

  // Project Window
  qaction = qmenu->addAction("Project Window");
  QtBridge::connect(qaction,
    boost::bind(&ActionShowWindow::Dispatch,std::string("project")));

  // History Window
  qaction = qmenu->addAction("History Window");
  QtBridge::connect(qaction,
    boost::bind(&ActionShowWindow::Dispatch,std::string("history")));

  //Tools Window
  qaction = qmenu->addAction("Tools Window");
  QtBridge::connect(qaction,
    boost::bind(&ActionShowWindow::Dispatch,std::string("tools")));

  // Layer Manager Window
  qaction = qmenu->addAction("Layer Manager Window");
  QtBridge::connect(qaction,
    boost::bind(&ActionShowWindow::Dispatch,std::string("layermanager")));

  // Measurement Window
  qaction = qmenu->addAction("Measurement Window");
  QtBridge::connect(qaction,
    boost::bind(&ActionShowWindow::Dispatch,std::string("measurement")));

  qmenu->addSeparator();

  // Controller Window
  qaction = qmenu->addAction("Controller Window");
  QtBridge::connect(qaction,
    boost::bind(&ActionShowWindow::Dispatch,std::string("controller")));  

}


} // end namespace
