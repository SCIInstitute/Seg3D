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

#ifndef INTERFACE_APPLICATION_APPLICATIONINTERFACE_H
#define INTERFACE_APPLICATION_APPLICATIONINTERFACE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef Q_MOC_RUN

// Qt includes
#include <QMainWindow>
#include <QDockWidget>
#include <QWidget>
#include <QPointer>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Application Interface components.
#include <Application/InterfaceManager/InterfaceManager.h>

#include <Core/Action/Actions.h>
#include <Core/Utils/ConnectionHandler.h>

#endif

namespace Seg3D
{

/// -- ApplicationInterface --
/// This class is the main application window
/// All the menus and viewers dock into this window
/// The event loop of this window is the main event
/// loop for the program

/// Although there is only one instance of this class
/// in the program


class ApplicationInterfacePrivate;


class ApplicationInterface : public QMainWindow, private Core::ConnectionHandler
{
Q_OBJECT

// -- constructor/destructor --
public:
  // Constructor
  ApplicationInterface( std::string file_to_view_on_open = "" );

  // virtual constructor needed by Qt to destroy this object
  virtual ~ApplicationInterface();

  /// OPEN_INITIAL_PROJECT:
  /// Opens a project. Intended for use on startup.
  bool open_initial_project ( std::string filename );

  /// ACTIVATE_SPLASH_SCREEN:
  /// opens the splash screen
  void activate_splash_screen();
//
//  // -- functions to handle the properties of the interface --
private:
  // TODO:
  // This functions need to go to the private interface
  void begin_progress( Core::ActionProgressHandle handle );
  void end_progress( Core::ActionProgressHandle handle );
  void report_progress( Core::ActionProgressHandle handle );
  void center_seg3d_gui_on_screen( QWidget *widget );

  /// SAVE_PREFERENCES:
  /// this function is called when the visibility of the PreferencesManager is changed and when it
  /// is changed to false, the preferences are saved
  void save_preferences( bool visibility );

  /// ADDDOCKWIDGET:
  /// Overload the default addDockWidget and upgrade it, so docks are added
  /// on top of each other
  void addDockWidget( Qt::DockWidgetArea area, QDockWidget* dock_widget );

  /// RAISE_ERROR_MESSAGEBOX:
  /// is called by the function that handles critical error signals, pops up a critical error
  /// messagebox
  void raise_error_messagebox( int msg_type, std::string message );

  void set_full_screen( bool full_screen );

  /// SET_PROJECT_NAME:
  /// Update the name of the project in the title bar
  void set_project_name( std::string project_name );

  /// UPDATE_PROJECT_CONNECTIONS:
  /// Update the project connections when a new project is opened
  void update_project_connections();

  /// HANDLE_OSX_FILE_OPEN_EVENT:
  /// Open a file via OS X file associations
  void handle_osx_file_open_event ( std::string filename );

  /// FILE_PROJECT_FILE:
  /// Finds the project file within a bundle directory, or returns the project file if already given
  std::string find_project_file ( std::string path );

  // NOTE:
  // We need to override these events for the progress widget that draws an transparent layer
  // over the full GUI.
protected:
  /// CLOSEEVENT:
  /// Function called when the closew button is pressed. The application needs to check whether
  /// data needs to be saved if this button is pressed.
  virtual void closeEvent ( QCloseEvent* event );

  /// RESIZEEVENT:
  /// Function called when window is redrawn. This function updates widgets that depend on being
  /// displayed on top of the full window.
  virtual void resizeEvent( QResizeEvent *event );

private:
  boost::shared_ptr< ApplicationInterfacePrivate > private_;

  // -- Main Window management functions --
public:
  typedef QPointer< ApplicationInterface > qpointer_type;

  /// NOTE: These functions are static to ensure that they can serve as targets
  /// for the signal/slot mechanism. As the main interface may be closed while
  /// there are still function callbacks in the loop, these functions test for
  /// the existence of the interface before executing.

  /// HANDLEBEGINPROGRESS:
  /// Open a modal window showing progress
  static void HandleBeginProgress( qpointer_type qpointer, Core::ActionProgressHandle handle);

  /// HANDLEENDPROGRESS:
  /// Close the modal window reporting on progress
  static void HandleEndProgress( qpointer_type qpointer, Core::ActionProgressHandle handle);

  /// HANDLEREPORTPROGRESS:
  /// Report progress to the modal window
  static void HandleReportProgress( qpointer_type qpointer, Core::ActionProgressHandle handle);

  /// HANDLEPREFERENCESMANAGERSAVE:
  /// Dispatches the SavePreferences action
  static void HandlePreferencesManagerSave( qpointer_type qpointer, bool visible );

  /// HANDLECRITICALERROR:
  /// handles critical error signals and passes them to a function that pops up an error message
  static void HandleCriticalErrorMessage(  qpointer_type qpointer, int msg_type, std::string message );

  /// SETFULLSCREEN:
  /// Set full screen mode of the Main Window
  static void SetFullScreen( qpointer_type app_interface, bool full_screen,
    Core::ActionSource source );

  /// SETPROJECTNAME:
  /// handles signals that the project name needs to change
  static void SetProjectName( qpointer_type qpointer, std::string project_name,
    Core::ActionSource source );

  /// UPDATEPROJECTCONNECTIONS
  /// Update the connections to the current project
  static void UpdateProjectConnections( qpointer_type qpointer );
};

} //end namespace

#endif
