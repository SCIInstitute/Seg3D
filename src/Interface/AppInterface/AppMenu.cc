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

// Boost includes
#include <boost/filesystem.hpp>

// Qt includes
#include <QMenuBar>
#include <QFileDialog>

// Core includes
#include <Core/State/State.h>
#include <Core/State/Actions/ActionSet.h>

//  Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/InterfaceManager/Actions/ActionShowWindow.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>
#include <Application/ProjectManager/Actions/ActionLoadProject.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/AppInterface/AppMenu.h>
#include <Interface/AppInterface/AppInterface.h>
#include <Interface/AppInterface/ViewerInterface.h>
#include <Interface/AppProjectWizard/AppProjectWizard.h>

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
  qaction = qmenu->addAction( tr( "&New Project" ) );
  qaction->setShortcut( tr( "Ctrl+P" ) );
  qaction->setToolTip( tr( "Start a new project." ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( open_project_wizard() ) );

  qaction = qmenu->addAction( tr( "&Open Project" ) );
  qaction->setShortcut( QKeySequence::Open );
  qaction->setToolTip( tr( "Open an existing project" ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( open_project_from_file() ) );
  
#if defined( _WIN32 ) || defined ( __APPLE__ )  
  qaction = qmenu->addAction( tr( "&Show Project Folder" ) );
  qaction->setShortcut( tr( "Ctrl+ALT+F" ) );
  qaction->setToolTip( tr( "Open the current project folder" ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( open_project_folder() ) );
#endif
  
  qaction = qmenu->addAction( tr( "&Save Project" ) );
  qaction->setShortcut( tr( "Ctrl+S" ) );
  qaction->setToolTip( tr( "Save the current project." ) );
  QtUtils::QtBridge::Connect( qaction, 
    boost::bind( &ActionSaveSession::Dispatch, 
    Core::Interface::GetWidgetActionContext(), false, "" ) );
  
  qmenu->addSeparator();
  
  qaction = qmenu->addAction( tr( "&Quit" ) );
  qaction->setShortcut( tr( "Ctrl+Q" ) );
  qaction->setToolTip( tr( "Open a file." ) );
  connect( qaction, SIGNAL( triggered() ), this->parent(), SLOT( close() ) );

}

void AppMenu::create_edit_menu( QMenu* qmenu )
{
  QAction* qaction;
  qaction = qmenu->addAction( tr( "Select All") );
  qaction->setShortcut( tr( "Ctrl+A" ) );
  qaction->setToolTip( tr( "Select all viewers" ) );
  QtUtils::QtBridge::Connect( qaction, 
    boost::bind( &Core::ActionSet::DispatchState< Core::StateValue<int> >,
      Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->active_viewer_state_, -1 ) );
}

void AppMenu::create_layer_menu( QMenu* qmenu )
{
  QAction* qaction;
  qaction = qmenu->addAction( tr( "Import Layer... ") );
  qaction->setShortcut( tr( "Ctrl+Shift+O" ) );
  qaction->setToolTip( tr( "Import a new layer into the layer manager" ) );
  QtUtils::QtBridge::Connect( qaction, 
    boost::bind( &AppLayerIO::Import,  this->main_window_ ) );

  qaction = qmenu->addAction( tr( "Export Layer...") );
  qaction->setShortcut( tr( "Ctrl+Shift+S" ) );
  qaction->setToolTip( tr( "Export the active layer" ) );
  QtUtils::QtBridge::Connect( qaction, 
    boost::bind( &AppLayerIO::Export, this->main_window_ ) );
}


void AppMenu::create_view_menu( QMenu* qmenu )
{
  QAction* qaction;

  // Full Screen Window
  qaction = qmenu->addAction( "Full Screen" );
  qaction->setShortcut( tr( "Ctrl+F" ) );
  qaction->setToolTip( tr( "Toggle the view between full screen and normal" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, InterfaceManager::Instance()->full_screen_state_ );

  qmenu->addSeparator();

  qaction = qmenu->addAction( tr( "Only One Viewer" ) );
  qaction->setShortcut( tr( "ALT+0" ) );
  qaction->setToolTip( tr( "Set the view to one large view" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::DispatchState< Core::StateOption >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, "single" ) );

  qaction = qmenu->addAction( tr( "One and One" ) );
  qaction->setShortcut( tr( "ALT+1" ) );
  qaction->setToolTip( tr( "Set the view to two large views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, "1and1" ) );

  qaction = qmenu->addAction( tr( "One and Two" ) );
  qaction->setShortcut( tr( "ALT+2" ) );
  qaction->setToolTip( tr( "Set the view one large and two smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, "1and2" ) );

  qaction = qmenu->addAction( tr( "One and Three" ) );
  qaction->setShortcut( tr( "ALT+3" ) );
  qaction->setToolTip( tr( "Set the view one large and three smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, "1and3" ) );

  qaction = qmenu->addAction( tr( "Two and Two" ) );
  qaction->setShortcut( tr( "ALT+4" ) );
  qaction->setToolTip( tr( "Set the view one large and three smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, "2and2" ) );

  qaction = qmenu->addAction( tr( "Two and Three" ) );
  qaction->setShortcut( tr( "ALT+5" ) );
  qaction->setToolTip( tr( "Set the view two larger and three smaller views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, "2and3" ) );

  qaction = qmenu->addAction( tr( "Three and Three" ) );
  qaction->setShortcut( tr( "ALT+6" ) );
  qaction->setToolTip( tr( "Set the view to 6 equally sized views" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind(
      &Core::ActionSet::Dispatch< Core::StateOptionHandle, std::string >,
    Core::Interface::GetWidgetActionContext(),
      ViewerManager::Instance()->layout_state_, "3and3" ) );
}

void AppMenu::create_tool_menu( QMenu* qmenu )
{
  ToolInfoList tool_types_list;

  ToolFactory::Instance()->list_tool_types( tool_types_list, ToolGroupType::TOOL_E );
  ToolInfoList::const_iterator it = tool_types_list.begin();
  ToolInfoList::const_iterator it_end = tool_types_list.end();

  QAction* qaction;
  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it ).menu_name_ ) );
    qaction->setShortcut( QString::fromStdString( ( *it ).shortcut_key_ ) );

    // Connect the action with dispatching a command in the ToolManager
    QtUtils::QtBridge::Connect( qaction, 
      boost::bind( &ActionOpenTool::Dispatch, 
        Core::Interface::GetWidgetActionContext(),
        ( *it ).type_ ) );
    ++it;
  }
}

void AppMenu::create_filter_menu( QMenu* qmenu )
{
  ToolInfoList tool_types_list;
  ToolInfoList::const_iterator it;
  ToolInfoList::const_iterator it_end;

  ToolFactory::Instance()->list_tool_types( tool_types_list, ToolGroupType::DATATODATA_E
      | ToolGroupType::FILTER_E );
    
  it = tool_types_list.begin();
  it_end = tool_types_list.end();
  QAction* qaction;

  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it ).menu_name_ ) );
    qaction->setShortcut( QString::fromStdString( ( *it ).shortcut_key_ ) );

    // Connect the action with dispatching a command in the ToolManager
    QtUtils::QtBridge::Connect( qaction, 
      boost::bind( &ActionOpenTool::Dispatch, 
        Core::Interface::GetWidgetActionContext(), ( *it ).type_ ) );
    ++it;
  }

  qmenu->addSeparator();

  ToolFactory::Instance()->list_tool_types( tool_types_list, ToolGroupType::DATATOMASK_E
      | ToolGroupType::FILTER_E );
    
  it = tool_types_list.begin();
  it_end = tool_types_list.end();
  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it ).menu_name_ ) );
    qaction->setShortcut( QString::fromStdString( ( *it ).shortcut_key_ ) );

    // Connect the action with dispatching a command in the ToolManager
    QtUtils::QtBridge::Connect( qaction, 
      boost::bind( &ActionOpenTool::Dispatch, 
        Core::Interface::GetWidgetActionContext(), ( *it ).type_ ) );
    ++it;
  }

  qmenu->addSeparator();

  ToolFactory::Instance()->list_tool_types( tool_types_list, ToolGroupType::MASKTOMASK_E
      | ToolGroupType::FILTER_E );
  it = tool_types_list.begin();
  it_end = tool_types_list.end();
  while ( it != it_end )
  {
    // Add menu option to open tool
    qaction = qmenu->addAction( QString::fromStdString( ( *it ).menu_name_ ) );
    qaction->setShortcut( QString::fromStdString( ( *it ).shortcut_key_ ) );

    // Connect the action with dispatching a command in the ToolManager
    QtUtils::QtBridge::Connect( qaction, 
      boost::bind( &ActionOpenTool::Dispatch, 
        Core::Interface::GetWidgetActionContext(), ( *it ).type_ ) );
    ++it;
  }
}

void AppMenu::create_window_menu( QMenu* qmenu )
{
  QAction* qaction = 0;

  // Project Window
  qaction = qmenu->addAction( "Project Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+P" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, 
    Core::Interface::GetWidgetActionContext(), std::string( "project" ) ) );

  // History Window
  qaction = qmenu->addAction( "History Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+H" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, 
    Core::Interface::GetWidgetActionContext(), std::string( "history" ) ) );

  //Tools Window
  qaction = qmenu->addAction( "Tools Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+T" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, 
    Core::Interface::GetWidgetActionContext(), std::string( "tools" ) ) );

  // Layer Manager Window
  qaction = qmenu->addAction( "Layer Manager Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+L" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, 
    Core::Interface::GetWidgetActionContext(), std::string( "layermanager" ) ) );

  // Measurement Window
  qaction = qmenu->addAction( "Measurement Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+M" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch, 
    Core::Interface::GetWidgetActionContext(), std::string( "measurement" ) ) );

  qmenu->addSeparator();

  // Controller Window
  qaction = qmenu->addAction( "Controller Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+C" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch,
      Core::Interface::GetWidgetActionContext(), std::string( "controller" ) ) );

  // Preferences Window
  qaction = qmenu->addAction( "Preferences Window" );
  qaction->setShortcut( tr( "Ctrl+ALT+P" ) );
  QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionShowWindow::Dispatch,
    Core::Interface::GetWidgetActionContext(), std::string( "preferences" ) ) );
}
  
void AppMenu::open_project_wizard()
{
  QMessageBox message_box;
  message_box.setText( QString::fromUtf8( "WARNING: You will lose changes to your current" 
    "project that haven't been saved.") );
  message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
  message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  message_box.setDefaultButton( QMessageBox::No );
  if( QMessageBox::Yes == message_box.exec() )
  {
    QPointer< AppProjectWizard > new_project_wizard_ = new AppProjectWizard( this->main_window_);
    new_project_wizard_->show();
  }
}

void AppMenu::open_project_from_file()
{
  QMessageBox message_box;
  message_box.setText( QString::fromUtf8( "WARNING: You will lose changes to your current project that haven't been saved.") );
  message_box.setInformativeText( QString::fromUtf8( "Are you sure you want to do this?" ) );
  message_box.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
  message_box.setDefaultButton( QMessageBox::No );
  if( QMessageBox::Yes == message_box.exec() )
  {
    boost::filesystem::path current_projects_path = complete( 
      boost::filesystem::path( ProjectManager::Instance()->
      current_project_path_state_->get().c_str(), boost::filesystem::native ) );

    boost::filesystem::path full_path = ( QFileDialog::getOpenFileName ( this->main_window_,
      tr( "Open Seg3D Project" ), QString::fromStdString( current_projects_path.string() ), 
      tr( "Seg3D Project File ( *.s3d )" ) ) ).toStdString(); 

    std::string path = full_path.parent_path().string();
    
    if( boost::filesystem::exists( full_path ) )
    { 
      ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );
    }
  }
}

void AppMenu::open_project_folder()
{
  boost::filesystem::path project_path = ProjectManager::Instance()->
    current_project_path_state_->get();

  boost::filesystem::path current_projects_path = complete( 
    boost::filesystem::path( ProjectManager::Instance()->
    current_project_path_state_->get().c_str(), boost::filesystem::native ) );

  project_path = project_path / ProjectManager::Instance()->current_project_->project_name_state_->get();
  QString qstring_path = QString::fromStdString( project_path.string() );

  if( boost::filesystem::exists( project_path ) )
  {
    QProcess process;
    process.setReadChannelMode( QProcess::MergedChannels );
#ifdef _WIN32
    qstring_path = qstring_path.replace( QString( "/" ), QString( "\\" ) );
    process.start( "explorer.exe", QStringList() << qstring_path );
#else
    process.start( "open", QStringList() << qstring_path );
#endif

    if( !process.waitForFinished() )
    {
      CORE_LOG_ERROR( "There was an issue when Seg3d2 tried to open the path: " 
        + process.errorString().toStdString() );
    } 
    return;
  }
  CORE_LOG_ERROR( "There current project path seems to be invalid." );
}




} // end namespace Seg3D
