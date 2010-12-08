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

#ifndef INTERFACE_APPINTERFACE_APPINTERFACE_H
#define INTERFACE_APPINTERFACE_APPINTERFACE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif 

// Qt includes
#include <QtGui/QMainWindow>
#include <QtGui/QDockWidget>
#include <QtGui/QWidget>
#include <QtCore/QPointer>

// Boost includes
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

// Application Interface components. 
#include <Application/InterfaceManager/InterfaceManager.h>

#include <Core/Action/Actions.h>
#include <Core/Utils/ConnectionHandler.h>

namespace Seg3D
{

// -- AppInterface --
// This class is the main application window
// All the menus and viewers dock into this window
// The event loop of this window is the main event
// loop for the program

// Although there is only one instance of this class
// in the program 
  
  
class AppInterfacePrivate;


class AppInterface : public QMainWindow, private Core::ConnectionHandler
{
Q_OBJECT

// -- constructor/destructor --
public:
  // Constructor
  AppInterface();

  // virtual constructor needed by Qt to destroy this object
  virtual ~AppInterface();

  // -- functions to handle the properties of the interface --
public:
  void set_full_screen( bool full_screen );
  void set_project_name( std::string project_name );

private:
  void begin_progress( Core::ActionProgressHandle handle );
  void end_progress( Core::ActionProgressHandle handle );
  void report_progress( Core::ActionProgressHandle handle );
  void center_seg3d_gui_on_screen( QWidget *widget );
  
  // SAVE_PREFERENCES:
  // this function is called when the visibility of the PreferencesManager is changed and when it
  // is changed to false, the preferences are saved
  void save_preferences( bool visibility );

  // Overload the default addDockWidget and upgrade it, so docks are added
  // on top of each other
  void addDockWidget( Qt::DockWidgetArea area, QDockWidget* dock_widget );
  
protected:
  virtual void closeEvent ( QCloseEvent* event );
  virtual void resizeEvent( QResizeEvent *event );

private:  
  boost::shared_ptr< AppInterfacePrivate > private_;


  // -- Main Window management functions --
public:
  typedef QPointer< AppInterface > qpointer_type;

  // NOTE: These functions are static to ensure that they can serve as targets
  // for the signal/slot mechanism. As the main interface may be closed while
  // there are still function callbacks in the loop, these functions test for
  // the existence of the interface before executing.

  // HANDLEBEGINPROGRESS:
  // Open a modal window showing progress
  static void HandleBeginProgress( qpointer_type qpointer, Core::ActionProgressHandle handle);

  // HANDLEENDPROGRESS:
  // Close the modal window reporting on progress
  static void HandleEndProgress( qpointer_type qpointer, Core::ActionProgressHandle handle);

  // HANDLEREPORTPROGRESS
  // Report progress to the modal window
  static void HandleReportProgress( qpointer_type qpointer, Core::ActionProgressHandle handle);
  
  static void HandlePreferencesManagerSave( qpointer_type qpointer, bool visible );

  // SETFULLSCREEN:
  // Set full screen mode of the Main Window
  static void SetFullScreen( qpointer_type app_interface, 
    bool full_screen, Core::ActionSource source );
    
  static void SetProjectName( qpointer_type app_interface, 
    std::string project_name, Core::ActionSource source );

    
  

};

} //end namespace

#endif

