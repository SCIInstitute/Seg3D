/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

// For the version numbers
#include "ApplicationConfiguration.h"

// Boost include
#include <boost/lexical_cast.hpp>

#include <QMessageBox>
#include <QDesktopWidget>
#include <QApplication>
#include <QCloseEvent>

// Core includes
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>
#include <Core/State/Actions/ActionSet.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/PreferencesManager/Actions/ActionSavePreferences.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>
#include <Application/ProjectManager/Actions/ActionNewProject.h>
#include <Application/ProjectManager/Actions/ActionLoadProject.h>

// QtUtils includes
#include <QtUtils/Utils/QtApplication.h>
#include <QtUtils/Utils/QtPointer.h>
#include <QtUtils/Bridge/QtBridge.h>

// Resource includes
#include <Resources/QtResources.h>

// Interface includes
#include <Interface/Application/ApplicationInterface.h>
#include <Interface/Application/StyleSheet.h>
#include <Interface/Application/ControllerInterface.h>
#include <Interface/Application/HistoryDockWidget.h>
#include <Interface/Application/LayerIOFunctions.h>
#include <Interface/Application/LayerManagerDockWidget.h>
#include <Interface/Application/RenderingDockWidget.h>
#include <Interface/Application/Menu.h>
#include <Interface/Application/MessageWindow.h>
#include <Interface/Application/PreferencesInterface.h>
#include <Interface/Application/ProjectDockWidget.h>
#include <Interface/Application/ShortcutsInterface.h>
#include <Interface/Application/SplashScreen.h>
#include <Interface/Application/StatusBarWidget.h>
#include <Interface/Application/ToolsDockWidget.h>
#include <Interface/Application/ProvenanceDockWidget.h>
#include <Interface/Application/ViewerInterface.h>
#include <Interface/Application/SaveProjectAsWizard.h>

#ifdef BUILD_WITH_PYTHON
#include <Interface/Application/PythonConsoleWidget.h>
#endif

#include <Interface/Application/ProgressWidget.h>

namespace Seg3D
{

class ApplicationInterfacePrivate {

public:
  // Pointer to the main canvas of the main window
  QPointer< ViewerInterface > viewer_interface_;

  // Pointers to dialog widgets
  QPointer< ControllerInterface > controller_interface_;
  QPointer< PreferencesInterface > preferences_interface_;
  QPointer< MessageWindow > message_widget_;
  QPointer< ShortcutsInterface > keyboard_shortcuts_;
  QPointer< SplashScreen > splash_screen_;
#ifdef BUILD_WITH_PYTHON
  QPointer< PythonConsoleWidget > python_console_;
#endif

  // The dock widgets
  //QPointer< HistoryDockWidget > history_dock_window_;
  QPointer< ProjectDockWidget > project_dock_window_;
  QPointer< ToolsDockWidget > tools_dock_window_;
  QPointer< LayerManagerDockWidget > layer_manager_dock_window_;
  QPointer< RenderingDockWidget > rendering_dock_window_;
  QPointer< ProvenanceDockWidget > provenance_dock_window_;
  QPointer< ProgressWidget > progress_;

  // Pointer to the new project wizard
  static QPointer< ProjectWizard > new_project_wizard_;

  // Application menu, statusbar
  QPointer< Menu > menu_;
  QPointer< StatusBarWidget > status_bar_;

  QPointer< QWidget > focused_item_;

  bool critical_message_active_;

};

ApplicationInterface::ApplicationInterface( std::string file_to_view_on_open ) :
  private_( new ApplicationInterfacePrivate )
{
  // Ensure that resources are available
  // This function ensures that all the images are available
    InitQtResources();

  this->setStyleSheet( StyleSheet::MAIN_STYLE_C );

  // Set the window information and set the version numbers
  this->setWindowTitle( QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) );
  this->setWindowIconText( QString::fromStdString( Core::Application::GetApplicationName() ) );

  // TODO: Do we need this one?
  this->setDocumentMode( false );

  // Tell Qt what size to start up in
  this->resize( 1280, 720 );

  // Tell Qt where to dock the toolbars
  this->setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
  this->setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
  this->setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
  this->setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

  // Define the main window viewer canvas
  this->private_->viewer_interface_ = new ViewerInterface( this );
  this->setCentralWidget( this->private_->viewer_interface_ );

  // Setup the menubar and statusbar
  this->private_->menu_ = new Menu( this );
  this->private_->status_bar_ = new StatusBarWidget( this );

  // Instantiate the peripheral windows
  this->private_->preferences_interface_ = new PreferencesInterface( this );
  this->private_->controller_interface_ = new ControllerInterface( this );
  this->private_->keyboard_shortcuts_ = new ShortcutsInterface( this );
  this->private_->message_widget_ = new MessageWindow( this );
  this->private_->splash_screen_ = new SplashScreen( this );
#ifdef BUILD_WITH_PYTHON
  this->private_->python_console_ = new PythonConsoleWidget( this );
#endif

  this->private_->critical_message_active_ = false;

  // Instantiate the dock widgets
  this->private_->rendering_dock_window_ = new RenderingDockWidget( this );
  this->addDockWidget( Qt::RightDockWidgetArea, this->private_->rendering_dock_window_ );

  this->private_->layer_manager_dock_window_ = new LayerManagerDockWidget( this );
  this->addDockWidget( Qt::RightDockWidgetArea, this->private_->layer_manager_dock_window_ );

  this->private_->project_dock_window_ = new ProjectDockWidget( this );
  this->addDockWidget( Qt::LeftDockWidgetArea, this->private_->project_dock_window_ );

  this->private_->provenance_dock_window_ = new ProvenanceDockWidget( this );
  this->addDockWidget( Qt::LeftDockWidgetArea, this->private_->provenance_dock_window_ );

  this->private_->tools_dock_window_ = new ToolsDockWidget( this );
  this->addDockWidget( Qt::LeftDockWidgetArea, this->private_->tools_dock_window_ );

  // Connect the windows and widgets to their visibility states
  QtUtils::QtBridge::Show( this->private_->rendering_dock_window_,
    InterfaceManager::Instance()->rendering_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->private_->layer_manager_dock_window_,
    InterfaceManager::Instance()->layermanager_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->private_->tools_dock_window_,
    InterfaceManager::Instance()->toolmanager_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->private_->project_dock_window_,
    InterfaceManager::Instance()->project_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->private_->provenance_dock_window_,
    InterfaceManager::Instance()->provenance_dockwidget_visibility_state_ );

//  QtUtils::QtBridge::Show( this->private_->history_dock_window_,
//    InterfaceManager::Instance()->history_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->private_->preferences_interface_,
    InterfaceManager::Instance()->preferences_manager_visibility_state_ );

  this->add_connection( InterfaceManager::Instance()->preferences_manager_visibility_state_->
    value_changed_signal_.connect( boost::bind(
    &ApplicationInterface::HandlePreferencesManagerSave, qpointer_type( this ), _1 ) ) );

  QtUtils::QtBridge::Show( this->private_->controller_interface_,
    InterfaceManager::Instance()->controller_visibility_state_ );

  QtUtils::QtBridge::Show( this->private_->message_widget_,
    InterfaceManager::Instance()->message_window_visibility_state_ );

  QtUtils::QtBridge::Show( this->private_->keyboard_shortcuts_,
    InterfaceManager::Instance()->keyboard_shortcut_visibility_state_ );

#ifdef BUILD_WITH_PYTHON
  QtUtils::QtBridge::Show( this->private_->python_console_,
    InterfaceManager::Instance()->python_console_visibility_state_ );
#endif

  this->add_connection( Core::ActionDispatcher::Instance()->begin_progress_signal_.connect(
    boost::bind( &ApplicationInterface::HandleBeginProgress, qpointer_type( this ), _1 ) ) );

  this->add_connection( Core::ActionDispatcher::Instance()->end_progress_signal_.connect(
    boost::bind( &ApplicationInterface::HandleEndProgress, qpointer_type( this ), _1 ) ) );

  this->add_connection( Core::ActionDispatcher::Instance()->report_progress_signal_.connect(
    boost::bind( &ApplicationInterface::HandleReportProgress, qpointer_type( this ), _1 ) ) );

  this->add_connection( Core::Log::Instance()->post_critical_signal_.connect(
    boost::bind( &ApplicationInterface::HandleCriticalErrorMessage, qpointer_type( this ),
                _1, _2 )));

  // NOTE: Connect state and reflect the current state (needs to be atomic, hence the lock)
  {
    // NOTE: State Engine is locked so the application thread cannot make
    // any changes to it
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    // Connect and update full screen state
    this->add_connection( InterfaceManager::Instance()->full_screen_state_->
      value_changed_signal_.connect( boost::bind( &ApplicationInterface::SetFullScreen,
      qpointer_type( this ), _1, _2 ) ) );

    this->add_connection( ProjectManager::Instance()->get_current_project()->project_name_state_->
      value_changed_signal_.connect( boost::bind( &ApplicationInterface::SetProjectName,
      qpointer_type( this ), _1, _2 ) ) );

    this->add_connection( ProjectManager::Instance()->current_project_changed_signal_.connect(
      boost::bind( &ApplicationInterface::UpdateProjectConnections,
      qpointer_type( this ) ) ) );
  }

  if( PreferencesManager::Instance()->full_screen_on_startup_state_->get() )
  {
    this->set_full_screen( true );
  }
  else
  {
    this->center_seg3d_gui_on_screen( this );
  }

  this->private_->progress_ = new ProgressWidget( this->private_->viewer_interface_->parentWidget() );

  // add signal connection for OS X file assocation open event
  this->add_connection( QtUtils::QtApplication::Instance()->osx_file_open_event_signal_.connect( boost::bind(&ApplicationInterface::handle_osx_file_open_event, this, _1 ) ) );

}

ApplicationInterface::~ApplicationInterface()
{
  this->disconnect_all();
}

void ApplicationInterface::activate_splash_screen()
{
  QtUtils::QtBridge::Show( this->private_->splash_screen_,
     InterfaceManager::Instance()->splash_screen_visibility_state_ );
  this->center_seg3d_gui_on_screen( this->private_->splash_screen_ );
}

void ApplicationInterface::closeEvent( QCloseEvent* event )
{
  // We are going to save the PreferencesManager when we exit
  ActionSavePreferences::Dispatch( Core::Interface::GetWidgetActionContext() );

  if ( ProjectManager::Instance()->get_current_project()->check_project_changed() )
  {

    // Check whether the users wants to save and whether the user wants to quit
    int ret = QMessageBox::warning( this, "Save Session ?",
      "Your current session has not been saved.\n"
      "Do you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );

    if ( ret == QMessageBox::Cancel )
    {
      event->ignore();
      return;
    }

    if ( ret == QMessageBox::Save )
    {
      this->disconnect_all();

      ProjectHandle current_project = ProjectManager::Instance()->get_current_project();
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      if ( current_project->project_files_generated_state_->get() == false ||
        current_project->project_files_accessible_state_->get() == false )
      {
        // We actually need to do a save as. Hence call that function
        SaveProjectAsWizard* save_project_as_wizard_ = new SaveProjectAsWizard( this );
        save_project_as_wizard_->exec();
      }
      else
      {
        ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), "" );
      }
    }
  }
  this->disconnect_all();


  if( this->private_->viewer_interface_ )
  {
    this->private_->viewer_interface_->close();
    this->private_->viewer_interface_->deleteLater();
  }

  if( this->private_->controller_interface_ )
  {
    this->private_->controller_interface_->close();
    this->private_->controller_interface_->deleteLater();
  }

  if( this->private_->preferences_interface_ )
  {
    this->private_->preferences_interface_->close();
    this->private_->preferences_interface_->deleteLater();
  }

  if( this->private_->splash_screen_ )
  {
    this->private_->splash_screen_->close();
    this->private_->splash_screen_->deleteLater();
  }

  if( this->private_->message_widget_ )
  {
    this->private_->message_widget_->close();
    this->private_->message_widget_->deleteLater();
  }

  if( this->private_->keyboard_shortcuts_ )
  {
    this->private_->keyboard_shortcuts_->close();
    this->private_->keyboard_shortcuts_->deleteLater();
  }

#ifdef BUILD_WITH_PYTHON
  if ( this->private_->python_console_ )
  {
    this->private_->python_console_->close();
    this->private_->python_console_->deleteLater();
  }
#endif
//  if( this->private_->history_dock_window_ )
//  {
//    this->private_->history_dock_window_->close();
//    this->private_->history_dock_window_->deleteLater();
//  }

  if( this->private_->project_dock_window_ )
  {
    this->private_->project_dock_window_->close();
    this->private_->project_dock_window_->deleteLater();
  }

  if( this->private_->tools_dock_window_ )
  {
    this->private_->tools_dock_window_->close();
    this->private_->tools_dock_window_->deleteLater();
  }

  if( this->private_->layer_manager_dock_window_ )
  {
    this->private_->layer_manager_dock_window_->close();
    this->private_->layer_manager_dock_window_->deleteLater();
  }

  if ( this->private_->rendering_dock_window_ )
  {
    this->private_->rendering_dock_window_->close();
    this->private_->rendering_dock_window_->deleteLater();
  }

  event->accept();
}

void ApplicationInterface::resizeEvent( QResizeEvent *event )
{
  if( this->private_->progress_ && this->private_->progress_->isVisible() )
  {
    this->private_->progress_->resize( event->size() );
  }

  event->accept();
}


void ApplicationInterface::center_seg3d_gui_on_screen( QWidget *widget )
{
  QRect rect = QApplication::desktop()->availableGeometry();

  widget->move( rect.center() - widget->rect().center() );
}

void ApplicationInterface::set_full_screen( bool full_screen )
{
  if( full_screen ) showFullScreen();
  else
  {
    showNormal();
    this->center_seg3d_gui_on_screen( this );
  }
}

void ApplicationInterface::set_project_name( std::string project_name )
{
  setWindowTitle( QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) +
    " - " + QString::fromStdString( project_name ) );
}

void ApplicationInterface::update_project_connections()
{
  this->add_connection( ProjectManager::Instance()->get_current_project()->project_name_state_->
    value_changed_signal_.connect( boost::bind( &ApplicationInterface::SetProjectName,
    qpointer_type( this ), _1, _2 ) ) );
}

void ApplicationInterface::begin_progress( Core::ActionProgressHandle handle )
{
  this->private_->focused_item_ = QApplication::focusWidget();

  // Disable updates from Qt.
  this->setUpdatesEnabled( false );

  // Disable the menubar while progress bar is shown.
  CORE_LOG_DEBUG( "-- Disabling the menubar --" );
  this->menuBar()->setEnabled( false );

  // Make picture of all the widgets, so things will not change in the background.
  CORE_LOG_DEBUG( "-- Picturizing the Viewer Interface --" );
  this->private_->viewer_interface_->set_pic_mode( true );

  // Put overlays on all floating widgets.
  CORE_LOG_DEBUG( "-- Putting overlays over all the floating dock widgets --" );
  this->private_->layer_manager_dock_window_->set_enabled( false );
  this->private_->rendering_dock_window_->set_enabled( false );
  this->private_->tools_dock_window_->set_enabled( false );
  this->private_->project_dock_window_->set_enabled( false );
//  this->private_->history_dock_window_->set_enabled( false );

  // Draw the progress widget on top of everything.
  CORE_LOG_DEBUG( "-- Start progress widget --" );
  this->private_->progress_->setup_progress_widget( handle );
  this->private_->progress_->resize( this->size() );

  // Allow Qt to send updates to the widgets behind the progress interface.
  this->setUpdatesEnabled( true );
}

void ApplicationInterface::end_progress( Core::ActionProgressHandle /*handle*/ )
{
  this->setUpdatesEnabled( false );

  CORE_LOG_DEBUG( "-- Removing overlays from all the floating dock widgets --" );
  this->private_->layer_manager_dock_window_->set_enabled( true );
  this->private_->rendering_dock_window_->set_enabled( true );
  this->private_->tools_dock_window_->set_enabled( true );
  this->private_->project_dock_window_->set_enabled( true );

  //this->private_->history_dock_window_->set_enabled( true );

  CORE_LOG_DEBUG( "-- Unpicturizing the Viewer Interface --" );
  this->private_->viewer_interface_->set_pic_mode( false );

  CORE_LOG_DEBUG( "-- Enabling the menubar --" );
  this->menuBar()->setEnabled( true );

  CORE_LOG_DEBUG( "-- Finish progress widget --" );
  this->private_->progress_->cleanup_progress_widget();

  this->setUpdatesEnabled( true );

  if( this->private_->focused_item_ ) // if the item still exists, give it focus.
  {
    this->private_->focused_item_->setFocus();
  }
}

void ApplicationInterface::report_progress( Core::ActionProgressHandle handle )
{
  if( this->private_->progress_.data() ) this->private_->progress_->update_progress();
}

void ApplicationInterface::addDockWidget( Qt::DockWidgetArea area, QDockWidget* dock_widget )
{
  // NOTE: This is an overloaded version of the Qt function, that adds the dock widgets in
  // the right area and allows them to be tabified.
  QMainWindow::addDockWidget( area, dock_widget );

  QList< QDockWidget* > object_list = findChildren< QDockWidget* > ();
  QList< QDockWidget* >::iterator it = object_list.begin();
  QList< QDockWidget* >::iterator it_end = object_list.end();
  while ( it != it_end )
  {
    if( ( dock_widget != *it ) && ( dockWidgetArea( *it ) == area ) )
    {
      tabifyDockWidget( *it, dock_widget );
      break;
    }
    ++it;
  }
}

void ApplicationInterface::save_preferences( bool visible )
{
  if( !visible )
  {
    ActionSavePreferences::Dispatch( Core::Interface::GetWidgetActionContext() );
  }
}

void ApplicationInterface::HandlePreferencesManagerSave( qpointer_type qpointer, bool visible )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer,
    boost::bind( &ApplicationInterface::save_preferences, qpointer.data(), visible ) ) );
}

void ApplicationInterface::HandleBeginProgress( qpointer_type qpointer, Core::ActionProgressHandle handle )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer,
    boost::bind( &ApplicationInterface::begin_progress, qpointer.data(), handle ) ) );
}

void ApplicationInterface::HandleEndProgress( qpointer_type qpointer, Core::ActionProgressHandle handle )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer,
    boost::bind( &ApplicationInterface::end_progress, qpointer.data(), handle ) ) );
}

void ApplicationInterface::HandleReportProgress( qpointer_type qpointer, Core::ActionProgressHandle handle )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer,
    boost::bind( &ApplicationInterface::report_progress, qpointer.data(), handle ) ) );
}

void ApplicationInterface::SetFullScreen( qpointer_type qpointer, bool full_screen, Core::ActionSource source )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
      &ApplicationInterface::set_full_screen, qpointer.data(), full_screen ) ) );
}

void ApplicationInterface::SetProjectName( qpointer_type qpointer, std::string project_name, Core::ActionSource source )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &ApplicationInterface::set_project_name, qpointer.data(), project_name ) ) );
}

void ApplicationInterface::UpdateProjectConnections( qpointer_type qpointer )
{
  // We should be on the application thread here
  ProjectHandle project = ProjectManager::Instance()->get_current_project();
  if ( project )
  {
    std::string project_name = project->project_name_state_->get();
    Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
      &ApplicationInterface::set_project_name, qpointer.data(), project_name ) ) );
  }

  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &ApplicationInterface::update_project_connections, qpointer.data() ) ) );
}

void ApplicationInterface::raise_error_messagebox( int msg_type, std::string message )
{

  if( this->private_->critical_message_active_ ) return;

  this->private_->critical_message_active_ = true;

  int return_value = QMessageBox::critical( this, "CRITICAL ERROR!!",
    QString::fromStdString( "CRITICAL ERROR!!\n\n" + message ) );

  if( return_value )
  {
    this->private_->critical_message_active_ = false;
  }

}

void ApplicationInterface::HandleCriticalErrorMessage( qpointer_type qpointer, int msg_type, std::string message )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer,
    boost::bind( &ApplicationInterface::raise_error_messagebox, qpointer.data(), msg_type, message ) ) );
}

void ApplicationInterface::handle_osx_file_open_event (std::string filename)
{
    // must do this to make sure a double-click on a project file doesn't use this executable session
    bool new_session = InterfaceManager::Instance()->splash_screen_visibility_state_->get();
    if ( !this->private_->splash_screen_ || this->private_->splash_screen_->get_user_interacted() )
    {
        new_session = false;
    }
    
    if ( !new_session )
    {
        boost::filesystem::path app_filepath;
        Core::Application::Instance()->get_application_filepath( app_filepath );
        
        std::string command = std::string( "" ) +
        app_filepath.parent_path().parent_path().string() + "/Contents/MacOS/Seg3D2 \"" + filename + "\" &";
        
        system( command.c_str() );
    }
    else
    {
        std::vector<std::string> project_file_extensions = Project::GetProjectFileExtensions();
        
        this->open_initial_project (filename);
    }
}


std::string ApplicationInterface::find_project_file ( std::string path )
{
  std::vector<std::string> project_file_extensions = Project::GetProjectFileExtensions();

  boost::filesystem::path full_path =  boost::filesystem::path(path);

  bool found_s3d_file = false;

  if ( boost::filesystem::is_directory( full_path ) )
  {
    boost::filesystem::directory_iterator dir_end;
    for( boost::filesystem::directory_iterator dir_itr( full_path );
         dir_itr != dir_end; ++dir_itr )
    {
      std::string file = dir_itr->path().filename().string();
      boost::filesystem::path dir_file = full_path / file;
      for ( size_t j = 0; j < project_file_extensions.size(); j++ )
      {
        if ( boost::filesystem::extension( dir_file ) ==project_file_extensions[ j ] )
        {
          full_path = dir_file;
          path = full_path.string();
          found_s3d_file = true;
          break;
        }
      }

      if ( found_s3d_file ) break;
    }
  }
  return path;
}

bool ApplicationInterface::open_initial_project ( std::string filename )
{
  filename = this->find_project_file ( filename );

  if ( filename == "" ) return false;

  std::string extension = boost::filesystem::extension( boost::filesystem::path( filename ) );

  Core::ActionSet::Dispatch( Core::Interface::GetWidgetActionContext(),
                             InterfaceManager::Instance()->splash_screen_visibility_state_, false );

  if ( ( extension != ".s3d" ) && ( extension != ".seg3dproj" ) )
  {
    // No location is set, so no project will be generated on disk for now
    ActionNewProject::Dispatch( Core::Interface::GetWidgetActionContext(),
                                "", "Untitled Project" );
    LayerIOFunctions::ImportFiles( this, filename );
    return true;
  }

  std::vector<std::string> project_file_extensions = Project::GetProjectFileExtensions();
    
  if ( std::find( project_file_extensions.begin(), project_file_extensions.end(), extension ) !=
       project_file_extensions.end() )
  {
    ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), filename );
  }

  std::vector<std::string> project_folder_extensions = Project::GetProjectPathExtensions();
    
  if ( std::find( project_folder_extensions.begin(), project_folder_extensions.end(), extension ) !=
       project_folder_extensions.end() )
  {
    ActionLoadProject::Dispatch( Core::Interface::GetWidgetActionContext(), filename );
  }
  return true;
}



} // end namespace Seg3D
