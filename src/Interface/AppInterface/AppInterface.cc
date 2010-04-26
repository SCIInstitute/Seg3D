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
#include "Seg3DConfiguration.h"

// Boost include
#include <boost/lexical_cast.hpp>

// Application includes
#include <Application/Application/Application.h>
#include <Application/Interface/Interface.h>

// Interface includes
#include <Interface/QtInterface/QtApplication.h>
#include <Interface/QtInterface/QtPointer.h>
#include <Interface/AppInterface/AppInterface.h>
#include <Interface/AppInterface/SplashStart.h>

namespace Seg3D
{

AppInterface::AppInterface()
{
  // Set the window information and set the version numbers
  setWindowTitle( QString( "Seg3D Version " ) + SEG3D_VERSION + QString( " " ) + SEG3D_BITS
      + QString( " " ) + SEG3D_DEBUG_VERSION );
  setWindowIconText( QString( "Seg3D" ) );

  // TODO: Do we need this one?
  setDocumentMode( true );

  // Tell Qt what size to start up in
  resize( 1280, 720 );

  SplashStart* splash_screen_ = new SplashStart();
  splash_screen_->exec();
  
  // Tell Qt where to doc the toolbars
  setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
  setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
  setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

  // Define the main window viewer canvas
  this->viewer_interface_ = new ViewerInterface( this );

  this->controller_interface_ = new AppController( this );
  this->controller_interface_->hide();

  // Setup the history dock widget
  add_windowids();

//  show_window( "history" );
//  show_window( "project" );
  show_window( "tools" );
//  show_window( "measurement" );
  show_window( "layermanager" );

  setCentralWidget( this->viewer_interface_ );

  this->application_menu_ = new AppMenu( this );
  this->status_bar_ = new AppStatusBar( this );


  this->add_connection( InterfaceManager::Instance()->show_window_signal_.connect( 
    boost::bind( &AppInterface::HandleShowWindow, qpointer_type( this ), _1 ) ) );

  this->add_connection( InterfaceManager::Instance()->close_window_signal_.connect( 
    boost::bind( &AppInterface::HandleCloseWindow, qpointer_type( this ), _1 ) ) );

  this->add_connection( ActionDispatcher::Instance()->begin_progress_signal_.connect( 
    boost::bind( &AppInterface::HandleBeginProgress, qpointer_type( this ), _1 ) ) );

  this->add_connection( ActionDispatcher::Instance()->end_progress_signal_.connect( 
    boost::bind( &AppInterface::HandleEndProgress, qpointer_type( this ), _1 ) ) );

  this->add_connection( ActionDispatcher::Instance()->report_progress_signal_.connect( 
    boost::bind( &AppInterface::HandleReportProgress, qpointer_type( this ), _1 ) ) );

  // NOTE: Connect state and reflect the current state (needs to be atomic, hence the lock)
  {
    // NOTE: State Engine is locked so the application thread cannot make
    // any changes to it
    StateEngine::lock_type lock( StateEngine::GetMutex() );

    // Connect and update full screen state
    set_full_screen( InterfaceManager::Instance()->full_screen_state_->get() );
    
    this->add_connection( InterfaceManager::Instance()->full_screen_state_->
      value_changed_signal_.connect( boost::bind( &AppInterface::SetFullScreen, 
      qpointer_type( this ), _1, _2 ) ) );
  }

  this->center_seg3d_gui_on_screen( this );
}

  
AppInterface::~AppInterface()
{
  //  viewer_interface_->writeSizeSettings();
}

void AppInterface::closeEvent( QCloseEvent* event )
{
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
  else showNormal();
}

ViewerInterface* AppInterface::viewer_interface()
{
  return this->viewer_interface_.data();
}

HistoryDockWidget*
AppInterface::history_dock_widget()
{
  return this->history_dock_window_.data();
}

ProjectDockWidget*
AppInterface::project_dock_widget()
{
  return this->project_dock_window_.data();
}

ToolsDockWidget*
AppInterface::tools_dock_widget()
{
  return this->tools_dock_window_.data();
}

LayerManagerDockWidget*
AppInterface::layer_manager_dock_widget()
{
  return this->layer_manager_dock_window_.data();
}

MeasurementDockWidget*
AppInterface::measurement_dock_widget()
{
  return this->measurement_dock_window_.data();
}

void AppInterface::add_windowids()
{
  InterfaceManager::Instance()->add_windowid( "controller" );
  InterfaceManager::Instance()->add_windowid( "project" );
  InterfaceManager::Instance()->add_windowid( "history" );
  InterfaceManager::Instance()->add_windowid( "layermanager" );
  InterfaceManager::Instance()->add_windowid( "tools" );
  InterfaceManager::Instance()->add_windowid( "measurement" );
}

void AppInterface::show_window( const std::string& windowid )
{
  std::string lower_windowid = Utils::StringToLower( windowid );
  if( lower_windowid == "controller" )
  {
    if( this->controller_interface_.isNull() )
    {
      this->controller_interface_ = new AppController( this );
      this->controller_interface_->show();
    }
    else
    {
      this->controller_interface_->show();
      this->controller_interface_->raise();
    }
  }
  else if( lower_windowid == "project" )
  {
    if( project_dock_window_.isNull() )
    {
      this->project_dock_window_ = new ProjectDockWidget( this );
      addDockWidget( Qt::LeftDockWidgetArea, this->project_dock_window_ );
      this->project_dock_window_->show();
    }
    else
    {
      this->project_dock_window_->show();
      this->project_dock_window_->raise();
    }
  }
  else if( lower_windowid == "layermanager" )
  {
    if( this->layer_manager_dock_window_.isNull() )
    {
      this->layer_manager_dock_window_ = new LayerManagerDockWidget( this );
      addDockWidget( Qt::RightDockWidgetArea, this->layer_manager_dock_window_ );
      this->layer_manager_dock_window_->show();
    }
    else
    {
      this->layer_manager_dock_window_->show();
      this->layer_manager_dock_window_->raise();
    }
  }
  else if( lower_windowid == "history" )
  {
    if( this->history_dock_window_.isNull() )
    {
      this->history_dock_window_ = new HistoryDockWidget( this );
      addDockWidget( Qt::LeftDockWidgetArea, this->history_dock_window_ );
      this->history_dock_window_->show();
    }
    else
    {
      this->history_dock_window_->show();
      this->history_dock_window_->raise();
    }
  }
  else if( lower_windowid == "tools" )
  {
    if( this->tools_dock_window_.isNull() )
    {
      this->tools_dock_window_ = new ToolsDockWidget( this );
      this->addDockWidget( Qt::LeftDockWidgetArea, this->tools_dock_window_ );
      tools_dock_window_->show();
    }
    else
    {
      this->tools_dock_window_->show();
      this->tools_dock_window_->raise();
    }
  }
  else if( lower_windowid == "measurement" )
  {
    if( this->measurement_dock_window_.isNull() )
    {
      this->measurement_dock_window_ = new MeasurementDockWidget( this );
      addDockWidget( Qt::RightDockWidgetArea, this->measurement_dock_window_ );
      this->measurement_dock_window_->show();
    }
    else
    {
      this->measurement_dock_window_->show();
      this->measurement_dock_window_->raise();
    }
  }
}

void AppInterface::close_window( const std::string& windowid )
{
  std::string lower_windowid = Utils::StringToLower( windowid );
  if( lower_windowid == "controller" )
  {
    if( !( this->controller_interface_.isNull() ) )
    {
      this->controller_interface_->close();
    }
  }
  else if( lower_windowid == "project" )
  {
    if( !( this->project_dock_window_.isNull() ) ) this->project_dock_window_->close();
  }
  else if( lower_windowid == "layermanager" )
  {
    if( !( this->layer_manager_dock_window_.isNull() ) ) this->layer_manager_dock_window_->close();
  }
  else if( lower_windowid == "history" )
  {
    if( !( this->history_dock_window_.isNull() ) ) this->history_dock_window_->close();
  }
  else if( lower_windowid == "tools" )
  {
    if( !( this->tools_dock_window_.isNull() ) ) this->tools_dock_window_->close();
  }
  else if( lower_windowid == "measurement" )
  {
    if( !( this->measurement_dock_window_.isNull() ) ) this->measurement_dock_window_->close();
  }
}

void AppInterface::begin_progress( ActionProgressHandle handle )
{

  // Step (1): delete any out standing progress messages
  if( this->progress_.data() )
  {
    this->progress_->done( 0 );
  }
  
  SCI_LOG_DEBUG( "Start progress widget" );
  this->progress_ = new ProgressWidget( handle, this );
  this->progress_->exec();
}

void AppInterface::end_progress( ActionProgressHandle handle )
{
  SCI_LOG_DEBUG( "Finish progress widget" );

  if( this->progress_.data() ) 
  {
    this->progress_->done( 0 );
  }
}

void AppInterface::report_progress( ActionProgressHandle handle )
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

void AppInterface::HandleShowWindow( qpointer_type qpointer, std::string windowid )
{
  Interface::PostEvent( CheckQtPointer( qpointer, boost::bind( &AppInterface::show_window,
      qpointer.data(), windowid ) ) );
}

void AppInterface::HandleCloseWindow( qpointer_type qpointer, std::string windowid )
{
  Interface::PostEvent( CheckQtPointer( qpointer, boost::bind( &AppInterface::close_window,
      qpointer.data(), windowid ) ) );
}

void AppInterface::HandleBeginProgress( qpointer_type qpointer, ActionProgressHandle handle )
{
  Interface::PostEvent( CheckQtPointer( qpointer, boost::bind( &AppInterface::begin_progress,
      qpointer.data(), handle ) ) );
}

void AppInterface::HandleEndProgress( qpointer_type qpointer, ActionProgressHandle handle )
{
  Interface::PostEvent( CheckQtPointer( qpointer, boost::bind( &AppInterface::end_progress,
      qpointer.data(), handle ) ) );
}

void AppInterface::HandleReportProgress( qpointer_type qpointer, ActionProgressHandle handle )
{
  Interface::PostEvent( CheckQtPointer( qpointer, boost::bind( &AppInterface::report_progress,
      qpointer.data(), handle ) ) );
}

void AppInterface::SetFullScreen( qpointer_type qpointer, bool full_screen, ActionSource source )
{
  Interface::PostEvent( CheckQtPointer( qpointer, boost::bind(
      &AppInterface::set_full_screen, qpointer.data(), full_screen ) ) );
}

} // end namespace Seg3D
