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

// For the version numbers
#include "ApplicationConfiguration.h"

// Boost include
#include <boost/lexical_cast.hpp>

#include <QtGui/QMessageBox>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>

// Core includes
#include <Core/Application/Application.h>
#include <Core/Interface/Interface.h>

// Application includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ProjectManager/Actions/ActionSaveSession.h>

// QtUtils includes
#include <QtUtils/Utils/QtPointer.h>
#include <QtUtils/Bridge/QtBridge.h>

// Resource includes
#include <Resources/QtResources.h>

// Interface includes
#include <Interface/AppInterface/AppInterface.h>

namespace Seg3D
{

AppInterface::AppInterface()
{
  // Ensure that resources are available
  InitQtResources();

  // Set the window information and set the version numbers
  this->setWindowTitle( QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) );
  this->setWindowIconText( QString::fromStdString( Core::Application::GetApplicationName() ) );

  // TODO: Do we need this one?
  this->setDocumentMode( true );

  // Tell Qt what size to start up in
  this->resize( 1280, 720 );
  
  // Tell Qt where to dock the toolbars
  this->setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
  this->setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
  this->setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
  this->setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );


  // Define the main window viewer canvas
  this->viewer_interface_ = new ViewerInterface( this );
  this->setCentralWidget( this->viewer_interface_ );
  
  // Setup the menubar and statusbar
  this->application_menu_ = new AppMenu( this );
  this->status_bar_ = new AppStatusBar( this );
  
  // Instantiate the peripheral windows
  this->preferences_interface_ = new AppPreferences( this );
  this->controller_interface_ = new AppController( this );
  this->message_widget_ = new MessageWindow( this );
  this->keyboard_shortcuts_ = new AppShortcuts( this );
  this->splash_interface_ = new AppSplash( this );
  
  // Instantiate the dock widgets
  this->layer_manager_dock_window_ = new LayerManagerDockWidget( this );
  this->addDockWidget( Qt::RightDockWidgetArea, this->layer_manager_dock_window_ );
  
  this->project_dock_window_ = new ProjectDockWidget( this );
  this->addDockWidget( Qt::LeftDockWidgetArea, this->project_dock_window_ );
  
  this->history_dock_window_ = new HistoryDockWidget( this );
  this->addDockWidget( Qt::LeftDockWidgetArea, this->history_dock_window_ );
  
  this->tools_dock_window_ = new ToolsDockWidget( this );
  this->addDockWidget( Qt::LeftDockWidgetArea, this->tools_dock_window_ );
  
  this->measurement_dock_window_ = new MeasurementDockWidget( this );
  this->addDockWidget( Qt::RightDockWidgetArea, this->measurement_dock_window_ );
  
  
  // Connect the windows and widgets to their visibility states
  QtUtils::QtBridge::Show( this->layer_manager_dock_window_, 
    InterfaceManager::Instance()->layermanager_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->tools_dock_window_, 
    InterfaceManager::Instance()->toolmanager_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->project_dock_window_, 
    InterfaceManager::Instance()->project_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->measurement_dock_window_, 
    InterfaceManager::Instance()->measurement_project_dockwidget_visibility_state_ );

  QtUtils::QtBridge::Show( this->history_dock_window_, 
    InterfaceManager::Instance()->history_dockwidget_visibility_state_ );
  
  QtUtils::QtBridge::Show( this->splash_interface_, 
    InterfaceManager::Instance()->splash_screen_visibility_state_ );
  this->center_seg3d_gui_on_screen( this->splash_interface_ );
  
  QtUtils::QtBridge::Show( this->preferences_interface_, 
    InterfaceManager::Instance()->preferences_manager_visibility_state_ );
    
  QtUtils::QtBridge::Show( this->controller_interface_, 
    InterfaceManager::Instance()->controller_visibility_state_ );
    
  QtUtils::QtBridge::Show( this->message_widget_, 
    InterfaceManager::Instance()->message_window_visibility_state_ );
    
  QtUtils::QtBridge::Show( this->keyboard_shortcuts_, 
    InterfaceManager::Instance()->keyboard_shortcut_visibility_state_ );  
    
  
  
  this->add_connection( Core::ActionDispatcher::Instance()->begin_progress_signal_.connect( 
    boost::bind( &AppInterface::HandleBeginProgress, qpointer_type( this ), _1 ) ) );

  this->add_connection( Core::ActionDispatcher::Instance()->end_progress_signal_.connect( 
    boost::bind( &AppInterface::HandleEndProgress, qpointer_type( this ), _1 ) ) );

  this->add_connection( Core::ActionDispatcher::Instance()->report_progress_signal_.connect( 
    boost::bind( &AppInterface::HandleReportProgress, qpointer_type( this ), _1 ) ) );
    
  
  // NOTE: Connect state and reflect the current state (needs to be atomic, hence the lock)
  {
    // NOTE: State Engine is locked so the application thread cannot make
    // any changes to it
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    // Connect and update full screen state
    this->add_connection( InterfaceManager::Instance()->full_screen_state_->
      value_changed_signal_.connect( boost::bind( &AppInterface::SetFullScreen, 
      qpointer_type( this ), _1, _2 ) ) );
      
    this->add_connection( ProjectManager::Instance()->current_project_->project_name_state_->
      value_changed_signal_.connect( boost::bind( &AppInterface::SetProjectName, 
      qpointer_type( this ), _1, _2 ) ) ); 
  }

  if( PreferencesManager::Instance()->full_screen_on_startup_state_->get() )
  {
    this->set_full_screen( true );
  }
  else 
  {
    this->center_seg3d_gui_on_screen( this );
  }
}

  
AppInterface::~AppInterface()
{
  this->disconnect_all();
}

void AppInterface::closeEvent( QCloseEvent* event )
{
  if ( ProjectManager::Instance()->current_project_->is_valid() && 
    ProjectManager::Instance()->current_project_->check_project_changed() )
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
      Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );
      ActionSaveSession::Dispatch( Core::Interface::GetWidgetActionContext(), false, 
        ProjectManager::Instance()->current_project_->current_session_name_state_->get() );   
    }
  }
  
  this->disconnect_all();

  if( this->viewer_interface_ )
  {
    this->viewer_interface_->close();
    this->viewer_interface_->deleteLater();
  }

  if( this->controller_interface_ )
  {
    this->controller_interface_->close();
    this->controller_interface_->deleteLater();
  }
  
  if( this->preferences_interface_ )
  {
    this->preferences_interface_->close();
    this->preferences_interface_->deleteLater();
  }
  
  if( this->splash_interface_ )
  {
    this->splash_interface_->close();
    this->splash_interface_->deleteLater();
  }
  
  if( this->message_widget_ )
  {
    this->message_widget_->close();
    this->message_widget_->deleteLater();
  }
  
  if( this->keyboard_shortcuts_ )
  {
    this->keyboard_shortcuts_->close();
    this->keyboard_shortcuts_->deleteLater();
  }
  
  if( this->history_dock_window_ )
  {
    this->history_dock_window_->close();
    this->history_dock_window_->deleteLater();
  }
  
  if( this->project_dock_window_ )
  {
    this->project_dock_window_->close();
    this->project_dock_window_->deleteLater();
  }
  
  if( this->tools_dock_window_ )
  {
    this->tools_dock_window_->close();
    this->tools_dock_window_->deleteLater();
  }
  
  if( this->layer_manager_dock_window_ )
  {
    this->layer_manager_dock_window_->close();
    this->layer_manager_dock_window_->deleteLater();
  }
  
  if( this->measurement_dock_window_ )
  {
    this->measurement_dock_window_->close();
    this->measurement_dock_window_->deleteLater();
  }
  
  if( this->progress_ )
  {
    this->progress_->close();
    this->progress_->deleteLater();
  }

  event->accept();
}

void AppInterface::center_seg3d_gui_on_screen( QWidget *widget ) 
{
  QRect rect = QApplication::desktop()->availableGeometry();

  widget->move( rect.center() - widget->rect().center() );
}

void AppInterface::set_full_screen( bool full_screen )
{
  if( full_screen ) showFullScreen();
  else
  {
    showNormal();
    this->center_seg3d_gui_on_screen( this );
  }
}

void AppInterface::set_project_name( std::string project_name )
{
  setWindowTitle( QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ) +
    " - " + QString::fromStdString( project_name ) );
}

void AppInterface::begin_progress( Core::ActionProgressHandle handle )
{

  // Step (1): delete any out standing progress messages
  if( this->progress_.data() )
  {
    this->progress_->done( 0 );
    this->progress_->deleteLater(); 
  }
  
  CORE_LOG_DEBUG( "-- Start progress widget --" );
  this->progress_ = new ProgressWidget( handle, this );
  this->progress_->show();
  
  CORE_LOG_DEBUG( "-- Disabling the AppInterface --" );
  this->setEnabled( false );
  if( !this->isEnabled() ) this->progress_->setEnabled( true );
  
  this->center_seg3d_gui_on_screen( this->progress_ );
}

void AppInterface::end_progress( Core::ActionProgressHandle /*handle*/ )
{
  CORE_LOG_DEBUG( "-- Finish progress widget --" );
  CORE_LOG_DEBUG( "-- Enabling the AppInterface --" );
  this->setEnabled( true );

  if( this->progress_.data() ) 
  {
    this->progress_->done( 0 );
    this->progress_->deleteLater(); 
  }
}

void AppInterface::report_progress( Core::ActionProgressHandle handle )
{
  if( this->progress_.data() ) progress_->update_progress();
}

void AppInterface::addDockWidget( Qt::DockWidgetArea area, QDockWidget* dock_widget )
{
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

void AppInterface::HandleBeginProgress( qpointer_type qpointer, Core::ActionProgressHandle handle )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &AppInterface::begin_progress, qpointer.data(), handle ) ) );
}

void AppInterface::HandleEndProgress( qpointer_type qpointer, Core::ActionProgressHandle handle )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &AppInterface::end_progress, qpointer.data(), handle ) ) );
}

void AppInterface::HandleReportProgress( qpointer_type qpointer, Core::ActionProgressHandle handle )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, 
    boost::bind( &AppInterface::report_progress, qpointer.data(), handle ) ) );
}

void AppInterface::SetFullScreen( qpointer_type qpointer, bool full_screen, Core::ActionSource source )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
      &AppInterface::set_full_screen, qpointer.data(), full_screen ) ) );
}

void AppInterface::SetProjectName( qpointer_type qpointer, std::string project_name, Core::ActionSource source )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( qpointer, boost::bind(
    &AppInterface::set_project_name, qpointer.data(), project_name ) ) );
}

} // end namespace Seg3D
