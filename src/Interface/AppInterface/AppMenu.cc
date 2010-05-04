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

#include <Core/State/State.h>
#include <Core/State/Actions/ActionSet.h>

#include <Application/ViewerManager/ViewerManager.h>

// Interface includes
#include <Interface/QtInterface/QtBridge.h>
#include <Interface/AppInterface/AppMenu.h>
#include <Interface/AppInterface/AppInterface.h>
#include <Interface/AppInterface/ViewerInterface.h>

namespace Seg3D
{

AppMenu::AppMenu( QMainWindow* parent ) :
  QObject( parent ),
  main_window_(parent)
{
  // Obtain the menubar from the main widget
  QMenuBar* menubar = parent->menuBar();

  // menus
  QMenu* file_menu = menubar->addMenu( tr( "&File" ) );
  QMenu* edit_menu = menubar->addMenu( tr( "&Edit" ) );
  QMenu* layer_menu = menubar->addMenu( tr( "&Layer" ) );
  QMenu* view_menu = menubar->addMenu( "View" );
  QMenu* tool_menu = menubar->addMenu( "Tools" );
  QMenu* filter_menu = menubar->addMenu( "Filters" );
  QMenu* window_menu = menubar->addMenu( "Window" );

  create_file_menu( file_menu );
  create_edit_menu( edit_menu );
  create_layer_menu( layer_menu );
  create_view_menu( view_menu );
  create_tool_menu( tool_menu );
  create_filter_menu( filter_menu );
  create_window_menu( window_menu );
}

AppMenu::~AppMenu()
{
}

void AppMenu::create_file_menu( QMenu* qmenu )
{
  QAction* qaction;
  qaction = qmenu->addAction( tr( "&Open Project" ) );
  qaction->setShortcut( QKeySequence::Open );
  qaction->setToolTip( tr( "Open a new project" ) );
  // Need to setup signal/slot part here

  qaction = qmenu->addAction( tr( "&Quit" ) );
  qaction->setShortcut( tr( "Ctrl+Q" ) );
  qaction->setToolTip( tr( "Open a file." ) );
  connect( qaction, SIGNAL( triggered() ), this->parent(), SLOT( close() ) );

}

void AppMenu::create_edit_menu( QMenu* qmenu )
{
}

void AppMenu::create_layer_menu( QMenu* qmenu )
{
  QAction* qaction;
  qaction = qmenu->addAction( tr( "Import Layer... ") );
  qaction->setShortcut( tr( "Ctrl+Shift+O" ) );
  qaction->setToolTip( tr( "Import a new layer into the layer manager" ) );
  QtBridge::Connect( qaction, boost::bind( &AppLayerIO::Import,  this->main_window_ ) );

  qaction = qmenu->addAction( tr( "Export Layer...") );
  qaction->setShortcut( tr( "Ctrl+Shift+S" ) );
  qaction->setToolTip( tr( "Export the active layer" ) );
  QtBridge::Connect( qaction, boost::bind( &AppLayerIO::Export, this->main_window_ ) );
}


void AppMenu::create_view_menu( QMenu* qmenu )
{
  QAction* qaction;

  // Full Screen Window
  qaction = qmenu->addAction( "Full Screen" );
  qaction->setShortcut( tr( "Ctrl+F" ) );
  qaction->setToolTip( tr( "Toggle the view between full screen and normal" ) );
  qaction->setCheckable( true );
  QtBridge::Connect( qaction, InterfaceManager::Instance()->full_screen_state_ );

  qmenu->addSeparator();

  qaction = qmenu->addAction( tr( "Only One Viewer" ) );
  qaction->setShortcut( tr( "ALT+0" ) );
  qaction->setToolTip( tr( "Set the view to one large view" ) );
  QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
      ViewerManager::Instance()->layout_state_, "single" ) );

  qaction = qmenu->addAction( tr( "One and One" ) );
  qaction->setShortcut( tr( "ALT+1" ) );
  qaction->setToolTip( tr( "Set the view to two large views" ) );
  QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
      ViewerManager::Instance()->layout_state_, "1and1" ) );

  qaction = qmenu->addAction( tr( "One and Two" ) );
  qaction->setShortcut( tr( "ALT+2" ) );
  qaction->setToolTip( tr( "Set the view one large and two smaller views" ) );
  QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
      ViewerManager::Instance()->layout_state_, "1and2" ) );

  qaction = qmenu->addAction( tr( "One and Three" ) );
  qaction->setShortcut( tr( "ALT+3" ) );
  qaction->setToolTip( tr( "Set the view one large and three smaller views" ) );
  QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
      ViewerManager::Instance()->layout_state_, "1and3" ) );

  qaction = qmenu->addAction( tr( "Two and Two" ) );
  qaction->setShortcut( tr( "ALT+4" ) );
  qaction->setToolTip( tr( "Set the view one large and three smaller views" ) );
  QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
      ViewerManager::Instance()->layout_state_, "2and2" ) );

  qaction = qmenu->addAction( tr( "Two and Three" ) );
  qaction->setShortcut( tr( "ALT+5" ) );
  qaction->setToolTip( tr( "Set the view two larger and three smaller views" ) );
  QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
      ViewerManager::Instance()->layout_state_, "2and3" ) );

  qaction = qmenu->addAction( tr( "Three and Three" ) );
  qaction->setShortcut( tr( "ALT+6" ) );
  qaction->setToolTip( tr( "Set the view to 6 equally sized views" ) );
  QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
      ViewerManager::Instance()->layout_state_, "3and3" ) );
}

void AppMenu::create_tool_menu( QMenu* qmenu )
{
  ToolFactory::tool_list_type tool_types_list;

  ToolFactory::Instance()->list_tool_types_with_interface( tool_types_list, ToolGroupType::TOOL_E );
  ToolFactory::tool_list_type::const_iterator it = tool_types_list.begin();
  ToolFactory::tool_list_type::const_iterator it_end = tool_types_list.end();

  QAction* qaction;
  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it )->menu_name() ) );
    qaction->setShortcut( QString::fromStdString( ( *it )->shortcut_key() ) );

    // Connect the action with dispatching a command in the ToolManager
    QtBridge::Connect( qaction, boost::bind( &ActionOpenTool::Dispatch, ( *it )->type() ) );
    ++it;
  }
}

void AppMenu::create_filter_menu( QMenu* qmenu )
{
  ToolFactory::tool_list_type tool_types_list;
  ToolFactory::tool_list_type::const_iterator it;
  ToolFactory::tool_list_type::const_iterator it_end;

  ToolFactory::Instance()->list_tool_types_with_interface( tool_types_list, ToolGroupType::DATATODATA_E
      | ToolGroupType::FILTER_E );
  it = tool_types_list.begin();
  it_end = tool_types_list.end();
  QAction* qaction;

  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it )->menu_name() ) );
    qaction->setShortcut( QString::fromStdString( ( *it )->shortcut_key() ) );

    // Connect the action with dispatching a command in the ToolManager
    QtBridge::Connect( qaction, boost::bind( &ActionOpenTool::Dispatch, ( *it )->type() ) );
    ++it;
  }

  qmenu->addSeparator();

  ToolFactory::Instance()->list_tool_types_with_interface( tool_types_list, ToolGroupType::DATATOMASK_E
      | ToolGroupType::FILTER_E );
  it = tool_types_list.begin();
  it_end = tool_types_list.end();
  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it )->menu_name() ) );
    qaction->setShortcut( QString::fromStdString( ( *it )->shortcut_key() ) );

    // Connect the action with dispatching a command in the ToolManager
    QtBridge::Connect( qaction, boost::bind( &ActionOpenTool::Dispatch, ( *it )->type() ) );
    ++it;
  }

  qmenu->addSeparator();

  ToolFactory::Instance()->list_tool_types_with_interface( tool_types_list, ToolGroupType::MASKTOMASK_E
      | ToolGroupType::FILTER_E );
  it = tool_types_list.begin();
  it_end = tool_types_list.end();
  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it )->menu_name() ) );
    qaction->setShortcut( QString::fromStdString( ( *it )->shortcut_key() ) );

    // Connect the action with dispatching a command in the ToolManager
    QtBridge::Connect( qaction, boost::bind( &ActionOpenTool::Dispatch, ( *it )->type() ) );
    ++it;
  }
}

void AppMenu::create_window_menu( QMenu* qmenu )
{
  QAction* qaction = 0;

  // Project Window
  qaction = qmenu->addAction( "Project Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+P" ) );
  QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, std::string( "project" ) ) );

  // History Window
  qaction = qmenu->addAction( "History Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+H" ) );
  QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, std::string( "history" ) ) );

  //Tools Window
  qaction = qmenu->addAction( "Tools Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+T" ) );
  QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, std::string( "tools" ) ) );

  // Layer Manager Window
  qaction = qmenu->addAction( "Layer Manager Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+L" ) );
  QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, std::string(
      "layermanager" ) ) );

  // Measurement Window
  qaction = qmenu->addAction( "Measurement Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+M" ) );
  QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, std::string(
      "measurement" ) ) );

  qmenu->addSeparator();

  // Controller Window
  qaction = qmenu->addAction( "Controller Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+C" ) );
  QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch,
      std::string( "controller" ) ) );

}

} // end namespace Seg3D
