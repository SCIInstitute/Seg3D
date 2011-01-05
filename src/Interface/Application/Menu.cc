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
#include <QtGui/QMenuBar>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QProcess>

// Core includes
#include <Core/State/State.h>
#include <Core/State/Actions/ActionSet.h>

//  Application includes
#include <Application/Tool/ToolFactory.h>
#include <Application/ToolManager/ToolManager.h>
#include <Application/LayerManager/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ToolManager/Actions/ActionOpenTool.h>
#include <Application/ViewerManager/ViewerManager.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>
#include <Application/ProjectManager/Actions/ActionLoadProject.h>
#include <Application/LayerManager/Actions/ActionActivateNextLayer.h>
#include <Application/LayerManager/Actions/ActionActivatePreviousLayer.h>
#include <Application/Tools/Actions/ActionCopy.h>
#include <Application/Tools/Actions/ActionPaste.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include <Application/UndoBuffer/Actions/ActionRedo.h>
#include <Application/UndoBuffer/Actions/ActionUndo.h>

// QtUtils includes
#include <QtUtils/Utils/QtPointer.h>
#include <QtUtils/Bridge/QtBridge.h>

// Interface includes
#include <Interface/Application/ApplicationInterface.h>
#include <Interface/Application/LayerIOFunctions.h>
#include <Interface/Application/Menu.h>
#include <Interface/Application/ProjectWizard.h>
#include <Interface/Application/SaveProjectAsWizard.h>
#include <Interface/Application/ViewAction.h>
#include <Interface/Application/ViewerInterface.h>


namespace Seg3D
{

Menu::Menu( QMainWindow* parent ) :
  QObject( parent ),
  main_window_(parent)
{
  // Obtain the menubar from the main widget
  QMenuBar* menubar = parent->menuBar();

  // menus
  QMenu* file_menu = menubar->addMenu( tr( "&File" ) );
  this->create_file_menu( file_menu );  
  
  QMenu* edit_menu = menubar->addMenu( tr( "&Edit" ) );
  this->create_edit_menu( edit_menu );
  
  QMenu* view_menu = menubar->addMenu( "&View" );
  this->create_view_menu( view_menu );
  
  this->create_tool_menus( menubar );
  
  QMenu* window_menu = menubar->addMenu( "&Window" );
  this->create_window_menu( window_menu );
  
  QMenu* help_menu = menubar->addMenu( "&Help" );
  this->create_help_menu( help_menu );
  
  // NOTE: Connect state and reflect the current state (needs to be atomic, hence the lock)
  {
    // NOTE: State Engine is locked so the application thread cannot make
    // any changes to it
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    // Connect and update full screen state
    this->add_connection( ProjectManager::Instance()->recent_projects_state_->
      value_changed_signal_.connect( 
      boost::bind( &Menu::SetRecentFileList, qpointer_type( this ), _1, _2 ) ) );
      
    this->add_connection( LayerManager::Instance()->layers_changed_signal_.connect( 
      boost::bind( &Menu::EnableDisableMaskActions, qpointer_type( this ) ) ) );
      
    this->add_connection( LayerManager::Instance()->active_layer_changed_signal_.connect( 
      boost::bind( &Menu::EnableDisableDataLayerActions, qpointer_type( this ) ) ) );
      
  }
}

Menu::~Menu()
{
}

void Menu::create_file_menu( QMenu* qmenu )
{
  QAction* qaction;
  qaction = qmenu->addAction( tr( "&New Project" ) );
  qaction->setShortcut( tr( "Ctrl+N" ) );
  qaction->setToolTip( tr( "Start a new project." ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( new_project_wizard() ) );

  qaction = qmenu->addAction( tr( "&Open Project" ) );
  qaction->setShortcut( QKeySequence::Open );
  qaction->setToolTip( tr( "Open an existing project" ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( open_project_from_file() ) );
  
#if defined( _WIN32 ) || defined ( __APPLE__ )  
  qaction = qmenu->addAction( tr( "Show Project Folder" ) );
  qaction->setShortcut( tr( "Ctrl+Shift+F" ) );
  qaction->setToolTip( tr( "Open the current project folder" ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( open_project_folder() ) );
#endif
  
#if defined( __APPLE__ )  

  std::string menu_label = std::string( "Launch Another Copy Of " ) + 
    Core::Application::GetApplicationName();
  std::string menu_tooltip = std::string( "Open another copy of " ) +
    Core::Application::GetApplicationName();
    
  qaction = qmenu->addAction( QString::fromStdString( menu_label ) );
  qaction->setToolTip( QString::fromStdString( menu_tooltip ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( mac_open_another_version() ) );

#endif
  
  qaction = qmenu->addAction( tr( "Save Project" ) );
  qaction->setShortcut( tr( "Ctrl+S" ) );
  qaction->setToolTip( tr( "Save the current project." ) );
  QtUtils::QtBridge::Connect( qaction, 
    boost::bind( &ActionSaveSession::Dispatch, 
    Core::Interface::GetWidgetActionContext(), false, "" ) );
  
  QtUtils::QtBridge::Enable( qaction, ProjectManager::Instance()->project_saved_state_ );
    
  qaction = qmenu->addAction( tr( "Save Project As..." ) );
  qaction->setShortcut( tr( "Ctrl+Shift+S" ) );
  qaction->setToolTip( tr( "Save the current project as..." ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( save_as_wizard() ) );
    
  qmenu->addSeparator();

  qaction = qmenu->addAction( tr( "Import Layer From Single File ... ") );
  qaction->setShortcut( tr( "Ctrl+Shift+O" ) );
  qaction->setToolTip( tr( "Import new layer(s) into the layer manager from a file(s)." ) );
  QtUtils::QtBridge::Connect( qaction, 
    boost::bind( &LayerIOFunctions::ImportFiles,  this->main_window_, "" ) );

  qaction = qmenu->addAction( tr( "Import Layer From Image Series... ") );
  qaction->setShortcut( tr( "Ctrl+Shift+I" ) );
  qaction->setToolTip( tr( "Import new data layer into the layer manager from a series." ) );
  QtUtils::QtBridge::Connect( qaction, 
    boost::bind( &LayerIOFunctions::ImportSeries,  this->main_window_ ) );

  qmenu->addSeparator();

  this->export_segmentation_qaction_ = qmenu->addAction( tr( "Export Segmentation..." ) );
  this->export_segmentation_qaction_->setShortcut( tr( "Ctrl+E" ) );
  this->export_segmentation_qaction_->setToolTip( tr( "Export masks as a segmentation." ) );
  QtUtils::QtBridge::Connect( this->export_segmentation_qaction_, 
    boost::bind( &LayerIOFunctions::ExportSegmentation, this->main_window_ ) );
  this->export_segmentation_qaction_->setEnabled( false );

  this->export_active_data_layer_qaction_ = qmenu->addAction( tr( "Export Active Data Layer...") );
  this->export_active_data_layer_qaction_->setShortcut( tr( "Ctrl+Shift+E" ) );
  this->export_active_data_layer_qaction_->setToolTip( tr( "Export the active data layer to file." ) );
  QtUtils::QtBridge::Connect( this->export_active_data_layer_qaction_, 
    boost::bind( &LayerIOFunctions::ExportLayer, this->main_window_ ) );
  this->export_active_data_layer_qaction_->setEnabled( false ); 
    
  qmenu->addSeparator();
    
  this->file_menu_recents_ = qmenu->addMenu( tr( "Recent Projects" ) );
  
  qmenu->addSeparator();

  qaction = qmenu->addAction( tr( "Quit" ) );
  qaction->setShortcut( tr( "Ctrl+Q" ) );
  qaction->setToolTip( tr( "Open a file." ) );
  connect( qaction, SIGNAL( triggered() ), this->parent(), SLOT( close() ) );

}

void Menu::create_edit_menu( QMenu* qmenu )
{
  this->undo_action_ = qmenu->addAction( tr( "Undo" ) );
  this->undo_action_->setShortcut( tr( "Ctrl+Z" ) );
  this->undo_action_->setToolTip( tr( "Undo last action that modified the layers" ) );
    QtUtils::QtBridge::Connect( this->undo_action_ , boost::bind(
      &ActionUndo::Dispatch, Core::Interface::GetWidgetActionContext() ) );

  this->redo_action_ = qmenu->addAction( tr( "Redo" ) );
  this->redo_action_->setShortcut( tr( "Shift+Ctrl+Z" ) );
  this->redo_action_->setToolTip( tr( "Redo last action that modified the layers" ) );
  QtUtils::QtBridge::Connect( this->redo_action_ , boost::bind(
      &ActionRedo::Dispatch, Core::Interface::GetWidgetActionContext() ) );
      
  qmenu->addSeparator();    
  
//  QAction* qaction;
//  qaction = qmenu->addAction( tr( "Activate Next Layer" ) );
//  qaction->setShortcut( Qt::Key_Right );
//  qaction->setToolTip( tr( "Change the active layer to the one below the one that is currently active." ) );
//  QtUtils::QtBridge::Connect( qaction , boost::bind(
//    ActionActivateNextLayer::Dispatch, Core::Interface::GetKeyboardActionContext() ) );
//  
//  qaction = qmenu->addAction( tr( "Activate Previous Layer" ) );
//  qaction->setShortcut( Qt::Key_Left );
//  qaction->setToolTip( tr( "Change the active layer to the one above the one that is currently active." ) );
//  QtUtils::QtBridge::Connect( qaction , boost::bind(
//    ActionActivatePreviousLayer::Dispatch, Core::Interface::GetKeyboardActionContext() ) );
//
//  qmenu->addSeparator();

  this->copy_qaction_ = qmenu->addAction( tr( "Copy Mask Slice" ) );
  this->copy_qaction_->setShortcut( tr( "Ctrl+C" ) );
  this->copy_qaction_->setToolTip( tr( "Copy the current mask slice" ) );
  QtUtils::QtBridge::Connect( this->copy_qaction_, boost::bind( &ActionCopy::Dispatch,
    Core::Interface::GetWidgetActionContext() ) );
  this->copy_qaction_->setEnabled( false );

  this->paste_qaction_ = qmenu->addAction( tr( "Paste Mask Slice" ) );
  this->paste_qaction_->setShortcut( tr( "Ctrl+V" ) );
  this->paste_qaction_->setToolTip( tr( "Paste to the current mask slice" ) );
  QtUtils::QtBridge::Connect( this->paste_qaction_, boost::bind( &ActionPaste::Dispatch,
    Core::Interface::GetWidgetActionContext(), false ) );
  this->paste_qaction_->setEnabled( false );

  this->punch_qaction_ = qmenu->addAction( tr( "Punch Through Volume" ) );
  this->punch_qaction_->setShortcut( tr( "Ctrl+P" ) );
  this->punch_qaction_->setToolTip( tr( "Punch the copied mask slice through the volume" ) );
  QtUtils::QtBridge::Connect( this->punch_qaction_, boost::bind( &ActionPaste::Dispatch,
    Core::Interface::GetWidgetActionContext(), true ) );
  this->punch_qaction_->setEnabled( false );

  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
  this->update_undo_tag( UndoBuffer::Instance()->get_undo_tag() );
  this->update_redo_tag( UndoBuffer::Instance()->get_redo_tag() );

  this->add_connection( UndoBuffer::Instance()->update_undo_tag_signal_.connect(
    boost::bind( &Menu::UpdateUndoTag, qpointer_type( this ), _1 ) ) ); 

  this->add_connection( UndoBuffer::Instance()->update_redo_tag_signal_.connect(
    boost::bind( &Menu::UpdateRedoTag, qpointer_type( this ), _1 ) ) ); 

}

void Menu::create_view_menu( QMenu* qmenu )
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

void Menu::create_tool_menus( QMenuBar* qmenubar )
{
  ToolMenuList menu_list;

  ToolFactory::Instance()->list_menus( menu_list );
  ToolMenuList::const_iterator mit = menu_list.begin();
  ToolMenuList::const_iterator mit_end = menu_list.end(); 

  while ( mit != mit_end )
  {
    QMenu* qmenu = qmenubar->addMenu( QString::fromUtf8( "&" ) + QString::fromStdString( *mit ) );

    ToolInfoList tool_types_list;

    ToolFactory::Instance()->list_tools( tool_types_list, *mit );
    ToolInfoList::const_iterator it = tool_types_list.begin();
    ToolInfoList::const_iterator it_end = tool_types_list.end();

    QAction* qaction;
    while ( it != it_end )
    {
      // Add menu option to open tool
      qaction = qmenu->addAction( QString::fromStdString( ( *it )->get_menu_label() ) );
      qaction->setShortcut( QString::fromStdString( ( *it )->get_shortcut_key() ) );

      // Connect the action with dispatching a command in the ToolManager
      QtUtils::QtBridge::Connect( qaction, boost::bind( &ActionOpenTool::Dispatch, 
          Core::Interface::GetWidgetActionContext(), ( *it )->get_name() ) );
      ++it;
    }
    ++mit;
  }
}

void Menu::create_window_menu( QMenu* qmenu )
{
  QAction* qaction = 0;

  // Project Window
  qaction = qmenu->addAction( "Project Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+P" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->project_dockwidget_visibility_state_ );

//  // History Widget
//  qaction = qmenu->addAction( "History Widget" );
//  qaction->setShortcut( tr( "Ctrl+Shift+H" ) );
//  qaction->setCheckable( true );
//  QtUtils::QtBridge::Connect( qaction, 
//    InterfaceManager::Instance()->history_dockwidget_visibility_state_ );

  //Tools Window
  qaction = qmenu->addAction( "Tools Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+T" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->toolmanager_dockwidget_visibility_state_ );

  // Layer Manager Window
  qaction = qmenu->addAction( "Layer Manager Window" );
  qaction->setCheckable( true );
  qaction->setShortcut( tr( "Ctrl+Shift+L" ) );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->layermanager_dockwidget_visibility_state_ );
  
//  boost::bind( &ActionShowWindow::Dispatch, 
//    Core::Interface::GetWidgetActionContext(), std::string( "layermanager" ) ) );

//  // Measurement Window
//  qaction = qmenu->addAction( "Measurement Window" );
//  qaction->setShortcut( tr( "Ctrl+Shift+M" ) );
//  qaction->setCheckable( true );
//  QtUtils::QtBridge::Connect( qaction, 
//    InterfaceManager::Instance()->measurement_project_dockwidget_visibility_state_ );

  qmenu->addSeparator();

  // Controller Window
  qaction = qmenu->addAction( "Controller Window" );
  qaction->setShortcut( tr( "Ctrl+Shift+C" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->controller_visibility_state_ );

  // Preferences Window
  qaction = qmenu->addAction( "Preferences Window" );
  qaction->setShortcut( tr( "Ctrl+," ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->preferences_manager_visibility_state_ );
}
  
void Menu::create_help_menu( QMenu* qmenu )
{
  QAction* qaction = 0;
  qaction = qmenu->addAction( tr( "&About" ) );
  std::string about = std::string( "About " ) + 
    Core::Application::GetApplicationNameAndVersion();
  qaction->setToolTip( QString::fromStdString( about ) );
  connect( qaction, SIGNAL( triggered() ), this, SLOT( about() ) );
  
  qaction = qmenu->addAction( tr( "&Keyboard Shortcuts" ) ); 
  qaction->setToolTip( QString( "List of the keyboard shortcuts or 'hotkeys' for " ) + 
    QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) );
  qaction->setShortcut( tr( "SHIFT+CTRL+ALT+K" ) );
  qaction->setCheckable( true );
  QtUtils::QtBridge::Connect( qaction, 
    InterfaceManager::Instance()->keyboard_shortcut_visibility_state_ );

}
  
void Menu::about()
{
  std::string about = std::string( "About " ) + 
    Core::Application::GetApplicationNameAndVersion();

  QMessageBox::about( this->main_window_, QString::fromStdString( about ), 
    QString( "<h3>" ) + 
    QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) +
    QString( "</h3>" ) +
    QString(
      "<p align=\"justify\">Seg3D is a free volume segmentation and image processing tool"
      " that was created by the NIH NCRR Center for Integrative Biomedical (CIBC) located "
      "at the Scientific Computing and Imaging Instititute (SCI) at the University of Utah"
      " and was developed in collaboration with Numira Biosciences. "
        "Seg3D combines a flexible manual segmentation interface with powerful "
        "image processing and segmentation tools from the Insight "
        "Toolkit.</p>") );
}

void Menu::new_project_wizard()
{
  if ( ProjectManager::Instance()->current_project_ )
  {
    if ( ProjectManager::Instance()->current_project_->check_project_changed() )
    {
      // Check whether the users wants to save and whether the user wants to quit
      int ret = QMessageBox::warning( this->main_window_, "Create a new Project?",
        "Your current session has not been saved.\n"
        "Do you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

      if ( ret == QMessageBox::Save )
      {
        Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
        ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), false, 
          ProjectManager::Instance()->current_project_->current_session_name_state_->get() );   
      }

      if ( ret != QMessageBox::Cancel )
      {
        QPointer< ProjectWizard > new_project_wizard_ = 
          new ProjectWizard( this->main_window_);
        new_project_wizard_->show();
      }
    }
    else
    {
      QPointer< ProjectWizard > new_project_wizard_ = 
        new ProjectWizard( this->main_window_);
      new_project_wizard_->show();
    }
  }
}

void Menu::open_project_from_file()
{
  if ( ProjectManager::Instance()->current_project_ )
  {
    if ( ProjectManager::Instance()->current_project_->check_project_changed() )
    {
      // Check whether the users wants to save and whether the user wants to quit
      int ret = QMessageBox::warning( this->main_window_, "Open a different Project?",
        "Your current session has not been saved.\n"
        "Do you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

      if ( ret == QMessageBox::Save )
      {
        Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
        ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), false, 
          ProjectManager::Instance()->current_project_->current_session_name_state_->get() );   
      }

      if ( ret != QMessageBox::Cancel )
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
          if ( ! ProjectManager::Instance()->check_if_file_is_valid_project( full_path ) )
          {
            QMessageBox::critical( this->main_window_, 
              "Error reading project file",
              "Error reading project file:\n"
              "The project file was saved with newer version of Seg3D" );
            return;
          }

          ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );
        }
      }
    }
    else
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
        if ( ! ProjectManager::Instance()->check_if_file_is_valid_project( full_path ) )
        {
          QMessageBox::critical( this->main_window_, 
            "Error reading project file",
            "Error reading project file:\n"
            "The project file was saved with newer version of Seg3D" );
          return;
        }

        ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );
      }
    }
  }
}

void Menu::open_project_folder()
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

void Menu::set_recent_file_list( std::vector< std::string > recent_projects )
{
  QAction* qaction = 0;
  this->file_menu_recents_->clear();

  for( size_t i = 0; i < recent_projects.size(); ++i )
  {
    if( recent_projects[ i ] != "" )
    {
      QString project_path = QString::fromStdString( ( Core::SplitString( 
        recent_projects[ i ], "|" ) )[ 0 ] );
        
      QString project_name = QString::fromStdString( ( Core::SplitString( 
        recent_projects[ i ], "|" ) )[ 1 ] );
      
      qaction = file_menu_recents_->addAction( project_name );
      qaction->setToolTip( tr( "Load this recent project" ) );
      
      boost::filesystem::path path = boost::filesystem::path( project_path.toStdString() ) /
        boost::filesystem::path( project_name.toStdString() );
      
      QtUtils::QtBridge::Connect( qaction, boost::bind( &Menu::ConfirmRecentFileLoad,
        qpointer_type( this ), path.string() ) );

    }
  }
}
void Menu::ConfirmRecentFileLoad( qpointer_type qpointer, const std::string& path )
{
  if ( ProjectManager::Instance()->current_project_->check_project_changed() )
  {
    int ret = QMessageBox::warning( qpointer->main_window_, "Open a different Project?",
      "Your current session has not been saved.\n"
      "Do you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

    if( ret == QMessageBox::Save )
    {
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), false,  
        ProjectManager::Instance()->current_project_->current_session_name_state_->get() );
    }
    
    if( ret != QMessageBox::Cancel )
    {
      ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );
    }
  }
  else
  {
    ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), path );
  }
}
  
void Menu::SetRecentFileList( qpointer_type qpointer, 
  std::vector< std::string > recent_projects, Core::ActionSource source )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::set_recent_file_list, qpointer.data(), recent_projects ) ) );
}

void Menu::enable_disable_mask_actions()
{
  bool mask_layer_found = false;
  std::vector< LayerHandle > layer_list;
  LayerManager::Instance()->get_layers( layer_list );
  for( size_t i = 0; i < layer_list.size(); ++i )
  {
    if( layer_list[ i ]->get_type() == Core::VolumeType::MASK_E )
    {
      mask_layer_found = true;
    }
  }
  
  this->export_segmentation_qaction_->setEnabled( mask_layer_found );
  
  this->copy_qaction_->setEnabled( mask_layer_found );
  this->paste_qaction_->setEnabled( mask_layer_found );
  this->punch_qaction_->setEnabled( mask_layer_found );
}

void Menu::enable_disable_data_layer_actions()
{
  bool data_layer_found = false;
  std::vector< LayerHandle > layer_list;
  LayerManager::Instance()->get_layers( layer_list );
  for( size_t i = 0; i < layer_list.size(); ++i )
  {
    if( ( layer_list[ i ]->get_type() == Core::VolumeType::DATA_E ) &&
      (  layer_list[ i ] == LayerManager::Instance()->get_active_layer() ) )
    {
      data_layer_found = true;
    }
  }
  this->export_active_data_layer_qaction_->setEnabled( data_layer_found );
}

void Menu::EnableDisableMaskActions( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::enable_disable_mask_actions, qpointer.data() ) ) );
}

void Menu::EnableDisableDataLayerActions( qpointer_type qpointer )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::enable_disable_data_layer_actions, qpointer.data() ) ) );
}

void Menu::UpdateUndoTag( qpointer_type qpointer, std::string tag )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::update_undo_tag, qpointer.data(), tag ) ) );
}

void Menu::update_undo_tag( std::string tag )
{
  if ( tag == "" )
  {
    this->undo_action_->setEnabled( false );
    this->undo_action_->setText( "Undo" );
  }
  else
  {
    this->undo_action_->setEnabled( true );
    QString text = QString( "Undo " ) + QString::fromStdString( tag );
    this->undo_action_->setText( text );    
  }
    
}

void Menu::UpdateRedoTag( qpointer_type qpointer, std::string tag )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &Menu::update_redo_tag, qpointer.data(), tag ) ) );
}

void Menu::update_redo_tag( std::string tag )
{
  if ( tag == "" )
  {
    this->redo_action_->setEnabled( false );
    this->redo_action_->setText( "Redo" );
  }
  else
  {
    this->redo_action_->setEnabled( true );
    QString text = QString( "Redo " ) + QString::fromStdString( tag );
    this->redo_action_->setText( text );    
  }
}   

void Menu::save_as_wizard()
{
  QPointer< SaveProjectAsWizard > save_project_as_wizard_ = 
    new SaveProjectAsWizard( this->main_window_);
  save_project_as_wizard_->show();
}

void Menu::mac_open_another_version()
{
#if defined( __APPLE__ )
  boost::filesystem::path app_filepath;
  Core::Application::Instance()->get_application_filepath( app_filepath );

  std::string command = std::string( "open -n " ) + 
    app_filepath.parent_path().parent_path().string() + " &";
  
  system( command.c_str() );

#endif
}


} // end namespace Seg3D
